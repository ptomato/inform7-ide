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
 
#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <gnome.h>

gboolean on_new_dialog_delete_event(GtkWidget *widget, GdkEvent *event,
                                    gpointer data);
void on_new_cancel_clicked(GnomeDruid *druid, gpointer data);
void on_new_druid_realize(GtkWidget *widget, gpointer data);
gboolean on_new_druid_page1_next(GnomeDruidPage *gnomedruidpage,
                                 GtkWidget *widget, gpointer data);
gboolean go_back_to_type_page(GnomeDruidPage *gnomedruidpage, GtkWidget *widget,
                              gpointer data);
void on_new_author_realize(GtkWidget *widget, gpointer data);
void on_new_druid_inform7_page_finish(GnomeDruidPage *gnomedruidpage,
                                      GtkWidget *widget, gpointer data);
void update_new_ok_button(GtkEditable *editable, gpointer data);
void update_new_ext_ok_button(GtkEditable *editable, gpointer data);
void on_new_druid_extension_page_finish(GnomeDruidPage *gnomedruidpage,
                                        GtkWidget *widget, gpointer data);
void on_new_druid_inform6_page_finish(GnomeDruidPage *gnomedruidpage,
                                      GtkWidget *widget, gpointer data);

#endif
