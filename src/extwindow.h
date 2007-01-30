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
 
#ifndef EXT_WINDOW_H
#define EXT_WINDOW_H

void
after_ext_window_realize               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_xclose_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xsave_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xsave_as_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xrevert_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xundo_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xredo_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xcut_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xcopy_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xpaste_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xselect_all_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_xfind_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

GtkWidget*
ext_code_create (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

gboolean
on_ext_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_ext_window_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

void jump_to_line_ext(GtkWidget *widget, gint line);
#endif
