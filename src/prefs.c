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
 
#include <ctype.h>
#include <gnome.h>
#include <glib/gstdio.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>
#include <pango/pango-font.h>

#include "prefs.h"
#include "tabsource.h"
#include "support.h"
#include "configfile.h"
#include "story.h"
#include "extension.h"
#include "file.h"
#include "appwindow.h"
#include "error.h"
#include "colorscheme.h"

/* Check whether the user has selected something (not an author name) that can
be removed, and if so, enable the remove button */
static void extension_browser_selection_changed(GtkTreeSelection *selection,
gpointer data) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        gtk_widget_set_sensitive(
          lookup_widget((GtkWidget *)data, "prefs_i7_extension_remove"),
          (gtk_tree_path_get_depth(path) == 2));
          /* Only enable the "Remove" button if the selection is 2 deep */
        gtk_tree_path_free(path);
    }
}

void
on_prefs_dialog_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GtkWidget *trash;
    gchar *scratch;
    
    /* Set all the controls to their current values according to GConf */
    trash = lookup_widget(widget, "prefs_font_set");
    gtk_combo_box_set_active(GTK_COMBO_BOX(trash),
      config_file_get_int("Fonts", "FontSet"));
    trash = lookup_widget(widget, "prefs_custom_font");
    scratch = config_file_get_string("Fonts", "CustomFont");
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(trash), scratch);
    g_free(scratch);
    trash = lookup_widget(widget, "prefs_font_styling");
    gtk_combo_box_set_active(GTK_COMBO_BOX(trash),
      config_file_get_int("Fonts", "FontStyling"));
    trash = lookup_widget(widget, "prefs_font_size");
    gtk_combo_box_set_active(GTK_COMBO_BOX(trash),
      config_file_get_int("Fonts", "FontSize"));
    trash = lookup_widget(widget, "prefs_change_colors");
    gtk_combo_box_set_active(GTK_COMBO_BOX(trash),
      config_file_get_int("Colors", "ChangeColors"));
    trash = lookup_widget(widget, "prefs_color_set");
    gtk_combo_box_set_active(GTK_COMBO_BOX(trash),
      config_file_get_int("Colors", "ColorSet"));
    trash = lookup_widget(widget, "prefs_change_colors");
    gtk_combo_box_set_active(GTK_COMBO_BOX(trash),
      config_file_get_int("Colors", "ChangeColors"));
    trash = lookup_widget(widget, "tab_ruler");
    gtk_range_set_value(GTK_RANGE(trash),
      (gdouble)config_file_get_int("Tabs", "TabWidth"));
      
    trash = lookup_widget(widget, "prefs_project_files_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "ProjectFiles"));
    trash = lookup_widget(widget, "prefs_notes_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "Notes"));
    trash = lookup_widget(widget, "prefs_headings_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "Headings"));
    trash = lookup_widget(widget, "prefs_skein_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "Skein"));
    trash = lookup_widget(widget, "prefs_watchpoints_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "Watchpoints"));
    trash = lookup_widget(widget, "prefs_breakpoints_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "Breakpoints"));
    trash = lookup_widget(widget, "prefs_search_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Inspectors", "Search"));
    
    trash = lookup_widget(widget, "prefs_enable_highlighting_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Syntax", "Highlighting"));
    trash = lookup_widget(widget, "prefs_indent_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Syntax", "Indenting"));
    trash = lookup_widget(widget, "prefs_follow_symbols_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Syntax", "Intelligence"));
    trash = lookup_widget(widget, "prefs_intelligent_inspector_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Syntax", "IntelligentIndexInspector"));
    trash = lookup_widget(widget, "prefs_auto_indent_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Syntax", "AutoIndent"));
    trash = lookup_widget(widget, "prefs_auto_number_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Syntax", "AutoNumberSections"));
    trash = lookup_widget(widget, "prefs_author");
    scratch = config_file_get_string("User", "Name");
    gtk_entry_set_text(GTK_ENTRY(trash), scratch);
    g_free(scratch);
    
    trash = lookup_widget(widget, "prefs_clean_build_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Cleaning", "BuildFiles"));
    trash = lookup_widget(widget, "prefs_clean_index_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Cleaning", "IndexFiles"));
    trash = lookup_widget(widget, "prefs_show_log_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Debugging", "ShowLog"));
    trash = lookup_widget(widget, "prefs_rebuild_compiler_toggle");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(trash),
      config_file_get_bool("Debugging", "RebuildCompiler"));
      
    populate_extension_lists(widget);
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      "", renderer, "text", 0, NULL); /* No title, text
      renderer, get the property "text" from column 0 */
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(lookup_widget(widget,
      "prefs_i7_extensions_view")), column);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(
      GTK_TREE_VIEW(lookup_widget(widget, "prefs_i7_extensions_view")));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed",
      G_CALLBACK(extension_browser_selection_changed), (gpointer)widget);
}


void
on_prefs_font_set_changed              (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    int setting = gtk_combo_box_get_active(combobox);
    config_file_set_int("Fonts", "FontSet", setting);
    
    update_font(lookup_widget(GTK_WIDGET(combobox), "source_example"));
    update_font(lookup_widget(GTK_WIDGET(combobox), "tab_example"));
}


void
on_prefs_custom_font_font_set          (GtkFontButton   *fontbutton,
                                        gpointer         user_data)
{
    /* Try to extract a font name from the string that we get from the button */
    gchar *fontname = g_strdup(gtk_font_button_get_font_name(fontbutton));
    /* do not free the original string */
    gchar *ptr = fontname;
    g_strreverse(ptr);
    while(isdigit(*ptr))   /* Remove the point size from the end */
        ptr++;
    while(isspace(*ptr))   /* Then the white space */
        ptr++;
    if(g_str_has_prefix(ptr, "cilatI ")) /* " Italic" */
        ptr += 7;
    if(g_str_has_prefix(ptr, "euqilbO ")) /* " Oblique" */
        ptr += 8;
    if(g_str_has_prefix(ptr, "dloB ")) /* " Bold" */
        ptr += 5;
    g_strreverse(ptr);
    
    config_file_set_string("Fonts", "CustomFont", ptr);
    update_font(lookup_widget(GTK_WIDGET(fontbutton), "source_example"));
    update_font(lookup_widget(GTK_WIDGET(fontbutton), "tab_example"));
    g_free(fontname);
}


void
on_prefs_font_styling_changed          (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    config_file_set_int("Fonts", "FontStyling",
      gtk_combo_box_get_active(combobox));
    update_style(GTK_SOURCE_VIEW(lookup_widget(GTK_WIDGET(combobox),
      "source_example")));
}


void
on_prefs_font_size_changed             (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    config_file_set_int("Fonts", "FontSize",
      gtk_combo_box_get_active(combobox));
    update_font(lookup_widget(GTK_WIDGET(combobox), "source_example"));
    update_font(lookup_widget(GTK_WIDGET(combobox), "tab_example"));
}


void
on_prefs_change_colors_changed         (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    config_file_set_int("Colors", "ChangeColors",
      gtk_combo_box_get_active(combobox));
    update_style(GTK_SOURCE_VIEW(lookup_widget(GTK_WIDGET(combobox),
      "source_example")));
}


void
on_prefs_color_set_changed             (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    config_file_set_int("Colors", "ColorSet",
      gtk_combo_box_get_active(combobox));
    update_style(GTK_SOURCE_VIEW(lookup_widget(GTK_WIDGET(combobox),
      "source_example")));
}

/* Create a GtkSourceView and -Buffer and fill it with the example text */
GtkWidget*
source_example_create (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
    GtkSourceBuffer *buffer = create_natural_inform_source_buffer();
    GtkWidget *source = gtk_source_view_new_with_buffer(buffer);
    gtk_widget_set_name(source, widget_name);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(source), GTK_WRAP_WORD);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer),
      "\nPart One - the Wharf\n\nThe Customs Wharf is a room. [change the "
      "description if the cask is open] \"Amid the bustle of the quayside, [if "
      "the case is open]many eyes stray to your broached cask. "
      "[otherwise]nobody takes much notice of a man heaving a cask about. "
      "[end if]Sleek gondolas jostle at the plank pier.\"", -1);
    return source;
}

/* Create another GtkSourceView with examples of tab stops */
GtkWidget*
tab_example_create (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
    GtkWidget *source = gtk_source_view_new();
    gtk_widget_set_name(source, widget_name);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source));
    gtk_text_buffer_set_text(buffer, "\tTab\tTab\tTab\tTab\tTab\tTab\tTab", -1);
    update_font(source);
    update_tabs(GTK_SOURCE_VIEW(source));
    return source;
}

void
on_tab_ruler_value_changed             (GtkRange        *range,
                                        gpointer         user_data)
{
    gint spaces = (gint)gtk_range_get_value(range);
    config_file_set_int("Tabs", "TabWidth", spaces);
    update_tabs(
      GTK_SOURCE_VIEW(lookup_widget(GTK_WIDGET(range), "tab_example")));
    update_tabs(
      GTK_SOURCE_VIEW(lookup_widget(GTK_WIDGET(range), "source_example")));
}


gchar*
on_tab_ruler_format_value              (GtkScale        *scale,
                                        gdouble          value,
                                        gpointer         user_data)
{
    if(value)
        return g_strdup_printf("%.*f spaces", gtk_scale_get_digits(scale),
          value);
    return g_strdup("default");
}



void
on_prefs_project_files_toggle_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "ProjectFiles",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_notes_toggle_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "Notes",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_headings_toggle_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "Headings",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_skein_toggle_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "Skein",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_watchpoints_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "Watchpoints",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_breakpoints_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "Breakpoints",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_search_toggle_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Inspectors", "Search",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_i7_extension_add_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Select the extensions to install",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
      GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    
    if (gtk_dialog_run(GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }
    GSList *extlist = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
    GSList *iter;
    
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter)) {
        install_extension((gchar *)iter->data);
        g_free(iter->data);
    }
    
    g_slist_free(extlist);
    gtk_widget_destroy(dialog);
    
    populate_extension_lists(GTK_WIDGET(button));
}


void
on_prefs_i7_extension_remove_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkTreeView *view = GTK_TREE_VIEW(lookup_widget(GTK_WIDGET(button),
      "prefs_i7_extensions_view"));
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        
        if(gtk_tree_path_get_depth(path) != 2) {
            gtk_tree_path_free(path);
            return;
        }
        
        gchar *extname;
        gtk_tree_model_get(model, &iter, 0, &extname, -1);
        gchar *author;
        gtk_tree_path_up(path);
        gtk_tree_model_get_iter(model, &iter, path);
        gtk_tree_model_get(model, &iter, 0, &author, -1);
        
        GtkWidget *dialog = gtk_message_dialog_new(
          GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
          GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
          GTK_BUTTONS_YES_NO, "Are you sure you want to remove %s by %s?", 
          extname, author);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        gchar *filename = get_extension_path(author, extname);
        if(g_remove(filename) == -1) {
            GtkWidget *dialog = gtk_message_dialog_new(
              GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
              GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
              "There was an error removing %s.", extname);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            g_free(filename);
            g_free(extname);
            g_free(author);
            gtk_tree_path_free(path);
            return;
        }
        g_free(filename);
        
        filename = get_extension_path(author, NULL); 
        if(g_rmdir(filename) == -1 && errno != ENOTEMPTY) {
            /* if it failed for any other reason than that there were other
            extensions in the directory */
            GtkWidget *dialog = gtk_message_dialog_new(
              GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
              GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
              GTK_BUTTONS_OK, "There was an error removing %s.", extname);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            /* here we are at the end of the function anyway so don't bother
            freeing and returning */
        }
        g_free(filename);
        g_free(extname);
        g_free(author);
        gtk_tree_path_free(path);
    }
    
    populate_extension_lists(GTK_WIDGET(button));
}


void
on_prefs_i6_extension_add_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
    error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))), NULL,
      "Inform 6 extensions are not implemented in this version of "
      "GNOME Inform 7.");
}


void
on_prefs_i6_extension_remove_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_prefs_enable_highlighting_toggle_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gboolean state = gtk_toggle_button_get_active(togglebutton);
    config_file_set_bool("Syntax", "Highlighting", state);
    /* make the other checkboxes dependent on this checkbox active or inactive*/
#if 0 /* Not implemented in this version */
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_indent_toggle"), state);
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_follow_symbols_toggle"), state);
    
    if(state)
        state = gtk_toggle_button_get_active(
          GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(togglebutton),
          "prefs_follow_symbols_toggle")));
    /* If the "enable_highlighting" toggle is true, then these three still
    depend on the status of the "follow_symbols" toggle */

    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_intelligent_inspector_toggle"), state);
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_auto_indent_toggle"), state);
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_auto_number_toggle"), state);
#endif
    
    for_each_story_buffer(&update_source_highlight);
    for_each_extension_buffer(&update_source_highlight);
}


void
on_prefs_indent_toggle_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Syntax", "Indenting",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_follow_symbols_toggle_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gboolean state = gtk_toggle_button_get_active(togglebutton);
    config_file_set_bool("Syntax", "Intelligence", state);
#if 0 /* Not implemented in this version */
    /* make the other checkboxes dependent on this checkbox active or inactive*/
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_intelligent_inspector_toggle"), state);
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_auto_indent_toggle"), state);
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_auto_number_toggle"), state);
#endif
}


void
on_prefs_intelligent_inspector_toggle_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Syntax", "IntelligentIndexInspector",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_auto_indent_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Syntax", "AutoIndent",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_auto_number_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Syntax", "AutoNumberSections",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_author_changed                (GtkEditable     *editable,
                                        gpointer         user_data)
{
    config_file_set_string("User", "Name",
      gtk_entry_get_text(GTK_ENTRY(editable)));
    /* do not free the string */
}


void
on_prefs_clean_build_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gboolean state = gtk_toggle_button_get_active(togglebutton);
    config_file_set_bool("Cleaning", "BuildFiles", state);
    /* make the other checkboxes dependent on this checkbox active or inactive*/
    gtk_widget_set_sensitive(lookup_widget(GTK_WIDGET(togglebutton),
      "prefs_clean_index_toggle"), state);
}


void
on_prefs_clean_index_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Cleaning", "IndexFiles",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_show_log_toggle_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gboolean state = gtk_toggle_button_get_active(togglebutton);
    config_file_set_bool("Debugging", "ShowLog", state);
    
    for_each_story_window(state? add_debug_tabs : remove_debug_tabs);
}


void
on_prefs_rebuild_compiler_toggle_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    config_file_set_bool("Debugging", "RebuildCompiler",
      gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_close_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    /* Do the font updating when the dialog closes, and not on the fly like the
    other settings */
    for_each_story_window(&update_app_window_fonts);
    for_each_extension_window(&update_ext_window_fonts);
    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}


gboolean
on_prefs_dialog_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    /* Do the font updating when the dialog closes, and not on the fly like the
    other settings */
    for_each_story_window(&update_app_window_fonts);
    for_each_extension_window(&update_ext_window_fonts);
    return FALSE; /* Propagate the signal further */
}

/* Get the language associated with this sourceview and update the highlighting
style */
void update_style(GtkSourceView *thiswidget) {
    GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(thiswidget)));
    /* Do not unreference the language */
    GtkSourceLanguage *language = gtk_source_buffer_get_language(buffer);
    set_highlight_styles(language);
}

/* Change the font that this widget uses */
void update_font(GtkWidget *thiswidget) {
    PangoFontDescription *font = pango_font_description_new();
    gchar *customfont;
    
    int setting = config_file_get_int("Fonts", "FontSet");
    switch(setting) {
        case FONT_SET_PROGRAMMER:
            pango_font_description_set_family(font, "Bitstream Vera Sans Mono,"
              "Monospace,Courier New");
            break;
        case FONT_SET_CUSTOM:
            customfont = config_file_get_string("Fonts", "CustomFont");
            pango_font_description_set_family(font, customfont);
            g_free(customfont);
            break;
        case FONT_SET_STANDARD:
        default:
            pango_font_description_set_family(font, "Bitstream Vera Sans,"
              "Sans,Arial");
    }
    
    int size = config_file_get_int("Fonts", "FontSize");
    switch(size) {
        case FONT_SIZE_MEDIUM:
            pango_font_description_set_size(font, SIZE_MEDIUM * PANGO_SCALE);
            break;
        case FONT_SIZE_LARGE:
            pango_font_description_set_size(font, SIZE_LARGE * PANGO_SCALE);
            break;
        case FONT_SIZE_HUGE:
            pango_font_description_set_size(font, SIZE_HUGE * PANGO_SCALE);
            break;
        case FONT_SIZE_STANDARD:
        default:
            pango_font_description_set_size(font, SIZE_STANDARD * PANGO_SCALE);
    }
    
    gtk_widget_modify_font(thiswidget, font);
    pango_font_description_free(font);
}

/* Change only the font size but not the face that this widget uses */
void update_font_size(GtkWidget *thiswidget) {
    PangoFontDescription *font = pango_font_description_new();

    int size = config_file_get_int("Fonts", "FontSize");
    switch(size) {
        case FONT_SIZE_MEDIUM:
            pango_font_description_set_size(font, SIZE_MEDIUM * PANGO_SCALE);
            break;
        case FONT_SIZE_LARGE:
            pango_font_description_set_size(font, SIZE_LARGE * PANGO_SCALE);
            break;
        case FONT_SIZE_HUGE:
            pango_font_description_set_size(font, SIZE_HUGE * PANGO_SCALE);
            break;
        case FONT_SIZE_STANDARD:
        default:
            pango_font_description_set_size(font, SIZE_STANDARD * PANGO_SCALE);
    }
    
    gtk_widget_modify_font(thiswidget, font);
    pango_font_description_free(font);
}

/* Update the tab stops for a GtkSourceView */
void update_tabs(GtkSourceView *thiswidget) {
    gint spaces = config_file_get_int("Tabs", "TabWidth");
    if(spaces == 0)
        spaces = 8; /* default is 8 */
    gtk_source_view_set_tabs_width(thiswidget, spaces);
}

/* Look in the user's extensions directory and list all the extensions there in
the treeview widget */
void populate_extension_lists(GtkWidget *thiswidget) {
    GError *err = NULL;
    gchar *extension_dir = get_extension_path(NULL, NULL);
    GDir *extensions = g_dir_open(extension_dir, 0, &err);
    g_free(extension_dir);
    if(err) {
        error_dialog(GTK_WINDOW(thiswidget), err, "Error opening extensions "
          "directory: ");
        return;
    }
    
    GtkTreeView *view = GTK_TREE_VIEW(lookup_widget(thiswidget,
      "prefs_i7_extensions_view"));
    GtkTreeStore *store = gtk_tree_store_new(1, G_TYPE_STRING);
    GtkTreeIter parent_iter, child_iter;
    
    const gchar *dir_entry;
    while((dir_entry = g_dir_read_name(extensions)) != NULL
      && strcmp(dir_entry, "Reserved")) {
        /* Read each extension dir, but skip "Reserved" */
        gtk_tree_store_append(store, &parent_iter, NULL);
        gtk_tree_store_set(store, &parent_iter, 0, dir_entry, -1);
        gchar *author_dir = get_extension_path(dir_entry, NULL);
        GDir *author = g_dir_open(author_dir, 0, &err);
        g_free(author_dir);
        if(err) {
            error_dialog(GTK_WINDOW(thiswidget), err, "Error opening extensions"
              " directory: ");
            g_free(extension_dir);
            return;
        }
        const gchar *author_entry;
        while((author_entry = g_dir_read_name(author)) != NULL) {
            gtk_tree_store_append(store, &child_iter, &parent_iter);
            gtk_tree_store_set(store, &child_iter, 0, author_entry, -1);
        }
        g_dir_close(author);
    }
    g_dir_close(extensions);    
    
    gtk_tree_view_set_model(view, GTK_TREE_MODEL(store));
}

/*
 * Here are the for_each_bla bla bla functions for changing settings in
 * each window
 */

/* Update the fonts and highlighting colors in this extension editing window */
void update_ext_window_fonts(GtkWidget *window) {
    GtkWidget *widget = lookup_widget(window, "ext_code");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    update_style(GTK_SOURCE_VIEW(widget));
}

/* Update the fonts and highlighting colors in this main window */
void update_app_window_fonts(GtkWidget *window) {
    GtkWidget *widget;

    widget = lookup_widget(window, "source_l");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    update_style(GTK_SOURCE_VIEW(widget));
    widget = lookup_widget(window, "source_r");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    update_style(GTK_SOURCE_VIEW(widget));
    
    widget = lookup_widget(window, "problems_l");
    update_font_size(widget);
    widget = lookup_widget(window, "problems_r");
    update_font_size(widget);
    widget = lookup_widget(window, "actions_l");
    update_font_size(widget);
    widget = lookup_widget(window, "actions_r");
    update_font_size(widget);
    widget = lookup_widget(window, "contents_l");
    update_font_size(widget);
    widget = lookup_widget(window, "contents_r");
    update_font_size(widget);
    widget = lookup_widget(window, "kinds_l");
    update_font_size(widget);
    widget = lookup_widget(window, "kinds_r");
    update_font_size(widget);
    widget = lookup_widget(window, "phrasebook_l");
    update_font_size(widget);
    widget = lookup_widget(window, "phrasebook_r");
    update_font_size(widget);
    widget = lookup_widget(window, "rules_l");
    update_font_size(widget);
    widget = lookup_widget(window, "rules_r");
    update_font_size(widget);
    widget = lookup_widget(window, "scenes_l");
    update_font_size(widget);
    widget = lookup_widget(window, "scenes_r");
    update_font_size(widget);
    widget = lookup_widget(window, "world_l");
    update_font_size(widget);
    widget = lookup_widget(window, "world_r");
    update_font_size(widget);
    widget = lookup_widget(window, "game_l");
    update_font_size(widget);
    widget = lookup_widget(window, "game_r");
    update_font_size(widget);
    widget = lookup_widget(window, "docs_l");
    update_font_size(widget);
    widget = lookup_widget(window, "docs_r");
    update_font_size(widget);
    
    /* Do the extra tabs too if the user has them switched on */
    if(config_file_get_bool("Debugging", "ShowLog")) {
        widget = gtk_bin_get_child(GTK_BIN(gtk_notebook_get_nth_page(
          GTK_NOTEBOOK(lookup_widget(window, "errors_notebook_l")),
          TAB_ERRORS_DEBUGGING)));
        update_font_size(widget);
        widget = gtk_bin_get_child(GTK_BIN(gtk_notebook_get_nth_page(
          GTK_NOTEBOOK(lookup_widget(window, "errors_notebook_r")),
          TAB_ERRORS_DEBUGGING)));
        update_font_size(widget);
        widget = gtk_bin_get_child(GTK_BIN(gtk_notebook_get_nth_page(
          GTK_NOTEBOOK(lookup_widget(window, "errors_notebook_l")),
          TAB_ERRORS_INFORM6)));
        update_font(widget);
        /*update_style(GTK_SOURCE_VIEW(widget));*/
        widget = gtk_bin_get_child(GTK_BIN(gtk_notebook_get_nth_page(
          GTK_NOTEBOOK(lookup_widget(window, "errors_notebook_r")),
          TAB_ERRORS_INFORM6)));
        update_font(widget);
        /*update_style(GTK_SOURCE_VIEW(widget));*/
    }
}

/* Turn source highlighting on or off in this source buffer */
void update_source_highlight(GtkSourceBuffer *buffer) {
    gtk_source_buffer_set_highlight(buffer, config_file_get_bool("Syntax",
      "Highlighting"));
}

/* Add the debugging tabs to this main window */
void add_debug_tabs(GtkWidget *window) {
    GtkWidget *notebook = lookup_widget(window, "errors_notebook_l");
    if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) != 2)
        return;
    
    GtkWidget *debugview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(debugview), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(debugview), FALSE);
    update_font_size(debugview);
    gtk_widget_show(debugview);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), debugview);
    gtk_widget_show(scroll);
    GtkTextBuffer *debugbuffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(debugview));
    GtkWidget *debuglabel = gtk_label_new("Debugging");
    gtk_widget_show(debuglabel);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll, debuglabel);
    
    GtkWidget *i6view = gtk_source_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(i6view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(i6view), FALSE);
    update_font(i6view);
    gtk_widget_show(i6view);
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), i6view);
    gtk_widget_show(scroll);
    GtkTextBuffer *i6buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(i6view));
    GtkWidget *i6label = gtk_label_new("Inform 6");
    gtk_widget_show(i6label);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll, i6label);
    
    notebook = lookup_widget(window, "errors_notebook_r");
    if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) != 2)
        return;
    
    debugview = gtk_text_view_new_with_buffer(debugbuffer);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(debugview), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(debugview), FALSE);
    update_font_size(debugview);
    gtk_widget_show(debugview);
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), debugview);
    gtk_widget_show(scroll);
    debuglabel = gtk_label_new("Debugging");
    gtk_widget_show(debuglabel);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll, debuglabel);
    
    i6view = gtk_source_view_new_with_buffer(GTK_SOURCE_BUFFER(i6buffer));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(i6view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(i6view), FALSE);
    update_font(i6view);
    gtk_widget_show(i6view);
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), i6view);
    gtk_widget_show(scroll);
    i6label = gtk_label_new("Inform 6");
    gtk_widget_show(i6label);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll, i6label);
    
    /* Set up the Inform 6 highlighting */
    GtkSourceLanguage *language;
    GtkSourceLanguagesManager *lmanager;
    GList ldirs;
    
    gchar *specfile = get_datafile_path("inform.lang");

    ldirs.data = g_path_get_dirname(specfile);
    ldirs.prev = NULL;
    ldirs.next = NULL;
    lmanager = GTK_SOURCE_LANGUAGES_MANAGER(g_object_new(
      GTK_TYPE_SOURCE_LANGUAGES_MANAGER, "lang_files_dirs", &ldirs, NULL));
    language = gtk_source_languages_manager_get_language_from_mime_type(
      lmanager, "text/x-inform");
    if(language != NULL) {
        set_highlight_styles(language);
		gtk_source_buffer_set_highlight(GTK_SOURCE_BUFFER(i6buffer), TRUE);
		gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(i6buffer), language);
    }
    g_object_unref((gpointer)lmanager);
}

/* Remove the debugging tabs from this window */
void remove_debug_tabs(GtkWidget *window) {
    GtkWidget *notebook = lookup_widget(window, "errors_notebook_l");
    if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) != 4)
        return;
    gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), -1);
    gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), -1);
    notebook = lookup_widget(window, "errors_notebook_r");
    if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) != 4)
        return;
    gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), -1);
    gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), -1);
}
