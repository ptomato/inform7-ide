/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2015, 2018, 2019, 2021, 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <math.h>
#include <string.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-if.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>

#include "app.h"
#include "builder.h"
#include "configfile.h"
#include "error.h"
#include "extension.h"
#include "file.h"
#include "history.h"
#include "html.h"
#include "panel.h"
#include "prefs.h"
#include "project-settings.h"
#include "skein-view.h"
#include "uri-scheme.h"

#define PUBLIC_LIBRARY_URI "https://ganelson.github.io/inform-public-library/"
#define PUBLIC_LIBRARY_HOME_URI PUBLIC_LIBRARY_URI "index-linux.html"

const char * const i7_panel_index_names[] = {
	"Welcome.html", "Contents.html", "Actions.html", "Kinds.html",
	"Phrasebook.html", "Rules.html", "Scenes.html", "World.html"
};

typedef struct {
	/* History list */
	GQueue *history; /* "front" is more recent, "back" is older */
	guint current;
	/* Webview settings */
	WebKitSettings *websettings;
	WebKitUserContentManager *content;
} I7PanelPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(I7Panel, i7_panel, GTK_TYPE_BOX);

/* JAVASCRIPT METHODS */

/* Helper function: convert a string JSCValue to a NULL-terminated UTF-8 C
string. Returns NULL error. Free result when done. */
static char *
js_string_value_to_string(JSCValue *string_val)
{
	if (!jsc_value_is_string(string_val))
		return NULL;
	return jsc_value_to_string(string_val);
}

/* The 'selectView()' function in JavaScript. Emits the 'select-view' signal,
which the Story is listening for, so that it can preferably open the requested
view in the _other_ panel.
The only argument understood is "source", but that's the only one Inform uses
currently. */
static void
js_select_view(WebKitUserContentManager *content, WebKitJavascriptResult *js_message, I7Panel *panel)
{
	JSCValue *arg = webkit_javascript_result_get_js_value(js_message);
	g_autofree char* view = js_string_value_to_string(arg);
	if (!view)
		return;

	if (g_str_equal(view, "source"))
		g_signal_emit_by_name(panel, "select-view", I7_PANE_SOURCE);
	else
		error_dialog(NULL, NULL, _("JavaScript error: Unknown argument to selectView()"));
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
static void
js_paste_code(WebKitUserContentManager *content, WebKitJavascriptResult *js_message, I7Panel *panel)
{
	JSCValue *arg = webkit_javascript_result_get_js_value(js_message);
	g_autofree char* code = js_string_value_to_string(arg);
	if (!code)
		return;

	GError *error = NULL;
	I7App *theapp = I7_APP(g_application_get_default());
	gchar *unescaped = g_regex_replace_eval(theapp->regices[I7_APP_REGEX_UNICODE_ESCAPE], code, -1, 0, 0, unescape_unicode, NULL, &error);
	if(!unescaped) {
		WARN(_("Cannot unescape unicode characters"), error);
		g_error_free(error);
		unescaped = g_strdup(code);
	}

	g_signal_emit_by_name(panel, "paste-code", unescaped);

	g_free(unescaped);
}

/* The 'openFile()' function in JavaScript. This simply opens its argument using
the system's default viewer. */
static void
js_open_file(WebKitUserContentManager *content, WebKitJavascriptResult *js_message, I7Panel *panel)
{
	GError *error = NULL;

	JSCValue *arg = webkit_javascript_result_get_js_value(js_message);
	g_autofree char* file = js_string_value_to_string(arg);
	if (!file)
		return;

	gchar *uri = g_filename_to_uri(file, NULL, &error);
	if(uri != NULL) {
		show_uri_externally(uri, NULL, file);
	} else {
		g_warning("Filename has no URI: %s", error->message);
		g_clear_error(&error);
	}

	g_free(uri);
}

/* The 'openUrl()' function in JavaScript. This also opens its argument in the
system's default viewer, but that viewer happens to be the web browser. */
static void
js_open_url(WebKitUserContentManager *content, WebKitJavascriptResult *js_message, I7Panel *panel)
{
	JSCValue *arg = webkit_javascript_result_get_js_value(js_message);
	g_autofree char* uri = js_string_value_to_string(arg);
	if (!uri)
		return;

	show_uri_externally(uri, NULL, NULL);
}

static void
on_download_succeeded_script_finished(WebKitWebView *webview, GAsyncResult *res, void *unused)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(WebKitJavascriptResult) js_result = webkit_web_view_run_javascript_finish(webview, res, &error);
	if (!js_result) {
		g_warning("Error notifying that the download succeeded, refreshing Public Library: %s", error->message);
		webkit_web_view_reload(webview);
	}
}

/* Helper function: Converts a library:/ URI to a real https:// URI, extracting
other information from the URI. Assumes an id parameter is given and that it is
the only (last?) query parameter. Unref return value when done */
static GFile *
library_uri_to_real_uri(const char *uri, char **author, char **title, char **id)
{
	if(id != NULL)
		*id = g_strdup(strstr(uri, "?id=") + strlen("?id="));

	char *path = g_strdup(uri + strlen("library:"));
	char *real_uri = g_strconcat(PUBLIC_LIBRARY_URI, path, NULL);
	g_free(path);
	GFile *retval = g_file_new_for_uri(real_uri);
	g_free(real_uri);

	char **components = g_strsplit(uri, "/", 5); /* 0 = library:, 1 = payloads */
	if(author != NULL)
		*author = g_uri_unescape_string(components[2], NULL);
	if(title != NULL)
		*title = g_uri_unescape_string(components[3], NULL);
	g_strfreev(components);

	return retval;
}

static void
notify_download_succeeded(WebKitWebView *webview, const char *id)
{
	g_autofree char *script = g_strconcat("downloadSucceeded(", id, ");", NULL);
	webkit_web_view_run_javascript(webview, script, NULL, (GAsyncReadyCallback)on_download_succeeded_script_finished, NULL);
}

static void
on_download_finished(gboolean success, const char *id, WebKitWebView *webview)
{
	if(success)
		notify_download_succeeded(webview, id);
}

static void
js_download_multi(WebKitUserContentManager *content, WebKitJavascriptResult *js_message, I7Panel *panel)
{
	JSCValue *array = webkit_javascript_result_get_js_value(js_message);
	if (!jsc_value_is_array(array))
		return;
	g_autoptr(JSCValue) length_js = jsc_value_object_get_property(array, "length");
	if (!jsc_value_is_number(length_js))
		return;
	int32_t length = jsc_value_to_int32(length_js);

	/* Download the extensions one by one */
	unsigned n_extensions = length / 3;
	char **ids = g_new0(char *, n_extensions);
	GFile **files = g_new0(GFile *, n_extensions);
	char **authors = g_new0(char *, n_extensions);
	char **titles = g_new0(char *, n_extensions);
	char **versions = g_new0(char *, n_extensions);
	unsigned ix;
	for(ix = 0; ix < n_extensions; ix++) {
		g_autoptr(JSCValue) id_val = jsc_value_object_get_property_at_index(array, 3 * ix);
		g_autoptr(JSCValue) uri_val = jsc_value_object_get_property_at_index(array, 3 * ix + 1);
		g_autoptr(JSCValue) desc_val = jsc_value_object_get_property_at_index(array, 3 * ix + 2);
		char *id = js_string_value_to_string(id_val);
		g_autofree char *uri = js_string_value_to_string(uri_val);
		char *desc = js_string_value_to_string(desc_val);
		if(id == NULL || uri == NULL || desc == NULL)
			goto finally;

		ids[ix] = id;
		files[ix] = library_uri_to_real_uri(uri, &authors[ix], &titles[ix], NULL);
		versions[ix] = desc;
	}

	I7Document *doc = I7_DOCUMENT(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_document_download_multiple_extensions(doc, n_extensions, ids, files, authors, titles, versions, (I7DocumentExtensionDownloadCallback)on_download_finished, panel->tabs[I7_PANE_EXTENSIONS]);

finally:
	for(ix = 0; ix < n_extensions; ix++) {
		g_free(ids[ix]);
		g_object_unref(files[ix]);
		g_free(authors[ix]);
		g_free(titles[ix]);
		g_free(versions[ix]);
	}
	g_free(ids);
	g_free(files);
	g_free(authors);
	g_free(titles);
	g_free(versions);
}

/* ACTIONS */

/* Go to the previously viewed pane in this panel */
static void
action_back(GSimpleAction *back, GVariant *parameter, I7Panel *self)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);

	if (priv->current == 0) {
		GAction *forward = g_action_map_lookup_action(G_ACTION_MAP(self->actions), "forward");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(forward), TRUE);
	}

	priv->current++;
	history_goto_current(self);

	if(priv->current == g_queue_get_length(priv->history) - 1)
		g_simple_action_set_enabled(back, FALSE);
}

/* Go forward to the next viewed pane in this panel (after having gone back
 * before) */
static void
action_forward(GSimpleAction *forward, GVariant *parameter, I7Panel *self)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);

	if (priv->current == g_queue_get_length(priv->history) - 1) {
		GAction *back = g_action_map_lookup_action(G_ACTION_MAP(self->actions), "back");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(back), TRUE);
	}

	priv->current--;
	history_goto_current(self);

	if(priv->current == 0)
		g_simple_action_set_enabled(forward, FALSE);
}

static void
action_jump_to_node(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	uintptr_t node = g_variant_get_uint64(parameter);
	I7SkeinView *skein_view = I7_SKEIN_VIEW(panel->tabs[I7_PANE_SKEIN]);
	i7_skein_view_show_node(skein_view, I7_NODE(node), I7_REASON_USER_ACTION);
}

/* Open the Skein layout popover */
static void
action_layout(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
    GtkPopover *popover = GTK_POPOVER(story->skein_spacing_popover);
    gtk_popover_set_relative_to(popover, panel->layout);
    gtk_popover_popup(popover);
}

/* Open the Skein trimming popover */
static void
action_trim(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
    GtkPopover *popover = GTK_POPOVER(story->skein_trim_popover);
    gtk_popover_set_relative_to(popover, panel->trim);
    gtk_popover_popup(popover);
}

/*
 * action_play_all:
 * @action: not used
 * @parameter: not used
 * @panel: the panel that this action was triggered on
 *
 * Signal handler for the action connected to the "Play All" button in the panel
 * toolbar when the Skein panel is displayed. Plays all the nodes currently
 * blessed in the Skein.
 */
static void
action_play_all(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_compile(story, FALSE, FALSE, (CompileActionFunc)i7_story_run_compiler_output_and_entire_skein, NULL);
}

/*
 * action_bless_all:
 * @action: not used
 * @parameter: not used
 * @panel: the panel that this action was triggered on
 * 
 * Signal handler for the action connected to the "Bless All" button in the
 * panel toolbar when the Transcript panel is displayed. Blesses all the nodes
 * currently shown in the transcript. (From the skein's "current node" up to
 * the root node.)
 */
static void
action_bless_all(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
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
 * @parameter: not used
 * @panel: panel this action was triggered on
 *
 * Signal handler for the action connected to the "Previous Difference" button
 * in the panel toolbar when the Transcript panel is displayed.
 */
static void
action_panel_previous_difference(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_previous_difference(story);
}

/*
 * action_panel_next_difference:
 * @action: not used
 * @parameter: not used
 * @panel: panel this action was triggered on
 *
 * Signal handler for the action connected to the "Next Difference" button in
 * the panel toolbar when the Transcript panel is displayed.
 */
static void
action_panel_next_difference(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_next_difference(story);
}

/*
 * action_panel_next_difference_skein:
 * @action: not used
 * @parameter: not used
 * @panel: panel this action was triggered on
 *
 * Signal handler for the action connected to the "Next Difference in Skein"
 * button in the panel toolbar when the Transcript panel is displayed.
 */
static void
action_panel_next_difference_skein(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7Story *story = I7_STORY(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
	i7_story_next_difference_skein(story);
}

/* Signal handler for the action connected to the "Home" button in the panel
toolbar when the Documentation panel is displayed. Displays index.html from the
documentation. */
static void
action_contents(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	i7_panel_goto_doc_uri(panel, "inform:///index.html");
}

/* Signal handler for the action connected to the "Examples" button in the panel
toolbar when the Documentation panel is displayed. Displays
examples_alphabetical.html from the documentation. */
static void
action_examples(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	i7_panel_goto_doc_uri(panel, "inform:///examples_alphabetical.html");
}

/* Signal handler for the action connected to the "General Index" button in the
panel toolbar when the Documentation panel is displayed. Displays
general_index.html from the documentation. */
static void
action_general_index(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	i7_panel_goto_doc_uri(panel, "inform:///general_index.html");
}

/* Signal handler for the action connected to the "Home" button in the panel
toolbar when the Extensions panel is displayed. Displays
Documentation/Extensions.html from the user's extensions folder. */
static void
action_extensions_home(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	I7App *theapp = I7_APP(g_application_get_default());
	GFile *docs_file = i7_app_get_extension_home_page(theapp);
	i7_panel_goto_extensions_docpage(panel, docs_file);
	g_object_unref(docs_file);
}

/* Helper function: turn everything back to normal when the Public Library is
loaded */
static void
on_public_library_load_changed(WebKitWebView *html, WebKitLoadEvent status, GSimpleAction *action)
{
	if(status != WEBKIT_LOAD_FINISHED)
		return;
	g_simple_action_set_enabled(action, TRUE);
	g_application_unmark_busy(g_application_get_default());
	g_signal_handlers_disconnect_by_func(html, on_public_library_load_changed, action);
}

/* Helper function: load the "disconnected" page if an error occurred */
static gboolean
on_public_library_load_failed(WebKitWebView *html, WebKitLoadEvent status, char *uri, GError *web_error)
{
	webkit_web_view_load_uri(html, "inform:///en/pl404.html");
	return TRUE; /* event handled */
}

static char *
build_extensions_javascript_source(I7App *app) {
	GtkTreeModel *model = GTK_TREE_MODEL(i7_app_get_installed_extensions_tree(app));
	GString *builder = g_string_new("window.EXTENSIONS = {");
	GtkTreeIter author, title;
	gtk_tree_model_get_iter_first(model, &author);
	do {
		g_autofree char *authorname;
		gtk_tree_model_get(model, &author, I7_APP_EXTENSION_TEXT, &authorname, -1);

		g_autofree char *author_escaped = g_strescape(authorname, NULL);
		g_string_append_printf(builder, "\"%s\": {", author_escaped);

		if(gtk_tree_model_iter_children(model, &title, &author))
		{
			do {
				g_autofree char *extname;
				g_autofree char *version;
				gboolean readonly;
				gtk_tree_model_get(model, &title,
					I7_APP_EXTENSION_TEXT, &extname,
					I7_APP_EXTENSION_READ_ONLY, &readonly,
					I7_APP_EXTENSION_VERSION, &version,
					-1);

				g_autofree char *name_escaped = g_strescape(extname, NULL);
				g_autofree char *version_escaped = g_strescape(version, NULL);
				g_string_append_printf(builder, "\"%s\": {builtin: %s, version: \"%s\"},",
					name_escaped, readonly ? "true" : "false", version_escaped);
			} while(gtk_tree_model_iter_next(model, &title));

			g_string_append(builder, "},");
		}
	} while(gtk_tree_model_iter_next(model, &author));

	g_string_append(builder, "};");

	return g_string_free(builder, FALSE);
}

static void
add_user_content(WebKitUserContentManager *content, I7App *theapp)
{
	const char *home = g_get_home_dir();
	g_autofree char *inform_dir_pattern = g_build_filename(home, "Inform", "*", NULL);
	g_autoptr(GFile) inform_dir = g_file_new_for_path(inform_dir_pattern);
	g_autofree char *inform_uri_pattern = g_file_get_uri(inform_dir);
	const char *javascript_allowed_uris[] = {
		"about:blank",
		"inform:///*",
		"inform://Extensions/*",
		PUBLIC_LIBRARY_URI "*",
		inform_uri_pattern,
		NULL,
	};

	g_autoptr(GError) error = NULL;
	g_autoptr(GBytes) content_javascript_source = g_resources_lookup_data("/com/inform7/IDE/internal.js",
		G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
	if (!content_javascript_source)
		g_error("failed to lookup resource: %s", error->message);

	g_autoptr(WebKitUserScript) content_javascript =
		webkit_user_script_new(g_bytes_get_data(content_javascript_source, NULL),
			WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
			javascript_allowed_uris, /* block_list = */ NULL);
	webkit_user_content_manager_add_script(content, content_javascript);

	g_autofree char *extensions_javascript_source = build_extensions_javascript_source(theapp);
	g_autoptr(WebKitUserScript) extensions_javascript = webkit_user_script_new(extensions_javascript_source,
		WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
		javascript_allowed_uris, /* block_list = */ NULL);
	webkit_user_content_manager_add_script(content, extensions_javascript);
}

/* Signal handler for the action connected to the "Public Library" button in the
panel toolbar when the Extensions panel is displayed. Displays the Inform public
extensions library website. */
static void
action_public_library(GSimpleAction *action, GVariant *parameter, I7Panel *panel)
{
	WebKitWebView *html = WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_EXTENSIONS]);

	/* First clear the webview, gray out the button, and set the cursor busy so
	that the button doesn't seem broken */
	html_load_blank(html);
	g_simple_action_set_enabled(action, FALSE);
	g_application_mark_busy(g_application_get_default());
	g_signal_connect(html, "load-changed", G_CALLBACK(on_public_library_load_changed), action);
	g_signal_connect(html, "load-failed", G_CALLBACK(on_public_library_load_failed), NULL);

	I7PanelPrivate *priv = i7_panel_get_instance_private(panel);
	webkit_user_content_manager_remove_all_scripts(priv->content);
	add_user_content(priv->content, I7_APP(g_application_get_default()));

	webkit_web_view_load_uri(html, PUBLIC_LIBRARY_HOME_URI);
}

typedef void (*ActionCallback)(GSimpleAction *action, GVariant *param, gpointer data);

static void
create_panel_actions(I7Panel *self)
{
	const GActionEntry actions[] = {
		{ "forward", (ActionCallback)action_forward },
		{ "back", (ActionCallback)action_back },
		{ "play-all-blessed", (ActionCallback)action_play_all },
		{ "trim", (ActionCallback)action_trim },
		{ "layout", (ActionCallback)action_layout },
		{ "jump-to-node", (ActionCallback)action_jump_to_node, "t" },
		{ "bless-all", (ActionCallback)action_bless_all },
		{ "previous-difference", (ActionCallback)action_panel_previous_difference },
		{ "next-difference", (ActionCallback)action_panel_next_difference },
		{ "next-difference-skein", (ActionCallback)action_panel_next_difference_skein },
		{ "goto-general-index", (ActionCallback)action_general_index },
		{ "goto-examples", (ActionCallback)action_examples },
		{ "goto-home", (ActionCallback)action_contents },
		{ "goto-extensions-home", (ActionCallback)action_extensions_home },
		{ "goto-public-library", (ActionCallback)action_public_library },
	};
	self->actions = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(self->actions), actions, G_N_ELEMENTS(actions), self);
}

void
after_source_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(self->notebook)) == I7_PANE_SOURCE)
		history_push_tab(self, I7_PANE_SOURCE, page_num);
}

/* TYPE SYSTEM */

enum _I7PanelSignalType {
	SELECT_VIEW_SIGNAL,
	PASTE_CODE_SIGNAL,
	JUMP_TO_LINE_SIGNAL,
	DISPLAY_DOCPAGE_SIGNAL,
	DISPLAY_EXTENSIONS_DOCPAGE_SIGNAL,
	DISPLAY_COMPILER_REPORT_SIGNAL,
	DISPLAY_INDEX_PAGE_SIGNAL,
	LAST_SIGNAL
};
static guint i7_panel_signals[LAST_SIGNAL] = { 0 };

static void
i7_panel_init(I7Panel *self)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);
	I7App *theapp = I7_APP(g_application_get_default());
	GSettings *prefs = i7_app_get_prefs(theapp);
	int foo;

	/* Initialize the history system */
	priv->history = g_queue_new();
	I7PanelHistory *item = g_slice_new0(I7PanelHistory);
	item->pane = I7_PANE_SOURCE;
	item->tab = I7_SOURCE_VIEW_TAB_SOURCE;
	g_queue_push_head(priv->history, item);
	priv->current = 0;

	/* Build the interface */
	g_type_ensure(WEBKIT_TYPE_WEB_CONTEXT);
	g_type_ensure(WEBKIT_TYPE_SETTINGS);
	g_type_ensure(WEBKIT_TYPE_WEB_VIEW);
	g_type_ensure(WEBKIT_TYPE_USER_CONTENT_MANAGER);
	g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/com/inform7/IDE/ui/panel.ui");
	gtk_builder_connect_signals(builder, self);

	create_panel_actions(self);
	GAction *back = g_action_map_lookup_action(G_ACTION_MAP(self->actions), "back");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(back), FALSE);
	GAction *forward = g_action_map_lookup_action(G_ACTION_MAP(self->actions), "forward");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(forward), FALSE);

	/* Reparent the widgets into our new VBox */
	self->toolbar = GTK_WIDGET(load_object(builder, "toolbar"));
	gtk_widget_insert_action_group(self->toolbar, "panel", G_ACTION_GROUP(self->actions));
	self->notebook = GTK_WIDGET(load_object(builder, "panel"));
	gtk_box_pack_start(GTK_BOX(self), self->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(self), self->notebook, TRUE, TRUE, 0);

	/* Add the I7SourceView widget and connect the after handler of the switch-page signal */
	self->sourceview = I7_SOURCE_VIEW(i7_source_view_new());
    i7_source_view_bind_settings(self->sourceview, prefs);
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
	GtkWidget *gamelabel = GTK_WIDGET(load_object(builder, "story_pane_label"));
	gtk_notebook_insert_page(GTK_NOTEBOOK(self->notebook), game, gamelabel, I7_PANE_STORY);
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_Z8, CHIMARA_IF_INTERPRETER_FROTZ);
	ChimaraIFInterpreter glulx_interpreter =
	    g_settings_get_enum(prefs, PREFS_INTERPRETER) == INTERPRETER_GIT ? CHIMARA_IF_INTERPRETER_GIT : CHIMARA_IF_INTERPRETER_GLULXE;
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_GLULX, glulx_interpreter);
	chimara_glk_set_interactive(CHIMARA_GLK(game), TRUE);
	chimara_glk_set_protect(CHIMARA_GLK(game), FALSE);

    /* Add the Settings pane */
    I7ProjectSettings *settings = i7_project_settings_new();
    gtk_widget_show_all(GTK_WIDGET(settings));
    GtkWidget *settings_label = GTK_WIDGET(load_object(builder, "settings_label"));
    gtk_notebook_insert_page(GTK_NOTEBOOK(self->notebook), GTK_WIDGET(settings), settings_label, I7_PANE_SETTINGS);

	/* Save public pointers to specific widgets */
	LOAD_WIDGET(debugging_scrolledwindow);
	LOAD_WIDGET(inform6_scrolledwindow);
	LOAD_WIDGET(labels);
	LOAD_WIDGET(layout);
	LOAD_WIDGET(trim);
	LOAD_WIDGET(play_all_blessed);
	LOAD_WIDGET(next_difference_skein);
	LOAD_WIDGET(next_difference);
	LOAD_WIDGET(previous_difference);
	LOAD_WIDGET(bless_all);
	LOAD_WIDGET(goto_home);
	LOAD_WIDGET(goto_examples);
	LOAD_WIDGET(goto_general_index);
	LOAD_WIDGET(goto_extensions_home);
	LOAD_WIDGET(goto_public_library);

	/* Add the Labels menu */
	self->labels_menu = g_menu_new();
	GtkWidget *labels_menu = gtk_menu_new_from_model(G_MENU_MODEL(self->labels_menu));
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(self->labels), labels_menu);

	/* Save the public pointers for all the tab arrays */
	self->tabs[I7_PANE_SOURCE] = self->sourceview->notebook;
	self->tabs[I7_PANE_RESULTS] = GTK_WIDGET(load_object(builder, "results_notebook"));
	self->tabs[I7_PANE_INDEX] = GTK_WIDGET(load_object(builder, "index_notebook"));
	self->tabs[I7_PANE_SKEIN] = skeinview;
	self->tabs[I7_PANE_TRANSCRIPT] = GTK_WIDGET(load_object(builder, "transcript"));
	self->tabs[I7_PANE_STORY] = game;
	self->tabs[I7_PANE_DOCUMENTATION] = GTK_WIDGET(load_object(builder, "documentation"));
	self->tabs[I7_PANE_EXTENSIONS] = GTK_WIDGET(load_object(builder, "extensions"));
	self->tabs[I7_PANE_SETTINGS] = GTK_WIDGET(settings);
	self->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS] = self->sourceview->headings;
	self->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE] = self->sourceview->source;
	const gchar *results_tab_names[] = { "progress", "debugging", "report", "inform6" };
	const gchar *index_tab_names[] = { "index_home", "contents", "actions", "kinds", "phrasebook", "rules", "scenes", "world" };
	for(foo = 0; foo < I7_INDEX_NUM_TABS; foo++) {
		if(foo < I7_RESULTS_NUM_TABS)
			self->results_tabs[foo] = GTK_WIDGET(load_object(builder, results_tab_names[foo]));
		self->index_tabs[foo] = GTK_WIDGET(load_object(builder, index_tab_names[foo]));
	}

	/* Set up the web context for this panel */
	WebKitWebContext *webcontext = WEBKIT_WEB_CONTEXT(load_object(builder, "webcontext"));
	i7_uri_scheme_register(webcontext);

	priv->websettings = WEBKIT_SETTINGS(load_object(builder, "websettings"));
	/* g_object_set(priv->websettings, "enable-developer-extras", TRUE, NULL); */

	priv->content = WEBKIT_USER_CONTENT_MANAGER(load_object(builder, "content"));
	if (!webkit_user_content_manager_register_script_message_handler(priv->content, "selectView") ||
	    !webkit_user_content_manager_register_script_message_handler(priv->content, "pasteCode") ||
	    !webkit_user_content_manager_register_script_message_handler(priv->content, "openFile") ||
	    !webkit_user_content_manager_register_script_message_handler(priv->content, "openUrl") ||
	    !webkit_user_content_manager_register_script_message_handler(priv->content, "downloadMultipleExtensions")) {
		g_error("Problem registering JavaScript message handlers");
	}
	g_signal_connect(priv->content, "script-message-received::selectView", G_CALLBACK(js_select_view), self);
	g_signal_connect(priv->content, "script-message-received::pasteCode", G_CALLBACK(js_paste_code), self);
	g_signal_connect(priv->content, "script-message-received::openFile", G_CALLBACK(js_open_file), self);
	g_signal_connect(priv->content, "script-message-received::openUrl", G_CALLBACK(js_open_url), self);
	g_signal_connect(priv->content, "script-message-received::downloadMultipleExtensions", G_CALLBACK(js_download_multi), self);
	add_user_content(priv->content, theapp);

	/* Load the documentation and extension pages */
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), "inform:///index.html");
	g_autoptr(GFile) file = i7_app_get_extension_home_page(theapp);
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_EXTENSIONS]), file);

	/* Set the initial font size */
	i7_panel_update_fonts(self);
    i7_panel_update_font_sizes(self);
}

static void
history_free_queue(I7Panel *self)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);

	g_queue_foreach(priv->history, (GFunc)i7_panel_history_free, NULL);
	g_queue_free(priv->history);
	priv->history = NULL;
	priv->current = 0;
}

static void
i7_panel_constructed(GObject *object)
{
	gtk_orientable_set_orientation(GTK_ORIENTABLE(object), GTK_ORIENTATION_VERTICAL);
}

static void
i7_panel_finalize(GObject *object)
{
	I7Panel *self = I7_PANEL(object);

	history_free_queue(self);

	G_OBJECT_CLASS(i7_panel_parent_class)->finalize(object);
}

static void
i7_panel_class_init(I7PanelClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	object_class->constructed = i7_panel_constructed;
	object_class->finalize = i7_panel_finalize;

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
	i7_panel_signals[DISPLAY_EXTENSIONS_DOCPAGE_SIGNAL] = g_signal_new(
		"display-extensions-docpage",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, display_extensions_docpage), NULL, NULL,
		g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	i7_panel_signals[DISPLAY_COMPILER_REPORT_SIGNAL] = g_signal_new(
		"display-compiler-report",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, display_compiler_report), NULL, NULL,
		g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	i7_panel_signals[DISPLAY_INDEX_PAGE_SIGNAL] = g_signal_new(
		"display-index-page",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, display_index_page), NULL, NULL,
		NULL, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
}

/* SIGNAL HANDLERS */

void
on_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *self)
{
	bool skein = page_num == I7_PANE_SKEIN;
	bool transcript = page_num == I7_PANE_TRANSCRIPT;
	bool documentation = page_num == I7_PANE_DOCUMENTATION;
	bool extensions = page_num == I7_PANE_EXTENSIONS;

	gtk_widget_set_visible(self->labels, skein);
	gtk_widget_set_visible(self->layout, skein);
	gtk_widget_set_visible(self->trim, skein);
	gtk_widget_set_visible(self->play_all_blessed, skein);
	gtk_widget_set_visible(self->next_difference_skein, transcript);
	gtk_widget_set_visible(self->next_difference, transcript);
	gtk_widget_set_visible(self->previous_difference, transcript);
	gtk_widget_set_visible(self->bless_all, transcript);
	gtk_widget_set_visible(self->goto_home, documentation);
	gtk_widget_set_visible(self->goto_examples, documentation);
	gtk_widget_set_visible(self->goto_general_index, documentation);
	gtk_widget_set_visible(self->goto_extensions_home, extensions);
	gtk_widget_set_visible(self->goto_public_library, extensions);
}

/* Internal function: given a filename, determine whether it is one of the HTML
files that comprise the index. Return one of the I7PaneIndexTab constants, or -1
(I7_INDEX_TAB_NONE) if it does not. */
static I7PaneIndexTab
filename_to_index_tab(const char *filename)
{
	I7PaneIndexTab retval;
	for(retval = 0; retval < I7_INDEX_NUM_TABS; retval++)
		if(strcmp(filename, i7_panel_index_names[retval]) == 0)
			return retval;
	return I7_INDEX_TAB_NONE;
}

static gboolean
i7_panel_decide_navigation_policy(I7Panel *self, WebKitWebView *webview, WebKitPolicyDecision *decision)
{
	WebKitNavigationAction *action = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
	WebKitURIRequest *request = webkit_navigation_action_get_request(action);
	const char *uri = webkit_uri_request_get_uri(request);
	gchar *scheme = g_uri_parse_scheme(uri);

	/* If no protocol found, treat it as a file:// */
	if(scheme == NULL)
		scheme = g_strdup("file");

	g_debug("Decide navigation policy %s", uri);

	if(strcmp(scheme, "about") == 0 || strcmp(scheme, "resource") == 0) {
		/* These are protocols that we explicitly allow WebKit to load */
		g_free(scheme);
		webkit_policy_decision_use(decision);
		g_debug("- about or resource: USE");
		return TRUE;  /* handled */

	} else if(strcmp(scheme, "file") == 0) {
		g_free(scheme);

		/* If the file is an index page that is not displayed in the correct
		webview, then redirect the request to the index page */
		char *path = g_filename_from_uri(uri, NULL, NULL);
		if (path == NULL) {
			webkit_policy_decision_use(decision);
			g_debug("- file with no filename: USE");
			return TRUE;  /* handled */
		}
		char *filename = g_path_get_basename(path);
		g_free(path);

		/* Chop off any URI parameters and save them for the signal emission */
		char *param_location = strchr(filename, '?');
		char *param = NULL;
		if(param_location != NULL) {
			*param_location = '\0';
			param = param_location + 1; /* freeing filename frees param now */
		}

		I7PaneIndexTab tabnum = filename_to_index_tab(filename);
		if (tabnum == I7_INDEX_TAB_NONE) {
			webkit_policy_decision_use(decision);
			g_debug("- don't know where to display this file: USE");
			return TRUE;  /* handled */
		}

		/* We've determined that this is an index page */
		if (webview != WEBKIT_WEB_VIEW(self->index_tabs[tabnum])) {
			g_signal_emit_by_name(self, "display-index-page", tabnum, param);
			g_free(filename);
			webkit_policy_decision_ignore(decision);
			g_debug("- index file, page %u (at %s): IGNORE", tabnum, param);
			return TRUE;  /* handled */
		}
		g_free(filename);
		webkit_policy_decision_use(decision);
		g_debug("- display in this webview: USE");
		return TRUE;  /* handled */

	} else if(strcmp(scheme, "inform") == 0) {
		/* The inform: protocol can mean files in any of several different
		locations */
		gboolean load_in_extensions_pane = g_str_has_prefix(uri, "inform://Extensions") ||
			g_str_has_prefix(uri, "inform:/Extensions") ||
			g_str_has_suffix(uri, "pl404.html");
		gboolean load_in_report_tab = g_str_has_prefix(uri, "inform:///en") &&
			!g_str_has_suffix(uri, "pl404.html");
		/* Most of them are only to be loaded in the documentation pane, but
		 * extension documentation should be loaded in the extensions pane, and
		 * compiler error pages should be loaded in the Report tab of the
		 * Results pane. */
		if (load_in_extensions_pane && webview != WEBKIT_WEB_VIEW(self->tabs[I7_PANE_EXTENSIONS])) {
			g_signal_emit_by_name(self, "display-extensions-docpage", uri);
			g_free(scheme);
			webkit_policy_decision_ignore(decision);
			g_debug("- display inform document in extensions pane: IGNORE");
			return TRUE;  /* handled */
		} else if (load_in_report_tab && webview != WEBKIT_WEB_VIEW(self->results_tabs[I7_RESULTS_TAB_REPORT])) {
			g_signal_emit_by_name(self, "display-compiler-report", uri);
			g_free(scheme);
			webkit_policy_decision_ignore(decision);
			g_debug("- display inform document in report tab: IGNORE");
			return TRUE;  /* handled */
		} else if (!load_in_extensions_pane && !load_in_report_tab &&
				webview != WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION])) {
			/* Others should be loaded in the documentation pane; if this is another
			pane, then redirect the request to the documentation pane */
			g_signal_emit_by_name(self, "display-docpage", uri);
			g_free(scheme);
			webkit_policy_decision_ignore(decision);
			g_debug("- display inform document in documentation pane: IGNORE");
			return TRUE;  /* handled */
		}

		webkit_policy_decision_use(decision);
		g_debug("- display inform document in this webview: USE");
		return TRUE;  /* handled */

	} else if (strcmp(scheme, "https") == 0 || strcmp(scheme, "http") == 0 ||
			strcmp(scheme, "mailto") == 0) {
		/* Allow the Public Library website, but nothing else */
		if (g_str_has_prefix(uri, PUBLIC_LIBRARY_HOME_URI)) {
			webkit_policy_decision_use(decision);
			g_debug("- display http page in this webview: USE");
			return TRUE;  /* handled */
		}

		show_uri_in_browser(uri,
			GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(self))), NULL);
		g_debug("- show http or mailto in browser: IGNORE");

	} else if(strcmp(scheme, "source") == 0) {
		guint line = 0;
		gchar *path = g_strdup(uri + strlen("source:"));
		gchar *ptr = strrchr(path, '#');
		gchar *anchor = g_strdup(ptr);
		*ptr = 0;

		/* If it links to the source file, just jump to the line */
		if(strcmp(path, "story.ni") == 0) {
			if(sscanf(anchor, "#line%u", &line))
				g_signal_emit_by_name(self, "jump-to-line", line);
			g_debug("- jump to source line %u: IGNORE", line);
		} else {
			GFile *file = g_file_new_for_path(path);
			/* Else it's a link to an extension, open it in a new window */
			GFile *real_file = get_case_insensitive_extension(file);
			g_object_unref(file);
			/* Check if we need to open the extension read-only */
			I7App *theapp = I7_APP(g_application_get_default());
			GFile *user_file = i7_app_get_extension_file(theapp, NULL, NULL);
			gboolean readonly = !g_file_has_prefix(real_file, user_file);
			g_object_unref(user_file);

			I7Extension *ext = i7_extension_new_from_file(theapp, real_file, readonly);
			if(ext != NULL) {
				if(sscanf(anchor, "#line%u", &line))
					i7_source_view_jump_to_line(ext->sourceview, line);
				g_debug("- jump to source line %u of extension %s: IGNORE", line, path);
			}
		}
		g_free(path);
		g_free(anchor);

	} else if(strcmp(scheme, "library") == 0) {
		char *id, *author, *title;
		GFile *remote_file = library_uri_to_real_uri(uri, &author, &title, &id);

		I7Document *doc = I7_DOCUMENT(gtk_widget_get_toplevel(GTK_WIDGET(self)));
		gboolean success = i7_document_download_single_extension(doc, remote_file, author, title);

		g_debug("- library download %s by %s (id %s): IGNORE", title, author, id);
		g_object_unref(remote_file);
		g_free(author);
		g_free(title);

		if (success)
			notify_download_succeeded(webview, id);
		g_free(id);

	} else {
		g_warning("Unrecognized URI scheme: %s", scheme);
	}

	g_free(scheme);

	webkit_policy_decision_ignore(decision);
	return TRUE;  /* handled */
}

/* This is the callback that handles custom behaviour and permissions of the
 * webview */
gboolean
on_decide_policy(WebKitWebView *webview, WebKitPolicyDecision *decision, WebKitPolicyDecisionType decision_type, I7Panel *self)
{
	switch (decision_type) {
	case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
		return i7_panel_decide_navigation_policy(self, webview, decision);
	case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
		webkit_policy_decision_ignore(decision);
		return TRUE;  /* handled */
	case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
		webkit_policy_decision_use(decision);
		return TRUE;  /* handled */
	default:
		return FALSE;  /* not handled */
	}
}

void
after_documentation_notify_uri(WebKitWebView *webview, GParamSpec *pspec, I7Panel *self)
{
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(self->notebook)) != I7_PANE_DOCUMENTATION)
		return;

	/* WebKit can send null, empty, or about:blank URIs asynchronously during loading */
	const char *uri = webkit_web_view_get_uri(webview);
	if (uri && *uri != '\0' && strcmp(uri, "about:blank") != 0)
		history_push_docpage(self, uri);
}

/* Signal handler to pick up navigations within the Extensions pane and push
them into the history queue. */
void
after_extensions_notify_uri(WebKitWebView *webview, GParamSpec *pspec, I7Panel *self)
{
	/* Only add it to the history if it was navigated from the current page */
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(self->notebook)) != I7_PANE_EXTENSIONS)
		return;

	/* WebKit can send null, empty, or about:blank URIs asynchronously during loading */
	const char *uri = webkit_web_view_get_uri(webview);
	if (uri && *uri != '\0' && strcmp(uri, "about:blank") != 0)
		history_push_extensions_page(self, uri);
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
 * @page_uri: (nullable): the URI to display in the documentation pane after
 * resetting the queue
 *
 * Set the tab and subtab and reset the history queue. Display that item in
 * the panel.
 */
void
i7_panel_reset_queue(I7Panel *self, I7PanelPane pane, int tab, const char *page_uri)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);
	history_free_queue(self);
	priv->history = g_queue_new();
	I7PanelHistory *item = g_slice_new0(I7PanelHistory);
	item->pane = pane;
	item->tab = tab;
	item->page = page_uri ? g_strdup(page_uri) : NULL;
	g_queue_push_head(priv->history, item);
	priv->current = 0;
	history_goto_current(self);
}

void
i7_panel_goto_docpage(I7Panel *self, GFile *file)
{
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), file);
}

/**
 * i7_panel_goto_doc_uri:
 * @self: the panel
 * @uri: the URI to display
 *
 * Display @uri in the documentation pane of @self.
 */
void
i7_panel_goto_doc_uri(I7Panel *self, const char *uri)
{
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), uri);
}

void
i7_panel_goto_docpage_at_anchor(I7Panel *self, GFile *file, const char *anchor)
{
	html_load_file_at_anchor(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), file, anchor);
}

/**
 * i7_panel_goto_extensions_docpage:
 * @self: the panel
 * @file: file reference pointing to the page
 *
 * Load @file in this panel's Extensions pane.
 */
void
i7_panel_goto_extensions_docpage(I7Panel *self, GFile *file)
{
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_EXTENSIONS]), file);
}

void
i7_panel_update_tabs(I7Panel *self)
{
	g_idle_add((GSourceFunc)update_tabs, GTK_SOURCE_VIEW(self->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE]));
	g_idle_add((GSourceFunc)update_tabs, GTK_SOURCE_VIEW(self->results_tabs[I7_RESULTS_TAB_INFORM6]));
}

/* Update the fonts of the widgets in this pane */
void
i7_panel_update_fonts(I7Panel *self)
{
	i7_panel_update_tabs(self);

	I7App *theapp = I7_APP(g_application_get_default());
	g_autoptr(PangoFontDescription) desc = i7_app_get_document_font_description(theapp);
    const char *font = pango_font_description_get_family(desc);
    int size_pt = pango_font_description_get_size(desc) / PANGO_SCALE;

	gchar *css = g_strdup_printf(
		"grid.normal { font-size: %d; }"
		"grid.user1 { color: #303030; background-color: #ffffff; }"
	    "buffer.default { font-family: '%s'; font-size: %d; }"
		"buffer.normal { font-size: %d; }"
        "buffer.emphasized { font-size: %d; font-style: italic; }"
		"buffer.header { font-size: %d; font-weight: bold; }"
		"buffer.subheader { font-size: %d; font-weight: bold; }"
		"buffer.alert { color: #aa0000; font-weight: bold; }"
		"buffer.note { color: #aaaa00; font-weight: bold; }"
		"buffer.block-quote { text-align: center; font-style: italic; }"
		"buffer.input { font-size: %d; color: #0000aa; font-style: italic; }"
		"buffer.user1 { }"
		"buffer.user2 { }",
		size_pt, font, size_pt, size_pt, size_pt,
		(int)(size_pt * RELATIVE_SIZE_MEDIUM), size_pt, size_pt);
	chimara_glk_set_css_from_string(CHIMARA_GLK(self->tabs[I7_PANE_STORY]), css);
	g_free(css);
}

/* Update the font sizes of WebViews in this pane */
void
i7_panel_update_font_sizes(I7Panel *self)
{
	I7App *theapp = I7_APP(g_application_get_default());
	double scale = i7_app_get_docs_font_scale(theapp);
	webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(self->results_tabs[I7_RESULTS_TAB_REPORT]), scale);
	for (int ix = 0; ix < I7_INDEX_NUM_TABS; ix++)
		webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(self->index_tabs[ix]), scale);
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), scale);
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_EXTENSIONS]), scale);
}

/* Empty the list of pages to go forward to */
static void
history_empty_forward_queue(I7Panel *self)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);

	/* Delete all the members of the queue before "current" */
	guint count;
	for(count = 0; count < priv->current; count++)
		i7_panel_history_free(g_queue_pop_head(priv->history));
	priv->current = 0;

	GAction *forward = g_action_map_lookup_action(G_ACTION_MAP(self->actions), "forward");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(forward), FALSE);
}

/* Empty the forward queue and push a new item to the front of the history */
void
i7_panel_push_history_item(I7Panel *self, I7PanelHistory *item)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);

	if (priv->current == g_queue_get_length(priv->history) - 1) {
		GAction *back = g_action_map_lookup_action(G_ACTION_MAP(self->actions), "back");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(back), TRUE);
	}

	history_empty_forward_queue(self);
	g_queue_push_head(priv->history, item);
	priv->current = 0;
}

I7PanelHistory *
i7_panel_get_current_history_item(I7Panel *self)
{
	I7PanelPrivate *priv = i7_panel_get_instance_private(self);
	return g_queue_peek_nth(priv->history, priv->current);
}
