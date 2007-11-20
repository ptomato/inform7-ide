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
#include <string.h>
#include <gtksourceview/gtksourcebuffer.h>

#include "interface.h"
#include "support.h"

#include "configfile.h"
#include "error.h"
#include "extension.h"
#include "newdialog.h"
#include "story.h"
#include "windowlist.h"

enum {
    TYPE_NOTHING,
    TYPE_INFORM7,
    TYPE_EXTENSION,
    TYPE_INFORM6_EMPTY,
    TYPE_INFORM6_ONE_ROOM
};

enum {
    COLUMN_TYPE,
    COLUMN_TYPE_NAME,
    COLUMN_DISPLAY_TEXT
};

/* Change the description text when the selection changes */
static void project_type_select_selection_changed(GtkTreeSelection *selection,
gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *description;
    int type;
    GnomeDruid *druid =
      GNOME_DRUID(lookup_widget((GtkWidget *)data, "new_druid"));

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COLUMN_TYPE, &type,
          COLUMN_DISPLAY_TEXT, &description, -1);
        gtk_label_set_text(GTK_LABEL(lookup_widget((GtkWidget *)data,
          "project_type_description")), description);
        gnome_druid_set_buttons_sensitive(druid,
          /* back */ FALSE, /* it's the first page */
          /* next */ (type == TYPE_INFORM7 || type == TYPE_EXTENSION),
          /* cancel */ TRUE, /* help */ FALSE);
        g_free (description);
    }
}

/*
 * FUNCTIONS FOR THE WHOLE DIALOG
 */
gboolean
on_new_dialog_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    on_new_cancel_clicked(GNOME_DRUID(lookup_widget(widget, "new_druid")), 
      user_data);
    return TRUE;
}

void
on_new_cancel_clicked                  (GnomeDruid      *druid,
                                        gpointer         user_data)
{
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(druid)));
    /* If we aren't editing a story, go back to the welcome dialog */
    if(get_num_app_windows() == 0) {
        GtkWidget *welcome_dialog = create_welcome_dialog();
        gtk_widget_show(welcome_dialog);
    }
}

void
on_new_druid_realize                   (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkTreeView *tree = GTK_TREE_VIEW(
      lookup_widget(widget, "project_type_select"));
    
    /* Create a tree store with one column, with a string in it, and have two
    columns of "hidden" data: an integer index and a description string */
    GtkTreeStore *store = gtk_tree_store_new(3, G_TYPE_INT, G_TYPE_STRING,
      G_TYPE_STRING);
    GtkTreeIter iter_parent, iter_child;
    
    /* Probably this ought to be redone with a selection function, that tells
    which selections are allowed and which aren't */
    gtk_tree_store_append(store, &iter_parent, NULL); /* Parent iterator */
    gtk_tree_store_set(store, &iter_parent,
      COLUMN_TYPE, TYPE_NOTHING,
      COLUMN_TYPE_NAME, "Inform 7",
      COLUMN_DISPLAY_TEXT, "Please choose a project type.",
      -1);
    gtk_tree_store_append(store, &iter_child, &iter_parent); /*Child iterator*/
    gtk_tree_store_set(store, &iter_child,
      COLUMN_TYPE, TYPE_INFORM7, 
      COLUMN_TYPE_NAME, "New project",
      COLUMN_DISPLAY_TEXT, "The Inform application can create and edit works of"
      " interactive fiction which use either Inform 7, a natural-language based"
      " system, or the more traditional computer programming language Inform 6."
      " By default, new projects are for Inform 7.",
      -1);
    gtk_tree_store_append(store, &iter_child, &iter_parent); 
    gtk_tree_store_set(store, &iter_child,
      COLUMN_TYPE, TYPE_EXTENSION,
      COLUMN_TYPE_NAME, "New set of extension rules",
      COLUMN_DISPLAY_TEXT, "For experienced users of Inform 7. An extension is "
      "a set of rules which can be used in more than one work of interactive "
      "fiction, either for your own use or to be contributed to the community.",
      -1);
    gtk_tree_store_append(store, &iter_parent, NULL);
    gtk_tree_store_set(store, &iter_parent,
      COLUMN_TYPE, TYPE_NOTHING,
      COLUMN_TYPE_NAME, "Inform 6.3",
      COLUMN_DISPLAY_TEXT, "Please choose a project type.",
      -1);
    gtk_tree_store_append(store, &iter_child, &iter_parent);
    gtk_tree_store_set(store, &iter_child,
      COLUMN_TYPE, TYPE_INFORM6_EMPTY,
      COLUMN_TYPE_NAME, "New project - empty",
      COLUMN_DISPLAY_TEXT, "Creates an Inform 6.3 project. Note that this will "
      "not compile on its own in its initially blank state, so the minimum "
      "required elements of a valid Inform 6 program will have to be pasted "
      "in. [Not yet implemented in GNOME Inform 7.]", -1);
    gtk_tree_store_append(store, &iter_child, &iter_parent); 
    gtk_tree_store_set(store, &iter_child,
      COLUMN_TYPE, TYPE_INFORM6_ONE_ROOM,
      COLUMN_TYPE_NAME, "New project - with one room",
      COLUMN_DISPLAY_TEXT, "Creates an Inform 6.3 project with a minimum "
      "template of source code in place so that it will compile to a one-room "
      "game. [Not yet implemented in GNOME Inform 7.]", -1);
    
    gtk_tree_view_set_model(tree, GTK_TREE_MODEL(store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      "", renderer, "text", COLUMN_TYPE_NAME, NULL); /* No title, text
      renderer, get the property "text" from column COLUMN_TYPE_NAME */
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(tree, column);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(tree);
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed",
      G_CALLBACK(project_type_select_selection_changed), (gpointer)widget);
      
    /* Select "Inform 7 project" by default. */
    GtkTreePath *default_path = gtk_tree_path_new_from_indices(0, 0, -1);
    gtk_tree_view_expand_to_path(tree, default_path);
    gtk_tree_selection_select_path(select, default_path);
    g_free(default_path);
}

gboolean
on_new_druid_page1_next                (GnomeDruidPage  *gnomedruidpage,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(
      GTK_TREE_VIEW(lookup_widget(widget, "project_type_select")));
    GnomeDruid *druid = GNOME_DRUID(lookup_widget(widget, "new_druid"));
    int type;

    gtk_tree_selection_get_selected(selection, &model, &iter);
    gtk_tree_model_get(model, &iter, COLUMN_TYPE, &type, -1);
    
    switch(type) {
        case TYPE_INFORM7:
            gnome_druid_set_page(druid, GNOME_DRUID_PAGE(lookup_widget(widget,
              "new_druid_inform7_page")));
            update_new_ok_button(
              GTK_EDITABLE(lookup_widget(widget, "new_name")), NULL);
            break;
        case TYPE_EXTENSION:
            gnome_druid_set_page(druid, GNOME_DRUID_PAGE(lookup_widget(widget,
              "new_druid_extension_page")));
            update_new_ext_ok_button(
              GTK_EDITABLE(lookup_widget(widget, "new_ext_name")), NULL);
            break;
        case TYPE_INFORM6_ONE_ROOM:
            gnome_druid_set_page(druid, GNOME_DRUID_PAGE(lookup_widget(widget,
              "new_druid_inform6_page")));
            gnome_druid_set_show_finish(druid, TRUE);
    }
    
    return TRUE; /* interrupt signal */
}

/* This is the callback for all the "Back" buttons on all the different pages */
gboolean
go_back_to_type_page                   (GnomeDruidPage  *gnomedruidpage,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
    gnome_druid_set_page(GNOME_DRUID(lookup_widget(widget, "new_druid")),
      GNOME_DRUID_PAGE(lookup_widget(widget, "new_druid_type_page")));
    gnome_druid_set_show_finish(GNOME_DRUID(lookup_widget(widget, "new_druid")),
      FALSE);
    return TRUE;
}

/* For new Inform projects and extensions; get the author's name from the config
file and put it in the "new_author" text entry automatically */
void
on_new_author_realize                  (GtkWidget       *widget,
                                        gpointer         user_data)
{
    gchar *author = g_strstrip(config_file_get_string("User", "Name"));
    gtk_entry_set_text(GTK_ENTRY(widget),
      (author == NULL || strlen(author) == 0)? g_get_real_name() : author);
    g_free(author);
}

/* Get the project name from the text entry and format it */
static gchar *new_dialog_get_name(GtkWidget *thiswidget) {
    gchar *name = g_strstrip(gtk_editable_get_chars(
      GTK_EDITABLE(lookup_widget(thiswidget, "new_name")), 0, -1));
    char *ext = strchr((char *)name, '.');
    if(ext)
        strcpy(ext, "");
    return name;
}

/* Get the author's name from the text entry and format it */
static gchar *new_dialog_get_author(GtkWidget *thiswidget) {
    gchar *author = g_strstrip(gtk_editable_get_chars(
      GTK_EDITABLE(lookup_widget(thiswidget, "new_author")), 0, -1));
    return author;
}

/* Get the filename from the text entry and format it */
static gchar *new_dialog_get_directory(GtkWidget *thiswidget) {
    gchar *path = gtk_file_chooser_get_filename(
      GTK_FILE_CHOOSER(lookup_widget(thiswidget, "new_directory")));
    gchar *name = new_dialog_get_name(thiswidget);
    gchar *file = g_strconcat(name, ".inform", NULL);
    gchar *directory = g_build_filename(path, file, NULL);
    g_free(path);
    g_free(name);
    g_free(file);
    return directory;
}

void
on_new_druid_inform7_page_finish       (GnomeDruidPage  *gnomedruidpage,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
    gchar *name = new_dialog_get_name(widget);
    gchar *author = new_dialog_get_author(widget);
    gchar *directory = new_dialog_get_directory(widget);

    /* Check that the project name is valid */
    const char* invalid = "\\/:*?\"<>|";
    if(strpbrk((char *)name, invalid)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(widget)), NULL,
          "The project name cannot contain any of the following: %s", invalid);
        return;
    }

    /* Save the author name to the config file */
    config_file_set_string("User", "Name", author);

    /* Create a new story struct and initialize it */
    Story *thestory = new_story();
    set_story_filename(thestory, directory);

    gchar *title = g_strconcat("\"", name, "\" by \"", author, "\"\n", NULL);
    gtk_source_buffer_begin_not_undoable_action(thestory->buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(thestory->buffer), title, -1);
    gtk_source_buffer_end_not_undoable_action(thestory->buffer);

    g_free(title);
    g_free(name);
    g_free(author);
    g_free(directory);

    gtk_widget_show(thestory->window);

    gtk_widget_destroy(gtk_widget_get_toplevel(widget));
}

/* Enable the OK button if data in all fields are filled in
   (filechooser always has a default directory set) */
void update_new_ok_button(GtkEditable *editable, gpointer user_data) {
    gchar *name = new_dialog_get_name(GTK_WIDGET(editable));
    gchar *author = new_dialog_get_author(GTK_WIDGET(editable));
    gboolean ok_finish = !(name == NULL || strlen(name) == 0 || author == NULL
      || strlen(author) == 0);
    gnome_druid_set_buttons_sensitive(
      GNOME_DRUID(lookup_widget(GTK_WIDGET(editable), "new_druid")),
      /* back */ TRUE, /* next */ ok_finish,
      /* cancel */ TRUE, /* help */ FALSE);
    gnome_druid_set_show_finish(
      GNOME_DRUID(lookup_widget(GTK_WIDGET(editable), "new_druid")), ok_finish);
    g_free(name);
    g_free(author);
}

/* Get the extension name from the text entry and format it */
static gchar *new_ext_get_name(GtkWidget *thiswidget) {
    gchar *name = g_strstrip(gtk_editable_get_chars(
      GTK_EDITABLE(lookup_widget(thiswidget, "new_ext_name")), 0, -1));
    return name;
}

/* Get the author's name from the text entry and format it */
static gchar *new_ext_get_author(GtkWidget *thiswidget) {
    gchar *author = g_strstrip(gtk_editable_get_chars(
      GTK_EDITABLE(lookup_widget(thiswidget, "new_ext_author")), 0, -1));
    return author;
}

/* Get the filename from the text entry and format it */
static gchar *new_ext_get_directory(GtkWidget *thiswidget) {
    gchar *path = gtk_file_chooser_get_filename(
      GTK_FILE_CHOOSER(lookup_widget(thiswidget, "new_ext_directory")));
    gchar *name = new_ext_get_name(thiswidget);
    gchar *directory = g_build_filename(path, name, NULL);
    g_free(path);
    g_free(name);
    return directory;
}

/* Enable the OK button if data in all fields are filled in
   (filechooser always has a default directory set) */
void
update_new_ext_ok_button               (GtkEditable     *editable,
                                        gpointer         user_data)
{
    gchar *name = new_ext_get_name(GTK_WIDGET(editable));
    gchar *author = new_ext_get_author(GTK_WIDGET(editable));
    gboolean ok_finish = !(name == NULL || strlen(name) == 0 || author == NULL
      || strlen(author) == 0);
    gnome_druid_set_buttons_sensitive(
      GNOME_DRUID(lookup_widget(GTK_WIDGET(editable), "new_druid")),
      /* back */ TRUE, /* next */ ok_finish,
      /* cancel */ TRUE, /* help */ FALSE);
    gnome_druid_set_show_finish(
      GNOME_DRUID(lookup_widget(GTK_WIDGET(editable), "new_druid")), ok_finish);
    g_free(name);
    g_free(author);
}


void
on_new_druid_extension_page_finish     (GnomeDruidPage  *gnomedruidpage,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
    gchar *name = new_ext_get_name(widget);
    gchar *author = new_ext_get_author(widget);
    gchar *directory = new_ext_get_directory(widget);

    /* Check that the filename is valid */
    const char* invalid = "\\/:*?\"<>|";
    if(strpbrk((char *)name, invalid)) {
        error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(widget)), NULL,
          "The filename cannot contain any of the following: %s", invalid);
        return;
    }

    /* Save the author name to the config file */
    config_file_set_string("User", "Name", author);

    /* Create a new extension struct and initialize it */
    Extension *ext = new_ext();
    set_ext_filename(ext, directory);

    gchar *text = g_strconcat(name, " by ", author, " begins here.\n\n", 
      name, " ends here.\n", NULL);
    gtk_source_buffer_begin_not_undoable_action(ext->buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(ext->buffer), text, -1);
    gtk_source_buffer_end_not_undoable_action(ext->buffer);

    g_free(text);
    g_free(name);
    g_free(author);
    g_free(directory);

    gtk_widget_show(ext->window);

    gtk_widget_destroy(gtk_widget_get_toplevel(widget));
}

/* This function will do something when Inform 6 projects are implemented. */
void
on_new_druid_inform6_page_finish       (GnomeDruidPage  *gnomedruidpage,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
    fprintf(stderr, "Finish!\n");
}
