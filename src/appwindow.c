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

#include "interface.h"
#include "support.h"

#include "appmenu.h"
#include "appwindow.h"
#include "configfile.h"
#include "datafile.h"
#include "error.h"
#include "file.h"
#include "history.h"
#include "html.h"
#include "inspector.h"
#include "story.h"
#include "taberrors.h"
#include "tabskein.h"
#include "windowlist.h"

/* The following function is from Damian Iverleigh's patch to GtkContainer,
http://mail.gnome.org/archives/gtk-devel-list/2001-October/msg00516.html */

/**
 * gtk_container_get_focus_child:
 * @container: a #GtkContainer
 * 
 * Retrieves the currently focused child of the container. See
 * gtk_container_set_focus_child().
 *
 * Return value: pointer to the widget of the focused child (NULL
 * if none is set
 **/
static GtkWidget *
gtk_container_get_focus_child (GtkContainer *container)
{
    g_return_val_if_fail (container != NULL, NULL);
    g_return_val_if_fail (GTK_IS_CONTAINER (container), NULL);

    return container->focus_child;
}

/* Get a pointer to the widget within the application that has the focus */
GtkWidget *
get_focused_widget(GtkWidget *thiswidget)
{
    GtkWidget *widget = lookup_widget(thiswidget, "app_window");
    while(!GTK_WIDGET_HAS_FOCUS(widget)) {
        if(GTK_IS_CONTAINER(widget))
            widget = gtk_container_get_focus_child(GTK_CONTAINER(widget));
        else
            g_assert_not_reached();
    }
    return widget;
}

/* Gets a pointer to either the left or the right notebook in this window */
GtkNotebook *get_notebook(GtkWidget *thiswidget, int right) {
    return GTK_NOTEBOOK(lookup_widget(thiswidget,
      right? "notebook_r" : "notebook_l"));
}

/* Gets the notebook that currently has the focus */
GtkNotebook *get_current_notebook(GtkWidget *thiswidget) {
    return GTK_NOTEBOOK(gtk_container_get_focus_child(GTK_CONTAINER(
      lookup_widget(thiswidget, "facingpages"))));
}
    
/* Tells whether the focus is in the left or the right notebook */
int get_current_notebook_side(GtkWidget *thiswidget) {
    GtkWidget *focus = gtk_container_get_focus_child(
      GTK_CONTAINER(lookup_widget(thiswidget, "facingpages")));
    if(focus == lookup_widget(thiswidget, "notebook_l"))
        return LEFT;
    else if(focus == lookup_widget(thiswidget, "notebook_r"))
        return RIGHT;
    return LEFT; /* who cares */
}

/* Chooses an appropriate notebook to display tab number newtab in. (From the
Windows source.) */
int choose_notebook(GtkWidget *thiswidget, int newtab) {
	/* If either panel is showing the same as the new, use that */
    int right = gtk_notebook_get_current_page(get_notebook(thiswidget, RIGHT));
	if(right == newtab)
		return RIGHT;
    int left = gtk_notebook_get_current_page(get_notebook(thiswidget, LEFT));
	if(left == newtab)
		return LEFT;
	/* Always try to use the left panel for source */
	if (newtab == TAB_SOURCE)
		return LEFT;
	/* If the right panel is not source, use that */
	if (right != TAB_SOURCE)
		return RIGHT;
	/* Use the left panel unless that is source too */
	return (left == TAB_SOURCE)? RIGHT : LEFT;
}

/* Displays the message text in the status bar of the current window. */
void display_status_message(GtkWidget *thiswidget, const gchar *message) {
    gnome_appbar_set_status(GNOME_APPBAR(lookup_widget(thiswidget,
      "main_appbar")), message);
}

/* Pulses the progress bar */
void display_status_busy(GtkWidget *thiswidget) {
    gtk_progress_bar_pulse(gnome_appbar_get_progress(GNOME_APPBAR(lookup_widget(
      thiswidget, "main_appbar"))));
}

/* Displays a percentage in the progress indicator */
void display_status_percentage(GtkWidget *thiswidget, gdouble fraction) {
    gtk_progress_bar_set_fraction(gnome_appbar_get_progress(
      GNOME_APPBAR(lookup_widget(thiswidget, "main_appbar"))),
      fraction);
}

/* Clears all status indicators */
void clear_status(GtkWidget *thiswidget) {
    GnomeAppBar *bar = GNOME_APPBAR(lookup_widget(thiswidget, "main_appbar"));
    gtk_progress_bar_set_fraction(gnome_appbar_get_progress(bar), 0.0);
    gtk_progress_bar_set_text(gnome_appbar_get_progress(bar), "");
    gnome_appbar_set_status(bar, "");
}

/* Create the Open Recent submenu */
GtkWidget *create_open_recent_submenu() {
#ifndef SUCKY_GNOME
    GtkWidget *recent_menu = gtk_recent_chooser_menu_new();
    gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(recent_menu), -1);
    GtkRecentFilter *filter = gtk_recent_filter_new();
    gtk_recent_filter_add_application(filter, "GNOME Inform 7");
    gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(recent_menu), filter);
    g_signal_connect(recent_menu, "item-activated", 
                     G_CALLBACK(on_open_recent_activate), NULL);
    return recent_menu;
#else
    return NULL;
#endif /* SUCKY_GNOME */
}

/* Create the Open Extension submenu and return it*/
GtkWidget *create_open_extension_submenu() {
    GError *err = NULL;
    gchar *extension_dir = get_extension_path(NULL, NULL);
    GDir *extensions = g_dir_open(extension_dir, 0, &err);
    g_free(extension_dir);
    if(err) {
        error_dialog(NULL, err, "Error opening extensions directory: ");
        return NULL;
    }
    
    GtkWidget *open_ext_menu = gtk_menu_new();
    
    const gchar *dir_entry;
    while((dir_entry = g_dir_read_name(extensions)) != NULL) {
        if(!strcmp(dir_entry, "Reserved"))
            continue;
        gchar *dirname = get_extension_path(dir_entry, NULL);
        if(g_file_test(dirname, G_FILE_TEST_IS_SYMLINK)
          || !g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
            g_free(dirname);
            continue;
        }
        g_free(dirname);
        /* Read each extension dir, but skip "Reserved", symlinks, and nondirs*/
        GtkWidget *authoritem = gtk_menu_item_new_with_label(dir_entry);
        GtkWidget *authormenu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(authoritem), authormenu);
        
        gchar *author_dir = get_extension_path(dir_entry, NULL);
        GDir *author = g_dir_open(author_dir, 0, &err);
        g_free(author_dir);
        if(err) {
            error_dialog(NULL, err, "Error opening extensions directory: ");
            return NULL;
        }
        const gchar *author_entry;
        while((author_entry = g_dir_read_name(author)) != NULL) {
            gchar *extname = get_extension_path(dir_entry, author_entry);
            if(g_file_test(extname, G_FILE_TEST_IS_SYMLINK)) {
                g_free(extname);
                continue;
            }
            g_free(extname);
            /* Read files in the dir, but skip symlinks */
            GtkWidget *extitem = gtk_menu_item_new_with_label(author_entry);
            gchar *path = get_extension_path(dir_entry, author_entry);
            g_signal_connect(extitem, "activate",
              G_CALLBACK(on_open_extension_activate),
              (gpointer)path);
            /* Do not free path */
            gtk_menu_shell_append(GTK_MENU_SHELL(authormenu), extitem);
            gtk_widget_show(extitem);   
        }
        g_dir_close(author);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(open_ext_menu), authoritem);
        gtk_widget_show(authoritem);
    }
    g_dir_close(extensions);
    
    return open_ext_menu;
}

/* 
 * CALLBACKS
 */

/* Set up things whenever an app_window is created. */ 
void
after_app_window_realize               (GtkWidget       *widget,
                                        gpointer         user_data)
{
    /* Set the last saved window size and slider position */
    gtk_window_resize(GTK_WINDOW(widget), 
                      config_file_get_int("Settings", "AppWindowWidth"),
                      config_file_get_int("Settings", "AppWindowHeight"));
    gtk_paned_set_position(GTK_PANED(lookup_widget(widget, "facingpages")),
                           config_file_get_int("Settings", "SliderPosition"));
    
    /* Create some submenus and attach them */
    GtkWidget *menu;
#ifndef SUCKY_GNOME
    if((menu = create_open_recent_submenu()))
        gtk_menu_item_set_submenu(
          GTK_MENU_ITEM(lookup_widget(widget, "open_recent")), menu);
#else
    gtk_widget_hide(lookup_widget(widget, "open_recent"));
#endif /* SUCKY_GNOME */
    if((menu = create_open_extension_submenu()))
        gtk_menu_item_set_submenu(
          GTK_MENU_ITEM(lookup_widget(widget, "open_extension")), menu);
    
    /* Create a text buffer for the Progress, Debugging and I6 text views */
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "compiler_output_l")), buffer);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "compiler_output_r")), buffer);
    buffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "debugging_l")), buffer);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "debugging_r")), buffer);
    GtkSourceBuffer *i6buffer = create_inform6_source_buffer();
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "inform6_l")), GTK_TEXT_BUFFER(i6buffer));
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(lookup_widget(widget,
      "inform6_r")), GTK_TEXT_BUFFER(i6buffer));
        
    /* Set the Errors/Progress to a monospace font */
    PangoFontDescription *font = pango_font_description_new();
    pango_font_description_set_family(font, "DejaVu Sans Mono,"
      "DejaVu LGC Sans Mono,Bitstream Vera Sans Mono,Courier New,Luxi Mono,"
      "Monospace");
    pango_font_description_set_size(font, 10 * PANGO_SCALE);
    gtk_widget_modify_font(lookup_widget(widget, "compiler_output_l"), font);
    gtk_widget_modify_font(lookup_widget(widget, "compiler_output_r"), font);
    pango_font_description_free(font);
    
    /* Attach the spelling checkers to the source views and ensure the correct
    state of the menu */
    if(config_file_get_bool("Settings", "SpellCheck"))
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(widget,
          "autocheck_spelling")), TRUE);
    else
        gtk_widget_set_sensitive(lookup_widget(widget, "check_spelling"),FALSE);

    /* Add extra pages in "Errors" if the user has them turned on */
    if(config_file_get_bool("Debugging", "ShowLog"))
        add_debug_tabs(widget);
    
    /* Load the documentation index page in "Documentation" */
    gchar *htmlfile = get_datafile_path_va("Documentation", "index.html", NULL);
    html_load_file(GTK_HTML(lookup_widget(widget, "docs_l")), htmlfile);
    html_load_file(GTK_HTML(lookup_widget(widget, "docs_r")), htmlfile);
    
    /* Turn the left page to "Source"  and the right page to "Documentation" */
    Story *thestory = get_story(widget);
    history_block_handlers(thestory, LEFT);
    gtk_notebook_set_current_page(get_notebook(widget, LEFT),
      TAB_SOURCE);
    thestory->current[LEFT] = g_new0(History, 1);
    thestory->current[LEFT]->tab = TAB_SOURCE;
    history_unblock_handlers(thestory, LEFT);
    history_block_handlers(thestory, RIGHT);
    gtk_notebook_set_current_page(get_notebook(widget, RIGHT),
      TAB_DOCUMENTATION);
    thestory->current[RIGHT] = g_new0(History, 1);
    thestory->current[RIGHT]->tab = TAB_DOCUMENTATION;
    thestory->current[RIGHT]->page = g_strdup(htmlfile);
    g_free(htmlfile);
    history_unblock_handlers(thestory, RIGHT);
    
    /* Create empty menus for the Headings buttons so they become active */
    gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(lookup_widget(widget,
                                  "source_headings_l")), gtk_menu_new());
    gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(lookup_widget(widget,
                                  "source_headings_r")), gtk_menu_new());
        
    /* Show the inspector window if necessary */
    if(config_file_get_bool("Settings", "InspectorVisible")) {
        extern GtkWidget *inspector_window;
        gtk_widget_show(inspector_window);
    }
}

/* Whenever a main window receives the focus, make the inspector display that
   story's data */
gboolean
on_app_window_focus_in_event           (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    refresh_inspector(get_story(widget));
    return FALSE;
}

void
on_back_l_clicked                      (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    go_back(get_story(GTK_WIDGET(toolbutton)), LEFT);
}


void
on_forward_l_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    go_forward(get_story(GTK_WIDGET(toolbutton)), LEFT);
}


void
on_back_r_clicked                      (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    go_back(get_story(GTK_WIDGET(toolbutton)), RIGHT);
}


void
on_forward_r_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    go_forward(get_story(GTK_WIDGET(toolbutton)), RIGHT);
}


void
after_notebook_l_switch_page           (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
    Story *thestory = get_story(GTK_WIDGET(notebook));
    switch(page_num) {
        case TAB_ERRORS:
            history_push_subtab(thestory, LEFT, page_num,
                                gtk_notebook_get_current_page(GTK_NOTEBOOK(
                                lookup_widget(GTK_WIDGET(notebook),
                                "errors_notebook_l"))));
            break;
        case TAB_INDEX:
            history_push_subtab(thestory, LEFT, page_num,
                                gtk_notebook_get_current_page(GTK_NOTEBOOK(
                                lookup_widget(GTK_WIDGET(notebook),
                                "index_notebook_l"))));
            break;
        case TAB_DOCUMENTATION:
            /* If we're switching to the documentation tab, then we have no URL
            to push. In that case we must be displaying the last one from the
            history. */
            history_push_docpage(thestory, LEFT,
                                 history_get_last_docpage(thestory, LEFT));
            break;
        default:
            history_push_tab(thestory, LEFT, page_num);
    }
#ifdef I_LIKE_SKEIN
    /* Redraw the skein if we are switching to it and it needs updating */
    if(page_num == TAB_SKEIN && thestory->drawflag[SKEIN_LEFT])
        skein_schedule_redraw(thestory->theskein, thestory);
#endif /* I_LIKE_SKEIN */
}


void
after_errors_notebook_l_switch_page    (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
    if(gtk_notebook_get_current_page(get_notebook(GTK_WIDGET(notebook), LEFT))
       == TAB_ERRORS)
        history_push_subtab(get_story(GTK_WIDGET(notebook)), LEFT, TAB_ERRORS,
                            page_num);
}


void
after_index_notebook_l_switch_page     (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
    if(gtk_notebook_get_current_page(get_notebook(GTK_WIDGET(notebook), LEFT))
       == TAB_INDEX)
        history_push_subtab(get_story(GTK_WIDGET(notebook)), LEFT, TAB_INDEX,
                            page_num);
}


void
after_notebook_r_switch_page           (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
    Story *thestory = get_story(GTK_WIDGET(notebook));
    switch(page_num) {
        case TAB_ERRORS:
            history_push_subtab(thestory, RIGHT, page_num,
                                gtk_notebook_get_current_page(GTK_NOTEBOOK(
                                lookup_widget(GTK_WIDGET(notebook),
                                "errors_notebook_r"))));
            break;
        case TAB_INDEX:
            history_push_subtab(thestory, RIGHT, page_num,
                                gtk_notebook_get_current_page(GTK_NOTEBOOK(
                                lookup_widget(GTK_WIDGET(notebook),
                                "index_notebook_r"))));
            break;
        case TAB_DOCUMENTATION:
            /* If we're switching to the documentation tab, then we have no URL
            to push. In that case we must be displaying the last one from the
            history. */
            history_push_docpage(thestory, RIGHT,
                                 history_get_last_docpage(thestory, RIGHT));
            break;
        default:
            history_push_tab(thestory, RIGHT, page_num);
    }
#ifdef I_LIKE_SKEIN
    /* Redraw the skein if we are switching to it and it needs updating */
    if(page_num == TAB_SKEIN && thestory->drawflag[SKEIN_RIGHT])
        skein_schedule_redraw(thestory->theskein, thestory);
#endif /* I_LIKE_SKEIN */
}


void
after_errors_notebook_r_switch_page    (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
    if(gtk_notebook_get_current_page(get_notebook(GTK_WIDGET(notebook), RIGHT))
       == TAB_ERRORS)
        history_push_subtab(get_story(GTK_WIDGET(notebook)), RIGHT, TAB_ERRORS,
                            page_num);
}


void
after_index_notebook_r_switch_page     (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
    if(gtk_notebook_get_current_page(get_notebook(GTK_WIDGET(notebook), RIGHT))
       == TAB_INDEX)
        history_push_subtab(get_story(GTK_WIDGET(notebook)), RIGHT, TAB_INDEX,
                            page_num);
}


/* Save window size and slider position */
static void
save_app_window_size(GtkWindow *window)
{
    gint w, h;
    gtk_window_get_size(window, &w, &h);
    config_file_set_int("Settings", "AppWindowWidth", w);
    config_file_set_int("Settings", "AppWindowHeight", h);
    config_file_set_int("Settings", "SliderPosition",
                        gtk_paned_get_position(
                        GTK_PANED(lookup_widget(GTK_WIDGET(window),
                        "facingpages"))));
}


gboolean
on_app_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    if(verify_save(widget)) {
        Story *thestory = get_story(GTK_WIDGET(widget));

        if(get_num_app_windows() == 1) {
            GtkWidget *dialog = gtk_message_dialog_new(
              GTK_WINDOW(thestory->window), GTK_DIALOG_DESTROY_WITH_PARENT,
              GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Quit GNOME Inform 7?");
            gint result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            if(result != GTK_RESPONSE_YES)
                return TRUE;
        }

        save_app_window_size(GTK_WINDOW(widget));
        delete_story(thestory);

        if(get_num_app_windows() == 0) {
            gtk_main_quit();
        }
        return FALSE;
    }
    return TRUE;
}

/* This function is called when our app_window is involuntarily destroyed while
the story has not been freed. It will ask if we want to save, but not offer a
choice to cancel. */
void
on_app_window_destroy                  (GtkObject       *object,
                                        gpointer         user_data)
{
    Story *thestory = get_story(GTK_WIDGET(object));
    if(thestory == NULL)
        return;
    
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(thestory->buffer))
      || gtk_text_buffer_get_modified(thestory->notes)) {
        gchar *filename = g_path_get_basename(thestory->filename);
        GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(
          GTK_WINDOW(thestory->window),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_NONE,
          "<b><big>Save changes to project '%s' before closing?</big></b>",
          filename);
        gtk_message_dialog_format_secondary_text(
          GTK_MESSAGE_DIALOG(save_changes_dialog),
          "If you don't save, your changes will be lost.");
        gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
          "Close _without saving", GTK_RESPONSE_REJECT,
          GTK_STOCK_SAVE, GTK_RESPONSE_OK,
          NULL);
        gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
        gtk_widget_destroy(save_changes_dialog);
        if(result == GTK_RESPONSE_OK)
            on_save_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(object),
            "save")), NULL);
        g_free(filename);
    }
    
    save_app_window_size(GTK_WINDOW(object));
    delete_story(thestory);
}
