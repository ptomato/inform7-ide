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
 
#ifndef PREFS_H
#define PREFS_H

#include <gnome.h>
#include <gtksourceview/gtksourceview.h>

/* Three options for editor font */
enum {
    FONT_SET_STANDARD,
    FONT_SET_PROGRAMMER,
    FONT_SET_CUSTOM
};

/* Three options for syntax highlighting */
enum {
    FONT_STYLING_NONE,
    FONT_STYLING_SUBTLE,
    FONT_STYLING_OFTEN
};

/* Four different text size options */
enum {
    FONT_SIZE_STANDARD,
    FONT_SIZE_MEDIUM,
    FONT_SIZE_LARGE,
    FONT_SIZE_HUGE
};

/* Pango point sizes of text size options */
enum {
    SIZE_STANDARD = 10,
    SIZE_MEDIUM = 11,
    SIZE_LARGE = 14,
    SIZE_HUGE = 18
};

/* Three options for color syntax highlighting */
enum {
    CHANGE_COLORS_NEVER,
    CHANGE_COLORS_OCCASIONALLY,
    CHANGE_COLORS_OFTEN
};

/* The three color sets */
enum {
    COLOR_SET_SUBDUED,
    COLOR_SET_STANDARD,
    COLOR_SET_PSYCHEDELIC
};

/* The tabs in the preferences window */
enum {
    TAB_STYLES,
    TAB_INSPECTORS,
    TAB_EXTENSIONS,
    TAB_INTELLIGENCE,
    TAB_ADVANCED
};

void
on_prefs_dialog_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_prefs_i7_extensions_view_drag_drop  (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        guint            time,
                                        gpointer         user_data);

void
on_prefs_i7_extensions_view_drag_data_received
                                        (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data);

void
on_prefs_font_set_realize              (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_prefs_font_styling_realize          (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_prefs_font_size_realize             (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_prefs_change_colors_realize         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_prefs_color_set_realize             (GtkWidget       *widget,
                                        gpointer         user_data);
                                        
void
on_prefs_font_set_changed              (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_prefs_custom_font_font_set          (GtkFontButton   *fontbutton,
                                        gpointer         user_data);

void
on_prefs_font_styling_changed          (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_prefs_font_size_changed             (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_prefs_change_colors_changed         (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_prefs_color_set_changed             (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_tab_ruler_value_changed             (GtkRange        *range,
                                        gpointer         user_data);

gchar*
on_tab_ruler_format_value              (GtkScale        *scale,
                                        gdouble          value,
                                        gpointer         user_data);
                                        
void
on_prefs_project_files_toggle_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_notes_toggle_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_headings_toggle_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_skein_toggle_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_watchpoints_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_breakpoints_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_search_toggle_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_i7_extension_add_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_i7_extension_remove_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_i6_extension_add_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_i6_extension_remove_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_enable_highlighting_toggle_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_indent_toggle_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_follow_symbols_toggle_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_intelligent_inspector_toggle_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_auto_indent_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_auto_number_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_author_changed                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_clean_build_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_clean_index_toggle_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_show_log_toggle_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_rebuild_compiler_toggle_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
void
on_prefs_close_clicked                 (GtkButton       *button,
                                        gpointer         user_data);
                                        
void update_style(GtkSourceView *thiswidget);
void update_font(GtkWidget *thiswidget);
void update_font_size(GtkWidget *thiswidget);
void update_tabs(GtkSourceView *thiswidget);
gchar *get_font_family(void);
int get_font_size(void);
PangoFontDescription *get_font_description(void);
#endif
