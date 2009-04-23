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

#ifndef GTKTERP_H
#define GTKTERP_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_TERP         (gtk_terp_get_type ())
#define GTK_TERP(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_TYPE_TERP, GtkTerp))
#define GTK_TERP_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GTK_TYPE_TERP, GtkTerpClass))
#define GTK_IS_TERP(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_TYPE_TERP))
#define GTK_IS_TERP_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_TYPE_TERP))
#define GTK_TERP_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GTK_TYPE_TERP, GtkTerpClass))

typedef struct _GtkTerp GtkTerp;
typedef struct _GtkTerpPrivate GtkTerpPrivate;
typedef struct _GtkTerpClass GtkTerpClass;
typedef enum {
    GTK_TERP_FROTZ,
    GTK_TERP_GLULXE,
	GTK_TERP_GIT
} GtkTerpInterpreter;

struct _GtkTerp {
	GtkSocket parent;
	GtkTerpPrivate *priv;
};

struct _GtkTerpClass {
	GtkSocketClass parent_class;
	
    void (* command_received) (GtkTerp *terp, const gchar *command);
    void (* text_replied) (GtkTerp *terp, const gchar *text);
    void (* title_changed) (GtkTerp *terp, const gchar *title);
    void (* game_finished) (GtkTerp *terp);
    void (* stopped) (GtkTerp *terp, gint exit_code);
};

GType gtk_terp_get_type();
GtkWidget *gtk_terp_new();

gboolean gtk_terp_load_game       (GtkTerp *terp, const gchar *filename,
                                   GError **error);
void     gtk_terp_unload_game     (GtkTerp *terp);
gboolean gtk_terp_get_game_loaded (GtkTerp *terp);
gchar*   gtk_terp_get_filename    (GtkTerp *terp);
gboolean gtk_terp_start_game      (GtkTerp *terp,
                                   const GtkTerpInterpreter format,
                                   GError **error);
void     gtk_terp_stop_game       (GtkTerp *terp);
gboolean gtk_terp_get_running     (GtkTerp *terp);
void     gtk_terp_set_interactive (GtkTerp *terp, const gboolean interactive);
gboolean gtk_terp_get_interactive (GtkTerp *terp);
void     gtk_terp_set_protected   (GtkTerp *terp, const gboolean protect);
gboolean gtk_terp_get_protected   (GtkTerp *terp);
void     gtk_terp_set_minimum_size(GtkTerp *terp, const guint x, const guint y);
void     gtk_terp_feed_command    (GtkTerp *terp, const gchar *command);
void     gtk_terp_feed_text       (GtkTerp *terp, const gchar *text);

/* GError codes */

GQuark   gtk_terp_error_quark     (void);
#define  GTK_TERP_ERROR           (gtk_terp_error_quark())

enum GtkTerpError {
    GTK_TERP_ERROR_FAILED,
    GTK_TERP_ERROR_GAME_NOT_LOADED,
    GTK_TERP_ERROR_GAME_IS_RUNNING,
    GTK_TERP_ERROR_FILE_NOT_FOUND,
    GTK_TERP_ERROR_NO_PROXY
};

G_END_DECLS

#endif /* GTKTERP_H */
