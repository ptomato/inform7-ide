/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libxml/HTMLparser.h>
#include <webkit2/webkit2.h>

#include "app.h"
#include "document.h"
#include "error.h"
#include "file.h"
#include "extension.h"
#include "searchwindow.h"
#include "story.h"

/* An index of the text of the documentation and example pages. Only built
the first time someone does a documentation search, and freed at the end of the
main program. */
static GList *doc_index = NULL;

typedef struct {
	gboolean is_example;
	gboolean is_recipebook;
	gchar *section;
	gchar *title;
	gchar *sort;
	gchar *body;
	GFile *file;
	char *anchor;

	char *example_title;
} DocText;

/* Columns for the search results tree view */
typedef enum {
	I7_RESULT_CONTEXT_COLUMN,
	I7_RESULT_FILE_COLUMN,
	I7_RESULT_ANCHOR_COLUMN,
	I7_RESULT_RESULT_TYPE_COLUMN,
	I7_RESULT_LINE_NUMBER_COLUMN,
	I7_RESULT_SORT_STRING_COLUMN,
	I7_RESULT_LOCATION_COLUMN,
	I7_RESULT_EXAMPLE_TITLE_COLUMN,
	I7_RESULT_BACKGROUND_COLOR_COLUMN
} I7ResultColumns;

typedef enum {
	I7_RESULT_TYPE_PROJECT,
	I7_RESULT_TYPE_EXTENSION,
	I7_RESULT_TYPE_DOCUMENTATION,
	I7_RESULT_TYPE_RECIPE_BOOK
} I7ResultType;

typedef struct {
	GString *chars;
	int ignore;
	gboolean in_ignore_section;

	/* Metadata */
	DocText *doctext;

	/* Temporary storage for subsections */
	GString *outer_chars;
	DocText *outer_doctext;
	GSList *completed_doctexts;
} Ctxt;

struct _I7SearchWindow {
	GtkDialog parent;

	/* template children */
	GtkTreeViewColumn *document_column;
	GtkCellRendererText *document_renderer;
	GtkEntry *entry;
	GtkButton *find;
	GtkCheckButton *ignore_case;
	GtkTreeViewColumn *result_column;
	GtkCellRendererText *result_renderer;
	GtkListStore *results;
	GtkLabel *results_label;
	GtkRevealer *results_revealer;
	GtkSpinner *spinner;
	GtkComboBoxText *search_type;
	GtkCheckButton *target_documentation;
	GtkCheckButton *target_extensions;
	GtkCheckButton *target_project;
	GtkTreeViewColumn *type_column;
	GtkCellRendererText *type_renderer;

	/* private */
	I7Document *document; /* Associated document window */
	bool search_in_progress : 1;  /* Some things not possible while in progress */
};

G_DEFINE_TYPE(I7SearchWindow, i7_search_window, GTK_TYPE_DIALOG);

/* CALLBACKS */

typedef struct {
	GtkWindow *win;
	GFile *temp_file;
} CopyResourceClosure;

/* Rarely-used callback for showing documentation page in browser after copying
 * the GResource to a temporary file where the browser can see it. This happpens
 * when searching an extension project (no facing pages).
 * Note: this won't work under Flatpak, but it's a rare case and will go away
 * when we have the new Extension projects with facing pages */
void
finish_copy_resource_to_temp(GFile *resource, GAsyncResult *res, CopyResourceClosure *data)
{
	g_autoptr(GtkWindow) win = data->win;
	g_autoptr(GFile) temp_file = data->temp_file;
	g_clear_pointer(&data, g_free);

	GError *error = NULL;
	if (!g_file_copy_finish(resource, res, &error)) {
		g_warning("Couldn't open temporary file for browser: %s", error->message);
		return;
	}
	show_file_in_browser(temp_file, win);
}

static void
on_entry_changed(GtkEditable *editable, I7SearchWindow *self)
{
	const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
	bool text_not_empty = !(text == NULL || strlen(text) == 0);
	gtk_widget_set_sensitive(GTK_WIDGET(self->find), text_not_empty);
}

/* Callback for double-clicking on one of the search results */
static void
on_results_view_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, I7SearchWindow *self)
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	g_return_if_fail(model);

	if(!(gtk_tree_model_get_iter(model, &iter, path)))
		return;
	GFile *file;
	int result_type, lineno;
	char *anchor;
	gtk_tree_model_get(model, &iter,
		I7_RESULT_FILE_COLUMN, &file,
		I7_RESULT_ANCHOR_COLUMN, &anchor,
		I7_RESULT_RESULT_TYPE_COLUMN, &result_type,
		I7_RESULT_LINE_NUMBER_COLUMN, &lineno,
		-1);

	switch(result_type) {
	case I7_RESULT_TYPE_DOCUMENTATION:
	case I7_RESULT_TYPE_RECIPE_BOOK:
	{
		/* Display the documentation page in the appropriate widget if this
		 is a story with facing pages. Otherwise, open the documentation in
		 the web browser. */
		if (I7_IS_STORY(self->document)) {
			g_autofree char *page = g_file_get_basename(file);
			g_autofree char *uri = g_strconcat("inform:///", page, NULL);
			g_autoptr(GFile) inform_file = g_file_new_for_uri(uri);
			/* Jump to the proper example */
			if(anchor != NULL) {
				i7_story_show_docpage_at_anchor(I7_STORY(self->document), inform_file, anchor);
				g_free(anchor);
			} else {
				i7_story_show_docpage(I7_STORY(self->document), inform_file);
			}
		} else {
			/* Copy the GResource to a temporary file so it's available to the
			 * browser */
			g_autoptr(GError) error = NULL;
			g_autoptr(GFileIOStream) stream = NULL;
			g_autoptr(GFile) temp_file = g_file_new_tmp("inform-documentation-XXXXXX", &stream, &error);
			if (temp_file == NULL) {
				g_warning("Couldn't open temporary file for browser: %s", error->message);
				break;
			}

			CopyResourceClosure *data = g_new(CopyResourceClosure, 1);
			data->temp_file = g_steal_pointer(&temp_file);
			data->win = GTK_WINDOW(g_object_ref(self));

			g_file_copy_async(file, data->temp_file, G_FILE_COPY_OVERWRITE, G_PRIORITY_DEFAULT,
				/* cancellable = */ NULL, /* progress_callback = */ NULL, /* data = */ NULL,
				(GAsyncReadyCallback)finish_copy_resource_to_temp, data);
		}
	}
		break;
	case I7_RESULT_TYPE_PROJECT:
		i7_document_jump_to_line(self->document, lineno);
		break;
	case I7_RESULT_TYPE_EXTENSION:
	{
		I7App *theapp = I7_APP(g_application_get_default());
		I7Document *ext = i7_app_get_already_open(theapp, file);
		if(ext == NULL) {
			ext = I7_DOCUMENT(i7_extension_new_from_file(theapp, file, FALSE));
		}
		i7_document_jump_to_line(ext, lineno);
	}
		break;
	default:
		g_assert_not_reached();
	}
	g_object_unref(file);
}

static gboolean
on_search_window_delete_event(I7SearchWindow *self, GdkEvent *event)
{
	if (self->search_in_progress)
		return GDK_EVENT_STOP;
	return GDK_EVENT_PROPAGATE;
}

/* This would be better done with two GtkCellRendererText-s in a GtkCellArea,
but that requires GTK 3. */
static void
result_data_func(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, I7SearchWindow *self)
{
	I7ResultType type;
	char *markup = NULL, *context, *example_title;

	gtk_tree_model_get(model, iter,
		I7_RESULT_RESULT_TYPE_COLUMN, &type,
		I7_RESULT_CONTEXT_COLUMN, &context,
		I7_RESULT_EXAMPLE_TITLE_COLUMN, &example_title,
		-1);

	switch(type) {
		case I7_RESULT_TYPE_DOCUMENTATION:
		case I7_RESULT_TYPE_RECIPE_BOOK:
			if(example_title != NULL) {
				/* TRANSLATORS: This string is from the search results window;
				it looks like "(Example 347: The Eye of the Idol) The Hexagonal
				Temple..." where the first string is the number and title of the
				example, and the second is the context containing the string the
				user searched for. */
				markup = g_strdup_printf(_("<small><i>(Example %s)</i></small> %s"), example_title, context);
				break;
			}
			/* else fall through */
		default:
			markup = g_strdup(context);
	}

	g_object_set(cell, "markup", markup, NULL);
	g_free(markup);
	g_free(context);
	g_free(example_title);
}

static void
location_data_func(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, I7SearchWindow *self)
{
	I7ResultType type;
	guint lineno;
	GFile *file;
	char *text = NULL, *filename, *location;

	gtk_tree_model_get(model, iter,
		I7_RESULT_RESULT_TYPE_COLUMN, &type,
		I7_RESULT_LINE_NUMBER_COLUMN, &lineno,
		I7_RESULT_FILE_COLUMN, &file,
		I7_RESULT_LOCATION_COLUMN, &location,
		-1);

	switch(type) {
		case I7_RESULT_TYPE_PROJECT:
			if (I7_IS_STORY(self->document))
				text = g_strdup_printf(_("Story, line %d"), lineno);
			else {
				g_autofree char *displayname = i7_document_get_display_name(self->document);
				if(displayname == NULL) {
					text = g_strdup_printf(_("Untitled story, line %d"), lineno);
				} else {
					text = g_strdup_printf(_("%s, line %d"), displayname, lineno);
				}
			}
			break;
		case I7_RESULT_TYPE_EXTENSION:
		{
			/* Get the file's display name */
			filename = file_get_display_name(file);

			text = g_strdup_printf(
				  /* TRANSLATORS: EXTENSION_NAME, line NUMBER */
				  _("%s, line %d"), filename, lineno);
			g_free(filename);
		}
			break;
		case I7_RESULT_TYPE_DOCUMENTATION:
		case I7_RESULT_TYPE_RECIPE_BOOK:
			text = g_strdup(location);
		default:
			;
	}

	g_object_set(cell, "text", text, NULL);
	g_object_unref(file);
	g_free(text);
}

static void
type_data_func(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter)
{
	I7ResultType type;
	gtk_tree_model_get(model, iter, I7_RESULT_RESULT_TYPE_COLUMN, &type, -1);
	switch(type) {
		case I7_RESULT_TYPE_DOCUMENTATION:
			g_object_set(cell, "text", _("Writing with Inform"), NULL);
			break;
		case I7_RESULT_TYPE_EXTENSION:
			g_object_set(cell, "text", _("Extension"), NULL);
			break;
		case I7_RESULT_TYPE_PROJECT:
			g_object_set(cell, "text", _("Source"), NULL);
			break;
		case I7_RESULT_TYPE_RECIPE_BOOK:
			g_object_set(cell, "text", _("The Inform Recipe Book"), NULL);
			break;
		default:
			g_object_set(cell, "text", _("unknown"), NULL);
	}
}

/* TYPE SYSTEM */

static void
i7_search_window_init(I7SearchWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	/* Build the rest of the interface */
	gtk_tree_view_column_set_cell_data_func(self->result_column, GTK_CELL_RENDERER(self->result_renderer),
		(GtkTreeCellDataFunc)result_data_func, self, NULL);
	gtk_tree_view_column_set_cell_data_func(self->document_column, GTK_CELL_RENDERER(self->document_renderer),
		(GtkTreeCellDataFunc)location_data_func, self, NULL);
	gtk_tree_view_column_set_cell_data_func(self->type_column, GTK_CELL_RENDERER(self->type_renderer),
		(GtkTreeCellDataFunc)type_data_func, NULL, NULL);
}

static void
i7_search_window_class_init(I7SearchWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	gtk_widget_class_set_template_from_resource(widget_class, "/com/inform7/IDE/ui/searchwindow.ui");
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, document_column);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, document_renderer);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, entry);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, find);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, ignore_case);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, result_column);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, result_renderer);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, results);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, results_label);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, results_revealer);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, search_type);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, spinner);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, target_documentation);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, target_extensions);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, target_project);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, type_column);
	gtk_widget_class_bind_template_child(widget_class, I7SearchWindow, type_renderer);
	gtk_widget_class_bind_template_callback(widget_class, gtk_widget_hide);
	gtk_widget_class_bind_template_callback(widget_class, gtk_widget_hide_on_delete);
	gtk_widget_class_bind_template_callback(widget_class, on_entry_changed);
	gtk_widget_class_bind_template_callback(widget_class, on_results_view_row_activated);
	gtk_widget_class_bind_template_callback(widget_class, on_search_window_delete_event);
}

/* INTERNAL FUNCTIONS */

static void
update_label(I7SearchWindow *self)
{
	const char *text = gtk_entry_get_text(self->entry);
	g_autofree char *label = g_strdup_printf(_("Search results for: \"%s\""), text);
	gtk_label_set_text(self->results_label, label);
}

/* Expand only the standard entities (gt, lt, amp, apos, quot) */
static xmlEntityPtr
entity_callback(Ctxt *ctxt, const xmlChar *name)
{
	return xmlGetPredefinedEntity(name);
}

static gboolean
is_ignore_element(const xmlChar *name)
{
	return xmlStrcasecmp(name, (xmlChar *)"style") == 0
		|| xmlStrcasecmp(name, (xmlChar *)"script") == 0;
}

static gboolean
is_newline_element(const xmlChar *name)
{
	return xmlStrcasecmp(name, (xmlChar *)"br") == 0
		|| xmlStrcasecmp(name, (xmlChar *)"p") == 0;
}

static void
start_element_callback(Ctxt *ctxt, const xmlChar *name, const xmlChar **atts)
{
	if(is_ignore_element(name))
		ctxt->ignore++;
	else if(is_newline_element(name) && ctxt->ignore == 0 && !ctxt->in_ignore_section)
		g_string_append_c(ctxt->chars, ' '); /* Add spaces instead of newlines */
}

static void
end_element_callback(Ctxt *ctxt, const xmlChar *name)
{
	if(is_ignore_element(name))
		ctxt->ignore--;
}

static void
character_callback(Ctxt *ctxt, const xmlChar *ch, int len)
{
	if(ctxt->ignore == 0 && !ctxt->in_ignore_section)
		g_string_append_len(ctxt->chars, (gchar *)ch, len);
}

static void
comment_callback(Ctxt *ctxt, const xmlChar *value)
{
	/* Extract metadata from comments. Use sscanf() to be lenient about space */
	if (sscanf((char *) value, " SEARCH TITLE \"%m[^\"]\"", &ctxt->doctext->title) == 1)
		return;
	if (sscanf((char *) value, " SEARCH SECTION \"%m[^\"]\"", &ctxt->doctext->section) == 1)
		return;
	if (sscanf((char *) value, " SEARCH SORT \"%m[^\"]\"", &ctxt->doctext->sort) == 1)
		return;

	/* From here on, these are particular subsections of the documentation page,
	such as examples. We assume that the above metadata always appear before a
	subsection can appear, and that subsections cannot be nested. */

	if (sscanf((char *) value, " START EXAMPLE \"%m[^\"]\" \"%m[^\"]\"", &ctxt->doctext->example_title, &ctxt->doctext->anchor) == 2) {
		ctxt->outer_chars = ctxt->chars;
		ctxt->chars = g_string_new("");

		ctxt->outer_doctext = ctxt->doctext;
		ctxt->doctext = g_slice_dup(DocText, ctxt->outer_doctext);
		ctxt->doctext->is_example = TRUE;
		g_object_ref(ctxt->doctext->file);
		ctxt->doctext->section = g_strdup(ctxt->outer_doctext->section);
		ctxt->doctext->title = g_strdup(ctxt->outer_doctext->title);
		ctxt->doctext->sort = g_strdup(ctxt->outer_doctext->sort);
		return;
	}

	char section_type[8];  /* len(EXAMPLE) + 1 */
	if (sscanf((char *) value, " START %7s", section_type) == 1) {
		if (strcmp(section_type, "IGNORE") == 0) {
			ctxt->in_ignore_section = true;
			return;
		}

		if (strcmp(section_type, "CODE") == 0 || strcmp(section_type, "PHRASE") == 0) {
			/* Ignore for now */
			return;
		}

		g_warning("Unhandled START %s section in doc comments", section_type);
	}

	if (sscanf((char *) value, " END %7s", section_type) == 1) {
		if (strcmp(section_type, "EXAMPLE") == 0) {
			ctxt->doctext->body = g_string_free(ctxt->chars, false);
			ctxt->completed_doctexts = g_slist_prepend(ctxt->completed_doctexts, ctxt->doctext);

			ctxt->doctext = ctxt->outer_doctext;
			ctxt->chars = ctxt->outer_chars;
			return;
		}

		if (strcmp(section_type, "IGNORE") == 0) {
			ctxt->in_ignore_section = false;
			return;
		}

		if (strcmp(section_type, "CODE") == 0 || strcmp(section_type, "PHRASE") == 0)
			return;

		g_warning("Unhandled END %s section in doc comments", section_type);
	}
}

xmlSAXHandler i7_html_sax = {
	.getEntity = (getEntitySAXFunc)entity_callback,
	.startElement = (startElementSAXFunc)start_element_callback,
	.endElement = (endElementSAXFunc)end_element_callback,
	.characters = (charactersSAXFunc)character_callback,
	.comment = (commentSAXFunc)comment_callback,
};

static GSList *
html_to_ascii(GFile *file, gboolean is_recipebook)
{
	Ctxt *ctxt = g_slice_new0(Ctxt);
	ctxt->chars = g_string_new("");
	DocText *doctext = g_slice_new0(DocText);
	doctext->is_recipebook = is_recipebook;
	doctext->file = g_object_ref(file);
	ctxt->doctext = doctext;

	g_autoptr(GError) error = NULL;
	g_autofree char *html_contents = NULL;
	/* file is in a GResource in memory, this doesn't block */
	if (!g_file_load_contents(doctext->file, /* cancellable = */ NULL,
		&html_contents, /* length = */ NULL, /* etag = */ NULL, &error)) {
		g_autofree char *uri = g_file_get_uri(doctext->file);
		g_warning("Error loading documentation resource '%s': %s", uri, error->message);
		return NULL;
	}

	// COMPAT: 2.11 - use htmlNewSAXParserCtxt()
	htmlParserCtxtPtr html_parser = htmlNewParserCtxt();
	html_parser->sax = &i7_html_sax;
	html_parser->userData = ctxt;
	xmlFreeDoc(htmlCtxtReadDoc(html_parser, (xmlChar *) html_contents,
		/* URL = */ NULL, /* encoding = */ NULL, HTML_PARSE_NONET));

	doctext->body = g_string_free(ctxt->chars, FALSE);
	GSList *retval = g_slist_prepend(ctxt->completed_doctexts, ctxt->doctext);
	g_slice_free(Ctxt, ctxt);
	return retval;
}

/* Borrow from searchbar.c */
extern gboolean find_no_wrap(const GtkTextIter *, const char *, gboolean, GtkTextSearchFlags, I7SearchFlags, GtkTextIter *, GtkTextIter *);

/* Helper function: extract some characters of context around the match, with
 the match itself highlighted in bold. String must be freed. */
static gchar *
extract_context(GtkTextBuffer *buffer, GtkTextIter *match_start, GtkTextIter *match_end)
{
	GtkTextIter context_start = *match_start, context_end = *match_end;

	/* Create a larger range to extract the context */
	gtk_text_iter_backward_chars(&context_start, 8);
	gtk_text_iter_forward_chars(&context_end, 32);

	/* Get the surrounding text as context */
	gchar *before = gtk_text_buffer_get_text(buffer, &context_start, match_start, TRUE);
	gchar *term = gtk_text_buffer_get_text(buffer, match_start, match_end, TRUE);
	gchar *after = gtk_text_buffer_get_text(buffer, match_end, &context_end, TRUE);
	gchar *context = g_strconcat(before, "<b>", term, "</b>", after, NULL);
	g_strdelimit(context, "\n\r\t", ' ');
	g_free(before);
	g_free(term);
	g_free(after);

	return context;
}

/* Helper function: search one documentation page */
static void
search_documentation(DocText *doctext, I7SearchWindow *self)
{
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;
	g_autoptr(GtkTextBuffer) buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, doctext->body, -1);
	gtk_text_buffer_get_start_iter(buffer, &search_from);
	bool ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	I7SearchFlags algorithm = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	const char *text = gtk_entry_get_text(GTK_ENTRY(self->entry));

	while (find_no_wrap(&search_from, text, TRUE,
		GTK_TEXT_SEARCH_TEXT_ONLY | (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0),
		algorithm, &match_start, &match_end))
	{
		while(gtk_events_pending())
			gtk_main_iteration();

		search_from = match_end;

		gchar *context = extract_context(buffer, &match_start, &match_end);
		gchar *location = g_strconcat(doctext->section, ": ", doctext->title, NULL);

		gtk_list_store_append(self->results, &result);
		gtk_list_store_set(self->results, &result,
			I7_RESULT_CONTEXT_COLUMN, context,
			I7_RESULT_SORT_STRING_COLUMN, doctext->sort,
			I7_RESULT_FILE_COLUMN, doctext->file,
			I7_RESULT_ANCHOR_COLUMN, doctext->anchor,
			I7_RESULT_RESULT_TYPE_COLUMN, doctext->is_recipebook?
				I7_RESULT_TYPE_RECIPE_BOOK : I7_RESULT_TYPE_DOCUMENTATION,
			I7_RESULT_LOCATION_COLUMN, location,
			I7_RESULT_EXAMPLE_TITLE_COLUMN, doctext->example_title,
			I7_RESULT_BACKGROUND_COLOR_COLUMN, doctext->is_recipebook?
				"#ffffe0" : "#ffffff",
			-1);
		g_free(context);
		g_free(location);
	}
}

/* Helper functions: start and stop the spinner, and keep it hidden when it is
 not going */
static void
start_spinner(I7SearchWindow *self)
{
	gtk_spinner_start(self->spinner);
	gtk_widget_show(GTK_WIDGET(self->spinner));
}

static void
stop_spinner(I7SearchWindow *self)
{
	gtk_spinner_stop(self->spinner);
	gtk_widget_hide(GTK_WIDGET(self->spinner));
}

/* Search the documentation pages for the string 'text', building the index
  if necessary */
static void
i7_search_window_search_documentation(I7SearchWindow *self)
{
	GError *err;

	if(doc_index == NULL) { /* documentation index hasn't been built yet */
		g_autoptr(GFile) doc_file = g_file_new_for_uri("resource:///com/inform7/IDE/inform");

		GFileEnumerator *docdir;
		if((docdir = g_file_enumerate_children(doc_file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &err)) == NULL) {
			IO_ERROR_DIALOG(GTK_WINDOW(self), doc_file, err, _("opening documentation directory"));
			g_object_unref(doc_file);
			return;
		}

		start_spinner(self);

		GFileInfo *info;
		while((info = g_file_enumerator_next_file(docdir, NULL, &err)) != NULL) {
			const char *basename = g_file_info_get_name(info);
			const char *displayname = g_file_info_get_display_name(info);

			if(!g_str_has_suffix(basename, ".html") ||
			   (!g_str_has_prefix(basename, "doc") && !g_str_has_prefix(basename, "Rdoc")))
				continue;

			char *label = g_strdup_printf(_("Please be patient, indexing %s…"), displayname);
			gtk_label_set_text(self->results_label, label);
			g_free(label);

			while(gtk_events_pending())
				gtk_main_iteration();

			GFile *file = g_file_get_child(doc_file, basename);
			GSList *doctexts = html_to_ascii(file, g_str_has_prefix(basename, "R"));
			g_object_unref(file);
			if(doctexts != NULL) {
				GSList *iter;
				/* Append the entries to the documentation index and search them
				right now while we're at it */
				for(iter = doctexts; iter != NULL; iter = g_slist_next(iter)) {
					doc_index = g_list_prepend(doc_index, iter->data);
					search_documentation(iter->data, self);
				}
				g_slist_free(doctexts);
			}
		}

		stop_spinner(self);
		update_label(self);
	} else {
		start_spinner(self);
		g_list_foreach(doc_index, (GFunc)search_documentation, self);
		stop_spinner(self);
	}
	return;
}

/* Search the project file for the string 'text' */
static void
i7_search_window_search_project(I7SearchWindow *self)
{
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(self->document));
	gtk_text_buffer_get_start_iter(buffer, &search_from);
	bool ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	I7SearchFlags algorithm = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	const char *text = gtk_entry_get_text(self->entry);

	start_spinner(self);

	while (find_no_wrap(&search_from, text, TRUE,
		GTK_TEXT_SEARCH_TEXT_ONLY | (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0),
		algorithm, &match_start, &match_end))
	{
		while(gtk_events_pending())
			gtk_main_iteration();

		search_from = match_end;

		/* Get the line number (counted from 0) */
		guint lineno = gtk_text_iter_get_line(&match_start) + 1;

		gchar *context = extract_context(buffer, &match_start, &match_end);

		/* Make a sort string */
		gchar *sort = g_strdup_printf("%04i", lineno);
		/* Put the full path to the project in */
		GFile *file = i7_document_get_file(self->document);

		gtk_list_store_append(self->results, &result);
		gtk_list_store_set(self->results, &result,
			I7_RESULT_CONTEXT_COLUMN, context,
			I7_RESULT_SORT_STRING_COLUMN, sort,
			I7_RESULT_FILE_COLUMN, file,
			I7_RESULT_RESULT_TYPE_COLUMN, I7_RESULT_TYPE_PROJECT,
			I7_RESULT_LINE_NUMBER_COLUMN, lineno,
			-1);
		g_free(context);
		g_free(sort);
		g_object_unref(file);
	}

	stop_spinner(self);
}

static void
extension_search_result(I7SearchWindow *self, GFile *file, const char *author_display_name, const char *ext_display_name)
{
	GError *err = NULL;
	char *contents;
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;

	if(!g_file_load_contents(file, NULL, &contents, NULL, NULL, &err)) {
		error_dialog_file_operation(GTK_WINDOW(self), file, err, I7_FILE_ERROR_OTHER,
		  /* TRANSLATORS: Error opening EXTENSION_NAME by AUTHOR_NAME */
		  _("Error opening extension '%s' by '%s':"), author_display_name, ext_display_name);
		return;
	}

	g_autofree char *basename = g_file_get_basename(file);

	g_autoptr(GtkTextBuffer) buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, contents, -1);
	g_free(contents);

	gtk_text_buffer_get_start_iter(buffer, &search_from);

	start_spinner(self);

	bool ignore_case = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->ignore_case));
	I7SearchFlags algorithm = gtk_combo_box_get_active(GTK_COMBO_BOX(self->search_type));
	const char *text = gtk_entry_get_text(self->entry);

	while (find_no_wrap(&search_from, text, TRUE,
		GTK_TEXT_SEARCH_TEXT_ONLY | (ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0),
		algorithm, &match_start, &match_end))
	{
		unsigned lineno;
		char *sort, *context;

		while(gtk_events_pending())
			gtk_main_iteration();

		search_from = match_end;

		/* Get the line number (counted from 0) */
		lineno = gtk_text_iter_get_line(&match_start) + 1;

		context = extract_context(buffer, &match_start, &match_end);

		/* Make a sort string */
		sort = g_strdup_printf("%s %04i", basename, lineno);

		gtk_list_store_append(self->results, &result);
		gtk_list_store_set(self->results, &result,
			I7_RESULT_CONTEXT_COLUMN, context,
			I7_RESULT_SORT_STRING_COLUMN, sort,
			I7_RESULT_FILE_COLUMN, file,
			I7_RESULT_RESULT_TYPE_COLUMN, I7_RESULT_TYPE_EXTENSION,
			I7_RESULT_LINE_NUMBER_COLUMN, lineno,
			-1);

		g_free(context);
		g_free(sort);
	}

	stop_spinner(self);
}

/**
 * i7_search_window_search_extensions:
 * @self: self
 *
 * Search the user-installed extensions for the search window's search text.
 */
static void
i7_search_window_search_extensions(I7SearchWindow *self)
{
	I7App *theapp = I7_APP(g_application_get_default());
	GtkTreeModel *model = GTK_TREE_MODEL(i7_app_get_installed_extensions_tree(theapp));

	GtkTreeIter author, title;

	if (!gtk_tree_model_get_iter_first(model, &author))
		return;

	do {
		g_autofree char *author_name = NULL;
		gtk_tree_model_get(model, &author, I7_APP_EXTENSION_TEXT, &author_name, -1);

		if (!gtk_tree_model_iter_children(model, &title, &author))
			continue;

		do {
			g_autofree char *ext_name = NULL;
			g_autoptr(GFile) extension_file = NULL;
			gtk_tree_model_get(model, &title,
				I7_APP_EXTENSION_TEXT, &ext_name,
				I7_APP_EXTENSION_FILE, &extension_file,
				-1);

			extension_search_result(self, extension_file, author_name, ext_name);
		} while (gtk_tree_model_iter_next(model, &title));
	} while (gtk_tree_model_iter_next(model, &author));
}

static void
start_searching(I7SearchWindow *self)
{
	self->search_in_progress = true;
}

/* Notify the window that no more searches will be done, so it is allowed to
 close itself if asked to */
static void
done_searching(I7SearchWindow *self)
{
	self->search_in_progress = false;
}

/* PUBLIC FUNCTIONS */

/* Create a new search results window */
I7SearchWindow *
i7_search_window_new(I7Document *document)
{
	I7SearchWindow *self = I7_SEARCH_WINDOW(g_object_new(I7_TYPE_SEARCH_WINDOW, NULL));

	self->document = document;

	/* Keep on top of the document window and close when document is closed */
	gtk_window_set_transient_for(GTK_WINDOW(self), GTK_WINDOW(document));

	return self;
}

void
i7_search_window_prefill_ui(I7SearchWindow *self, const char *text, I7SearchTarget target, I7SearchFlags flags)
{
	gtk_entry_set_text(self->entry, text);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->target_project), !!(target & I7_SEARCH_TARGET_SOURCE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->target_extensions), !!(target & I7_SEARCH_TARGET_EXTENSIONS));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->target_documentation), !!(target & I7_SEARCH_TARGET_DOCUMENTATION));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->ignore_case), !!(flags & I7_SEARCH_IGNORE_CASE));
	gtk_combo_box_set_active(GTK_COMBO_BOX(self->search_type), flags & I7_SEARCH_ALGORITHM_MASK);
}

void
i7_search_window_do_search(I7SearchWindow *self)
{
	/* Show the results widget */
	gtk_revealer_set_reveal_child(self->results_revealer, TRUE);

	start_searching(self);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->target_project)))
		i7_search_window_search_project(self);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->target_extensions)))
		i7_search_window_search_extensions(self);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->target_documentation)))
		i7_search_window_search_documentation(self);

	done_searching(self);
}

/**
 * i7_search_window_free_index:
 *
 * Free the documentation index, if one has been created. Should be called at
 * the end of the main program.
 */
void
i7_search_window_free_index(void)
{
	if(doc_index == NULL)
		return;

	GList *iter;
	for(iter = doc_index; iter != NULL; iter = g_list_next(iter)) {
		DocText *text = (DocText *)(iter->data);
		g_free(text->section);
		g_free(text->title);
		g_free(text->sort);
		g_free(text->body);
		g_object_unref(text->file);
		g_free(text->anchor);
		g_free(text->example_title);
		g_slice_free(DocText, text);
	}
	g_list_free(doc_index);
	doc_index = NULL;
}
