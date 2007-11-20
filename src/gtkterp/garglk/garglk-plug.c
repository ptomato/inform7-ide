/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gtkterp
 * Copyright (C) P.F. Chimento 2007 <philip.chimento@gmail.com>
 * 
 */

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include "garglk-plug.h"
#include "dbus-server.h"
#include "glk.h"
#include "garglk.h"

#if DEBUG
static void
message(gchar *text)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, 
                                    "%s", text);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
#endif /* DEBUG */

typedef struct _GarglkPlugPrivate GarglkPlugPrivate;
struct _GarglkPlugPrivate
{
	gboolean interactive;
	gboolean protect;
};

#define GARGLK_PLUG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GARGLK_TYPE_PLUG, GarglkPlugPrivate))

enum {
	COMMAND_TEXT_SIGNAL,
    REPLY_TEXT_SIGNAL,
    STORY_TITLE_SIGNAL,
    READY_SIGNAL,
    STOPPED_SIGNAL,
	LAST_SIGNAL
};

static guint garglk_plug_signals[LAST_SIGNAL] = { 0 };
static GtkPlugClass* parent_class = NULL;

static void
garglk_plug_init (GarglkPlug *object)
{
    /* Initialise the DBus connection */
    GError *err = NULL;
	DBusGProxy *driver_proxy;
	GarglkPlugClass *klass = GARGLK_PLUG_GET_CLASS(object);
	guint request_ret;
    
	/* Register DBUS path */
    pid_t pid = getpid();
    gchar *pathname = g_strdup_printf("/org/informfiction/garglk/%d", pid);
	dbus_g_connection_register_g_object(klass->connection, pathname,
			                            G_OBJECT(object));
    g_free(pathname);

	/* Register the service name, the constants here are defined in dbus-glib-bindings.h */
	driver_proxy = dbus_g_proxy_new_for_name(klass->connection,
			                                 DBUS_SERVICE_DBUS,
			                                 DBUS_PATH_DBUS,
			                                 DBUS_INTERFACE_DBUS);
	if(!org_freedesktop_DBus_request_name(driver_proxy,
			                              "org.informfiction.garglk",
			                              0, &request_ret, &err))
        g_error("Unable to register service: %s", err->message);
	g_object_unref (driver_proxy);

	GARGLK_PLUG_PRIVATE(object)->interactive = TRUE;
    GARGLK_PLUG_PRIVATE(object)->protect = FALSE;
    
    g_signal_emit(object, garglk_plug_signals[READY_SIGNAL], 0);
}

static void
garglk_plug_finalize (GObject *object)
{
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
garglk_plug_class_init (GarglkPlugClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	parent_class = GTK_PLUG_CLASS (g_type_class_peek_parent (klass));

	g_type_class_add_private (klass, sizeof (GarglkPlugPrivate));

	object_class->finalize = garglk_plug_finalize;

    /* Set up the DBus signals */
    garglk_plug_signals[COMMAND_TEXT_SIGNAL] =
        g_signal_new("command_text",
                     G_OBJECT_CLASS_TYPE (klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);
    garglk_plug_signals[REPLY_TEXT_SIGNAL] =
        g_signal_new("reply_text",
                     G_OBJECT_CLASS_TYPE (klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);
    garglk_plug_signals[STORY_TITLE_SIGNAL] =
        g_signal_new("story_title",
                     G_OBJECT_CLASS_TYPE (klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);
    garglk_plug_signals[READY_SIGNAL] =
        g_signal_new("ready",
                     G_OBJECT_CLASS_TYPE (klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);
    garglk_plug_signals[STOPPED_SIGNAL] =
        g_signal_new("stopped",
                     G_OBJECT_CLASS_TYPE (klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);
    
    /* Init the DBus connection, per-klass */
    GError *err = NULL;
	klass->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &err);
	if (klass->connection == NULL)
        g_error("Unable to connect to dbus: %s", err->message);
    dbus_g_object_type_install_info(GARGLK_TYPE_PLUG,
                                    &dbus_glib_garglk_object_info);
}

GType
garglk_plug_get_type (void)
{
	static GType our_type = 0;

	if(our_type == 0)
	{
		static const GTypeInfo our_info =
		{
			sizeof (GarglkPlugClass), /* class_size */
			(GBaseInitFunc) NULL, /* base_init */
			(GBaseFinalizeFunc) NULL, /* base_finalize */
			(GClassInitFunc) garglk_plug_class_init, /* class_init */
			(GClassFinalizeFunc) NULL, /* class_finalize */
			NULL /* class_data */,
			sizeof (GarglkPlug), /* instance_size */
			0, /* n_preallocs */
			(GInstanceInitFunc) garglk_plug_init, /* instance_init */
			NULL /* value_table */
		};

		our_type = g_type_register_static (GTK_TYPE_PLUG, "GarglkPlug",
		                                   &our_info, 0);
	}

	return our_type;
}

GtkWidget *
garglk_plug_new(GdkNativeWindow socket_id)
{
	GtkWidget *obj;
	obj = GTK_WIDGET(g_object_new(GARGLK_TYPE_PLUG, NULL));
    gtk_plug_construct(&(GARGLK_PLUG(obj)->parent_instance), socket_id);
	return obj;
}

void
garglk_plug_send_command_text (GarglkPlug *plug, const gchar *text)
{
    g_signal_emit(plug, garglk_plug_signals[COMMAND_TEXT_SIGNAL], 0, text);
}

void
garglk_plug_send_reply_text (GarglkPlug *plug, const gchar *text)
{
    g_signal_emit(plug, garglk_plug_signals[REPLY_TEXT_SIGNAL], 0, text);
}

void
garglk_plug_send_story_title (GarglkPlug *plug, const gchar *title)
{
    g_signal_emit(plug, garglk_plug_signals[STORY_TITLE_SIGNAL], 0, title);
}

void
garglk_plug_send_stopped (GarglkPlug *plug)
{
    g_signal_emit(plug, garglk_plug_signals[STOPPED_SIGNAL], 0);
}

gboolean
garglk_plug_get_interactive (GarglkPlug *plug)
{
	return GARGLK_PLUG_PRIVATE(plug)->interactive;
}

gboolean
garglk_plug_get_protected (GarglkPlug *plug)
{
	return GARGLK_PLUG_PRIVATE(plug)->protect;
}

/**
 * D-BUS method implementation
 */
gboolean
garglk_set_interactive(GarglkPlug *plug, gboolean interactive, GError **error)
{
    GARGLK_PLUG_PRIVATE(plug)->interactive = interactive;
    return TRUE;
}

gboolean
garglk_set_protected(GarglkPlug *plug, gboolean protect, GError **error)
{
    GARGLK_PLUG_PRIVATE(plug)->protect = protect;
    return TRUE;
}

gboolean
garglk_feed_text(GarglkPlug *plug, gchar *text, GError **error)
{
    /* Feed the letters one by one as keystrokes */
    int pos;
    for(pos = 0; pos < strlen(text); pos++) {
        if(text[pos] == '\n')
            gli_input_handle_key(keycode_Return);
        else if(isprint(text[pos]))
            gli_input_handle_key(text[pos]);
    }
    return TRUE;
}

gboolean
garglk_request_window(GarglkPlug *plug, guint *windowid, GError **error)
{
    *windowid = (guint)gtk_plug_get_id(GTK_PLUG(plug));
    return TRUE;
}

gboolean
garglk_set_minimum_size(GarglkPlug *plug, guint x, guint y, GError **error)
{
    gli_cols = (x - 2 * gli_wmarginx) / gli_cellw;
    gli_rows = (y - 2 * gli_wmarginy) / gli_cellh;
    
    gtk_widget_set_size_request(GTK_WIDGET(plug), x, y);
    gli_force_redraw = 1;
	gli_windows_size_change();
    
    return TRUE;
}

gboolean
garglk_grab_focus(GarglkPlug *plug, GError **error)
{
    gtk_widget_grab_focus(gtk_bin_get_child(GTK_BIN(plug)));
    return TRUE;
}

gboolean
garglk_stop(GarglkPlug *plug, GError **error)
{
    glk_exit();
    return TRUE;
}
