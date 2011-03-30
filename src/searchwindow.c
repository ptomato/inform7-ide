/* Copyright (C) 2006-2009, 2010, 2011 P. F. Chimento
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

#include <stdarg.h>
#include <libxml/HTMLparser.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceiter.h>
#include <webkit/webkit.h>
#include "searchwindow.h"
#include "app.h"
#include "builder.h"
#include "document.h"
#include "error.h"
#include "extension.h"
#include "story.h"

/* These functions are stubs if the GTK version is less than 2.20. See the end
 of the file. SUCKY DEBIAN */
static GtkWidget *pack_spinner_in_box(GtkWidget *box);
static void start_spinner(I7SearchWindow *self);
static void stop_spinner(I7SearchWindow *self);

/* An index of the text of the documentation and example pages. Only built
the first time someone does a documentation search, and freed atexit. */
static GList *doc_index = NULL;

typedef struct {
	gboolean is_example;
	gboolean is_recipebook;
	gchar *section;
	gchar *title;
	gchar *sort;
	gchar *body;
	gchar *file;
} DocText;


/* Columns for the search results tree view */
typedef enum {
	I7_RESULT_CONTEXT_COLUMN,
	I7_RESULT_FILE_COLUMN,
	I7_RESULT_RESULT_TYPE_COLUMN,
	I7_RESULT_LINE_NUMBER_COLUMN,
	I7_RESULT_SORT_STRING_COLUMN,
	I7_RESULT_LOCATION_COLUMN
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
	gboolean in_example;
	/* Metadata */
	DocText *doctext;
} Ctxt;

typedef struct _I7SearchWindowPrivate I7SearchWindowPrivate;
struct _I7SearchWindowPrivate
{
	GtkListStore *results;
	I7Document *document; /* Associated document window */
	gchar *text; /* Search string */
	gboolean ignore_case;
	I7SearchType algorithm;
};

#define I7_SEARCH_WINDOW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_SEARCH_WINDOW, I7SearchWindowPrivate))
#define I7_SEARCH_WINDOW_USE_PRIVATE(o,n) I7SearchWindowPrivate *n = I7_SEARCH_WINDOW_PRIVATE(o)

/* CALLBACKS */

/* Callback for double-clicking on one of the search results */
void
on_results_view_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, I7SearchWindow *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	g_return_if_fail(model);

	if(!(gtk_tree_model_get_iter(model, &iter, path)))
		return;
	gchar *filename;
	int result_type, lineno;
	gtk_tree_model_get(model, &iter,
		I7_RESULT_FILE_COLUMN, &filename,
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
		if(I7_IS_STORY(priv->document)) {
			GMatchInfo *match;
			I7App *theapp = i7_app_get();
			/* Jump to the proper example */
			if(g_regex_match(theapp->regices[I7_APP_REGEX_EXTENSION_FILE_NAME], filename, 0, &match)) {
				gchar *number = g_match_info_fetch_named(match, "number");
				gchar *anchor = g_strconcat("e", number, NULL);
				g_free(number);
				i7_story_show_docpage_at_anchor(I7_STORY(priv->document), filename, anchor);
				g_free(anchor);
			} else
				i7_story_show_docpage(I7_STORY(priv->document), filename);
			g_match_info_free(match);
		} else {
			GError *err = NULL;
			gchar *uri = g_filename_to_uri(filename, NULL, &err);
			if(!uri) {
				WARN_S(_("Could not convert filename to URI"), filename, err);
				break;
			}
			if(!gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &err))
				error_dialog(GTK_WINDOW(self), err, _("The page \"%s\" should have opened in your browser:"), uri);
		}
	}
		break;
	case I7_RESULT_TYPE_PROJECT:
		i7_document_jump_to_line(priv->document, lineno);
		break;
	case I7_RESULT_TYPE_EXTENSION:
	{
		I7Document *ext = i7_app_get_already_open(i7_app_get(), filename);
		if(ext == NULL)
			ext = I7_DOCUMENT(i7_extension_new_from_file(i7_app_get(), filename, FALSE));
		i7_document_jump_to_line(ext, lineno);
	}
		break;
	default:
		g_assert_not_reached();
	}
	g_free(filename);
}

static gboolean
on_search_window_delete_event(I7SearchWindow *self, GdkEvent *event)
{
	return TRUE; /* block deletion */
}

static void
location_data_func(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, I7SearchWindow *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);

	I7ResultType type;
	guint lineno;
	gchar *text = NULL, *path, *filename, *location;

	gtk_tree_model_get(model, iter,
		I7_RESULT_RESULT_TYPE_COLUMN, &type,
		I7_RESULT_LINE_NUMBER_COLUMN, &lineno,
		I7_RESULT_FILE_COLUMN, &path,
		I7_RESULT_LOCATION_COLUMN, &location,
		-1);

	switch(type) {
		case I7_RESULT_TYPE_PROJECT:
			if(I7_IS_STORY(priv->document))
				text = g_strdup_printf(_("Story, line %d"), lineno);
			else {
				gchar *displayname = i7_document_get_display_name(priv->document);
				text = g_strdup_printf(_("%s, line %d"), displayname, lineno);
				g_free(displayname);
			}
			break;
		case I7_RESULT_TYPE_EXTENSION:
			filename = g_filename_display_basename(path);
			text = g_strdup_printf(
				  /* TRANSLATORS: EXTENSION_NAME, line NUMBER */
				  _("%s, line %d"), filename, lineno);
			g_free(filename);
			break;
		case I7_RESULT_TYPE_DOCUMENTATION:
		case I7_RESULT_TYPE_RECIPE_BOOK:
			text = g_strdup(location);
		default:
			;
	}

	g_object_set(cell, "text", text, NULL);
	g_free(path);
	g_free(text);
}

static void
type_data_func(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter)
{
	I7ResultType type;
	gtk_tree_model_get(model, iter, I7_RESULT_RESULT_TYPE_COLUMN, &type, -1);
	switch(type) {
		case I7_RESULT_TYPE_DOCUMENTATION:
			g_object_set(cell, "text", _("Documentation"), NULL);
			break;
		case I7_RESULT_TYPE_EXTENSION:
			g_object_set(cell, "text", _("Extension"), NULL);
			break;
		case I7_RESULT_TYPE_PROJECT:
			g_object_set(cell, "text", _("Project File"), NULL);
			break;
		case I7_RESULT_TYPE_RECIPE_BOOK:
			g_object_set(cell, "text", _("Recipe Book"), NULL);
			break;
		default:
			g_object_set(cell, "text", _("unknown"), NULL);
	}
}

/* TYPE SYSTEM */

G_DEFINE_TYPE(I7SearchWindow, i7_search_window, GTK_TYPE_WINDOW);

static void
i7_search_window_init(I7SearchWindow *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);

	priv->document = NULL;
	priv->text = NULL;
	priv->ignore_case = FALSE;
	priv->algorithm = I7_SEARCH_CONTAINS;

	gtk_window_set_destroy_with_parent(GTK_WINDOW(self), TRUE);
	gtk_window_set_icon_name(GTK_WINDOW(self), "inform7");
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(self), TRUE);
	gtk_window_set_title(GTK_WINDOW(self), _("Search Results"));
	gtk_window_set_type_hint(GTK_WINDOW(self), GDK_WINDOW_TYPE_HINT_UTILITY);
	gtk_container_set_border_width(GTK_CONTAINER(self), 12);
	gtk_window_set_default_size(GTK_WINDOW(self), 400, 400);

	/* Build the interface from the builder file */
	gchar *filename = i7_app_get_datafile_path(i7_app_get(), "ui/searchwindow.ui");
	GtkBuilder *builder = create_new_builder(filename, self);
	g_free(filename);

	/* Build the rest of the interface */
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(load_object(builder, "search_window")));
	GtkWidget *box = GTK_WIDGET(load_object(builder, "search_text_box"));
	self->spinner = pack_spinner_in_box(box);
	priv->results = GTK_LIST_STORE(load_object(builder, "results"));
	gtk_tree_view_column_set_cell_data_func(GTK_TREE_VIEW_COLUMN(load_object(builder, "file_column")),
		GTK_CELL_RENDERER(load_object(builder, "file_renderer")),
		(GtkTreeCellDataFunc)location_data_func, self, NULL);
	gtk_tree_view_column_set_cell_data_func(GTK_TREE_VIEW_COLUMN(load_object(builder, "type_column")),
		GTK_CELL_RENDERER(load_object(builder, "type_renderer")),
		(GtkTreeCellDataFunc)type_data_func, NULL, NULL);
	g_signal_connect(self, "delete-event", G_CALLBACK(on_search_window_delete_event), NULL);

	/* Save public pointers to other widgets */
	LOAD_WIDGET(search_text);
	LOAD_WIDGET(results_view);

	/* Builder object not needed anymore */
	g_object_unref(builder);
}

static void
i7_search_window_finalize(GObject *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);
	g_free(priv->text);

	G_OBJECT_CLASS(i7_search_window_parent_class)->finalize(self);
}

static void
i7_search_window_class_init(I7SearchWindowClass *klass)
{
	g_type_class_add_private(klass, sizeof(I7SearchWindowPrivate));

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_search_window_finalize;
}

/* INTERNAL FUNCTIONS */

static void
update_label(I7SearchWindow *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);
	gchar *label = g_strdup_printf(_("Search results for: \"%s\""), priv->text);
	gtk_label_set_text(GTK_LABEL(self->search_text), label);
	g_free(label);
}

/* Free the internal private documentation index. No need to call this function;
it is connected to atexit. */
static void
free_doc_index()
{
	GList *iter;
	for(iter = doc_index; iter != NULL; iter = g_list_next(iter)) {
		DocText *text = (DocText *)(iter->data);
		g_free(text->section);
		g_free(text->title);
		g_free(text->sort);
		g_free(text->body);
		g_free(text->file);
		g_slice_free(DocText, text);
	}
	g_list_free(doc_index);
	doc_index = NULL;
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
	else if(is_newline_element(name) && ctxt->ignore == 0)
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
	if((!ctxt->doctext->is_example || ctxt->in_example) && ctxt->ignore == 0)
		g_string_append_len(ctxt->chars, (gchar *)ch, len);
}

static gchar *
get_quoted_contents(const xmlChar *string)
{
	const xmlChar *q1 = xmlStrchr(string, '"') + 1;
	const xmlChar *q2 = xmlStrchr(q1, '"');
	return (gchar *)xmlStrndup(q1, q2 - q1);
}

static void
comment_callback(Ctxt *ctxt, const xmlChar *value)
{
	/* Extract metadata from comments */
	if(g_str_has_prefix((gchar *)value, " SEARCH TITLE "))
		ctxt->doctext->title = get_quoted_contents(value + 14);
	else if(g_str_has_prefix((gchar *)value, " SEARCH SECTION ")) {
		gchar *meta = get_quoted_contents(value + 16);
		ctxt->doctext->section = ctxt->doctext->is_recipebook?
			g_strconcat(_("Recipe Book, "), meta, NULL) : g_strdup(meta);
		g_free(meta);
	} else if(g_str_has_prefix((gchar *)value, " SEARCH SORT "))
		ctxt->doctext->sort = get_quoted_contents(value + 13);
	else if(g_str_has_prefix((gchar *)value, " EXAMPLE START "))
		ctxt->in_example = TRUE;
	else if(g_str_has_prefix((gchar *)value, " EXAMPLE END "))
		ctxt->in_example = FALSE;
}

xmlSAXHandler i7_html_sax = {
	NULL, /*internalSubset*/
	NULL, /*isStandalone*/
	NULL, /*hasInternalSubset*/
	NULL, /*hasExternalSubset*/
	NULL, /*resolveEntity*/
	(getEntitySAXFunc)entity_callback, /*getEntity*/
	NULL, /*entityDecl*/
	NULL, /*notationDecl*/
	NULL, /*attributeDecl*/
	NULL, /*elementDecl*/
	NULL, /*unparsedEntityDecl*/
	NULL, /*setDocumentLocator*/
	NULL, /*startDocument*/
	NULL, /*endDocument*/
	(startElementSAXFunc)start_element_callback, /*startElement*/
	(endElementSAXFunc)end_element_callback, /*endElement*/
	NULL, /*reference*/
	(charactersSAXFunc)character_callback, /*characters*/
	NULL, /*ignorableWhitespace*/
	NULL, /*processingInstruction*/
	(commentSAXFunc)comment_callback, /*comment*/
	NULL, /*warning*/
	NULL, /*error*/
	NULL /*fatalError: unused*/
};

static DocText *
html_to_ascii(const gchar *filename, gboolean is_example, gboolean is_recipebook)
{
	Ctxt *ctxt = g_slice_new0(Ctxt);
	ctxt->chars = g_string_new("");
	DocText *doctext = g_slice_new0(DocText);
	doctext->is_example = is_example;
	doctext->is_recipebook = is_recipebook;
	ctxt->doctext = doctext;

	htmlSAXParseFile(filename, NULL, &i7_html_sax, ctxt);

	doctext->file = g_strdup(filename);
	doctext->body = g_string_free(ctxt->chars, FALSE);
	g_slice_free(Ctxt, ctxt);
	return doctext;
}

/* Borrow from document-search.c */
extern gboolean find_no_wrap(const GtkTextIter *, const gchar *, gboolean, GtkSourceSearchFlags, I7SearchType, GtkTextIter *, GtkTextIter *);

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
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(gtk_source_buffer_new(NULL));
	gtk_text_buffer_set_text(buffer, doctext->body, -1);
	gtk_text_buffer_get_start_iter(buffer, &search_from);

	while(find_no_wrap(&search_from, priv->text, TRUE,
		GTK_SOURCE_SEARCH_TEXT_ONLY | (priv->ignore_case? GTK_SOURCE_SEARCH_CASE_INSENSITIVE : 0),
		priv->algorithm, &match_start, &match_end))
	{
		while(gtk_events_pending())
			gtk_main_iteration();

		search_from = match_end;

		gchar *context = extract_context(buffer, &match_start, &match_end);
		gchar *location = g_strconcat(doctext->section, ": ", doctext->title, NULL);

		gtk_list_store_append(priv->results, &result);
		gtk_list_store_set(priv->results, &result,
			I7_RESULT_CONTEXT_COLUMN, context,
			I7_RESULT_SORT_STRING_COLUMN, doctext->sort,
			I7_RESULT_FILE_COLUMN, doctext->file,
			I7_RESULT_RESULT_TYPE_COLUMN, doctext->is_recipebook?
				I7_RESULT_TYPE_RECIPE_BOOK : I7_RESULT_TYPE_DOCUMENTATION,
			I7_RESULT_LOCATION_COLUMN, location,
			-1);
		g_free(context);
		g_free(location);
	}

	g_object_unref(buffer);
}

/* PUBLIC FUNCTIONS */

/* Create a new search results window */
GtkWidget *
i7_search_window_new(I7Document *document, const gchar *text, gboolean ignore_case, I7SearchType algorithm)
{
	I7SearchWindow *self = I7_SEARCH_WINDOW(g_object_new(I7_TYPE_SEARCH_WINDOW, NULL));
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);

	priv->document = document;
	priv->text = g_strdup(text);
	priv->ignore_case = ignore_case;
	priv->algorithm = algorithm;

	/* Keep on top of the document window and close when document is closed */
	gtk_window_set_transient_for(GTK_WINDOW(self), GTK_WINDOW(document));

	update_label(self);

	/* Bring window to front */
	gtk_widget_show_all(GTK_WIDGET(self));
	gtk_window_present(GTK_WINDOW(self));

	return GTK_WIDGET(self);
}

/* Search the documentation pages for the string 'text', building the index
  if necessary */
void
i7_search_window_search_documentation(I7SearchWindow *self)
{
	GError *err;

	if(doc_index == NULL) { /* documentation index hasn't been built yet */
		gboolean example = FALSE;
		gchar *docpath = i7_app_get_datafile_path_va(i7_app_get(), "Documentation", "Sections", NULL);

		GDir *docdir;
		if((docdir = g_dir_open(docpath, 0, &err)) == NULL) {
			error_dialog(GTK_WINDOW(self), err, _("Could not open documentation directory: "));
			g_free(docpath);
			return;
		}

		start_spinner(self);

		const gchar *filename;
		while((filename = g_dir_read_name(docdir)) != NULL) {
			if(!g_str_has_suffix(filename, ".html"))
				continue;

			if(g_str_has_prefix(filename, "doc") || g_str_has_prefix(filename, "Rdoc"))
				example = FALSE;
			else if(g_str_has_prefix(filename, "ex") || g_str_has_prefix(filename, "Rex"))
				example = TRUE;
			else
				continue;

			gchar *label = g_strdup_printf(_("Please be patient, indexing %s..."), filename);
			gtk_label_set_text(GTK_LABEL(self->search_text), label);
			g_free(label);

			while(gtk_events_pending())
				gtk_main_iteration();

			gchar *path = g_build_filename(docpath, filename, NULL);
			DocText *doctext = html_to_ascii(path, example, g_str_has_prefix(filename, "R"));
			g_free(path);
			if(doctext) {
				/* Append the entry to the documentation index and
				 search it right now while we're at it */
				doc_index = g_list_prepend(doc_index, doctext);
				search_documentation(doctext, self);
			}
		}
		g_free(docpath);

		stop_spinner(self);
		update_label(self);

		g_atexit(free_doc_index);
	} else {
		start_spinner(self);
		g_list_foreach(doc_index, (GFunc)search_documentation, self);
		stop_spinner(self);
	}
	return;
}

/* Search the project file for the string 'text' */
void
i7_search_window_search_project(I7SearchWindow *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(priv->document));
	gtk_text_buffer_get_start_iter(buffer, &search_from);

	start_spinner(self);

	while(find_no_wrap(&search_from, priv->text, TRUE,
		GTK_SOURCE_SEARCH_TEXT_ONLY | (priv->ignore_case? GTK_SOURCE_SEARCH_CASE_INSENSITIVE : 0),
		priv->algorithm, &match_start, &match_end))
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
		gchar *filename = i7_document_get_path(priv->document);

		gtk_list_store_append(priv->results, &result);
		gtk_list_store_set(priv->results, &result,
			I7_RESULT_CONTEXT_COLUMN, context,
			I7_RESULT_SORT_STRING_COLUMN, sort,
			I7_RESULT_FILE_COLUMN, filename,
			I7_RESULT_RESULT_TYPE_COLUMN, I7_RESULT_TYPE_PROJECT,
			I7_RESULT_LINE_NUMBER_COLUMN, lineno,
			-1);
		g_free(context);
		g_free(sort);
		g_free(filename);
	}

	stop_spinner(self);
}

/* Search the user-installed extensions for the string 'text' */
void
i7_search_window_search_extensions(I7SearchWindow *self)
{
	I7_SEARCH_WINDOW_USE_PRIVATE(self, priv);
	GError *err = NULL;
	gchar *extension_dir = i7_app_get_extension_path(i7_app_get(), NULL, NULL);
	GDir *extensions = g_dir_open(extension_dir, 0, &err);
	g_free(extension_dir);
	if(err) {
		error_dialog(GTK_WINDOW(self), err, _("Error opening extensions directory: "));
		return;
	}

	const gchar *dir_entry;
	while((dir_entry = g_dir_read_name(extensions)) != NULL && strcmp(dir_entry, "Reserved") != 0) {
		/* Read each extension dir, but skip "Reserved" */
		gchar *author_dir = i7_app_get_extension_path(i7_app_get(), dir_entry, NULL);
		GDir *author = g_dir_open(author_dir, 0, &err);
		g_free(author_dir);
		if(err) {
			error_dialog(GTK_WINDOW(self), err, _("Error opening extensions directory: "));
			return;
		}
		const gchar *author_entry;
		while((author_entry = g_dir_read_name(author)) != NULL) {
			gchar *filename = i7_app_get_extension_path(i7_app_get(), dir_entry, author_entry);
			gchar *contents;
			if(!g_file_get_contents(filename, &contents, NULL, &err)) {
				error_dialog(GTK_WINDOW(self), err,
				  /* TRANSLATORS: Error opening EXTENSION_NAME by AUTHOR_NAME */
				  _("Error opening extension '%s' by '%s':"), author_entry, dir_entry);
				g_free(filename);
				return;
			}

			GtkTextBuffer *buffer = GTK_TEXT_BUFFER(gtk_source_buffer_new(NULL));
			gtk_text_buffer_set_text(buffer, contents, -1);
			g_free(contents);

			GtkTreeIter result;
			GtkTextIter search_from, match_start, match_end;
			gtk_text_buffer_get_start_iter(buffer, &search_from);

			start_spinner(self);

			while(find_no_wrap(&search_from, priv->text, TRUE,
				GTK_SOURCE_SEARCH_TEXT_ONLY | (priv->ignore_case? GTK_SOURCE_SEARCH_CASE_INSENSITIVE : 0),
				priv->algorithm, &match_start, &match_end))
			{
				while(gtk_events_pending())
					gtk_main_iteration();

				search_from = match_end;

				/* Get the line number (counted from 0) */
				guint lineno = gtk_text_iter_get_line(&match_start) + 1;

				gchar *context = extract_context(buffer, &match_start, &match_end);

				/* Make a sort string */
				gchar *sort = g_strdup_printf("%s %04i", author_entry, lineno);

				gtk_list_store_append(priv->results, &result);
				gtk_list_store_set(priv->results, &result,
					I7_RESULT_CONTEXT_COLUMN, context,
					I7_RESULT_SORT_STRING_COLUMN, sort,
					I7_RESULT_FILE_COLUMN, filename,
					I7_RESULT_RESULT_TYPE_COLUMN, I7_RESULT_TYPE_EXTENSION,
					I7_RESULT_LINE_NUMBER_COLUMN, lineno,
					-1);
				g_free(context);
				g_free(sort);
			}

			stop_spinner(self);
			g_object_unref(buffer);
			g_free(filename);
		}
		g_dir_close(author);
	}
	g_dir_close(extensions);
}

/* Notify the window that no more searches will be done, so it is allowed to
 close itself if asked to */
void
i7_search_window_done_searching(I7SearchWindow *self)
{
	g_signal_handlers_disconnect_by_func(self, on_search_window_delete_event, NULL);
}

/* SUCKY DEBIAN move to builder file */
static GtkWidget *
pack_spinner_in_box(GtkWidget *box)
{
	GtkWidget *spinner = gtk_spinner_new();
	gtk_widget_set_no_show_all(spinner, TRUE);
	gtk_box_pack_end(GTK_BOX(box), spinner, FALSE, FALSE, 0);
	return spinner;
}

static void
start_spinner(I7SearchWindow *self)
{
	gtk_spinner_start(GTK_SPINNER(self->spinner));
	gtk_widget_show(self->spinner);
}

static void
stop_spinner(I7SearchWindow *self)
{
	gtk_spinner_stop(GTK_SPINNER(self->spinner));
	gtk_widget_hide(self->spinner);
}

