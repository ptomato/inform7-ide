/* Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 P. F. Chimento
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

#include "config.h"

#include <math.h>
#include <string.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-if.h>
#include <webkit/webkit.h>
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
#include "panel-private.h"
#include "skein-view.h"
#include "transcript-renderer.h"

#define PUBLIC_LIBRARY_URI "http://www.emshort.com/pl/"
#define PUBLIC_LIBRARY_HOME_URI PUBLIC_LIBRARY_URI "index.html"

const char * const i7_panel_index_names[] = {
	"Welcome.html", "Contents.html", "Actions.html", "Kinds.html",
	"Phrasebook.html", "Rules.html", "Scenes.html", "World.html"
};

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

/* Helper function: convert a string JSValueRef argument to a NULL-terminated
UTF-8 C string. Returns NULL and fills @exception on error. Free result when
done. */
static char *
js_string_value_to_string(JSContextRef ctx, JSValueRef value, JSValueRef *exception)
{
	JSStringRef jsstring = JSValueToStringCopy(ctx, value, exception);
	if(*exception != NULL)
		return NULL;
	size_t bufsize = JSStringGetMaximumUTF8CStringSize(jsstring);
	char *string = g_malloc(bufsize);
	JSStringGetUTF8CString(jsstring, string, bufsize);
	JSStringRelease(jsstring);
	return string;
}

/* Helper function: convert a NULL-terminated UTF-8 C string to a string
JSValueRef. Release result when done (or return from a Javascript function.) */
static JSValueRef
string_to_js_string_value(JSContextRef ctx, const char *string)
{
	JSStringRef js_string = JSStringCreateWithUTF8CString(string);
	JSValueRef retval = JSValueMakeString(ctx, js_string);
	JSStringRelease(js_string);
	return retval;
}

/* The 'pasteCode' function in JavaScript. Unescapes the code to paste, and
 * emits the 'paste-code' signal, which the I7Story is listening for. */
static JSValueRef
js_paste_code(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	if(argumentCount != 1)
		return NULL;

	I7Panel *panel = (I7Panel *)JSObjectGetPrivate(thisObject);
	char *code = js_string_value_to_string(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;

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

	char *file = js_string_value_to_string(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;

	gchar *uri = g_filename_to_uri(file, NULL, &error);
	if(uri != NULL) {
		show_uri_externally(uri, NULL, file);
	} else {
		g_warning("Filename has no URI: %s", error->message);
		g_clear_error(&error);
	}

	g_free(uri);
	g_free(file);
	return NULL;
}

/* The 'openUrl()' function in JavaScript. This also opens its argument in the
system's default viewer, but that viewer happens to be the web browser. */
static JSValueRef
js_open_url(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	if(argumentCount != 1)
		return NULL;

	char *uri = js_string_value_to_string(ctx, arguments[0], exception);
	if(*exception != NULL)
		return NULL;

	show_uri_externally(uri, NULL, NULL);

	g_free(uri);
	return NULL;
}

/* Helper function: convert the internal representation of an extension version
to the form that the Public Library page's Javascript expects it in. Free return
value when done. */
static char *
version_to_public_library_version(char *version)
{
	if(*version == '\0')
		return g_strdup("Version 1");
	return g_strconcat("Version ", version, NULL);
}

/* The 'askInterfaceForLocalVersion(author, title, compareWith)' function in
Javascript. This asks the app to compare the extension "@title by @author" the
app has with version @compareWith. It returns, in this order of consideration:
(1) the string ‘!’ if the app has a built-in copy of the given extension; or
(2) the string ‘=’ if the app has the exact same version installed already; or
(3) the string ‘<’ if the app has a lower version; or (4) the string ‘>’ if the
app has a newer version; or (5) the empty string if the app has no version of
this extension. (The Javascript on the page then adjusts all the download
buttons appropriately.) */
static JSValueRef
js_local_version_compare(JSContextRef ctx, JSObjectRef function, JSObjectRef this_object, size_t argument_count, const JSValueRef arguments[], JSValueRef *exception)
{
	if(argument_count != 3)
		return NULL;

	char *author = js_string_value_to_string(ctx, arguments[0], exception);
	char *title = js_string_value_to_string(ctx, arguments[1], exception);
	char *compare_with = js_string_value_to_string(ctx, arguments[2], exception);
	if(author == NULL || title == NULL || compare_with == NULL) {
		g_free(author);
		g_free(title);
		g_free(compare_with);
		return NULL;
	}

	gboolean builtin;
	char *version = i7_app_get_extension_version(i7_app_get(), author, title, &builtin);
	g_free(author);
	g_free(title);

	if(version == NULL) {
		g_free(compare_with);
		return string_to_js_string_value(ctx, "");
	}

	/* Format the version string in the way the page's JS expects it */
	char *compare_version = version_to_public_library_version(version);
	g_free(version);

	int comparison = strcmp(compare_version, compare_with);
	g_free(compare_version);
	g_free(compare_with);

	if(builtin)
		return string_to_js_string_value(ctx, "!");
	if(comparison < 0)
		return string_to_js_string_value(ctx, "<");
	if(comparison > 0)
		return string_to_js_string_value(ctx, ">");
	return string_to_js_string_value(ctx, "=");
}

/* The 'askInterfaceForLocalVersionText(author, title)' function in Javascript.
Returns the app's version of the extension as a string: e.g., '8', or
'8/110516'. (Seems the Public Library page actually expects it to have the
string 'Version ' prefixed to it.) */
static JSValueRef
js_get_local_version(JSContextRef ctx, JSObjectRef function, JSObjectRef this_object, size_t argument_count, const JSValueRef arguments[], JSValueRef *exception)
{
	if(argument_count != 2)
		return NULL;

	char *author = js_string_value_to_string(ctx, arguments[0], exception);
	char *title = js_string_value_to_string(ctx, arguments[1], exception);
	if(author == NULL || title == NULL) {
		g_free(author);
		g_free(title);
		return NULL;
	}

	char *version = i7_app_get_extension_version(i7_app_get(), author, title, NULL);
	g_free(author);
	g_free(title);
	if(version == NULL)
		return NULL;  /* return undefined in JS */

	char *formatted_version = version_to_public_library_version(version);
	g_free(version);

	JSValueRef retval = string_to_js_string_value(ctx, formatted_version);
	g_free(formatted_version);
	return retval;
}

/* Helper function: Converts a library:/ URI to a real http:// URI, extracting
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
on_download_finished(gboolean success, const char *id, WebKitWebView *webview)
{
	if(success) {
		char *script = g_strconcat("downloadSucceeded(", id, ");", NULL);
		webkit_web_view_execute_script(webview, script);
		g_free(script);
	}
}

static JSValueRef
js_download_multi(JSContextRef ctx, JSObjectRef function, JSObjectRef this_object, size_t argument_count, const JSValueRef arguments[], JSValueRef *exception)
{
	I7Panel *panel = (I7Panel *)JSObjectGetPrivate(this_object);

	if(argument_count != 1)
		return NULL;

	/* Trick to get array length in less code */
	JSStringRef script_js = JSStringCreateWithUTF8CString("return arguments[0].length;");
	JSObjectRef get_length = JSObjectMakeFunction(ctx, NULL, 0, NULL, script_js, NULL, 1, exception);
	JSStringRelease(script_js);
	JSValueRef length_js = JSObjectCallAsFunction(ctx, get_length, this_object, 1, (JSValueRef *)&arguments[0], exception);
	if(length_js == NULL)
		return NULL;
	double length = JSValueToNumber(ctx, length_js, exception);
	if(isnan(length))
		return NULL;

	JSObjectRef array = JSValueToObject(ctx, arguments[0], exception);
	if(array == NULL)
		return NULL;

	/* Download the extensions one by one */
	unsigned n_extensions = length / 3.0;
	char **ids = g_new0(char *, n_extensions);
	GFile **files = g_new0(GFile *, n_extensions);
	char **authors = g_new0(char *, n_extensions);
	char **titles = g_new0(char *, n_extensions);
	char **versions = g_new0(char *, n_extensions);
	unsigned ix;
	for(ix = 0; ix < n_extensions; ix++) {
		JSValueRef id_val = JSObjectGetPropertyAtIndex(ctx, array, 3 * ix, exception);
		JSValueRef uri_val = JSObjectGetPropertyAtIndex(ctx, array, 3 * ix + 1, exception);
		JSValueRef desc_val = JSObjectGetPropertyAtIndex(ctx, array, 3 * ix + 2, exception);
		char *id = js_string_value_to_string(ctx, id_val, exception);
		char *uri = js_string_value_to_string(ctx, uri_val, exception);
		char *desc = js_string_value_to_string(ctx, desc_val, exception);
		if(id == NULL || uri == NULL || desc == NULL)
			goto finally;

		ids[ix] = id;
		files[ix] = library_uri_to_real_uri(uri, &authors[ix], &titles[ix], NULL);
		versions[ix] = desc;
		g_free(uri);
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
	GSettings *skein_settings = g_settings_new(SCHEMA_SKEIN);

	/* Save old values in case the user decides to cancel */
	double old_horizontal_spacing = g_settings_get_double(skein_settings, PREFS_SKEIN_HORIZONTAL_SPACING);
	double old_vertical_spacing = g_settings_get_double(skein_settings, PREFS_SKEIN_VERTICAL_SPACING);

	int response = 1; /* 1 = "Use defaults" */
	while(response == 1)
		response = gtk_dialog_run(GTK_DIALOG(story->skein_spacing_dialog));
		/* If "Use defaults" clicked, then restart the dialog */
	gtk_widget_hide(story->skein_spacing_dialog);

	if(response != GTK_RESPONSE_OK) {
		g_settings_set_double(skein_settings, PREFS_SKEIN_HORIZONTAL_SPACING, old_horizontal_spacing);
		g_settings_set_double(skein_settings, PREFS_SKEIN_VERTICAL_SPACING, old_vertical_spacing);
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

/* Helper function: display a documentation page in this panel */
static void
display_docpage_in_panel(I7Panel *panel, const char *filename)
{
	GFile *docs_file = i7_app_get_data_file_va(i7_app_get(), "Documentation", filename, NULL);
	i7_panel_goto_docpage(panel, docs_file);
	g_object_unref(docs_file);
}

/* Signal handler for the action connected to the "Home" button in the panel
toolbar when the Documentation panel is displayed. Displays index.html from the
documentation. */
void
action_contents(GtkAction *action, I7Panel *panel)
{
	display_docpage_in_panel(panel, "index.html");
}

/* Signal handler for the action connected to the "Examples" button in the panel
toolbar when the Documentation panel is displayed. Displays
examples_alphabetical.html from the documentation. */
void
action_examples(GtkAction *action, I7Panel *panel)
{
	display_docpage_in_panel(panel, "examples_alphabetical.html");
}

/* Signal handler for the action connected to the "General Index" button in the
panel toolbar when the Documentation panel is displayed. Displays
general_index.html from the documentation. */
void
action_general_index(GtkAction *action, I7Panel *panel)
{
	display_docpage_in_panel(panel, "general_index.html");
}

/* Signal handler for the action connected to the "Home" button in the panel
toolbar when the Extensions panel is displayed. Displays
Documentation/Extensions.html from the user's extensions folder. */
void
action_extensions_home(GtkAction *action, I7Panel *panel)
{
	GFile *docs_file = i7_app_get_extension_home_page(i7_app_get());
	i7_panel_goto_extensions_docpage(panel, docs_file);
	g_object_unref(docs_file);
}

/* Signal handler for the action connected to the "Definitions" button in the
panel toolbar when the Extensions panel is displayed. Displays
Documentation/ExtIndex.html from the user's extensions folder. */
void
action_definitions(GtkAction *action, I7Panel *panel)
{
	GFile *docs_file = i7_app_get_extension_index_page(i7_app_get());
	i7_panel_goto_extensions_docpage(panel, docs_file);
	g_object_unref(docs_file);
}

/* Helper function: turn everything back to normal when the Public Library is
loaded */
static void
on_public_library_load_notify(WebKitWebView *html, GParamSpec *pspec, GtkAction *action)
{
	WebKitLoadStatus status = webkit_web_view_get_load_status(html);
	if(status != WEBKIT_LOAD_FINISHED && status != WEBKIT_LOAD_FAILED)
		return;
	gtk_action_set_sensitive(action, TRUE);
	i7_app_set_busy(i7_app_get(), FALSE);
	g_signal_handlers_disconnect_by_func(html, on_public_library_load_notify, action);
}

/* Helper function: load the "disconnected" page if an error occurred */
static gboolean
on_public_library_load_error(WebKitWebView *html, WebKitWebFrame *frame, char *uri, GError *web_error)
{
	GFile *pl404 = i7_app_get_data_file_va(i7_app_get(), "Resources", "en", "pl404.html", NULL);
	html_load_file(html, pl404);
	g_object_unref(pl404);
	return TRUE; /* event handled */
}

/* Signal handler for the action connected to the "Public Library" button in the
panel toolbar when the Extensions panel is displayed. Displays the Inform public
extensions library website. */
void
action_public_library(GtkAction *action, I7Panel *panel)
{
	WebKitWebView *html = WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_EXTENSIONS]);

	/* First clear the webview, gray out the button, and set the cursor busy so
	that the button doesn't seem broken */
	html_load_blank(html);
	gtk_action_set_sensitive(action, FALSE);
	i7_app_set_busy(i7_app_get(), TRUE);
	g_signal_connect(html, "notify::load-status", G_CALLBACK(on_public_library_load_notify), action);
	g_signal_connect(html, "load-error", G_CALLBACK(on_public_library_load_error), NULL);

	webkit_web_view_open(html, PUBLIC_LIBRARY_HOME_URI);
}

/* TYPE SYSTEM */

enum _I7PanelSignalType {
	SELECT_VIEW_SIGNAL,
	PASTE_CODE_SIGNAL,
	JUMP_TO_LINE_SIGNAL,
	DISPLAY_DOCPAGE_SIGNAL,
	DISPLAY_EXTENSIONS_DOCPAGE_SIGNAL,
	DISPLAY_INDEX_PAGE_SIGNAL,
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
	GFile *file = i7_app_get_data_file_va(theapp, "ui", "panel.ui", NULL);
	GtkBuilder *builder = create_new_builder(file, self);
	g_object_unref(file);
	
	/* Make the action groups */
	priv->common_action_group = GTK_ACTION_GROUP(load_object(builder, "panel_actions"));
	priv->skein_action_group = GTK_ACTION_GROUP(load_object(builder, "skein_actions"));
	priv->transcript_action_group = GTK_ACTION_GROUP(load_object(builder, "transcript_actions"));
	priv->documentation_action_group = GTK_ACTION_GROUP(load_object(builder, "documentation_actions"));
	priv->extensions_action_group = GTK_ACTION_GROUP(load_object(builder, "extensions_actions"));

	/* Build the toolbar from the GtkUIManager file. The UI manager owns the action groups now. */
	priv->ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->common_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->skein_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->transcript_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->documentation_action_group, 0);
	gtk_ui_manager_insert_action_group(priv->ui_manager, priv->extensions_action_group, 0);
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
	gtk_activatable_set_related_action(GTK_ACTIVATABLE(self->labels), self->labels_action);

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
	GtkWidget *gamelabel = GTK_WIDGET(load_object(builder, "story_pane_label"));
	gtk_notebook_insert_page(GTK_NOTEBOOK(self->notebook), game, gamelabel, I7_PANE_STORY);
	chimara_if_set_preferred_interpreter(CHIMARA_IF(game), CHIMARA_IF_FORMAT_Z8, CHIMARA_IF_INTERPRETER_FROTZ);
	ChimaraIFInterpreter glulx_interpreter =
	    g_settings_get_enum(prefs, PREFS_INTERPRETER) == INTERPRETER_GIT ? CHIMARA_IF_INTERPRETER_GIT : CHIMARA_IF_INTERPRETER_GLULXE;
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
	LOAD_WIDGET(z8);
	LOAD_WIDGET(glulx);
	LOAD_WIDGET(blorb);
	LOAD_WIDGET(nobble_rng);
	LOAD_WIDGET(debugging_scrolledwindow);
	LOAD_WIDGET(inform6_scrolledwindow);
	LOAD_WIDGET(transcript_menu);
	g_object_ref(self->transcript_menu);

	/* Save the public pointers for all the tab arrays */
	self->tabs[I7_PANE_SOURCE] = self->sourceview->notebook;
	self->tabs[I7_PANE_RESULTS] = GTK_WIDGET(load_object(builder, "results_notebook"));
	self->tabs[I7_PANE_INDEX] = GTK_WIDGET(load_object(builder, "index_notebook"));
	self->tabs[I7_PANE_SKEIN] = skeinview;
	self->tabs[I7_PANE_TRANSCRIPT] = GTK_WIDGET(load_object(builder, "transcript"));
	self->tabs[I7_PANE_STORY] = game;
	self->tabs[I7_PANE_DOCUMENTATION] = GTK_WIDGET(load_object(builder, "documentation"));
	self->tabs[I7_PANE_EXTENSIONS] = GTK_WIDGET(load_object(builder, "extensions"));
	self->tabs[I7_PANE_SETTINGS] = GTK_WIDGET(load_object(builder, "settings"));
	self->source_tabs[I7_SOURCE_VIEW_TAB_CONTENTS] = self->sourceview->headings;
	self->source_tabs[I7_SOURCE_VIEW_TAB_SOURCE] = self->sourceview->source;
	const gchar *results_tab_names[] = { "progress", "debugging", "report", "inform6" };
	const gchar *index_tab_names[] = { "index_home", "contents", "actions", "kinds", "phrasebook", "rules", "scenes", "world" };
	for(foo = 0; foo < I7_INDEX_NUM_TABS; foo++) {
		if(foo < I7_RESULTS_NUM_TABS)
			self->results_tabs[foo] = GTK_WIDGET(load_object(builder, results_tab_names[foo]));
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
		{ "askInterfaceForLocalVersion", js_local_version_compare, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "askInterfaceForLocalVersionText", js_get_local_version, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "downloadMultipleExtensions", js_download_multi, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
		{ NULL, NULL, 0 }
	};
	JSClassDefinition project_class_definition = {
		/* version */ 0, kJSClassAttributeNone, "ProjectClass",
		/* parent */ NULL, /* static values */ NULL, project_class_functions,
		/* callbacks */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL
	};
	priv->js_class = JSClassCreate(&project_class_definition);

	/* Load the documentation and extension pages */
	file = i7_app_get_data_file_va(theapp, "Documentation", "index.html", NULL);
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_DOCUMENTATION]), file);
	g_object_unref(file);
	file = i7_app_get_extension_home_page(theapp);
	html_load_file(WEBKIT_WEB_VIEW(self->tabs[I7_PANE_EXTENSIONS]), file);
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
	i7_panel_signals[DISPLAY_EXTENSIONS_DOCPAGE_SIGNAL] = g_signal_new(
		"display-extensions-docpage",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, display_extensions_docpage), NULL, NULL,
		g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	i7_panel_signals[DISPLAY_INDEX_PAGE_SIGNAL] = g_signal_new(
		"display-index-page",
		G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(I7PanelClass, display_index_page), NULL, NULL,
		NULL, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
	g_type_class_add_private(klass, sizeof(I7PanelPrivate));
}

/* SIGNAL HANDLERS */

void
on_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *panel)
{
	I7_PANEL_USE_PRIVATE(panel, priv);

	gboolean skein = FALSE, transcript = FALSE, documentation = FALSE, extensions = FALSE;

	switch(page_num) {
		case I7_PANE_SKEIN:
			skein = TRUE;
			break;
		case I7_PANE_TRANSCRIPT:
			transcript = TRUE;
			break;
		case I7_PANE_DOCUMENTATION:
			documentation = TRUE;
			break;
		case I7_PANE_EXTENSIONS:
			extensions = TRUE;
			break;
		default:
			;
	}

	gtk_action_group_set_visible(priv->skein_action_group, skein);
	gtk_action_group_set_visible(priv->transcript_action_group, transcript);
	gtk_action_group_set_visible(priv->documentation_action_group, documentation);
	gtk_action_group_set_visible(priv->extensions_action_group, extensions);
}

void
after_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *panel)
{
	switch(page_num) {
		case I7_PANE_SOURCE:
			history_push_tab(panel, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_SOURCE])));
			break;
		case I7_PANE_RESULTS:
			history_push_tab(panel, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_RESULTS])));
			break;
		case I7_PANE_INDEX:
			history_push_tab(panel, page_num,
				gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->tabs[I7_PANE_INDEX])));
			break;
		case I7_PANE_DOCUMENTATION:
			history_push_docpage(panel, NULL);
			break;
		case I7_PANE_EXTENSIONS:
			history_push_extensions_page(panel, NULL);
			break;
		default:
			history_push_pane(panel, page_num);
	}
}

void
after_source_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_SOURCE)
		history_push_tab(panel, I7_PANE_SOURCE, page_num);
}

void
after_results_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_RESULTS)
		history_push_tab(panel, I7_PANE_RESULTS, page_num);
}

void
after_index_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, unsigned page_num, I7Panel *panel)
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_INDEX)
		history_push_tab(panel, I7_PANE_INDEX, page_num);
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

	parent = i7_app_get_data_file_va(theapp, "Resources", "doc_images", NULL);
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if(g_file_query_exists(real_file, NULL))
		goto finally;
	g_object_unref(real_file);
	g_object_unref(parent);

	parent = i7_app_get_data_file_va(theapp, "Resources", "bg_images", NULL);
	real_file = g_file_resolve_relative_path(parent, relative_path);
	if(g_file_query_exists(real_file, NULL))
		goto finally;
	g_object_unref(real_file);
	g_object_unref(parent);

	parent = i7_app_get_data_file_va(theapp, "Resources", NULL);
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
	const gchar *uri = webkit_network_request_get_uri(request);
	gchar *scheme = g_uri_parse_scheme(uri);

	/* If no protocol found, treat it as a file:// */
	if(scheme == NULL)
		scheme = g_strdup("file");

	if(strcmp(scheme, "about") == 0) {
		/* These are protocols that we explicitly allow WebKit to load */
		g_free(scheme);
		return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

	} else if(strcmp(scheme, "file") == 0) {
		g_free(scheme);

		/* If the file is an index page that is not displayed in the correct
		webview, then redirect the request to the index page */
		char *path = g_filename_from_uri(uri, NULL, NULL);
		if(path == NULL)
			return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;
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
		if(tabnum == I7_INDEX_TAB_NONE)
			return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

		/* We've determined that this is an index page */
		if(webview != WEBKIT_WEB_VIEW(panel->index_tabs[tabnum])) {
			g_signal_emit_by_name(panel, "display-index-page", tabnum, param);
			g_free(filename);
			return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
		}
		g_free(filename);
		return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

	} else if(strcmp(scheme, "inform") == 0) {
		/* The inform: protocol can mean files in any of several different
		locations */
		gboolean load_in_extensions_pane = g_str_has_prefix(uri, "inform://Extensions");
		/* Most of them are only to be loaded in the documentation pane, but extension
		documentation should be loaded in the extensions pane. */
		if(load_in_extensions_pane && webview != WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_EXTENSIONS])) {
			g_signal_emit_by_name(panel, "display-extensions-docpage", uri);
			g_free(scheme);
			return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
		} else if(!load_in_extensions_pane && webview != WEBKIT_WEB_VIEW(panel->tabs[I7_PANE_DOCUMENTATION])) {
			/* Others should be loaded in the documentation pane; if this is another
			pane, then redirect the request to the documentation pane */
			g_signal_emit_by_name(panel, "display-docpage", uri);
			g_free(scheme);
			return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
		}

		GFile *real_file = find_real_filename_for_inform_protocol(uri);
		html_load_file(webview, real_file);
		g_object_unref(real_file);

	} else if(strcmp(scheme, "http") == 0 || strcmp(scheme, "mailto") == 0) {
		/* Allow the Public Library website, but nothing else */
		if(g_str_has_prefix(uri, PUBLIC_LIBRARY_HOME_URI))
			return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;

		show_uri_in_browser(uri,
			GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(panel))), NULL);

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
			GFile *real_file = get_case_insensitive_extension(file);
			g_object_unref(file);
			/* Check if we need to open the extension read-only */
			GFile *user_file = i7_app_get_extension_file(i7_app_get(), NULL, NULL);
			gboolean readonly = !g_file_has_prefix(real_file, user_file);
			g_object_unref(user_file);

			I7Extension *ext = i7_extension_new_from_file(i7_app_get(), real_file, readonly);
			if(ext != NULL) {
				if(sscanf(anchor, "#line%u", &line))
					i7_source_view_jump_to_line(ext->sourceview, line);
			}
		}
		g_free(path);
		g_free(anchor);

	} else if(strcmp(scheme, "library") == 0) {
		char *id, *author, *title;
		GFile *remote_file = library_uri_to_real_uri(uri, &author, &title, &id);

		I7Document *doc = I7_DOCUMENT(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
		gboolean success = i7_document_download_single_extension(doc, remote_file, author, title);

		g_object_unref(remote_file);
		g_free(author);
		g_free(title);

		if(success) {
			/* Notify the Public Library that the download was OK */
			char *script = g_strconcat("downloadSucceeded(", id, ");", NULL);
			webkit_web_view_execute_script(webview, script);
			g_free(script);
		}
		g_free(id);

	} else {
		g_warning("Unrecognized URI scheme: %s", scheme);
	}

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

/* Signal handler to pick up navigations within the Extensions pane and push
them into the history queue. */
int
after_extensions_navigation_requested(WebKitWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, I7Panel *panel)
{
	/* Only add it to the history if it was navigated from the current page */
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(panel->notebook)) == I7_PANE_EXTENSIONS)
		history_push_extensions_page(panel, webkit_network_request_get_uri(request));
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
	g_idle_add((GSourceFunc)update_font_tabs, GTK_SOURCE_VIEW(self->results_tabs[I7_RESULTS_TAB_INFORM6]));

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
	chimara_glk_set_css_from_string(CHIMARA_GLK(self->tabs[I7_PANE_STORY]), css);
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
