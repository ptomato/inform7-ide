/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gnome-inform7
 * Copyright (C) P.F. Chimento 2007 <philip.chimento@gmail.com>
 * 
 * gnome-inform7 is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * gnome-inform7 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with gnome-inform7.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifndef _SKEIN_H_
#define _SKEIN_H_

#include <glib-object.h>

typedef struct {
    gchar *  line;
    gchar *  label;
    gchar *  id;
    
    gchar *  text_transcript;
    gchar *  text_expected;
    
    gboolean played;
    gboolean changed;
    gboolean temp;
    int      score;
    
    double   width;
    double   linewidth;
    double   labelwidth;
    double   x;
} NodeData;

enum {
    GOT_LINE,
    GOT_TRANSCRIPT,
    GOT_USER_ACTION
};

G_BEGIN_DECLS

#define I7_TYPE_SKEIN             (skein_get_type ())
#define I7_SKEIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), I7_TYPE_SKEIN, Skein))
#define I7_SKEIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), I7_TYPE_SKEIN, SkeinClass))
#define I7_IS_SKEIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), I7_TYPE_SKEIN))
#define I7_IS_SKEIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), I7_TYPE_SKEIN))
#define I7_SKEIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), I7_TYPE_SKEIN, SkeinClass))

typedef struct _SkeinClass SkeinClass;
typedef struct _Skein Skein;

struct _SkeinClass
{
	GObjectClass parent_class;

	/* Signals */
	void(* tree_changed) (Skein *self);
	void(* thread_changed) (Skein *self);
	void(* node_text_changed) (Skein *self);
	void(* node_color_changed) (Skein *self);
	void(* lock_changed) (Skein *self);
	void(* transcript_thread_changed) (Skein *self);
	void(* show_node) (Skein *self, guint why, gpointer node);
};

struct _Skein
{
	GObject parent_instance;
};

GType skein_get_type(void) G_GNUC_CONST;
Skein *skein_new(void);
void skein_free(Skein *skein);

GNode *skein_get_root_node(Skein *skein);
GNode *skein_get_current_node(Skein *skein);
void skein_set_current_node(Skein *skein, GNode *node);
gboolean in_current_thread(Skein *skein, GNode *node);
GNode *skein_get_played_node(Skein *skein);
void skein_load(Skein *skein, const gchar *path);
void skein_save(Skein *skein, const gchar *path);
void skein_reset(Skein *skein, gboolean current);
void skein_layout(Skein *skein, double spacing);
void skein_new_line(Skein *skein, const gchar *line);
gboolean skein_next_line(Skein *skein, gchar **line);
GSList *skein_get_commands(Skein *skein);
void skein_update_after_playing(Skein *skein, const gchar *transcript);
gboolean skein_get_line_from_history(Skein *skein, gchar **line, int history);
GNode *skein_add_new(Skein *skein, GNode *node);
GNode *skein_add_new_parent(Skein *skein, GNode *node);
gboolean skein_remove_all(Skein *skein, GNode *node, gboolean notify);
gboolean skein_remove_single(Skein *skein, GNode *node);
void skein_set_line(Skein *skein, GNode *node, const gchar *line);
void skein_set_label(Skein *skein, GNode *node, const gchar *label);
void skein_lock(Skein *skein, GNode *node);
void skein_unlock(Skein *skein, GNode *node, gboolean notify);
void skein_trim(Skein *skein, GNode *node, gboolean notify);
gboolean skein_has_labels(Skein *skein);
void skein_bless(Skein *skein, GNode *node, gboolean all);
gboolean skein_can_bless(Skein *skein, GNode *node, gboolean all);
void skein_set_expected_text(Skein *skein, GNode *node, const gchar *text);
GNode *skein_get_thread_top(Skein *skein, GNode *node);
GNode *skein_get_thread_bottom(Skein *skein, GNode *node);
gboolean skein_get_modified(Skein *skein);

GNode *node_create(const gchar *line, const gchar *label,
                  const gchar *transcript, const gchar *expected,
                  gboolean played, gboolean changed, gboolean temp, int score);
void node_destroy(GNode *node);
const gchar *node_get_line(GNode *node);
void node_set_line(GNode *node, const gchar *line);
const gchar *node_get_label(GNode *node);
void node_set_label(GNode *node, const gchar *label);
gboolean node_has_label(GNode *node);
const gchar *node_get_expected_text(GNode *node);
gboolean node_get_changed(GNode *node);
gboolean node_get_temporary(GNode *node);
void node_set_played(GNode *node);
void node_set_temporary(GNode *node, gboolean temp);
void node_new_transcript_text(GNode *node, const gchar *transcript);
void node_bless(GNode *node);
void node_set_expected_text(GNode *node, const gchar *text);
double node_get_line_width(GNode *node, PangoLayout *layout);
double node_get_line_text_width(GNode *node);
double node_get_label_text_width(GNode *node);
double node_get_tree_width(GNode *node, PangoLayout *layout, double spacing);
const gchar *node_get_unique_id(GNode *node);
double node_get_x(GNode *node);
gboolean node_in_thread(GNode *node, GNode *endnode);

/* DEBUG */
void skein_dump(Skein *skein);

G_END_DECLS

#endif /* _SKEIN_H_ */
