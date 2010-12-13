/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "error.h"

/* Create and display an error dialog box, with parent window parent, and
message format string msg. If err is not NULL, tack the error message on to the
end of the format string. */
void 
error_dialog(GtkWindow *parent, GError *err, const gchar *msg, ...) 
{
    va_list ap;
    
    va_start(ap, msg);
    gchar buffer[1024];
    g_vsnprintf(buffer, 1024, msg, ap);
    va_end(ap);
    
    gchar *message;
    if(err) {
        message = g_strconcat(buffer, err->message, NULL);
        g_error_free(err);
    } else
        message = g_strdup(buffer);
    
    GtkWidget *dialog = gtk_message_dialog_new(parent,
          parent? GTK_DIALOG_DESTROY_WITH_PARENT : 0,
          GTK_MESSAGE_ERROR,
          GTK_BUTTONS_OK,
          message);

	/* WTF doesn't gtk_dialog_run() do this anymore? */
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
	
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(message);
}
