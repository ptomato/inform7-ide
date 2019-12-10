/* Copyright (C) 2006-2009, 2010, 2011, 2012, 2014, 2015 P. F. Chimento
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

#include <stdarg.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <libxml/HTMLparser.h>
#include <webkit2/webkit2.h>

#include "app.h"
#include "builder.h"
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

typedef struct _I7SearchWindowPrivate I7SearchWindowPrivate;
struct _I7SearchWindowPrivate
{
	GtkListStore *results;
	I7Document *document; /* Associated document window */
	gchar *text; /* Search string */
	gboolean ignore_case;
	I7SearchType algorithm;
};

G_DEFINE_TYPE_WITH_PRIVATE(I7SearchWindow, i7_search_window, GTK_TYPE_WINDOW);

/* CALLBACKS */

/* Callback for double-clicking on one of the search results */
void
on_results_view_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, I7SearchWindow *self)
{
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);
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
		if(I7_IS_STORY(priv->document)) {
			/* Jump to the proper example */
			char *filepath = g_file_get_path(file);
			if(anchor != NULL) {
				i7_story_show_docpage_at_anchor(I7_STORY(priv->document), file, anchor);
				g_free(anchor);
			} else
				i7_story_show_docpage(I7_STORY(priv->document), file);

			g_free(filepath);
		} else {
			show_file_in_browser(file, GTK_WINDOW(self));
		}
	}
		break;
	case I7_RESULT_TYPE_PROJECT:
		i7_document_jump_to_line(priv->document, lineno);
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
	return TRUE; /* block deletion */
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
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);

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
			if(I7_IS_STORY(priv->document))
				text = g_strdup_printf(_("Story, line %d"), lineno);
			else {
				gchar *displayname = i7_document_get_display_name(priv->document);
				if(displayname == NULL) {
					text = g_strdup_printf(_("Untitled story, line %d"), lineno);
				} else {
					text = g_strdup_printf(_("%s, line %d"), displayname, lineno);
					g_free(displayname);
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
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);

	priv->document = NULL;
	priv->text = NULL;
	priv->ignore_case = FALSE;
	priv->algorithm = I7_SEARCH_CONTAINS;

	gtk_window_set_destroy_with_parent(GTK_WINDOW(self), TRUE);
	gtk_window_set_icon_name(GTK_WINDOW(self), "com.inform7.IDE");
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(self), TRUE);
	gtk_window_set_title(GTK_WINDOW(self), _("Search Results"));
	gtk_window_set_type_hint(GTK_WINDOW(self), GDK_WINDOW_TYPE_HINT_UTILITY);
	gtk_container_set_border_width(GTK_CONTAINER(self), 12);
	gtk_window_set_default_size(GTK_WINDOW(self), 400, 400);

	/* Build the interface from the builder file */
	g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/com/inform7/IDE/ui/searchwindow.ui");
	gtk_builder_connect_signals(builder, self);

	/* Build the rest of the interface */
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(load_object(builder, "search_window")));
	priv->results = GTK_LIST_STORE(load_object(builder, "results"));
	gtk_tree_view_column_set_cell_data_func(GTK_TREE_VIEW_COLUMN(load_object(builder, "result_column")),
		GTK_CELL_RENDERER(load_object(builder, "result_renderer")),
		(GtkTreeCellDataFunc)result_data_func, self, NULL);
	gtk_tree_view_column_set_cell_data_func(GTK_TREE_VIEW_COLUMN(load_object(builder, "document_column")),
		GTK_CELL_RENDERER(load_object(builder, "document_renderer")),
		(GtkTreeCellDataFunc)location_data_func, self, NULL);
	gtk_tree_view_column_set_cell_data_func(GTK_TREE_VIEW_COLUMN(load_object(builder, "type_column")),
		GTK_CELL_RENDERER(load_object(builder, "type_renderer")),
		(GtkTreeCellDataFunc)type_data_func, NULL, NULL);
	g_signal_connect(self, "delete-event", G_CALLBACK(on_search_window_delete_event), NULL);

	/* Save public pointers to other widgets */
	LOAD_WIDGET(search_text);
	LOAD_WIDGET(results_view);
	LOAD_WIDGET(spinner);
}

static void
i7_search_window_finalize(GObject *object)
{
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(I7_SEARCH_WINDOW(object));
	g_free(priv->text);

	G_OBJECT_CLASS(i7_search_window_parent_class)->finalize(object);
}

static void
i7_search_window_class_init(I7SearchWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = i7_search_window_finalize;
}

/* INTERNAL FUNCTIONS */

static void
update_label(I7SearchWindow *self)
{
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);
	gchar *label = g_strdup_printf(_("Search results for: \"%s\""), priv->text);
	gtk_label_set_text(GTK_LABEL(self->search_text), label);
	g_free(label);
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
get_quoted_contents(const xmlChar *comment, char **quote1, char **quote2)
{
	char *retval1, *retval2;
	int matched = sscanf((const char *)comment, "\"%m[^\"]\" \"%m[^\"]\"", &retval1, &retval2);
	if(quote1 != NULL)
		*quote1 = (matched >= 1)? retval1 : NULL;
	if(quote2 != NULL)
		*quote2 = (matched >= 2)? retval2 : NULL;
}

#define SEARCH_TITLE_LEN 14   /* length(" SEARCH TITLE ") */
#define SEARCH_SECTION_LEN 16 /* length(" SEARCH SECTION ") */
#define SEARCH_SORT_LEN 13    /* length(" SEARCH SORT ") */
#define START_EXAMPLE_LEN 15  /* length(" START EXAMPLE ") */

static void
comment_callback(Ctxt *ctxt, const xmlChar *value)
{
	/* Extract metadata from comments */
	if(g_str_has_prefix((gchar *)value, " SEARCH TITLE "))
		get_quoted_contents(value + SEARCH_TITLE_LEN, &ctxt->doctext->title, NULL);
	else if(g_str_has_prefix((char *)value, " SEARCH SECTION "))
		get_quoted_contents(value + SEARCH_SECTION_LEN, &ctxt->doctext->section, NULL);
	else if(g_str_has_prefix((char *)value, " SEARCH SORT "))
		get_quoted_contents(value + SEARCH_SORT_LEN, &ctxt->doctext->sort, NULL);

	/* From here on, these are particular subsections of the documentation page,
	such as examples. We assume that the above metadata always appear before a
	subsection can appear, and that subsections cannot be nested. */
	else if(g_str_has_prefix((char *)value, " START EXAMPLE ")) {
		ctxt->outer_chars = ctxt->chars;
		ctxt->chars = g_string_new("");

		ctxt->outer_doctext = ctxt->doctext;
		ctxt->doctext = g_slice_dup(DocText, ctxt->outer_doctext);
		ctxt->doctext->is_example = TRUE;
		g_object_ref(ctxt->doctext->file);
		ctxt->doctext->section = g_strdup(ctxt->outer_doctext->section);
		ctxt->doctext->title = g_strdup(ctxt->outer_doctext->title);
		ctxt->doctext->sort = g_strdup(ctxt->outer_doctext->sort);
		get_quoted_contents(value + START_EXAMPLE_LEN, &ctxt->doctext->example_title, &ctxt->doctext->anchor);
	} else if(g_str_has_prefix((char *)value, " END EXAMPLE ")) {
		ctxt->doctext->body = g_string_free(ctxt->chars, FALSE);
		ctxt->completed_doctexts = g_slist_prepend(ctxt->completed_doctexts, ctxt->doctext);

		ctxt->doctext = ctxt->outer_doctext;
		ctxt->chars = ctxt->outer_chars;
	} else if(g_str_has_prefix((char *)value, " START IGNORE "))
		ctxt->in_ignore_section = TRUE;
	else if(g_str_has_prefix((char *)value, " END IGNORE "))
		ctxt->in_ignore_section = FALSE;
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

	char *path = g_file_get_path(file);
	htmlSAXParseFile(path, NULL, &i7_html_sax, ctxt);
	g_free(path);

	doctext->body = g_string_free(ctxt->chars, FALSE);
	GSList *retval = g_slist_prepend(ctxt->completed_doctexts, ctxt->doctext);
	g_slice_free(Ctxt, ctxt);
	return retval;
}

/* Borrow from document-search.c */
extern gboolean find_no_wrap(const GtkTextIter *, const gchar *, gboolean, GtkTextSearchFlags, I7SearchType, GtkTextIter *, GtkTextIter *);

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
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(gtk_source_buffer_new(NULL));
	gtk_text_buffer_set_text(buffer, doctext->body, -1);
	gtk_text_buffer_get_start_iter(buffer, &search_from);

	while(find_no_wrap(&search_from, priv->text, TRUE,
		GTK_TEXT_SEARCH_TEXT_ONLY | (priv->ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0),
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

	g_object_unref(buffer);
}

/* Helper functions: start and stop the spinner, and keep it hidden when it is
 not going */
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

/* PUBLIC FUNCTIONS */

/* Create a new search results window */
GtkWidget *
i7_search_window_new(I7Document *document, const gchar *text, gboolean ignore_case, I7SearchType algorithm)
{
	I7SearchWindow *self = I7_SEARCH_WINDOW(g_object_new(I7_TYPE_SEARCH_WINDOW, NULL));
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);

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

			char *label = g_strdup_printf(_("Please be patient, indexing %s..."), displayname);
			gtk_label_set_text(GTK_LABEL(self->search_text), label);
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
void
i7_search_window_search_project(I7SearchWindow *self)
{
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(i7_document_get_buffer(priv->document));
	gtk_text_buffer_get_start_iter(buffer, &search_from);

	start_spinner(self);

	while(find_no_wrap(&search_from, priv->text, TRUE,
		GTK_TEXT_SEARCH_TEXT_ONLY | (priv->ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0),
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
		GFile *file = i7_document_get_file(priv->document);

		gtk_list_store_append(priv->results, &result);
		gtk_list_store_set(priv->results, &result,
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
extension_search_result(GFile *parent, GFileInfo *info, gpointer unused, I7SearchWindow *self)
{
	I7SearchWindowPrivate *priv = i7_search_window_get_instance_private(self);
	GError *err = NULL;
	const char *basename = g_file_info_get_name(info);
	GFile *file = g_file_get_child(parent, basename);
	char *contents;
	GtkTextBuffer *buffer;
	GtkTreeIter result;
	GtkTextIter search_from, match_start, match_end;

	if(!g_file_load_contents(file, NULL, &contents, NULL, NULL, &err)) {
		char *author_display_name = file_get_display_name(parent);
		const char *ext_display_name = g_file_info_get_display_name(info);

		error_dialog_file_operation(GTK_WINDOW(self), file, err, I7_FILE_ERROR_OTHER,
		  /* TRANSLATORS: Error opening EXTENSION_NAME by AUTHOR_NAME */
		  _("Error opening extension '%s' by '%s':"), author_display_name, ext_display_name);

		g_free(author_display_name);
		g_object_unref(file);
		return;
	}

	buffer = GTK_TEXT_BUFFER(gtk_source_buffer_new(NULL));
	gtk_text_buffer_set_text(buffer, contents, -1);
	g_free(contents);

	gtk_text_buffer_get_start_iter(buffer, &search_from);

	start_spinner(self);

	while(find_no_wrap(&search_from, priv->text, TRUE,
		GTK_TEXT_SEARCH_TEXT_ONLY | (priv->ignore_case? GTK_TEXT_SEARCH_CASE_INSENSITIVE : 0),
		priv->algorithm, &match_start, &match_end))
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

		gtk_list_store_append(priv->results, &result);
		gtk_list_store_set(priv->results, &result,
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

	g_object_unref(buffer);
	g_object_unref(file);
}

/**
 * i7_search_window_search_extensions:
 * @self: self
 *
 * Search the user-installed extensions for the search window's search text.
 */
void
i7_search_window_search_extensions(I7SearchWindow *self)
{
	I7App *theapp = I7_APP(g_application_get_default());
	i7_app_foreach_installed_extension(theapp, FALSE, NULL, NULL,
	    (I7AppExtensionFunc)extension_search_result, self, NULL);
}

/* Notify the window that no more searches will be done, so it is allowed to
 close itself if asked to */
void
i7_search_window_done_searching(I7SearchWindow *self)
{
	g_signal_handlers_disconnect_by_func(self, on_search_window_delete_event, NULL);
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
