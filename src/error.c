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
#include <stdarg.h>

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
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(message);
}
