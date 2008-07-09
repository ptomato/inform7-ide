/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gnome.h>

#include "support.h"

#include "appwindow.h"
#include "configfile.h"
#include "findreplace.h"
#include "inspector.h"
#include "searchwindow.h"
#include "story.h"
#include "tabskein.h"
#include "tabsource.h"

#define I_LIKE_SKEIN

/* The global pointer to the inspector window */
GtkWidget *inspector_window;

/* The private pointer to the story we are currently inspecting */
static Story *inspecting = NULL;

void
after_inspector_window_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
    /* Set the find algorithm in the Search inspector to "contains" */
    gtk_combo_box_set_active(
      GTK_COMBO_BOX(lookup_widget(widget, "search_inspector_algorithm")),
      FIND_CONTAINS);
    
    /* Make the column of the headings inspector */
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      "Contents",
      renderer,
      "text", HEADING_TITLE,
      NULL);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    /* Why do I have to set the damn sizing every time */
    gtk_tree_view_append_column(
      GTK_TREE_VIEW(lookup_widget(widget, "headings")),
      column);
    
    update_inspectors();

    /* Move the window to the saved position */    
    gtk_window_move(GTK_WINDOW(inspector_window),
      config_file_get_int("Settings", "InspectorPosX"),
      config_file_get_int("Settings", "InspectorPosY"));
}


/* Text changed; activate the Search button if there is text in it */
void
on_search_inspector_text_changed       (GtkEditable     *editable,
                                        gpointer         user_data)
{
    /* Do not free or modify the strings from gtk_entry_get_text */
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(editable), "search_inspector_search"), 
      !(text == NULL || strlen(text) == 0));
}


/* The 'Search' button is clicked in the search inspector */
void
on_search_inspector_search_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
    /* Find out what we have to search */
    gboolean project = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(button),
      "search_inspector_search_project")));
    gboolean extensions = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(button),
      "search_inspector_search_extensions")));
    gboolean documentation = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(button),
      "search_inspector_search_documentation")));
    
    /* Find out how we have to search it */
    gboolean ignore_case = !gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(button),
      "search_inspector_case_sensitive")));
    int algorithm = gtk_combo_box_get_active(
      GTK_COMBO_BOX(lookup_widget(GTK_WIDGET(button),
      "search_inspector_algorithm")));
    
    /* The search string (do not free) */
    const gchar *search_text = gtk_entry_get_text(
      GTK_ENTRY(lookup_widget(GTK_WIDGET(button), "search_inspector_text")));
    GList *docs_results = NULL;
    GList *ext_results = NULL;
    GList *proj_results = NULL;
    
    if(documentation) {
        docs_results = search_doc(search_text, ignore_case, algorithm);
        display_status_message(inspecting->window,"Searching documentation...");
        /* display_status_busy(inspecting->window);*/
    }
    if(extensions) {
        ext_results = search_extensions(search_text, ignore_case, algorithm);
        display_status_message(inspecting->window, "Searching extensions...");
        /*display_status_busy(inspecting->window);*/
    }
    if(project) {
        proj_results = search_project(search_text, inspecting, ignore_case,
          algorithm);
        display_status_message(inspecting->window,"Searching project...");
        /*display_status_busy(inspecting->window);*/
    }
    
    GList *results = g_list_concat(docs_results, ext_results);
    results = g_list_concat(results, proj_results);
    
    display_status_message(inspecting->window, "Search complete.");
    GtkWidget *search_window = new_search_window(inspecting->window,
      search_text, results);
    /* 'results' is freed in new_search_window, and so are the concatenated
    lists */
    gtk_widget_show(search_window);
}


/* Hide the window instead of deleting it */
gboolean
on_inspector_window_delete             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_widget_hide(widget);
    config_file_set_bool("Settings", "InspectorVisible", FALSE);
    return TRUE; /* Interrupt the event */
}


/* Jump to the heading when its index entry is double-clicked */
void
on_headings_row_activated              (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    gint lineno;
    gtk_tree_model_get(model, &iter, HEADING_LINE, &lineno, -1);
    jump_to_line(inspecting->window, lineno);
}

/* Show or hide the inspector in the inspector window */
static void show_inspector(int which, gboolean show) {
    GtkWidget *inspector = NULL;
    switch(which) {
      case INSPECTOR_NOTES:
        inspector = lookup_widget(inspector_window, "notes_inspector");
        break;
      case INSPECTOR_HEADINGS:
        inspector = lookup_widget(inspector_window, "headings_inspector");
        break;
#ifdef I_LIKE_SKEIN
      case INSPECTOR_SKEIN:
        inspector = lookup_widget(inspector_window, "skein_inspector");
        if(show && !GTK_WIDGET_VISIBLE(inspector)
           && inspecting && inspecting->drawflag[SKEIN_INSPECTOR])
            skein_schedule_redraw(inspecting->theskein, inspecting);
        break;
#endif /* I_LIKE_SKEIN */
      case INSPECTOR_SEARCH_FILES:
        inspector = lookup_widget(inspector_window, "search_inspector");
        break;
      default:
        return;
    }
    if(show) {
        gtk_widget_show(inspector);
        gtk_expander_set_expanded(GTK_EXPANDER(inspector), TRUE);
        gtk_widget_hide(lookup_widget(inspector_window, "no_inspector"));
    } else
        gtk_widget_hide(inspector);
}

/* Show or hide the inspectors according to the user's preferences */
void update_inspectors() {
    /* Show the message that no inspectors are showing; then, if one is showing,
    hide it again */
    gtk_widget_show(lookup_widget(inspector_window, "no_inspector"));
    show_inspector(INSPECTOR_NOTES,
      config_file_get_bool("InspectorSettings", "NotesVisible"));
    show_inspector(INSPECTOR_HEADINGS,
      config_file_get_bool("InspectorSettings", "HeadingsVisible"));
    show_inspector(INSPECTOR_SKEIN,
      config_file_get_bool("InspectorSettings", "SkeinVisible"));
    show_inspector(INSPECTOR_SEARCH_FILES,
      config_file_get_bool("InspectorSettings", "SearchVisible"));
}

/* Display the data from the story in the inspector. (Do not check whether we
   are already displaying the data from the same story, because this function is
   also called when we just want to refresh the data. */
void refresh_inspector(Story *thestory) {
#ifdef I_LIKE_SKEIN
    /* Erase the previous story's Skein canvas */
    GtkWidget *canvas = lookup_widget(inspector_window, 
                                      "skein_inspector_canvas");
    clear_gnome_canvas_impolitely(GNOME_CANVAS(canvas));
#endif /* I_LIKE_SKEIN */
    
    /* Set the story we are currently inspecting */
    inspecting = thestory;
    
    /* Refresh the notes inspector */
    gtk_text_view_set_buffer(
      GTK_TEXT_VIEW(lookup_widget(inspector_window, "notes")),
      thestory->notes);
    
    /* Refresh the headings inspector. Only if we are not already displaying
    these same headings, otherwise all the trees will close every time we click
    on the project window. */
    if(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(inspector_window,
       "headings"))) != GTK_TREE_MODEL(thestory->headings)) {
        gtk_tree_view_set_model(
          GTK_TREE_VIEW(lookup_widget(inspector_window, "headings")),
          GTK_TREE_MODEL(thestory->headings));
        if(config_file_get_bool("SyntaxSettings", 
                                "IntelligentHeadingsInspector")) {
            g_idle_remove_by_data(GINT_TO_POINTER(IDLE_REINDEX_HEADINGS));
            g_idle_add((GSourceFunc)reindex_headings,
              GINT_TO_POINTER(IDLE_REINDEX_HEADINGS));
        }
    }
#ifdef I_LIKE_SKEIN    
    /* Refresh the skein inspector */
    skein_schedule_redraw(inspecting->theskein, inspecting);
#endif /* I_LIKE_SKEIN */
}

/* Get the position of the inspector window and save it for the next run */
void save_inspector_window_position() {
    gint x, y;
    gtk_window_get_position(GTK_WINDOW(inspector_window), &x, &y);
    config_file_set_int("Settings", "InspectorPosX", x);
    config_file_set_int("Settings", "InspectorPosY", y);
}


/* Reindex the section headings and update them in the inspector window */
gboolean reindex_headings(gpointer data) {
    if(!inspecting)
        return FALSE; /* do nothing */
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(inspecting->buffer);
    GtkTextIter pos, end;
    gtk_text_buffer_get_start_iter(buffer, &pos);
    
    GtkTreeStore *tree = inspecting->headings;
    gtk_tree_store_clear(tree);
    GtkTreeIter volume, book, part, chapter, section;
    gboolean volume_used = FALSE, book_used = FALSE, part_used = FALSE,
      chapter_used = FALSE;
    
    while(gtk_text_iter_get_char(&pos) != 0) {
        if(gtk_text_iter_get_char(&pos) == '\n') {
            gtk_text_iter_forward_line(&pos);
            end = pos;
            gtk_text_iter_forward_line(&end);
            if(gtk_text_iter_get_char(&end) == '\n') {
                /* Preceded and followed by a blank line */
                /* Get the entire line and its line number, chop the \n */
                gchar *text = g_strchomp(gtk_text_iter_get_text(&pos, &end));
                gchar *lcase = g_utf8_strdown(text, -1);
                gint lineno = gtk_text_iter_get_line(&pos) + 1;
                /* Line numbers counted from 0 */
                
                if(g_str_has_prefix(lcase, "volume ")) {
                    gtk_tree_store_append(tree, &volume, NULL);
                    volume_used = TRUE;
                    gtk_tree_store_set(tree, &volume,
                      HEADING_TITLE, text,
                      HEADING_LINE, lineno,
                      -1);
                } else if(g_str_has_prefix(lcase, "book ")) {
                    gtk_tree_store_append(tree, &book,
                      volume_used? &volume : NULL);
                    book_used = TRUE;
                    gtk_tree_store_set(tree, &book,
                      HEADING_TITLE, text,
                      HEADING_LINE, lineno,
                      -1);
                } else if(g_str_has_prefix(lcase, "part ")) {
                    gtk_tree_store_append(tree, &part,
                      book_used? &book :
                      volume_used? &volume : NULL);
                    part_used = TRUE;
                    gtk_tree_store_set(tree, &part,
                      HEADING_TITLE, text,
                      HEADING_LINE, lineno,
                      -1);
                } else if(g_str_has_prefix(lcase, "chapter ")) {
                    gtk_tree_store_append(tree, &chapter,
                      part_used? &part :
                      book_used? &book :
                      volume_used? &volume : NULL);
                    chapter_used = TRUE;
                    gtk_tree_store_set(tree, &chapter,
                      HEADING_TITLE, text,
                      HEADING_LINE, lineno,
                      -1);
                } else if(g_str_has_prefix(lcase, "section ")) {
                    gtk_tree_store_append(tree, &section,
                      chapter_used? &chapter :
                      part_used? &part :
                      book_used? &book :
                      volume_used? &volume : NULL);
                    gtk_tree_store_set(tree, &section,
                      HEADING_TITLE, text,
                      HEADING_LINE, lineno,
                      -1);
                }
                g_free(text);
                g_free(lcase);
            }
        }    
        gtk_text_iter_forward_line(&pos);
    }
    
    return FALSE; /* One-shot idle function */
}
