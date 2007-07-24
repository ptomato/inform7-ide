/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gtkterp
 * Copyright (C) P.F. Chimento 2007 <philip.chimento@gmail.com>
 * 
 */

#ifndef _GARGLK_PLUG_H_
#define _GARGLK_PLUG_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <dbus/dbus-glib-bindings.h>

G_BEGIN_DECLS

#define GARGLK_TYPE_PLUG             (garglk_plug_get_type ())
#define GARGLK_PLUG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GARGLK_TYPE_PLUG, GarglkPlug))
#define GARGLK_PLUG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GARGLK_TYPE_PLUG, GarglkPlugClass))
#define GARGLK_IS_PLUG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GARGLK_TYPE_PLUG))
#define GARGLK_IS_PLUG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GARGLK_TYPE_PLUG))
#define GARGLK_PLUG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GARGLK_TYPE_PLUG, GarglkPlugClass))

typedef struct _GarglkPlugClass GarglkPlugClass;
typedef struct _GarglkPlug GarglkPlug;

struct _GarglkPlugClass
{
	GtkPlugClass parent_class;
    DBusGConnection *connection;
};

struct _GarglkPlug
{
	GtkPlug parent_instance;
};

GtkWidget *garglk_plug_new(GdkNativeWindow socket_id);
GType garglk_plug_get_type (void) G_GNUC_CONST;
void garglk_plug_send_command_text (GarglkPlug *plug, const gchar *text);
void garglk_plug_send_reply_text (GarglkPlug *plug, const gchar *text);
void garglk_plug_send_story_title (GarglkPlug *plug, const gchar *title);
void garglk_plug_send_stopped (GarglkPlug *plug);
gboolean garglk_plug_get_interactive (GarglkPlug *plug);
gboolean garglk_plug_get_protected (GarglkPlug *plug);

gboolean garglk_set_interactive(GarglkPlug *plug, gboolean interactive, GError **error);
gboolean garglk_set_protected(GarglkPlug *plug, gboolean protect, GError **error);
gboolean garglk_feed_text(GarglkPlug *plug, gchar *text, GError **error);
gboolean garglk_request_window(GarglkPlug *plug, guint *windowid, GError **error);
gboolean garglk_grab_focus(GarglkPlug *plug, GError **error);
gboolean garglk_stop(GarglkPlug *plug, GError **error);

G_END_DECLS

#endif /* _GARGLK_PLUG_H_ */
