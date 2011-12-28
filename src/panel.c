/* Copyright (C) 2008, 2009, 2010, 2011 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>
#include <libchimara/chimara-if.h>

#include "panel.h"
#include "panel-private.h"
#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "error.h"
#include "extension.h"
#include "file.h"
#include "history.h"
#include "html.h"
#include "skein-view.h"
#include "transcript-renderer.h"

/* Forward declarations */
gboolean on_documentation_scrollbar_policy_changed(WebKitWebFrame *frame);

/* JAVASCRIPT METHODS */

/* The 'selectView()' function in JavaScript. Emits the 'select-view' signal,
which the Story is listening for, so that it can preferably open the requested
view in the _other_ panel.
The only argument understood is "source", but that's the only one Inform uses
currently. */
static JSValueRef
js_select_view(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	if(argumentCount != 1)
		return NULL;
	if(!JSValueIsString(ctx, arguments[0]))
		return NULL;

	I7Panel *panel = (I7Panel *)JSObjectGetPrivate(thisObject);
	JSStringRef arg_js = JSValueToStringCopy(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;
	if(JSStringIsEqualToUTF8CString(arg_js, "source"))
		g_signal_emit_by_name(panel, "select-view", I7_PANE_SOURCE);
	else
		error_dialog(NULL, NULL, _("JavaScript error: Unknown argument to selectView()"));

	JSStringRelease(arg_js);
	return NULL;
}

/* Regex match callback; replaces a unicode escape with the requested unicode
 * character. */
static gboolean
unescape_unicode(const GMatchInfo *match, GString *result, gpointer data)
{
	gchar *capture = g_match_info_fetch(match, 1);
	gunichar unicode = g_ascii_strtoll(capture, NULL, 16);
	g_string_append_unichar(result, unicode);
	g_free(capture);
	return FALSE; /* keep going */
}

/* The 'pasteCode' function in JavaScript. Unescapes the code to paste, and
 * emits the 'paste-code' signal, which the I7Story is listening for. */
static JSValueRef
js_paste_code(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	if(argumentCount != 1)
		return NULL;
	if(!JSValueIsString(ctx, arguments[0]))
		return NULL;

	/* Get the string of code to paste */
	I7Panel *panel = (I7Panel *)JSObjectGetPrivate(thisObject);
	JSStringRef arg_js = JSValueToStringCopy(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;
	size_t length = JSStringGetMaximumUTF8CStringSize(arg_js);
	gchar *code = g_malloc(length);
	JSStringGetUTF8CString(arg_js, code, length);
	JSStringRelease(arg_js);

	GError *error = NULL;
	I7App *theapp = i7_app_get();
	gchar *unescaped = g_regex_replace_eval(theapp->regices[I7_APP_REGEX_UNICODE_ESCAPE], code, -1, 0, 0, unescape_unicode, NULL, &error);
	if(!unescaped) {
		WARN(_("Cannot unescape unicode characters"), error);
		g_error_free(error);
		unescaped = g_strdup(code);
	}
	g_free(code);

	g_signal_emit_by_name(panel, "paste-code", unescaped);

	g_free(unescaped);

	return NULL;
}

/* The 'openFile()' function in JavaScript. This simply opens its argument using
the system's default viewer. */
static JSValueRef
js_open_file(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	GError *error = NULL;

	if(argumentCount != 1)
		return NULL;
	if(!JSValueIsString(ctx, arguments[0]))
		return NULL;

	JSStringRef arg_js = JSValueToStringCopy(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;
	size_t bufsize = JSStringGetMaximumUTF8CStringSize(arg_js);
	gchar *file = g_new0(gchar, bufsize);
	JSStringGetUTF8CString(arg_js, file, bufsize);

	gchar *uri = g_filename_to_uri(file, NULL, &error);
	if(!uri) {
		error_dialog(NULL, error, _("Error converting '%s' to URI: "), file);
		goto finally;
	}
	if(!gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &error))
		error_dialog(NULL, error, _("Error opening external viewer for %s: "), uri);

	g_free(uri);
finally:
	g_free(file);
	JSStringRelease(arg_js);
	return NULL;
}

/* The 'openUrl()' function in JavaScript. This also opens its argument in the
system's default viewer, but that viewer happens to be the web browser. */
static JSValueRef
js_open_url(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	GError *error = NULL;

	if(argumentCount != 1)
		return NULL;
	if(!JSValueIsString(ctx, arguments[0]))
		return NULL;

	JSStringRef arg_js = JSValueToStringCopy(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;
	size_t bufsize = JSStringGetMaximumUTF8CStringSize(arg_js);
	gchar *uri = g_new0(gchar, bufsize);
	JSStringGetUTF8CString(arg_js, uri, bufsize);

	if(!gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &error))
		error_dialog(NULL, error, _("Error opening external viewer for %s: "), uri);

	g_free(uri);
	JSStringRelease(arg_js);
	return NULL;
}

/* ACTIONS */

/* Go to the previously viewed pane in this panel */
void
action_back(GtkAction *back, I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	if(priv->current == 0)
		gtk_action_set_sensitive(gtk_action_group_get_action(priv->common_action_group, "forward"), TRUE);

	priv->current++;
	history_goto_current(panel);

	if(priv->current == g_queue_get_length(priv->history) - 1)
		gtk_action_set_sensitive(back, FALSE);
}

/* Go forward to the next viewed pane in this panel (after having gone back
 * before) */
void
action_forward(GtkAction *forward, I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	if(priv->current == g_queue_get_length(priv->history) - 1)
		gtk_action_set_sensitive(gtk_action_group_get_action(priv->common_action_group, "back"), TRUE);

	priv->current--;
	history_goto_current(panel);

	if(priv->current == 0)
		gtk_action_set_sensitive(forward, FALSE);
}

/* Pop up a menu of all the Skein labels, on the Labels button */
void
action_labels(GtkAction *action, I7Panel *panel)
{
	gtk_menu_popup(GTK_MENU(panel->labels_menu), NULL, NULL, NULL, NULL, 1, gtk_get_current_event_time());
}

/* Open the Skein layout dialog */
void
action_layout(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));

	/* Save old values in case the user decides to cancel */
	g_object_set_data(G_OBJECT(story->skein_spacing_dialog), "old-horizontal-spacing", GINT_TO_POINTER(config_file_get_int(PREFS_HORIZONTAL_SPACING)));
	g_object_set_data(G_OBJECT(story->skein_spacing_dialog), "old-vertical-spacing", GINT_TO_POINTER(config_file_get_int(PREFS_VERTICAL_SPACING)));

	int response = 1; /* 1 = "Use defaults" */
	while(response == 1)
		response = gtk_dialog_run(GTK_DIALOG(story->skein_spacing_dialog));
		/* If "Use defaults" clicked, then restart the dialog */
	gtk_widget_hide(story->skein_spacing_dialog);

	if(response != GTK_RESPONSE_OK) {
		config_file_set_int(PREFS_HORIZONTAL_SPACING, GPOINTER_TO_INT(g_object_get_data(G_OBJECT(story->skein_spacing_dialog), "old-horizontal-spacing")));
		config_file_set_int(PREFS_VERTICAL_SPACING, GPOINTER_TO_INT(g_object_get_data(G_OBJECT(story->skein_spacing_dialog), "old-vertical-spacing")));
	}
}

/* Open the Skein trimming dialog */
void
action_trim(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	I7Skein *skein = i7_story_get_skein(story);

	int response = gtk_dialog_run(GTK_DIALOG(story->skein_trim_dialog));
	gtk_widget_hide(story->skein_trim_dialog);

	if(response == GTK_RESPONSE_OK) {
		int pruning = 31 - (int)gtk_range_get_value(GTK_RANGE(story->skein_trim_slider));
		if(pruning < 1)
			pruning = 1;
		i7_skein_trim(skein, i7_skein_get_root_node(skein), pruning);
	}
}

/*
 * action_play_all:
 * @action: not used
 * @panel: the panel that this action was triggered on
 *
 * Signal handler for the action connected to the "Play All" button in the panel
 * toolbar when the Skein panel is displayed. Plays all the nodes currently
 * blessed in the Skein.
 */
void
action_play_all(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_set_compile_finished_action(story, (CompileActionFunc)i7_story_run_compiler_output_and_entire_skein, NULL);
	i7_story_compile(story, FALSE, FALSE);
}

void
action_contents(GtkAction *action, I7Panel *panel)
{
	GFile *docs_file = i7_app_get_data_file_va(i7_app_get(), "Documentation", "index.html", NULL);
	html_load_file(WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_DOCUMENTATION]), docs_file);
	g_object_unref(docs_file);
}

/*
 * action_bless_all:
 * @action: not used
 * @panel: the panel that this action was triggered on
 * 
 * Signal handler for the action connected to the "Bless All" button in the
 * panel toolbar when the Transcript panel is displayed. Blesses all the nodes
 * currently shown in the transcript. (From the skein's "current node" up to
 * the root node.)
 */
void
action_bless_all(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	I7Skein *skein = i7_story_get_skein(story);

	/* Display a confirmation dialog (as this can't be undone. Well, not easily) */
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(story),
	    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_QUESTION,
	    GTK_BUTTONS_YES_NO,
	    _("Are you sure you want to bless all the items in the transcript?"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
	    _("This will 'Bless' all the items currently in the transcript so that "
		"they appear as the 'expected' text in the right-hand column. This "
		"operation cannot be undone."));
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
		i7_skein_bless(skein, i7_skein_get_current_node(skein), TRUE);
	}
	gtk_widget_destroy(dialog);
}

/*
 * action_panel_previous_difference:
 * @action: not used
 * @panel: panel this action was triggered on
 *
 * Signal handler for the action connected to the "Previous Difference" button
 * in the panel toolbar when the Transcript panel is displayed.
 */
void
action_panel_previous_difference(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_previous_difference(story);
}

/*
 * action_panel_next_difference:
 * @action: not used
 * @panel: panel this action was triggered on
 *
 * Signal handler for the action connected to the "Next Difference" button in
 * the panel toolbar when the Transcript panel is displayed.
 */
void
action_panel_next_difference(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_next_difference(story);
}

/*
 * action_panel_next_difference_skein:
 * @action: not used
 * @panel: panel this action was triggered on
 *
 * Signal handler for the action connected to the "Next Difference in Skein"
 * button in the panel toolbar when the Transcript panel is displayed.
 */
void
action_panel_next_difference_skein(GtkAction *action, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_next_difference_skein(story);
}

/* TYPE SYSTEM */

enum _I7PanelSignalType {
	SELECT_VIEW_SIGNAL,
	PASTE_CODE_SIGNAL,
	JUMP_TO_LINE_SIGNAL,
	DISPLAY_DOCPAGE_SIGNAL,
	LAST_SIGNAL
};
static guint i7_panel_signals[LAST_SIGNAL] = { 0 };

static GtkVBoxClass *parent_class = NULL;
G_DEFINE_TYPE(I7Panel, i7_panel, GTK_TYPE_VBOX);

static void
i7_panel_init(I7Panel *self)
{
	GError *error = NULL;
	I7_PANEL_USE_PRIVATE(self, priv);
	I7App *theapp = i7_app_get();
	int foo;

	/* Initialize the history system */
	priv->history = g_queue_new();
	I7PanelHistory *item = g_slice_new0(I7PanelHistory);
	item->pane = I7_PANE_SOURCE;
	item->tab = I7_SOURCE_VIEW_TAB_SOURCE;
	g_queue_push_head(priv->history, item);
	priv->current = 0;

	/* Build the interface */
	GFile *file = i7_app_get_data_file_va(theapp, "ui", "panel.ui", NULL);
	GtkBuilder *builder = create_new_builder(file, self);
	g_object_unref(file);
	
	/* Make the action groups */
	priv->common_action_group = GTK_ACTION_GROUP(load_object(builder, "panel_actions"));
	priv->skein_action_group = GTK_ACTION_GROUP(load_object(builder, "skein_actions"));
	priv->transcript_action_group = GTK_ACTION_GROUP(load_object(builder, "transcript_actions"));
	priv->documentation_action_group = GTK_ACTION_GROUP(load_object(builder, "documentation_actions"));
	
	/* Build the toolbar from the GtkUIManager file. The UI manager owns the action groups now. */
	priv->ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->common_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->skein_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->transcript_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->documentation_action_group, 0);
	file = i7_app_get_data_file_va(theapp, "ui", "panel.uimanager.xml", NULL);
	char *path = g_file_get_path(file);
	gtk_ui_manager_add_ui_from_file(priv->ui_manager, path, &error);
	g_free(path);
	g_object_unref(file);
	if(error)
		ERROR(_("Building menus failed"), error);
	self->toolbar = gtk_ui_manager_get_widget(priv->ui_manager, "/PanelToolbar");
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(self->toolbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(self->toolbar), GTK_TOOLBAR_BOTH_HORIZ);

	/* Add the Labels menu; apparently GtkUIManager can't build menu tool items */
	self->labels = gtk_menu_tool_button_new(NULL, NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(self->toolbar), self->labels, 3);
	self->labels_menu = gtk_menu_new();
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(self->labels), self->labels_menu);
	self->labels_action = gtk_action_group_get_action(priv->skein_action_group, "labels");
	gtk_action_connect_proxy(self->labels_action, GTK_WIDGET(self->labels));

	/* Reparent the widgets into our new VBox */
	self->notebook = GTK_WIDGET(load_object(builder, "panel"));
	gtk_box_pack_start(GTK_BOX(self), self->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(self), self->notebook, TRUE, TRUE, 0);

	/* Add the I7SourceView widget and connect the after handler of the switch-page signal */
	self->sourceview = I7_SOURCE_VIEW(i7_source_view_new());
	gtk_widget_show(GTK_WIDGET(self->sourceview));
	GtkWidget *sourcelabel = GTK_WIDGET(load_object(builder, "source_pane_label"));
	gtk_notebook_insert_page(GTK_NOTEBOOK(self->notebook), GTK_WIDGET(self->sourceview), sourcelabel, I7_PANE_SOURCE);
	g_signal_connect_after(self->sourceview->notebook, "switch-page", G_CALLBACK(after_source_notebook_switch_page), self);

	/* Add the I7SkeinView widget */
	GtkWidget *skeinview = i7_skein_view_new();
	gtk_widget_show(skeinview);
	GtkWidget *skein_scrolledwindow = GTK_WIDGET(load_object(builder, "skein_scrolledwindow"));
	gtk_container_add(GTK_CONTAINER(skein_scrolledwindow), skeinview);

	/* Add the Chimara widget */
	GtkWidget *game = chimara_if_new();
	gtk_widget_show(game);
	GtkWidget *gamelabel = GTK_WIDGET(load_object(builder, "game_pane_label"));
	gtk_notebook_insert_page(GTK_NOTEBOOK(self->notebook), game, gamelabel, I7_PANE_GAME);
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_Z5, CHIMARA_IF_INTERPRETER_FROTZ);
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_Z6, CHIMARA_IF_INTERPRETER_FROTZ);
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_Z8, CHIMARA_IF_INTERPRETER_FROTZ);
	ChimaraIFInterpreter glulx_interpreter = config_file_get_bool(PREFS_USE_GIT)? CHIMARA_IF_INTERPRETER_GIT : CHIMARA_IF_INTERPRETER_GLULXE;
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_GLULX, glulx_interpreter);
	chimara_glk_set_interactive(CHIMARA_GLK(game), TRUE);
	chimara_glk_set_protect(CHIMARA_GLK(game), FALSE);

	/* Add the transcript cell renderer */
	self->transcript_cell = GTK_CELL_RENDERER(i7_cell_renderer_transcript_new());
	gtk_cell_renderer_set_padding(self->transcript_cell, 4, 4);
	self->transcript_column = GTK_TREE_VIEW_COLUMN(load_object(builder, "transcript_column"));
	gtk_tree_view_column_pack_start(self->transcript_column, self->transcript_cell, TRUE);
	gtk_tree_view_column_set_attributes(self->transcript_column, self->transcript_cell,
	    "command", I7_SKEIN_COLUMN_COMMAND,
	    "transcript-text", I7_SKEIN_COLUMN_TRANSCRIPT_TEXT,
	    "expected-text", I7_SKEIN_COLUMN_EXPECTED_TEXT,
	    "match-type", I7_SKEIN_COLUMN_MATCH_TYPE,
	    "current", I7_SKEIN_COLUMN_CURRENT,
	    "played", I7_SKEIN_COLUMN_PLAYED,
	    "changed", I7_SKEIN_COLUMN_CHANGED,
	    NULL);
	
	/* Save public pointers to specific widgets */
	LOAD_WIDGET(z5);
	LOAD_WIDGET(z8);
	LOAD_WIDGET(z6);
	LOAD_WIDGET(glulx);
	LOAD_WIDGET(blorb);
	LOAD_WIDGET(nobble_rng);
	LOAD_WIDGET(debugging_scrolledwindow);
	LOAD_WIDGET(inform6_scrolledwindow);
	LOAD_WIDGET(transcript_menu);
	g_object_ref(self->transcript_menu);

	/* Save the public pointers for all the tab arrays */
	self->tabs[I7_PANE_SOURCE] = self->sourceview->notebook;
	self->tabs[I7_PANE_ERRORS] = GTK_WIDGET(load_object(builder, "errors_notebook"));
	self->tabs[I7_PANE_INDEX] = GTK_WIDGET(load_object(builder, "index_notebook"));
	self->tabs[I7_PANE_SKEIN] = skeinview;
	self->tabs[I7_PANE_TRANSCRIPT] = GTK_WIDGET(load_object(builder, "transcript"));
	self->tabs[I7_PANE_GAME] = game;
	self->tabs[I7_PANE_DOCUMENTATION] = GTK_WIDGET(load_object(builder, "documentation"));
	self->tabs[I7_PANE_SETTINGS] = GTK_WIDGET(load_object(builder, "settings"));
	self->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS] = self->sourceview->headings;
	self->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE] = self->sourceview->source;
	const gchar *errors_tab_names[] = { "progress", "debugging", "problems", "inform6" };
	const gchar *index_tab_names[] = { "actions", "contents", "kinds", "phrasebook", "rules", "scenes", "world" };
	for(foo = 0; foo < I7_INDEX_NUM_TABS; foo++) {
		if(foo < I7_ERRORS_NUM_TABS)
			self->errors_tabs[foo] = GTK_WIDGET(load_object(builder, errors_tab_names[foo]));
		self->index_tabs[foo] = GTK_WIDGET(load_object(builder, index_tab_names[foo]));
	}

	/* Update the web settings for this panel */
	priv->websettings = WEBKIT_WEB_SETTINGS(load_object(builder, "websettings"));
	/* Parse the font descriptions */
	PangoFontDescription *stdfont = get_desktop_standard_font();
	PangoFontDescription *monofont = get_desktop_monospace_font();
	gint stdsize = (gint)((gdouble)get_font_size(stdfont) / PANGO_SCALE);
	gint monosize = (gint)((gdouble)get_font_size(monofont) / PANGO_SCALE);
	g_object_set(priv->websettings,
		"default-font-family", pango_font_description_get_family(stdfont),
		"monospace-font-family", pango_font_description_get_family(monofont),
		"default-font-size", stdsize,
		"default-monospace-font-size", monosize,
		"minimum-font-size", MIN(stdsize, monosize),
		NULL);
	pango_font_description_free(stdfont);
	pango_font_description_free(monofont);

	/* Make sure the scrollbars are always visible in the documentation pane */
	WebKitWebFrame *frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]));
	g_signal_connect(frame, "scrollbars-policy-changed", G_CALLBACK(on_documentation_scrollbar_policy_changed), NULL);

	/* Builder object not needed anymore */
	g_object_unref(builder);

	/* Declare the JavaScript Project class */
	JSStaticFunction project_class_functions[] = {
		{ "selectView", js_select_view, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "pasteCode", js_paste_code, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "openFile", js_open_file, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "openUrl", js_open_url, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ NULL, NULL, 0 }
	};
	JSClassDefinition project_class_definition = {
		/* version */ 0, kJSClassAttributeNone, "ProjectClass",
		/* parent */ NULL, /* static values */ NULL, project_class_functions,
		/* callbacks */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL
	};
	priv->js_class = JSClassCreate(&project_class_definition);

	/* Load the documentation page */
	file = i7_app_get_data_file_va(theapp, "Documentation", "index.html", NULL);
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), file);
	g_object_unref(file);
}

static void
i7_panel_finalize(GObject *self)
{
	I7_PANEL_USE_PRIVATE(self, priv);

	history_free_queue(I7_PANEL(self));
	JSClassRelease(priv->js_class);
	g_object_unref(priv->ui_manager);
	g_object_unref(I7_PANEL(self)->transcript_menu);

	G_OBJECT_CLASS(parent_class)->finalize(self);
}

static void
i7_panel_class_init(I7PanelClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_panel_finalize;

	parent_class = g_type_class_peek_parent(klass);

	i7_panel_signals[SELECT_VIEW_SIGNAL] = g_signal_new("select-view",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, select_view), NULL, NULL,
		g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
	i7_panel_signals[PASTE_CODE_SIGNAL] = g_signal_new("paste-code",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, paste_code), NULL, NULL,
		g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	i7_panel_signals[JUMP_TO_LINE_SIGNAL] = g_signal_new("jump-to-line",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, jump_to_line), NULL, NULL,
		g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
	i7_panel_signals[DISPLAY_DOCPAGE_SIGNAL] = g_signal_new("display-docpage",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, display_docpage), NULL, NULL,
		g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	g_type_class_add_private(klass, sizeof(I7PanelPrivate));
}

/* SIGNAL HANDLERS */

void
on_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	switch(page_num) {
		case I7_PANE_SKEIN:
			gtk_action_group_set_visible(priv->skein_action_group, TRUE);
			gtk_action_group_set_visible(priv->transcript_action_group, FALSE);
			gtk_action_group_set_visible(priv->documentation_action_group, FALSE);
			break;
		case I7_PANE_TRANSCRIPT:
			gtk_action_group_set_visible(priv->skein_action_group, FALSE);
			gtk_action_group_set_visible(priv->transcript_action_group, TRUE);
			gtk_action_group_set_visible(priv->documentation_action_group, FALSE);
			break;
		case I7_PANE_DOCUMENTATION:
			gtk_action_group_set_visible(priv->skein_action_group, FALSE);
			gtk_action_group_set_visible(priv->transcript_action_group, FALSE);
			gtk_action_group_set_visible(priv->documentation_action_group, TRUE);
			break;
		default:
			gtk_action_group_set_visible(priv->skein_action_group, FALSE);
			gtk_action_group_set_visible(priv->transcript_action_group, FALSE);
			gtk_action_group_set_visible(priv->documentation_action_group, FALSE);
	}
}

void
after_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel)
{
	switch(page_num) {
		case I7_PANE_SOURCE:
			history_push_tab(panel, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_SOURCE])));
			break;
		case I7_PANE_ERRORS:
			history_push_tab(panel, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_ERRORS])));
			break;
		case I7_PANE_INDEX:
			history_push_tab(panel, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_INDEX])));
			break;
		case I7_PANE_DOCUMENTATION:
			history_push_docpage(panel, NULL);
			break;
		default:
			history_push_pane(panel, page_num);
	}
}

void
after_source_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_SOURCE)
		history_push_tab(panel, I7_PANE_SOURCE, page_num);
}

void
after_errors_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_ERRORS)
		history_push_tab(panel, I7_PANE_ERRORS, page_num);
}

void
after_index_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_INDEX)
		history_push_tab(panel, I7_PANE_INDEX, page_num);
}

/* Internal function: find the real filename referred to by a URI starting with
 inform:. Returns NULL if not found, but if that happens then there is a bug in
 the HTML generated by Inform. TODO: make this use a cache. */
static GFile *
find_real_filename_for_inform_protocol(const char *uri)
{
	g_return_val_if_fail(g_str_has_prefix(uri, "inform:"), NULL);

	I7App *theapp = i7_app_get();
	GFile *parent, *real_file;

	/* Get the rest of the URI after the scheme, a colon, and up to two slashes */
	const char *rest = uri + 7; /* strlen("inform:") */
	if(*rest == '/' && *++rest == '/')
		rest++;

	/* Remove %xx escapes */
	char *unescaped = g_uri_unescape_string(rest, "");

	/* Replace the slashes by platform-dependent path separators */
	char **elements = g_strsplit(unescaped, "/", -1);

	g_free(unescaped);
	char *relative_path;
	if(elements[0] && strcmp(elements[0], "Extensions") == 0) {
		/* inform://Extensions is an exception; change it so it will be picked
		 up by the last attempt below, in the home directory */
		relative_path = g_build_filenamev(elements + 1);
	} else
		relative_path = g_build_filenamev(elements);
	g_strfreev(elements);

	parent = i7_app_get_data_file_va(theapp, "Documentation", "doc_images", NULL);
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if(g_file_query_exists(real_file, NULL))
		goto finally;
	g_object_unref(real_file);
	g_object_unref(parent);

	parent = i7_app_get_data_file_va(theapp, "Documentation", "Sections", NULL);
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if(g_file_query_exists(real_file, NULL))
		goto finally;
	g_object_unref(real_file);
	g_object_unref(parent);

	parent = i7_app_get_data_file_va(theapp, "Documentation", NULL);
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if(g_file_query_exists(real_file, NULL))
		goto finally;
	g_object_unref(real_file);
	g_object_unref(parent);

	GFile *home_file = g_file_new_for_path(g_get_home_dir());
	parent = g_file_resolve_relative_path(home_file, "Inform/Documentation");
	g_object_unref(home_file);
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if(g_file_query_exists(real_file, NULL))
		goto finally;
	g_object_unref(real_file);
	g_object_unref(parent);
	g_free(relative_path);

	g_warning("Could not locate real filename for URI %s. There may be a bug in"
		" the HTML generated by Inform.", uri);
	return NULL;

finally:
	g_object_unref(parent);
	g_free(relative_path);
	return real_file;
}

/* This is the callback that handles the custom protocols and
 disallows any funny stuff */
gint
on_navigation_requested(WebKitWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, I7Panel *panel)
{
	GError *error = NULL;
	const gchar *uri = webkit_network_request_get_uri(request);
	gchar *scheme = g_uri_parse_scheme(uri);

	/* If no protocol found, just go on -- it's a file:// */
	if(!scheme)
		return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

	if(strcmp(scheme, "about") == 0) {
		/* These are protocols that we explicitly allow WebKit to load */
		g_free(scheme);
		return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

	} else if(strcmp(scheme, "file") == 0) {
		/* If the "resource-request-starting" signal is available, as it
		 is from 1.1.14 onward, then no special manipulation is needed */
		g_free(scheme);
		return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

	} else if(strcmp(scheme, "inform") == 0) {
		/* The inform: protocol can mean files in any of several different
		locations */
		/* Only load them in the documentation page; if this is another page,
		then redirect the request to the documentation page */
		if(webview != WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_DOCUMENTATION])) {
			g_signal_emit_by_name(panel, "display-docpage", uri);
			g_free(scheme);
			return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
		}

		GFile *real_file = find_real_filename_for_inform_protocol(uri);
		html_load_file(webview, real_file);
		g_object_unref(real_file);

	} else if(strcmp(scheme, "http") == 0 || strcmp(scheme, "mailto") == 0) {
		if(!gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &error)) {
			error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(panel))), error, _("Error opening external viewer for %s: "), uri);
		}

	} else if(strcmp(scheme, "source") == 0) {
		guint line;
		gchar *path = g_strdup(uri + strlen("source:"));
		gchar *ptr = strrchr(path, '#');
		gchar *anchor = g_strdup(ptr);
		*ptr = 0;

		/* If it links to the source file, just jump to the line */
		if(strcmp(path, "story.ni") == 0) {
			if(sscanf(anchor, "#line%u", &line))
				g_signal_emit_by_name(panel, "jump-to-line", line);
		} else {
			GFile *file = g_file_new_for_path(path);
			/* Else it's a link to an extension, open it in a new window */
			char *realpath = get_case_insensitive_extension(path); // FIXME
			GFile *real_file = g_file_new_for_path(realpath);
			g_free(realpath);
			g_object_unref(file);
			/* Check if we need to open the extension read-only */
			GFile *user_file = i7_app_get_extension_file(i7_app_get(), NULL, NULL);
			gboolean readonly = !g_file_has_prefix(real_file, user_file);
			g_object_unref(user_file);

			realpath = g_file_get_path(real_file); // FIXME
			I7Extension *ext = i7_extension_new_from_file(i7_app_get(), realpath, readonly);
			g_free(realpath);
			if(ext != NULL) {
				if(sscanf(anchor, "#line%u", &line))
					i7_source_view_jump_to_line(ext->sourceview, line);
			}
		}
		g_free(path);
		g_free(anchor);

	} else
		g_warning(_("Unrecognized protocol: %s\n"), scheme);

	g_free(scheme);

	return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
}

void
on_resource_request_starting(WebKitWebView *self, WebKitWebFrame *frame, WebKitWebResource *resource, WebKitNetworkRequest *request, WebKitNetworkResponse *response)
{
	const gchar *uri = webkit_network_request_get_uri(request);
	if(!g_str_has_prefix(uri, "inform:"))
		return;
	GFile *real_file = find_real_filename_for_inform_protocol(uri);
	char *real_uri = g_file_get_uri(real_file);
	webkit_network_request_set_uri(request, real_uri);
	g_object_unref(real_file);
	g_free(real_uri);
}

gint
after_documentation_navigation_requested(WebKitWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_DOCUMENTATION)
		history_push_docpage(panel, webkit_network_request_get_uri(request));
	return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;
}

void
on_documentation_window_object_cleared(WebKitWebView *webview, WebKitWebFrame *frame, JSGlobalContextRef ctx, JSObjectRef window, I7Panel *panel)
{
	JSValueRef exception = NULL;
	JSStringRef varname = JSStringCreateWithUTF8CString("Project");
	JSObjectSetProperty(ctx, window, varname,
		JSObjectMake(ctx, I7_PANEL_PRIVATE(panel)->js_class, panel),
		kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly,
		&exception);
	g_assert(exception == NULL);
	JSStringRelease(varname);
}

gboolean
on_documentation_scrollbar_policy_changed(WebKitWebFrame *frame)
{
	return TRUE; /* Ignore scrollbar policy change */
}

/* PUBLIC FUNCTIONS */

GtkWidget *
i7_panel_new()
{
	return GTK_WIDGET(g_object_new(I7_TYPE_PANEL, NULL));
}

/**
 * i7_panel_reset_queue:
 * @self: the panel
 * @pane: the pane to go to after resetting the history queue
 * @tab: the subtab of @pane to go to after resetting the queue
 * @page_file: the file to display in the documentation pane after resetting
 * the queue, or %NULL
 *
 * Set the tab and subtab and reset the history queue. Display that item in
 * the panel.
 */
void
i7_panel_reset_queue(I7Panel *self, I7PanelPane pane, int tab, GFile *page_file)
{
	I7_PANEL_USE_PRIVATE(self, priv);
	history_free_queue(self);
	priv->history = g_queue_new();
	I7PanelHistory *item = g_slice_new0(I7PanelHistory);
	item->pane = pane;
	item->tab = tab;
	item->page = page_file? g_file_get_uri(page_file) : NULL;
	g_queue_push_head(priv->history, item);
	priv->current = 0;
	history_goto_current(self);
}

void
i7_panel_goto_docpage(I7Panel *self, GFile *file)
{
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), file);
}

void
i7_panel_goto_docpage_at_anchor(I7Panel *self, GFile *file, const char *anchor)
{
	html_load_file_at_anchor(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), file, anchor);
}

void
i7_panel_update_tabs(I7Panel *self)
{
	g_idle_add((GSourceFunc)update_tabs, GTK_SOURCE_VIEW(self->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]));
	g_idle_add((GSourceFunc)update_tabs, GTK_SOURCE_VIEW(self->errors_tabs[I7_ERRORS_TAB_INFORM6]));
}

static gboolean
update_font_tabs(GtkSourceView *view)
{
	update_font(GTK_WIDGET(view));
	update_tabs(view);
	return FALSE; /* one-shot idle function */
}

/* Update the fonts of the widgets in this pane */
void
i7_panel_update_fonts(I7Panel *self)
{
	g_idle_add((GSourceFunc)update_font_tabs, GTK_SOURCE_VIEW(self->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]));
	g_idle_add((GSourceFunc)update_font_tabs, GTK_SOURCE_VIEW(self->errors_tabs[I7_ERRORS_TAB_INFORM6]));

	WebKitWebSettings *settings = I7_PANEL_PRIVATE(self)->websettings;
	PangoFontDescription *fontdesc = get_font_description();
	g_object_set(G_OBJECT(settings),
		"default-font-family", pango_font_description_get_family(fontdesc),
		NULL);

	const gchar *font = pango_font_description_get_family(fontdesc);
	gint size = pango_font_description_get_size(fontdesc) / PANGO_SCALE;
	gchar *css = g_strdup_printf(
		"grid.normal { font-size: %d; }"
		"grid.user1 { color: #303030; background-color: #ffffff; }"
	    "buffer.default { font-family: '%s'; }"
		"buffer.normal { font-size: %d; }"
		"buffer.header { font-size: %d; font-weight: bold; }"
		"buffer.subheader { font-size: %d; font-weight: bold; }"
		"buffer.alert { color: #aa0000; font-weight: bold; }"
		"buffer.note { color: #aaaa00; font-weight: bold; }"
		"buffer.block-quote { text-align: center; font-style: italic; }"
		"buffer.input { color: #0000aa; font-style: italic; }"
		"buffer.user1 { }"
		"buffer.user2 { }"
		"buffer.pager { color: #ffffff; background-color: #aa0000; }",
		size, font, size, (gint)(size * RELATIVE_SIZE_MEDIUM), size);
	chimara_glk_set_css_from_string(CHIMARA_GLK(self->tabs[I7_PANE_GAME]), css);
	g_free(css);
	pango_font_description_free(fontdesc);
}

/* Update the font sizes of WebViews in this pane */
void
i7_panel_update_font_sizes(I7Panel *self)
{
	WebKitWebSettings *settings = I7_PANEL_PRIVATE(self)->websettings;
	PangoFontDescription *stdfont = get_desktop_standard_font();
	PangoFontDescription *monofont = get_desktop_monospace_font();
	gint stdsize = (gint)((gdouble)get_font_size(stdfont) / PANGO_SCALE);
	gint monosize = (gint)((gdouble)get_font_size(monofont) / PANGO_SCALE);
	g_object_set(G_OBJECT(settings),
		"default-font-size", stdsize,
		"default-monospace-font-size", monosize,
		"minimum-font-size", MIN(stdsize, monosize),
		NULL);
	pango_font_description_free(stdfont);
	pango_font_description_free(monofont);
}
