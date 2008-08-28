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
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcestyle.h>
#include <pango/pango-font.h>
#include <gconf/gconf-client.h>

#include "support.h"

#include "appwindow.h"
#include "colorscheme.h"
#include "configfile.h"
#include "datafile.h"
#include "error.h"
#include "extension.h"
#include "file.h"
#include "prefs.h"
#include "story.h"
#include "tabsource.h"

/* Global pointer to preferences window */
GtkWidget *prefs_dialog;

/* Look in the user's extensions directory and list all the extensions there in
the treeview widget */
static void 
populate_extension_lists(GtkWidget *thiswidget) 
{
    GError *err = NULL;
    gchar *extension_dir = get_extension_path(NULL, NULL);
    GDir *extensions = g_dir_open(extension_dir, 0, &err);
    g_free(extension_dir);
    if(err) {
        error_dialog(GTK_WINDOW(thiswidget), err, 
          _("Error opening extensions directory: "));
        return;
    }
    
    GtkTreeView *view = GTK_TREE_VIEW(lookup_widget(thiswidget,
      "prefs_i7_extensions_view"));
    GtkTreeStore *store = gtk_tree_store_new(1, G_TYPE_STRING);
    GtkTreeIter parent_iter, child_iter;
    
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
        /* Read each extension dir, but skip "Reserved", symlinks and nondirs*/
        gtk_tree_store_append(store, &parent_iter, NULL);
        gtk_tree_store_set(store, &parent_iter, 0, dir_entry, -1);
        gchar *author_dir = get_extension_path(dir_entry, NULL);
        GDir *author = g_dir_open(author_dir, 0, &err);
        g_free(author_dir);
        if(err) {
            error_dialog(GTK_WINDOW(thiswidget), err, 
              _("Error opening extensions directory: "));
            return;
        }
        const gchar *author_entry;
        while((author_entry = g_dir_read_name(author)) != NULL) {
            gchar *extname = get_extension_path(dir_entry, author_entry);
            if(g_file_test(extname, G_FILE_TEST_IS_SYMLINK)) {
                g_free(extname);
                continue;
            }
            g_free(extname);         
            /* Read each file, but skip symlinks */
            /* Remove .i7x from the filename, if it is there */
            gchar *displayname;
            if(g_str_has_suffix(author_entry, ".i7x"))
                displayname = g_strndup(author_entry, strlen(author_entry) - 4);
            else
                displayname = g_strdup(author_entry);
            gtk_tree_store_append(store, &child_iter, &parent_iter);
            gtk_tree_store_set(store, &child_iter, 0, displayname, -1);
            g_free(displayname);
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

/* Only update the tabs in an extension editing window */
gboolean 
update_ext_window_tabs(GtkWidget *widget) 
{
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(widget, "ext_code")));
    return FALSE; /* One-shot idle function */
}

/* Only update the tabs in this main window */
gboolean 
update_app_window_tabs(GtkWidget *widget) 
{
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(widget, "source_l")));
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(widget, "source_r")));
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(widget, "inform6_l")));
    update_tabs(GTK_SOURCE_VIEW(lookup_widget(widget, "inform6_r")));
    return FALSE; /* One-shot idle function */
}

/* Update the fonts and highlighting colors in this extension editing window */
gboolean 
update_ext_window_fonts(GtkWidget *extwindow) 
{
    GtkWidget *widget = lookup_widget(extwindow, "ext_code");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    update_style(GTK_SOURCE_VIEW(widget));
    return FALSE; /* One-shot idle function */
}

/* Update the fonts and highlighting colors in this main window, but not the
widgets that only need their font size updated */
gboolean 
update_app_window_fonts(GtkWidget *window) 
{
    GtkWidget *widget = lookup_widget(window, "source_l");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    update_style(GTK_SOURCE_VIEW(widget));
    widget = lookup_widget(window, "source_r");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    update_style(GTK_SOURCE_VIEW(widget));
    widget = lookup_widget(window, "inform6_l");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    widget = lookup_widget(window, "inform6_r");
    update_font(widget);
    update_tabs(GTK_SOURCE_VIEW(widget));
    return FALSE; /* One-shot idle function */
}

/* Update the font sizes of widgets in this main window that don't have
styles */
gboolean 
update_app_window_font_sizes(GtkWidget *window) 
{
    GtkWidget *widget;
    
    gchar *widget_names[] = {
        "problems_l",   "problems_r",
        "actions_l",    "actions_r",
        "contents_l",   "contents_r",
        "kinds_l",      "kinds_r",
        "phrasebook_l", "phrasebook_r",
        "rules_l",      "rules_r",
        "scenes_l",     "scenes_r",
        "world_l",      "world_r",
        "game_l",       "game_r",
        "docs_l",       "docs_r",
        "debugging_l",  "debugging_r"
    };
#define NUM_WIDGET_NAMES (sizeof(widget_names) / sizeof(widget_names[0]))
    
    int foo;
    for(foo = 0; foo < NUM_WIDGET_NAMES; foo++) {
        widget = lookup_widget(window, widget_names[foo]);
        update_font_size(widget);
        while(gtk_events_pending())
            gtk_main_iteration();
    }
    return FALSE; /* One-shot idle function */
}

/* Turn source highlighting on or off in this source buffer */
void 
update_source_highlight(GtkSourceBuffer *buffer) 
{
    gtk_source_buffer_set_highlight_syntax(buffer, 
      config_file_get_bool("SyntaxSettings", "SyntaxHighlighting"));
}

/* Update the "Open Extensions" menu in this main window */
static gboolean 
update_app_window_extensions_menu(GtkWidget *widget) 
{
    GtkWidget *menu;
    if((menu = create_open_extension_submenu()))
        gtk_menu_item_set_submenu(
          GTK_MENU_ITEM(lookup_widget(widget, "open_extension")), menu);
    return FALSE; /* One-shot idle function */
}

/* Update the "Open Extensions" menu in this main window */
static gboolean 
update_ext_window_extensions_menu(GtkWidget *widget) 
{
    GtkWidget *menu;
    if((menu = create_open_extension_submenu()))
        gtk_menu_item_set_submenu(
          GTK_MENU_ITEM(lookup_widget(widget, "xopen_extension")), menu);
    return FALSE; /* One-shot idle function */
}

/*
 * CALLBACKS
 */

/* Check whether the user has selected something (not an author name) that can
be removed, and if so, enable the remove button */
static void 
extension_browser_selection_changed(GtkTreeSelection *selection,
                                    GtkWidget *widget) 
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        gtk_widget_set_sensitive(
          lookup_widget(widget, "prefs_i7_extension_remove"),
          (gtk_tree_path_get_depth(path) == 2));
          /* Only enable the "Remove" button if the selection is 2 deep */
        gtk_tree_path_free(path);
    } else
        gtk_widget_set_sensitive(
          lookup_widget(widget, "prefs_i7_extension_remove"), FALSE);
          /* if there is no selection */
}

void
on_prefs_dialog_realize(GtkWidget *widget, gpointer data)
{
    /* Set all the controls to their current values according to GConf */
    trigger_config_keys();

    /* List all the installed extensions in the extension widgets */
    populate_extension_lists(widget);
    GtkWidget *view = lookup_widget(widget, "prefs_i7_extensions_view");
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      "", renderer, "text", 0, NULL); /* No title, text
      renderer, get the property "text" from column 0 */
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE); /* Only select
    one at a time */
    g_signal_connect(G_OBJECT(select), "changed",
      G_CALLBACK(extension_browser_selection_changed), (gpointer)widget);
      
    /* Connect the drag'n'drop stuff */
    gtk_drag_dest_set(view,
      GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT,
      NULL, 0, GDK_ACTION_COPY);
    /* For some reason GTK_DEST_DEFAULT_DROP causes two copies of every file to
    be sent to the widget when dropped, so we omit that. Also,
    GTK_DEST_DEFAULT_HIGHLIGHT seems to be broken. Including it anyway. */
    gtk_drag_dest_add_uri_targets(view);
}

gboolean
on_prefs_i7_extensions_view_drag_drop(GtkWidget *widget, 
                                      GdkDragContext *drag_context, gint x,
                                      gint y, guint time, gpointer data)
{
    /* Iterate through the list of target types provided by the source */
    GdkAtom target_type = NULL;
    GList *iter;
    for(iter = drag_context->targets; iter != NULL; iter = g_list_next(iter)) {
        gchar *type_name = gdk_atom_name(GDK_POINTER_TO_ATOM(iter->data));
        /* Select 'text/uri-list' from the list of available targets */
        if(!strcmp(type_name, "text/uri-list")) {
            g_free(type_name);
            target_type = GDK_POINTER_TO_ATOM(iter->data);
            break;
        }
        g_free(type_name);
    }
    /* If URI list not supported, then cancel */
    if(!target_type)
        return FALSE;
                
    /* Request the data from the source. */
    gtk_drag_get_data(
      widget,         /* this widget, which will get 'drag-data-received' */
      drag_context,   /* represents the current state of the DnD */
      target_type,    /* the target type we want */
      time);            /* time stamp */
    return TRUE;
}


void
on_prefs_i7_extensions_view_drag_data_received(GtkWidget *widget,
                                               GdkDragContext *drag_context,
                                               gint x, gint y,
                                               GtkSelectionData *selectiondata,
                                               guint info, guint time,
                                               gpointer data)
{
    gboolean dnd_success = TRUE;
    gchar *type_name = NULL;
    
    /* Check that we got data from source */
    if((selectiondata == NULL) || (selectiondata->length < 0))
        dnd_success = FALSE;
    
    /* Check that we got the format we can use */
    else if(strcmp((type_name = gdk_atom_name(selectiondata->type)), 
      "text/uri-list"))
        dnd_success = FALSE;
    
    else {  
        /* Do stuff with the data */
        gchar **extension_files = g_uri_list_extract_uris(
          (gchar *)selectiondata->data);
        int foo;
        /* Get a list of URIs to the dropped files */
        for(foo = 0; extension_files[foo] != NULL; foo++) {
            GError *err = NULL;
            gchar *filename = g_filename_from_uri(
              extension_files[foo], NULL, &err);
            if(!filename) {
                g_warning(_("Invalid URI: %s"), err->message);
                g_error_free(err);
                continue;
            }
            
            /* Check whether a directory was dropped. if so, install contents */
            /* NOTE: not recursive (that would be kind of silly anyway */
            if(g_file_test(filename, G_FILE_TEST_IS_DIR)) {
                GDir *dir = g_dir_open(filename, 0, &err);
                if(err) {
                    error_dialog(NULL, err, _("Error opening directory %s: "),
                      filename);
                    g_free(filename);
                    return;
                }
                const gchar *dir_entry;
                while((dir_entry = g_dir_read_name(dir)) != NULL)
                    if(!g_file_test(dir_entry, G_FILE_TEST_IS_DIR)) {
                        gchar *entry_with_path = g_build_filename(
                          filename, dir_entry, NULL);
                        install_extension(entry_with_path);
                        g_free(entry_with_path);
                    }
                g_dir_close(dir);
        
            } else
                /* just install it */
                install_extension(filename);
            
            g_free(filename);
        }
        g_strfreev(extension_files);
        populate_extension_lists(widget);
    }
    
    if(type_name)
        g_free(type_name);
    gtk_drag_finish(drag_context, dnd_success, FALSE, time);
}


void
on_prefs_font_set_changed(GtkComboBox *combobox, gpointer data)
{
    config_file_set_enum("EditorSettings", "FontSet", 
      gtk_combo_box_get_active(combobox), font_set_lookup_table);
}

void
on_prefs_custom_font_font_set(GtkFontButton *fontbutton, gpointer data)
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
    
    config_file_set_string("EditorSettings", "CustomFont", ptr);
    g_free(fontname);
}

void
on_prefs_font_styling_changed(GtkComboBox *combobox, gpointer data)
{
    config_file_set_enum("EditorSettings", "FontStyling", 
      gtk_combo_box_get_active(combobox), font_styling_lookup_table);
}

void
on_prefs_font_size_changed(GtkComboBox *combobox, gpointer data)
{
    config_file_set_enum("EditorSettings", "FontSize", 
      gtk_combo_box_get_active(combobox), font_size_lookup_table);
}

void
on_prefs_change_colors_changed(GtkComboBox *combobox, gpointer data)
{
    config_file_set_enum("EditorSettings", "ChangeColors", 
      gtk_combo_box_get_active(combobox), change_colors_lookup_table);
}

void
on_prefs_color_set_changed(GtkComboBox *combobox, gpointer data)
{
    config_file_set_enum("EditorSettings", "ColorSet", 
      gtk_combo_box_get_active(combobox), color_set_lookup_table);
}

void
on_tab_ruler_value_changed(GtkRange *range, gpointer data)
{
    config_file_set_int("EditorSettings", "TabWidth", 
      (int)gtk_range_get_value(range));
}

gchar*
on_tab_ruler_format_value(GtkScale *scale, gdouble value, gpointer data)
{
    if(value)
        return g_strdup_printf("%.*f spaces", gtk_scale_get_digits(scale),
          value);
    return g_strdup("default");
}

void
on_prefs_notes_toggle_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    config_file_set_bool("InspectorSettings", "NotesVisible",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_headings_toggle_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    config_file_set_bool("InspectorSettings", "HeadingsVisible",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_skein_toggle_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    config_file_set_bool("InspectorSettings", "SkeinVisible",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_search_toggle_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    config_file_set_bool("InspectorSettings", "SearchVisible",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_i7_extension_add_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
      _("Select the extensions to install"),
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
      GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    
    if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
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
    
    /* Refresh all lists of extensions */
    populate_extension_lists(GTK_WIDGET(button));
    for_each_story_window_idle((GSourceFunc)update_app_window_extensions_menu);
    for_each_extension_window_idle(
      (GSourceFunc)update_ext_window_extensions_menu);
}

void
on_prefs_i7_extension_remove_clicked(GtkButton *button, gpointer data)
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
          GTK_BUTTONS_YES_NO, 
          /* TRANSLATORS: Are you sure you want to remove EXTENSION_NAME by 
          AUTHOR_NAME?*/
          _("Are you sure you want to remove %s by %s?"), 
          extname, author);
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        /* Delete the extension and remove its directory if empty */
        if(result == GTK_RESPONSE_YES)
            delete_extension(author, extname);
        
        g_free(extname);
        g_free(author);
        gtk_tree_path_free(path);
    }
    
    /* Refresh all lists of extensions */
    populate_extension_lists(GTK_WIDGET(button));
    for_each_story_window_idle((GSourceFunc)update_app_window_extensions_menu);
    for_each_extension_window_idle(
      (GSourceFunc)update_ext_window_extensions_menu);
}

void
on_prefs_enable_highlighting_toggle_toggled(GtkToggleButton *togglebutton,
                                            gpointer data)
{
    config_file_set_bool("SyntaxSettings", "SyntaxHighlighting", 
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_follow_symbols_toggle_toggled(GtkToggleButton *togglebutton,
                                       gpointer data)
{
    config_file_set_bool("SyntaxSettings", "Intelligence", 
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_intelligent_inspector_toggle_toggled(GtkToggleButton *togglebutton,
                                              gpointer data)
{
    config_file_set_bool("SyntaxSettings", "IntelligentHeadingsInspector",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_auto_indent_toggle_toggled(GtkToggleButton *togglebutton,
                                    gpointer data)
{
    config_file_set_bool("SyntaxSettings", "AutoIndent",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_auto_number_toggle_toggled(GtkToggleButton *togglebutton,
                                    gpointer data)
{
    config_file_set_bool("SyntaxSettings", "AutoNumberSections",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_author_changed(GtkEditable *editable, gpointer data)
{
    config_file_set_string("AppSettings", "AuthorName",
      gtk_entry_get_text(GTK_ENTRY(editable)));
}

void
on_prefs_clean_build_toggle_toggled(GtkToggleButton *togglebutton,
                                    gpointer data)
{
    config_file_set_bool("IDESettings", "CleanBuildFiles", 
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_clean_index_toggle_toggled(GtkToggleButton *togglebutton,
                                    gpointer data)
{
    config_file_set_bool("IDESettings", "CleanIndexFiles",
      gtk_toggle_button_get_active(togglebutton));
}

void
on_prefs_show_log_toggle_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    config_file_set_bool("IDESettings", "DebugLogVisible", 
        gtk_toggle_button_get_active(togglebutton));
}

/* Get the language associated with this sourceview and update the highlighting
styles */
void 
update_style(GtkSourceView *thiswidget) 
{
    GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(thiswidget)));
    set_highlight_styles(buffer);
}

/* Change the font that this widget uses */
void 
update_font(GtkWidget *thiswidget) 
{
    PangoFontDescription *font = get_font_description();
    gtk_widget_modify_font(thiswidget, font);
    pango_font_description_free(font);
}

/* Change only the font size but not the face that this widget uses */
void 
update_font_size(GtkWidget *thiswidget) 
{
    PangoFontDescription *font = pango_font_description_new();
    pango_font_description_set_size(font, get_font_size() * PANGO_SCALE);
    gtk_widget_modify_font(thiswidget, font);
    pango_font_description_free(font);
}

/* Update the tab stops for a GtkSourceView */
void 
update_tabs(GtkSourceView *thiswidget) 
{
    gint spaces = config_file_get_int("EditorSettings", "TabWidth");
    if(spaces == 0)
        spaces = 8; /* default is 8 */
    gtk_source_view_set_tab_width(thiswidget, spaces);
}

/* Return a string of font families for the font setting. String must be freed*/
gchar *
get_font_family(void)
{
    gchar *customfont;
    switch(config_file_get_enum("EditorSettings", "FontSet", 
           font_set_lookup_table)) {
        case FONT_SET_PROGRAMMER:
            return g_strdup(
              /* TRANSLATORS: This string is a list of monospace fonts in order
              of preference. Only change it if you need other fonts to display
              your language. */
              _("DejaVu Sans Mono,DejaVu Sans LGC Mono,"
              "Bitstream Vera Sans Mono,Courier New,Luxi Mono,Monospace"));
        case FONT_SET_CUSTOM:
            customfont = config_file_get_string("EditorSettings", "CustomFont");
            if(customfont)
                return customfont;
            /* else fall through */
        default:
            ;
    }
    return g_strdup(
     /* TRANSLATORS: This string is a list of sans-serif proportional fonts in
     order of preference. Only change it if you need other fonts to display your
     language. */
     _("DejaVu Sans,DejaVu LGC Sans,Bitstream Vera Sans,Arial,Luxi Sans,Sans"));
}

/* Return the font size in points for the font size setting */
int
get_font_size(void)
{
    switch(config_file_get_enum("EditorSettings", "FontSize", 
      font_size_lookup_table)) {
        case FONT_SIZE_MEDIUM:
            return SIZE_MEDIUM;
        case FONT_SIZE_LARGE:
            return SIZE_LARGE;
        case FONT_SIZE_HUGE:
            return SIZE_HUGE;
        default:
            ;
    }
    return SIZE_STANDARD;
}

/* Get the current font as a PangoFontDescription.
Must be freed with pango_font_description_free. */
PangoFontDescription *
get_font_description(void)
{
    PangoFontDescription *font = pango_font_description_new();
    gchar *fontfamily = get_font_family();
    pango_font_description_set_family(font, fontfamily);
    g_free(fontfamily);
    pango_font_description_set_size(font, get_font_size() * PANGO_SCALE);
    return font;
}
