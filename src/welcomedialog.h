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
 
#ifndef WELCOME_DIALOG_H
#define WELCOME_DIALOG_H

#include <gnome.h>

#if !GTK_CHECK_VERSION(2,10,0)
# define SUCKY_GNOME 1
#endif

void after_welcome_dialog_realize(GtkWidget *widget, gpointer data);
void on_welcome_new_button_clicked(GtkButton *button, gpointer data);
void on_welcome_open_button_clicked(GtkButton *button, gpointer data);
void on_welcome_reopen_button_clicked(GtkButton *button, gpointer data);
gboolean on_welcome_dialog_delete_event(GtkWidget *widget, GdkEvent *event,
                                        gpointer data);
#endif
