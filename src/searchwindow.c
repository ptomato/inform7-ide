/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
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

#include <gnome.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <gtkhtml/gtkhtml.h>
#include <gtksourceview/gtksourceiter.h>

#include "interface.h"
#include "support.h"

#include "appwindow.h"
#include "datafile.h"
#include "error.h"
#include "extension.h"
#include "extwindow.h"
#include "file.h"
#include "findreplace.h"
#include "history.h"
#include "html.h"
#include "searchwindow.h"
#include "story.h"
#include "tabsource.h"

/* An index of the text of the documentation and example pages. Only built
the first time someone does a documentation search, and freed atexit. */
static GList *doc_index = NULL;

typedef struct {
	gboolean example;
    gboolean recipebook;
	gchar *section;
	gchar *title;
	gchar *sort;
	gchar *body;
	gchar *file;
} DocText;


/* Columns for the search results tree view */
enum {
    SEARCH_WINDOW_CONTEXT_COLUMN,
    SEARCH_WINDOW_LOCATION_COLUMN,
    SEARCH_WINDOW_FILE_COLUMN,
    SEARCH_WINDOW_RESULT_TYPE_COLUMN,
    SEARCH_WINDOW_LINE_NUMBER_COLUMN,
    SEARCH_WINDOW_NUM_COLUMNS
};

enum {
    RESULT_TYPE_PROJECT,
    RESULT_TYPE_EXTENSION,
    RESULT_TYPE_DOCUMENTATION,
    RESULT_TYPE_RECIPE_BOOK
};

typedef struct {
    gchar *context;
    gchar *source_sort;
    gchar *source_location;
    gchar *source_file;
    int result_type;
    int lineno;
} Result;


/* Free the data in a Result structure */
static void
result_free(Result *foo)
{
    g_free(foo->context);
    g_free(foo->source_sort);
    g_free(foo->source_location);
    g_free(foo->source_file);
    g_free(foo);
}

/* Callback for double-clicking on one of the search results */
void
on_search_results_view_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                     GtkTreeViewColumn *column, 
                                     GtkWidget *main_window)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    g_return_if_fail(model);
    
    if(!(gtk_tree_model_get_iter(model, &iter, path)))
        return;
    gchar *filename;
    int result_type, lineno;
    gtk_tree_model_get(model, &iter,
      SEARCH_WINDOW_FILE_COLUMN, &filename,
      SEARCH_WINDOW_RESULT_TYPE_COLUMN, &result_type,
      SEARCH_WINDOW_LINE_NUMBER_COLUMN, &lineno,
      -1);
    
    gchar *anchor;
    Extension *ext;
    switch(result_type) {
    case RESULT_TYPE_DOCUMENTATION:
    case RESULT_TYPE_RECIPE_BOOK:
        /* Check if there is an anchor we need to jump to */
        
        if((anchor = strchr(filename, '#')))
            *(anchor++) = '\0';
        
        /* Display the documentation page in the appropriate widget */
        int panel = choose_notebook(main_window, TAB_DOCUMENTATION);
        GtkHTML *html = GTK_HTML(lookup_widget(main_window,
          (panel == LEFT)? "docs_l" : "docs_r"));
        html_load_file(html, filename);
		history_push_docpage(get_story(main_window), panel, filename);
        if(anchor)
            gtk_html_jump_to_anchor(html, anchor);
        /* Show the widget */
        gtk_notebook_set_current_page(get_notebook(main_window, panel),
          TAB_DOCUMENTATION);
        break;
    case RESULT_TYPE_PROJECT:
        jump_to_line(main_window, lineno);
        break;
    case RESULT_TYPE_EXTENSION:
        ext = get_extension_if_open(filename);
        if(ext == NULL)
            ext = open_extension(filename);
        jump_to_line_ext(ext->window, lineno);
        gtk_widget_show(ext->window);
        break;
    default:
        g_assert_not_reached();
    }
    g_free(filename);
}


/* Create a new search results window with the results listed.
Frees the results */
GtkWidget *
new_search_window(GtkWidget *main_window, const gchar *text, GList *results)
{
    GtkWidget *search_window = create_search_window();
    
    gtk_label_set_text(
      GTK_LABEL(lookup_widget(search_window, "search_text_label")), text);
    
    GtkTreeView *view = GTK_TREE_VIEW(lookup_widget(search_window,
      "search_results_view"));
    
    /* Construct the model */
    GtkListStore *store = gtk_list_store_new(SEARCH_WINDOW_NUM_COLUMNS,
      G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
    GtkTreeIter tree_iter;
    
    GList *iter;
    for(iter = results; iter != NULL; iter = g_list_next(iter)) {
        Result *foo = (Result *)(iter->data);
        gtk_list_store_append(store, &tree_iter);
        gtk_list_store_set(store, &tree_iter,
          SEARCH_WINDOW_CONTEXT_COLUMN, foo->context,
          SEARCH_WINDOW_LOCATION_COLUMN, foo->source_location,
          SEARCH_WINDOW_FILE_COLUMN, foo->source_file,
          SEARCH_WINDOW_RESULT_TYPE_COLUMN, foo->result_type,
          SEARCH_WINDOW_LINE_NUMBER_COLUMN, foo->lineno,
          -1);
        result_free(foo);
    }
    g_list_free(results);
    
    gtk_tree_view_set_model(view, GTK_TREE_MODEL(store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      _("Context"), renderer, 
      "text", SEARCH_WINDOW_CONTEXT_COLUMN,
      NULL);
    gtk_tree_view_append_column(view, column);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
      _("Location"), renderer, 
      "text", SEARCH_WINDOW_LOCATION_COLUMN,
      NULL);
    gtk_tree_view_append_column(view, column);
    
    g_signal_connect(G_OBJECT(view), "row-activated",
      G_CALLBACK(on_search_results_view_row_activated),
      (gpointer)main_window);
    
    return search_window;
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
        g_free(text);
    }
    g_list_free(doc_index);
    doc_index = NULL;
}

/* Relevant tags and stuff for the HTML decoder */
struct tag {
	const gchar *name;
	int len;
	gboolean remove;
	gboolean cr;
};

static struct tag tags[] = {
	{"a",           1, FALSE, FALSE},
	{"B>",          2, FALSE, FALSE},
	{"b>",          2, FALSE, FALSE},
	{"blockquote", 10, FALSE, FALSE},
	{"br",          2, FALSE, TRUE },
	{"font",        4, FALSE, FALSE},
	{"h",           1, FALSE, FALSE},
	{"i>",          2, FALSE, FALSE},
	{"img",         3, FALSE, FALSE},
	{"p>",          2, FALSE, TRUE },
    {"pre>",        4, FALSE, FALSE},
	{"script",      6, TRUE,  FALSE},
	{"table",       5, FALSE, FALSE},
	{"TABLE",       5, FALSE, FALSE},
	{"td",          2, FALSE, FALSE},
	{"TD",          2, FALSE, FALSE},
	{"tr",          2, FALSE, FALSE},
	{"TR",          2, FALSE, FALSE},
	{"div",         3, FALSE, FALSE}
};

#define NUM_TAGS (sizeof(tags) / sizeof(tags[0]))

struct literal {
	const gchar *name;
	int len;
	gchar replace;
};

static struct literal literals[] = {
	{"quot;", 5, '\"'},
	{"nbsp;", 5, ' '},
	{"lt;",   3, '<'},
	{"gt;",   3, '>'}
};

#define NUM_LITERALS (sizeof(literals) / sizeof(literals[0]))

/* Read the specified HTML file and convert it to plain text. Store the text and
the metadata in the comments in the DocText structure. Returns TRUE on success,
FALSE on failure. */
static gboolean
html_decode(const gchar *filename, DocText **doc_text)
{
	g_return_val_if_fail(filename != NULL, FALSE);
	
	GError *err;
	gchar *html;
	GString *text;
	
	(*doc_text)->title = NULL;
	(*doc_text)->section = NULL;
	(*doc_text)->sort = NULL;
	
	/* Open the file */
	if(!g_file_get_contents(filename, &html, NULL, &err)) {
		g_critical(_("Error opening file: %s\n"), err->message);
		g_error_free(err);
		return FALSE;
	}
	
	/* Get the body text */
	gchar *body1 = strstr(html, "<body"); /* <body bgcolor="blabla"> */
	gchar *body2 = strstr(html, "</body>");
	if(body1 == NULL || body2 == NULL || body2 <= body1)
		return FALSE;
    gchar *end_of_body_tag = strchr(body1 + 6, '>');
	gchar *body_html = g_strndup(end_of_body_tag + 1,
                                 body2 - end_of_body_tag - 1);
	g_free(html);
	
	/* Reserve space for the main text */
	int len = strlen(body_html);
	text = g_string_sized_new(len);

	/* Scan the text, removing markup */
	gboolean example = FALSE;
	gboolean white = FALSE;
	const gchar *p1 = body_html;
	const gchar *p2 = p1 + len;
	while(p1 < p2) {
		/* Look for a markup element */
		if(*p1 == '<' && (isalpha(*(p1 + 1)) || *(p1 + 1) == '/')) {
			
			/* Check for a closing markup element */
			gboolean closing = FALSE;
			if(*(p1 + 1) == '/') {
				closing = TRUE;
				p1++;
			}
			
			/* Scan for a known markup element */
			gboolean found = FALSE;
			int i = 0;
			while(!found && i < NUM_TAGS) {
				if(strncmp(p1 + 1, tags[i].name, tags[i].len) == 0)
					found = TRUE;
				if(!found)
					i++;
			}
			g_assert(found);
			
			/* Remove the markup */
			if(found && tags[i].remove) {
				g_assert(!closing);
				
				/* Remove everything until the closing element */
				gchar *search = g_strconcat("</", tags[i].name, ">", NULL);
				p1 = strstr(p1, search);
				if(p1 != NULL)
					p1 += strlen(search) - 1;
				else
					p1 = p2;
				g_free(search);
			} else {
				/* Remove just the element */
				while(p1 < p2 && *p1 != '>')
					p1++;
			}
			g_assert(*p1 == '>');
			
			/* Add a carriage return for appropriate markup */
			if(found && !closing && tags[i].cr)
				if(!((*doc_text)->example) || example)
					g_string_append_c(text, '\n');
      		white = FALSE;
			
		} else if(*p1 == '<' && *(p1 + 1) == '!') {
			/* Extract metadata from comments */
			gchar *meta = (gchar *)g_malloc(p2 - p1);
			if(sscanf(p1, "<!-- SEARCH TITLE \"%[^\"]", meta) == 1)
				(*doc_text)->title = g_strdup(meta);
			else if(sscanf(p1, "<!-- SEARCH SECTION \"%[^\"]", meta) == 1)
				(*doc_text)->section = ((*doc_text)->recipebook)?
                  g_strconcat(_("Recipe Book, "), meta, NULL) : g_strdup(meta);
			else if(sscanf(p1, "<!-- SEARCH SORT \"%[^\"]", meta) == 1)
				(*doc_text)->sort = g_strdup(meta);
			else if(strncmp(p1, "<!-- EXAMPLE START -->", 22) == 0)
				example = TRUE;
			else if(strncmp(p2, "<!-- EXAMPLE END -->", 20) == 0)
				example = FALSE;
			g_free(meta);
			
			p1 = strstr(p1, "-->");
			if(p1 != NULL)
				p1 += 2;
			else
				p1 = p2;
			
		} else if(*p1 == '&') {
			/* Scan for a known literal */
			gboolean found = FALSE;
			int i = 0;
			while(!found && i < NUM_LITERALS) {
				if(strncmp(p1 + 1, literals[i].name, literals[i].len) == 0)
					found = TRUE;
				if(!found)
					i++;
			}
			
			/* Replace the literal */
			if(found) {
				g_string_append_c(text, literals[i].replace);
				p1 += literals[i].len;
			} else
				if(!((*doc_text)->example) || example)
					g_string_append_c(text, *p1);
			white = FALSE;
		
		} else if(isspace(*p1)) {
			if(!white)
				if(!((*doc_text)->example) || example)
					g_string_append_c(text, ' ');
			white = TRUE;
			
		} else {
			if(!((*doc_text)->example) || example)
				g_string_append_c(text, *p1);
      		white = FALSE;
		}
		
		p1++;
	}
	
	g_free(body_html);
	(*doc_text)->body = g_strdup(text->str);
	g_string_free(text, TRUE);
	return TRUE;
}

/* Comparison function to sort the results by source string, placing recipe book
after regular documentation entries */
static int
sort_by_source(gconstpointer a, gconstpointer b)
{
    if(((Result *)a)->result_type == ((Result *)b)->result_type)
        return strcmp(((Result *)a)->source_sort, ((Result *)b)->source_sort);
    if(((Result *)a)->result_type == RESULT_TYPE_RECIPE_BOOK &&
       ((Result *)b)->result_type == RESULT_TYPE_DOCUMENTATION)
        return 1;
    return -1;
}

/* Function to search through a GtkSourceBuffer, putting context, line number
and new search point into the pointers indicated. Returns whether there was a
match. */
static gboolean
search_buffer(const gchar *search_text, gboolean ignore_case, int algorithm,
              GtkSourceBuffer *buffer, GtkTextIter *search_from,
              gchar **context, gint *lineno)
{
    /* Run the main loop, because this function will probably be called many
      times in succession */
    while(gtk_events_pending())
        gtk_main_iteration();  
      
    GtkTextIter match_start, match_end;
    gboolean retval, do_it_again;
    
    do {
        do_it_again = FALSE;
        retval = gtk_source_iter_forward_search(search_from, search_text,
          GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY |
          (ignore_case? GTK_SOURCE_SEARCH_CASE_INSENSITIVE : 0),
          &match_start, &match_end, NULL);
        if(retval) {
            if(algorithm == FIND_STARTS_WITH) {
                if(!gtk_text_iter_starts_word(&match_start))
                    do_it_again = TRUE;
            } else if(algorithm == FIND_FULL_WORD) {
                if(!gtk_text_iter_starts_word(&match_start)
                  || !gtk_text_iter_ends_word(&match_end))
                    do_it_again = TRUE;
            }      
        }
        *search_from = match_end; /* Whether we do it again or not */
    } while(do_it_again);
        
    if(!retval)
        return FALSE;        
        
    /* Get the line number (counted from 0) */
    *lineno = gtk_text_iter_get_line(&match_start) + 1;  
    
    /* Create a larger range to extract the context */
    gtk_text_iter_backward_chars(&match_start, 8);
    gtk_text_iter_forward_chars(&match_end, 32);
      
    /* Get the surrounding text as context */
    *context = gtk_text_buffer_get_text(
      GTK_TEXT_BUFFER(buffer), &match_start, &match_end, FALSE);
    g_strdelimit(*context, "\n\r\t", ' ');
      
    return TRUE;
}

/* Search the documentation pages for the string 'text' */
GList *
search_doc(const gchar *text, gboolean ignore_case, int algorithm)
{
    GError *err;
    GList *results = NULL;
	
	if(doc_index == NULL) { /* documentation index hasn't been built yet */
		gboolean example = FALSE;
        gchar *docpath = get_datafile_path_va("Documentation", "Sections",NULL);
        
        GDir *docdir;
        if((docdir = g_dir_open(docpath, 0, &err)) == NULL) {
            error_dialog(NULL, err, 
              _("Could not open documentation directory: "));
            g_free(docpath);
            return NULL;
        }
        const gchar *filename;
        while((filename = g_dir_read_name(docdir)) != NULL) {
            if((g_str_has_prefix(filename, "doc")
                || g_str_has_prefix(filename, "Rdoc"))
               && g_str_has_suffix(filename, ".html"))
                example = FALSE;
            else if((g_str_has_prefix(filename, "ex")
                     || g_str_has_prefix(filename, "Rex"))
                    && g_str_has_suffix(filename, ".html"))
                example = TRUE;
            else
                continue;

            DocText *doc_text = g_new0(DocText, 1);
            doc_text->example = example;
            doc_text->recipebook = g_str_has_prefix(filename, "R");
            doc_text->file = g_build_filename(docpath, filename, NULL);
            if(html_decode(doc_text->file, &doc_text))
                doc_index = g_list_prepend(doc_index, (gpointer)doc_text);
        }
        g_free(docpath);
        
        g_atexit(free_doc_index);
    } /* doc_index == NULL */
    
    GList *iter;
    for(iter = doc_index; iter != NULL; iter = g_list_next(iter)) {
        DocText *doc_text = (DocText *)iter->data;
        
        GtkSourceBuffer *buffer = gtk_source_buffer_new(NULL);
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), doc_text->body, -1);
        
        GtkTextIter search_from;
        gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), 
          &search_from);
        
        gchar *context;
        gint lineno;
        while(search_buffer(text, ignore_case, algorithm, buffer,
          &search_from, &context, &lineno)) {
            Result *result = g_new0(Result, 1);
            result->context = context;
            result->source_location = g_strconcat(doc_text->section, ": ",
              doc_text->title, NULL);
            result->source_sort = g_strdup(doc_text->sort);
            result->source_file = g_strdup(doc_text->file);
            result->result_type = doc_text->recipebook?
                  RESULT_TYPE_RECIPE_BOOK : RESULT_TYPE_DOCUMENTATION;
            /* For examples, add a reference to the example section */
            if(doc_text->example) {
                gchar *name = g_path_get_basename(doc_text->file);
                int number = 0;
                if(sscanf(name, "ex%d.html", &number) == 1) {
                    gchar *section = g_strdup_printf("%s#e%d",
                      result->source_file, number);
                    g_free(result->source_file);
                    result->source_file = section;
                }
                g_free(name);
            }
            
            results = g_list_prepend(results, (gpointer)result);
        }
        
        g_object_unref(buffer);
    }
    
    results = g_list_sort(results, sort_by_source);
    return results;
}


/* Search the project file for the string 'text' */
GList *
search_project(const gchar *text, Story *thestory, gboolean ignore_case,
                      int algorithm)
{
    GList *results = NULL;
	
	GtkTextIter search_from;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(thestory->buffer),
      &search_from);
    
    gchar *context;
    gint lineno;
    while(search_buffer(text, ignore_case, algorithm, thestory->buffer,
      &search_from, &context, &lineno)) {
        Result *result = g_new0(Result, 1);
        result->context = context;
        result->source_location = g_strdup_printf(_("Story, line %d"), lineno);
        result->source_sort = g_strdup_printf("%04i", lineno);
        result->source_file = g_strdup("");
        result->result_type = RESULT_TYPE_PROJECT;
        result->lineno = lineno;
          
        results = g_list_prepend(results, (gpointer)result);
    }
    
    results = g_list_reverse(results);
    return results;
}


/* Search the user-installed extensions for the string 'text' */
GList *
search_extensions(const gchar *text, gboolean ignore_case, int algorithm)
{
    GList *results = NULL;
	GError *err = NULL;
    gchar *extension_dir = get_extension_path(NULL, NULL);
    GDir *extensions = g_dir_open(extension_dir, 0, &err);
    g_free(extension_dir);
    if(err) {
        error_dialog(NULL, err, _("Error opening extensions directory: "));
        return NULL;
    }
    
    const gchar *dir_entry;
    while((dir_entry = g_dir_read_name(extensions)) != NULL
      && strcmp(dir_entry, "Reserved")) {
        /* Read each extension dir, but skip "Reserved" */
        gchar *author_dir = get_extension_path(dir_entry, NULL);
        GDir *author = g_dir_open(author_dir, 0, &err);
        g_free(author_dir);
        if(err) {
            error_dialog(NULL, err, _("Error opening extensions directory: "));
            return NULL;
        }
        const gchar *author_entry;
        while((author_entry = g_dir_read_name(author)) != NULL) {
            gchar *filename = get_extension_path(dir_entry, author_entry);
            gchar *contents;
            if(!g_file_get_contents(filename, &contents, NULL, &err)) {
                error_dialog(NULL, err, 
                  /* TRANSLATORS: Error opening EXTENSION_NAME by AUTHOR_NAME */
                  _("Error opening extension '%s' by '%s':"),
                  author_entry, dir_entry);
                g_free(filename);
                return NULL;
            }
            
            GtkSourceBuffer *buffer = gtk_source_buffer_new(NULL);
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), contents, -1);
            
            GtkTextIter search_from;
            gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), 
              &search_from);
            
            gchar *context;
            gint lineno;
            while(search_buffer(text, ignore_case, algorithm, buffer,
              &search_from, &context, &lineno)) {
                Result *result = g_new0(Result, 1);
                result->context = context;
                result->source_location = g_strdup_printf(
                  /* TRANSLATORS: EXTENSION_NAME by AUTHOR_NAME, line NUMBER */
                  _("%s by %s, line %d"),
                  author_entry, dir_entry, lineno);
                result->source_sort = g_strdup_printf(
                  /* TRANSLATORS: EXTENSION_NAME LINE_NUMBER */
                  _("%s %04i"), author_entry,
                  lineno);
                result->source_file = g_strdup(filename);
                result->result_type = RESULT_TYPE_EXTENSION;
                result->lineno = lineno;
                
                results = g_list_prepend(results, (gpointer)result);
            }
            
            g_object_unref(buffer);
            g_free(filename);
        }
        g_dir_close(author);
    }
    g_dir_close(extensions);    
    
    results = g_list_sort(results, sort_by_source);
    return results;
}
