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
#include <gtkspell/gtkspell.h>
#include <gtksourceview/gtksourceview.h>

#include "interface.h"
#include "support.h"

#include "appmenu.h"
#include "appwindow.h"
#include "configfile.h"
#include "error.h"
#include "extension.h"
#include "extwindow.h"
#include "file.h"
#include "findreplace.h"
#include "prefs.h"
#include "windowlist.h"
#include "tabsource.h"

/* Callbacks for the extension editing window; most of them just call the
callbacks for the main window */

void
after_ext_window_realize(GtkWidget *widget, gpointer data)
{
    /* Set the last saved window size and slider position */
    gtk_window_resize(GTK_WINDOW(widget), 
      config_file_get_int("WindowSettings", "ExtWindowWidth"),
      config_file_get_int("WindowSettings", "ExtWindowHeight"));
    
    /* Create some submenus and attach them */
    GtkWidget *menu;
#ifndef SUCKY_GNOME
    if((menu = create_open_recent_submenu()))
        gtk_menu_item_set_submenu(
          GTK_MENU_ITEM(lookup_widget(widget, "xopen_recent")), menu);
#else
    gtk_widget_hide(lookup_widget(widget, "xopen_recent"));
#endif /* SUCKY_GNOME */
    if((menu = create_open_extension_submenu()))
        gtk_menu_item_set_submenu(
          GTK_MENU_ITEM(lookup_widget(widget, "xopen_extension")), menu);
    
    /* Attach the spelling checker to the source view and ensure the correct
    state of the menu */
    if(config_file_get_bool("IDESettings", "SpellCheckDefault"))
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(widget,
          "xautocheck_spelling")), TRUE);
    else
        gtk_widget_set_sensitive(lookup_widget(widget,"xcheck_spelling"),FALSE);
}

void
on_xclose_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));

    /* If this was the last window open, ask if we really want to quit */
    if(verify_save_ext(GTK_WIDGET(menuitem))) {
        if(get_num_app_windows() == 1 && do_quit_dialog() != GTK_RESPONSE_YES)
            return;
        delete_ext(ext);

        if(get_num_app_windows() == 0)
            gtk_main_quit();
    }
}

void
on_xsave_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));

    if(ext->filename == NULL)
        on_xsave_as_activate(menuitem, data);
    else
        save_extension(GTK_WIDGET(menuitem));
}

void
on_xsave_as_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));
        
    /* Create a file chooser for saving the extension */
    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save File"),
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(dialog),
      TRUE);

    if (ext->filename == NULL) {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
          _("Untitled document"));
    } else
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),     
          ext->filename);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *path = gtk_file_chooser_get_filename(
          GTK_FILE_CHOOSER(dialog));
        set_ext_filename(ext, path);
        save_extension(GTK_WIDGET(menuitem));
        g_free(path);
    }

    gtk_widget_destroy (dialog);
}


void
on_xrevert_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));
    
    if(ext->filename == NULL)
        return; /* No saved version to revert to */        
    if(!gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(ext->buffer)))
        return; /* Text has not changed since last save */
    
    /* Ask if the user is sure */
    GtkWidget *revert_dialog = gtk_message_dialog_new_with_markup(
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
      GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
      _("Are you sure you want to revert to the last saved version?"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(revert_dialog),
      _("All unsaved changes will be lost."));
    gtk_dialog_add_buttons(GTK_DIALOG(revert_dialog),
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_REVERT_TO_SAVED, GTK_RESPONSE_OK,
      NULL);
    gint result = gtk_dialog_run(GTK_DIALOG(revert_dialog));
    gtk_widget_destroy(revert_dialog);
    if(result != GTK_RESPONSE_OK)
        return; /* Only go on if the user clicked revert */

    /* Store the filename, close the window and reopen it */
    gchar *filename = g_strdup(ext->filename);
    delete_ext(ext);
    ext = open_extension(filename);
    g_free(filename);
    gtk_widget_show(ext->window);
}


void
on_xundo_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));

    if(gtk_source_buffer_can_undo(ext->buffer))
        gtk_source_buffer_undo(ext->buffer);
}

void
on_xredo_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));

    if(gtk_source_buffer_can_redo(ext->buffer))
        gtk_source_buffer_redo(ext->buffer);
}

void
on_xcut_activate(GtkMenuItem *menuitem, gpointer data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "ext_code")),
      "cut-clipboard", NULL);
}

void
on_xcopy_activate(GtkMenuItem *menuitem, gpointer data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "ext_code")),
      "copy-clipboard", NULL);
}

void
on_xpaste_activate(GtkMenuItem *menuitem, gpointer data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "ext_code")),
      "paste-clipboard", NULL);
}

void
on_xselect_all_activate(GtkMenuItem *menuitem, gpointer data)
{
    g_signal_emit_by_name(
      G_OBJECT(lookup_widget(GTK_WIDGET(menuitem), "ext_code")),
      "select-all", TRUE, NULL);
}

void
on_xfind_activate(GtkMenuItem *menuitem, gpointer data)
{
    gpointer ext = (gpointer)get_ext(GTK_WIDGET(menuitem));
    GtkWidget *dialog = create_find_dialog();
    /* Do the same thing as in the main window, but with different callbacks
    that expect an extension struct instead of a story */
    g_signal_connect((gpointer)lookup_widget(dialog, "find_next"),
      "clicked", G_CALLBACK(on_xfind_next_clicked), ext);
    g_signal_connect((gpointer)lookup_widget(dialog, "find_previous"),
      "clicked", G_CALLBACK(on_xfind_previous_clicked), ext);
    g_signal_connect((gpointer)lookup_widget(dialog, "find_replace_find"),
      "clicked", G_CALLBACK(on_xfind_replace_find_clicked), ext);
    g_signal_connect((gpointer)lookup_widget(dialog, "find_replace_all"),
      "clicked", G_CALLBACK(on_xfind_replace_all_clicked), ext);
    gtk_widget_show(dialog);
}


void
on_xautocheck_spelling_activate(GtkMenuItem *menuitem, gpointer data)
{
    gboolean check = gtk_check_menu_item_get_active(
      GTK_CHECK_MENU_ITEM(menuitem));
    gtk_widget_set_sensitive(
      lookup_widget(GTK_WIDGET(menuitem), "xcheck_spelling"), check);
    config_file_set_bool("IDESettings", "SpellCheckDefault", check);
    /* Set the default for new windows to whatever the user chose last */
    
    if(check) {
        GError *err = NULL;
        if(!gtkspell_new_attach(
          GTK_TEXT_VIEW(lookup_widget(GTK_WIDGET(menuitem), "ext_code")),
          NULL, &err))
            error_dialog(
              GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(menuitem))),
              err, _("Error initializing spell checking: "));
    } else
        gtkspell_detach(gtkspell_get_from_text_view(
          GTK_TEXT_VIEW(lookup_widget(GTK_WIDGET(menuitem), "ext_code"))));
}


void
on_xcheck_spelling_activate(GtkMenuItem *menuitem, gpointer data)
{
    GtkSpell *spellchecker = gtkspell_get_from_text_view(
      GTK_TEXT_VIEW(lookup_widget(GTK_WIDGET(menuitem), "ext_code")));
    if(spellchecker)
        gtkspell_recheck_all(spellchecker);
}

void
on_xshift_selection_right_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));
    shift_selection_right(GTK_TEXT_BUFFER(ext->buffer));
}

void
on_xshift_selection_left_activate(GtkMenuItem *menuitem, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(menuitem));
    shift_selection_left(GTK_TEXT_BUFFER(ext->buffer));
}

/* Save window size */
static void
save_ext_window_size(GtkWindow *window)
{
    gint w, h;
    gtk_window_get_size(window, &w, &h);
    config_file_set_int("WindowSettings", "ExtWindowWidth", w);
    config_file_set_int("WindowSettings", "ExtWindowHeight", h);
}

gboolean
on_ext_window_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    if(verify_save_ext(widget)) {
        Extension *ext = get_ext(GTK_WIDGET(widget));

        if(get_num_app_windows() == 1 && do_quit_dialog() != GTK_RESPONSE_YES)
            return TRUE;

        save_ext_window_size(GTK_WINDOW(widget));
        delete_ext(ext);

        if(get_num_app_windows() == 0) {
            gtk_main_quit();
        }
        return FALSE;
    }
    return TRUE;
}


/* This function is called when the window is about to be destroyed and we can't
do anything about it. It asks whether we want to save, but doesn't offer the
option to cancel. */
void
on_ext_window_destroy(GtkObject *object, gpointer data)
{
    Extension *ext = get_ext(GTK_WIDGET(object));
    if(ext == NULL)
        return;
    
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(ext->buffer))) {
        gchar *filename = g_path_get_basename(ext->filename);
        GtkWidget *save_changes_dialog = gtk_message_dialog_new_with_markup(
          GTK_WINDOW(ext->window),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_NONE,
          _("<b><big>Save changes to '%s' before closing?</big></b>"),
          filename);
        gtk_message_dialog_format_secondary_text(
          GTK_MESSAGE_DIALOG(save_changes_dialog),
          _("If you don't save, your changes will be lost."));
        gtk_dialog_add_buttons(GTK_DIALOG(save_changes_dialog),
          _("Close _without saving"), GTK_RESPONSE_REJECT,
          GTK_STOCK_SAVE, GTK_RESPONSE_OK,
          NULL);
        gint result = gtk_dialog_run(GTK_DIALOG(save_changes_dialog));
        gtk_widget_destroy(save_changes_dialog);
        if(result == GTK_RESPONSE_OK)
            on_save_activate(GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(object),
            "save")), NULL);
        g_free(filename);
    }
    
    save_ext_window_size(GTK_WINDOW(object));
    delete_ext(ext);
}

/* Scroll to the requested line number. */
void 
jump_to_line_ext(GtkWidget *widget, gint line) 
{
    Extension *ext = get_ext(widget);
    GtkTextIter cursor, line_end;
        
    gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(ext->buffer), &cursor,
      line - 1); /* line is counted from 0 */
    line_end = cursor;
    gtk_text_iter_forward_to_line_end(&line_end);
    gtk_text_buffer_select_range(GTK_TEXT_BUFFER(ext->buffer), &cursor,
      &line_end);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(lookup_widget(ext->window,
      "ext_code")),
    gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(ext->buffer)),
      0.25, FALSE, 0.0, 0.0);
}
