/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2011 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _SKEIN_H_
#define _SKEIN_H_

#include "config.h"

#include <stdbool.h>

#include <cairo.h>
#include <glib.h>
#include <glib-object.h>
#include <goocanvas.h>

#include "node.h"

typedef enum {
	I7_REASON_COMMAND,
	I7_REASON_TRANSCRIPT,
	I7_REASON_USER_ACTION
} I7SkeinShowNodeReason;

G_BEGIN_DECLS

#define I7_TYPE_SKEIN             (i7_skein_get_type ())
#define I7_SKEIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), I7_TYPE_SKEIN, I7Skein))
#define I7_SKEIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), I7_TYPE_SKEIN, I7SkeinClass))
#define I7_IS_SKEIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), I7_TYPE_SKEIN))
#define I7_IS_SKEIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), I7_TYPE_SKEIN))
#define I7_SKEIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), I7_TYPE_SKEIN, I7SkeinClass))

typedef struct {
	gchar *label;
	I7Node *node;
} I7SkeinNodeLabel;

typedef struct _I7SkeinClass I7SkeinClass;
typedef struct _I7Skein I7Skein;

struct _I7SkeinClass
{
	GooCanvasGroupModelClass parent_class;

	/* Signals */
	void(* needs_layout) (I7Skein *self);
	void(* node_activate) (I7Skein *self, I7Node *node);
	void(* differs_badge_activate) (I7Skein *self, I7Node *node);
	void(* transcript_thread_changed) (I7Skein *self);
	void(* labels_changed) (I7Skein *self);
	void(* show_node) (I7Skein *self, guint why, I7Node *node);
	void(* modified) (I7Skein *self);
};

struct _I7Skein
{
	GooCanvasGroupModel parent_instance;
};

typedef enum _I7SkeinError {
	I7_SKEIN_ERROR_XML,
	I7_SKEIN_ERROR_BAD_FORMAT,
} I7SkeinError;

#define I7_SKEIN_ERROR i7_skein_error_quark()

GQuark i7_skein_error_quark(void);
GType i7_skein_get_type(void) G_GNUC_CONST;
I7Skein *i7_skein_new(void);
void i7_skein_free_node_label_list(GSList *labels);

I7Node *i7_skein_get_root_node(I7Skein *self);
I7Node *i7_skein_get_current_node(I7Skein *self);
void i7_skein_set_current_node(I7Skein *self, I7Node *node);
gboolean i7_skein_is_node_in_current_thread(I7Skein *self, I7Node *node);
I7Node *i7_skein_get_played_node(I7Skein *self);
gboolean i7_skein_load(I7Skein *self, GFile *file, GError **error);
void i7_skein_save_async(I7Skein *self, GFile *file, int priority, GCancellable *cancel, GAsyncReadyCallback callback, void *data);
bool i7_skein_save_finish(I7Skein *self, GAsyncResult *res, GError **error);
gboolean i7_skein_import(I7Skein *self, GFile *file, GError **error);
void i7_skein_reset(I7Skein *self, gboolean current);
void i7_skein_draw(I7Skein *self, GooCanvas *canvas);
void i7_skein_schedule_draw(I7Skein *self, GooCanvas *canvas);
I7Node *i7_skein_new_command(I7Skein *self, const gchar *command);
gboolean i7_skein_next_command(I7Skein *self, gchar **command);
GSList *i7_skein_get_commands(I7Skein *self);
GSList *i7_skein_get_commands_to_node(I7Skein *self, I7Node *from_node, I7Node *to_node);
void i7_skein_update_after_playing(I7Skein *self, const gchar *transcript);
gboolean i7_skein_get_line_from_history(I7Skein *self, gchar **line, int history);
I7Node *i7_skein_add_new(I7Skein *self, I7Node *node);
I7Node *i7_skein_add_new_parent(I7Skein *self, I7Node *node);
gboolean i7_skein_remove_all(I7Skein *self, I7Node *node);
gboolean i7_skein_remove_single(I7Skein *self, I7Node *node);
void i7_skein_lock(I7Skein *self, I7Node *node);
void i7_skein_unlock(I7Skein *self, I7Node *node);
void i7_skein_trim(I7Skein *self, I7Node *node, gint max_temps);
GSList *i7_skein_get_labels(I7Skein *self);
gboolean i7_skein_has_labels(I7Skein *self);
void i7_skein_bless(I7Skein *self, I7Node *node, gboolean all);
gboolean i7_skein_can_bless(I7Skein *self, I7Node *node, gboolean all);
I7Node *i7_skein_get_thread_top(I7Skein *self, I7Node *node);
I7Node *i7_skein_get_thread_bottom(I7Skein *self, I7Node *node);
GSList *i7_skein_get_blessed_thread_ends(I7Skein *self);
gboolean i7_skein_get_modified(I7Skein *self);
void i7_skein_set_font(I7Skein *self, PangoFontDescription *font);

/* DEBUG */
void i7_skein_dump(I7Skein *self);

G_END_DECLS

#endif /* _SKEIN_H_ */
