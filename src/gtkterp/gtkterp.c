/***************************************************************************
 *            gtkterp.c
 *
 *  Sun Apr 29 02:27:45 2007
 *  Copyright  2007  P.F. Chimento
 *  philip.chimento@gmail.com
 ***************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <gtk/gtk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

#include "gtkterp.h"
#include "dbus-client.h"

/* Forward declarations */
static void gtk_terp_class_init(GtkTerpClass *klass);
static void gtk_terp_init(GtkTerp *sp);
static void gtk_terp_finalize(GObject *object);
static void interpreter_finished(GPid pid, gint status, GtkTerp *terp);
static gboolean do_not_destroy_socket(GtkSocket *socket, gpointer data);
static void on_command_text(DBusGProxy *proxy, gchar *text, GtkTerp *terp);
static void on_reply_text(DBusGProxy *proxy, gchar *text, GtkTerp *terp);
static void on_story_title(DBusGProxy *proxy, gchar *text, GtkTerp *terp);
static void on_ready(DBusGProxy *proxy, GtkTerp *terp);
static void on_grab_focus(GtkWidget *widget, gpointer data);

struct _GtkTerpPrivate {
    guint loaded : 1;
    guint running : 1;
    guint interactive : 1;
    guint protect : 1;
    gchar *filename;
    DBusGConnection *connection;
    DBusGProxy *proxy;
    GPid pid;
};

typedef struct _GtkTerpSignal GtkTerpSignal;
typedef enum _GtkTerpSignalType GtkTerpSignalType;

enum _GtkTerpSignalType {
	COMMAND_RECEIVED_SIGNAL,
    TEXT_REPLIED_SIGNAL,
    TITLE_CHANGED_SIGNAL,
    GAME_FINISHED_SIGNAL,
    STOPPED_SIGNAL,
	LAST_SIGNAL
};

struct _GtkTerpSignal {
	GtkTerp *object;
};

static guint gtk_terp_signals[LAST_SIGNAL] = { 0 };
static GtkSocketClass *parent_class = NULL;

GType
gtk_terp_get_type()
{
	static GType type = 0;

	if(type == 0) {
		static const GTypeInfo our_info = {
			sizeof (GtkTerpClass),
			NULL,
			NULL,
			(GClassInitFunc)gtk_terp_class_init,
			NULL,
			NULL,
			sizeof (GtkTerp),
			0,
			(GInstanceInitFunc)gtk_terp_init,
            NULL
		};

		type = g_type_register_static(GTK_TYPE_SOCKET, 
			"GtkTerp", &our_info, 0);
	}

	return type;
}

static void
gtk_terp_class_init(GtkTerpClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = gtk_terp_finalize;
	
	gtk_terp_signals[COMMAND_RECEIVED_SIGNAL] = g_signal_new(
      "command-received",
      G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET(GtkTerpClass, command_received),
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);
    
    gtk_terp_signals[TEXT_REPLIED_SIGNAL] = g_signal_new(
      "text-replied",
      G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET(GtkTerpClass, text_replied),
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);
    
    gtk_terp_signals[TITLE_CHANGED_SIGNAL] = g_signal_new(
      "title-changed",
      G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET(GtkTerpClass, title_changed),
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);
    
    gtk_terp_signals[GAME_FINISHED_SIGNAL] = g_signal_new(
      "game-finished",
      G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET(GtkTerpClass, game_finished),
      NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE, 0);
    
    gtk_terp_signals[STOPPED_SIGNAL] = g_signal_new(
      "stopped",
      G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET(GtkTerpClass, stopped),
      NULL, NULL,
      g_cclosure_marshal_VOID__INT,
      G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
gtk_terp_init(GtkTerp *obj)
{
	obj->priv = g_new0(GtkTerpPrivate, 1);
    
    obj->priv->loaded = 0;
	obj->priv->running = 0;
    obj->priv->interactive = 1;
    obj->priv->filename = NULL;
    
    GError *err = NULL;
    DBusGConnection *connection = dbus_g_bus_get(DBUS_BUS_SESSION, &err);
    /*g_return_if_fail(connection != NULL);*/
    if(connection == NULL)
        g_error("Error getting bus connection: %s\n", err->message);
    obj->priv->connection = connection;
    obj->priv->proxy = NULL;
    
    /* Pass the focus on to the interpreter */
    g_object_set(obj, 
                 "can-focus", TRUE,
                 "events", GDK_ALL_EVENTS_MASK,
                 NULL);
    g_signal_connect(GTK_WIDGET(obj), "grab-focus", G_CALLBACK(on_grab_focus),
                     NULL);
    
    /* Prevent the socket from being destroyed when the interpreter exits */
    g_signal_connect(obj, "plug-removed", G_CALLBACK(do_not_destroy_socket),
                     NULL);
}

static void
gtk_terp_finalize(GObject *object)
{
	GtkTerp *cobj;
	cobj = GTK_TERP(object);
	
    if(cobj->priv->filename)
        g_free(cobj->priv->filename);   
    
    if(cobj->priv->proxy)
        g_object_unref(cobj->priv->proxy);
    
	g_free(cobj->priv);
	G_OBJECT_CLASS(parent_class)->finalize(object);
}

/* Check the exit code of the interpreter and emit the "stopped" signal */
static void
interpreter_finished(GPid pid, gint status, GtkTerp *terp)
{
    int exit_code = WIFEXITED(status)? WEXITSTATUS(status) : -1;
    g_spawn_close_pid(pid);
    terp->priv->running = 0;
    
    /* Destroy the D-bus proxy */
    g_object_unref(terp->priv->proxy);
    terp->priv->proxy = NULL;
    
    /* Emit signal */
    g_signal_emit(terp, gtk_terp_signals[STOPPED_SIGNAL], 0, exit_code);
}

/* Return TRUE so that the socket is not destroyed when the plug exits */
static gboolean
do_not_destroy_socket(GtkSocket *socket, gpointer data)
{
    return TRUE;
}

/* Handle the "CommandText" signal from D-Bus */
static void
on_command_text(DBusGProxy *proxy, gchar *text, GtkTerp *terp)
{
    g_signal_emit(terp, gtk_terp_signals[COMMAND_RECEIVED_SIGNAL], 0, text);
}

/* Handle the "ReplyText" signal from D-Bus */
static void
on_reply_text(DBusGProxy *proxy, gchar *text, GtkTerp *terp)
{
    g_signal_emit(terp, gtk_terp_signals[TEXT_REPLIED_SIGNAL], 0, text);
}

/* Handle the "StoryTitle" signal from D-Bus */
static void
on_story_title(DBusGProxy *proxy, gchar *text, GtkTerp *terp)
{
    g_signal_emit(terp, gtk_terp_signals[TITLE_CHANGED_SIGNAL], 0, text);
}

/* Finish initializing the interpreter, after it emits the "Ready" signal 
We have to do this, because the interpreter does not register its interface
immediately on D-Bus */
static void
on_ready(DBusGProxy *proxy, GtkTerp *terp)
{
    /* Set initial state */
    GError *err = NULL;
    if(!org_informfiction_garglk_set_interactive(proxy, terp->priv->interactive,
                                                 &err))
        g_error("Cannot set interactive: %s", err->message);
    if(!org_informfiction_garglk_set_protected(proxy, terp->priv->protect,
                                               &err))
        g_error("Cannot set protected: %s", err->message);
    
    /* request the window ID and embed the interpreter */
    guint windowid;
    if(!org_informfiction_garglk_request_window(proxy, &windowid, &err)) {
        g_error("Cannot request window id: %s", err->message);
    }
    gtk_socket_add_id(GTK_SOCKET(terp), windowid);

    /* Update socket state */
    terp->priv->running = 1;
}
    
static void
on_grab_focus(GtkWidget *widget, gpointer data)
{
    if(GTK_TERP(widget)->priv->running == 1)
        g_assert(
          org_informfiction_garglk_grab_focus(GTK_TERP(widget)->priv->proxy,
                                              NULL));
}
    
static void
on_stop(DBusGProxy *proxy, GtkTerp *terp)
{
    g_signal_emit(terp, gtk_terp_signals[GAME_FINISHED_SIGNAL], 0);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * gtk_terp_new:
 *
 * Creates a new interpreter widget.
 *
 * Returns: a new #GtkTerp.
 */
GtkWidget *
gtk_terp_new()
{
	GtkWidget *obj;
	
	obj = GTK_WIDGET(g_object_new(GTK_TYPE_TERP, NULL));
	
	return obj;
}

/**
 * gtk_terp_load_game:
 * @terp: a #GtkTerp
 * @filename: The name of the file to load in the interpreter
 * @error: Return location for error
 *
 * Loads a game file into the interpreter. If a game is already loaded but not
 * running, replaces it. If a game is already running, or if the file is not a
 * valid game file, returns an error.
 *
 * Returns: %TRUE on success, %FALSE if error was set
 */
gboolean
gtk_terp_load_game(GtkTerp *terp, const gchar *filename, GError **error)
{
    g_return_val_if_fail(terp != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    g_return_val_if_fail(filename != NULL, FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
    
    if(terp->priv->running == 1) {
        g_set_error(error, GTK_TERP_ERROR, GTK_TERP_ERROR_GAME_IS_RUNNING,
                    "Tried to load game while one was already running");
        return FALSE;
    }
    if(!g_file_test(filename, G_FILE_TEST_EXISTS)) {
        g_set_error(error, GTK_TERP_ERROR, GTK_TERP_ERROR_FILE_NOT_FOUND,
                    "Tried to load non-existent game file '%s'", filename);
        return FALSE;
    }
    
    /* Only unload the previous game if we are sure we can load the new one */
    if(terp->priv->loaded == 1)
        gtk_terp_unload_game(terp);
    
    /* Load game */
    terp->priv->loaded = 1;
    if(terp->priv->filename != NULL)
        g_free(terp->priv->filename);
    terp->priv->filename = g_strdup(filename);
    
    return TRUE;
}

/**
 * gtk_terp_unload_game:
 * @terp: a #GtkTerp
 *
 * Removes the game currently loaded in the interpreter. If there is no game
 * loaded, or if a game is currently running, does nothing.
 */
void
gtk_terp_unload_game(GtkTerp *terp) 
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    g_return_if_fail(terp->priv->loaded == 1);
    g_return_if_fail(terp->priv->running == 0);
    
    /* Unload the game */
    
    terp->priv->loaded = 0;
    if(terp->priv->filename != NULL)
        g_free(terp->priv->filename);
    terp->priv->filename = NULL;
}

/**
 * gtk_terp_get_game_loaded:
 * @terp: a #GtkTerp
 *
 * Returns whether a game is currently loaded in the interpreter.
 *
 * Returns: %TRUE if a game is loaded, %FALSE otherwise
 */
gboolean
gtk_terp_get_game_loaded(GtkTerp *terp)
{
    g_return_val_if_fail(terp != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    return terp->priv->loaded == 1;
}

/**
 * gtk_terp_get_filename:
 * @terp: a #GtkTerp
 *
 * Returns the path of the currently loaded game file.
 *
 * Returns: path to the currently loaded file in a newly-allocated string, or a
 * newly-allocated empty string if there is none.
 */
gchar *
gtk_terp_get_filename(GtkTerp *terp)
{
    g_return_val_if_fail(terp != NULL, NULL);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    
    if(terp->priv->filename != NULL)
        return g_strdup(terp->priv->filename);
    return g_strdup("");
}

/**
 * gtk_terp_start_game:
 * @terp: a #GtkTerp
 * @format: a value of #GtkTerpInterpreter telling which interpreter to load
 * @error: return location for error.
 *
 * Starts the interpreter specified by format. Does nothing if there is no game
 * file loaded. The #GtkTerp must have been added into a toplevel window for
 * this to work.
 *
 * Returns: %TRUE on success, %FALSE if error was set.
 */
gboolean
gtk_terp_start_game(GtkTerp *terp, const GtkTerpInterpreter format,
                    GError **error)
{
    g_return_val_if_fail(terp != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
    
    /* Check for runtime errors */
    if(terp->priv->loaded == 0) {
        g_set_error(error, GTK_TERP_ERROR, GTK_TERP_ERROR_GAME_NOT_LOADED,
                    "Tried to start game while none was loaded");
        return FALSE;
    }
    if(terp->priv->running == 1) {
        g_set_error(error, GTK_TERP_ERROR, GTK_TERP_ERROR_GAME_IS_RUNNING,
                    "Tried to start game while one was already running");
        return FALSE;
    }
    
    /* Spawn the interpreter process */
	gchar *args[3];
    switch(format) {
        case GTK_TERP_GLULXE:
            args[0] = g_strdup("gtkterp-glulxe");
            break;
        case GTK_TERP_FROTZ:
        default:
            args[0] = g_strdup("gtkterp-frotz");
    }
    args[1] = g_strdup(terp->priv->filename);
    args[2] = NULL;
    GPid child_pid;
	if(!g_spawn_async(NULL, /* working directory */
                      args, /* command to spawn */
                      NULL, /* environment */
                      G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                      NULL, /* setup function */
                      NULL, /* setup function data */
                      &child_pid,
                      error)) {
        g_assert(error == NULL || *error != NULL);
        g_strfreev(args);
        return FALSE;
    }
    
    /* Get D-Bus connection to process */
    DBusGConnection *connection = dbus_g_bus_get(DBUS_BUS_SESSION, error);
    if(connection == NULL) {
        g_assert(error == NULL || *error != NULL);
        return FALSE;
    }
    gchar *pathname = g_strdup_printf("/org/informfiction/garglk/%d",child_pid);
    DBusGProxy *proxy = dbus_g_proxy_new_for_name(terp->priv->connection,
                                                  "org.informfiction.garglk",
                                                  pathname,
                                                  "org.informfiction.garglk");
    g_free(pathname);
    if(proxy == NULL) {
        g_set_error(error, GTK_TERP_ERROR, GTK_TERP_ERROR_NO_PROXY,
                    "Cannot create D-BUS proxy object");
        return FALSE;
    }
    terp->priv->proxy = proxy;

    /* Set up listeners for D-Bus signals */
    dbus_g_proxy_add_signal(terp->priv->proxy, "CommandText", G_TYPE_STRING,
                            G_TYPE_INVALID);
    dbus_g_proxy_add_signal(terp->priv->proxy, "ReplyText", G_TYPE_STRING,
                            G_TYPE_INVALID);
    dbus_g_proxy_add_signal(terp->priv->proxy, "StoryTitle", G_TYPE_STRING,
                            G_TYPE_INVALID);
    dbus_g_proxy_add_signal(terp->priv->proxy, "Ready", G_TYPE_INVALID);
    dbus_g_proxy_add_signal(terp->priv->proxy, "Stopped", G_TYPE_INVALID);
    dbus_g_proxy_connect_signal(terp->priv->proxy, "CommandText",
                                G_CALLBACK(on_command_text), terp, NULL);
    dbus_g_proxy_connect_signal(terp->priv->proxy, "ReplyText",
                                G_CALLBACK(on_reply_text), terp, NULL);
    dbus_g_proxy_connect_signal(terp->priv->proxy, "StoryTitle",
                                G_CALLBACK(on_story_title), terp, NULL);
    dbus_g_proxy_connect_signal(terp->priv->proxy, "Ready",
                                G_CALLBACK(on_ready), terp, NULL);
    dbus_g_proxy_connect_signal(terp->priv->proxy, "Stopped",
                                G_CALLBACK(on_stop), terp, NULL);

    /* Set up the PID watch */
    g_child_watch_add(child_pid, (GChildWatchFunc)interpreter_finished, terp);
    terp->priv->pid = child_pid;

    /* The rest of the initialization is completed in interpreter_ready() */
    return TRUE;
}

/**
 * gtk_terp_stop_game
 * @terp: a #GtkTerp
 *
 * Stops the currently running game and ends the interpreter process. Does
 * nothing if there is no game loaded. Note that calling this function will
 * cause the #stopped signal to be emitted if the game had not finished.
 */
void
gtk_terp_stop_game(GtkTerp *terp)
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    g_return_if_fail(terp->priv->loaded == 1);
    
    /* Stop the interpreter if still running*/
    if(terp->priv->running == 1)
        dbus_g_proxy_call_no_reply(terp->priv->proxy, "Stop", G_TYPE_INVALID);
    /* Kill the process */
    kill((pid_t)terp->priv->pid, SIGTERM);
}

/**
 * gtk_terp_get_running:
 * @terp: a #GtkTerp
 *
 * Returns whether a game is currently running.
 *
 * Returns: %TRUE if a game is running, %FALSE otherwise.
 */
gboolean
gtk_terp_get_running(GtkTerp *terp)
{
    g_return_val_if_fail(terp != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    return terp->priv->running == 1;
}

/**
 * gtk_terp_set_interactive:
 * @terp: a #GtkTerp
 * @interactive: %TRUE to set interactive mode
 *
 * Sets whether the interpreter should be in interactive mode. In interactive
 * mode, the user will be prompted to press a key when the widget's screen is
 * full, asked whether he/she is sure about quitting, etc. If interactive mode
 * is off, the interpreter will assume that the commands are being entered
 * automatically. Interactive mode is on by deafult.
 */
void
gtk_terp_set_interactive(GtkTerp *terp, const gboolean interactive)
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    terp->priv->interactive = interactive? 1 : 0;
    
    if(terp->priv->running == 1)
        g_return_if_fail(
          org_informfiction_garglk_set_interactive(terp->priv->proxy,
                                                   interactive, NULL));
}

/**
 * gtk_terp_get_interactive:
 * @terp: a #GtkTerp
 *
 * Returns whether the interpreter is in interactive mode (see
 * gtk_terp_set_interactive()).
 *
 * Returns: %TRUE if interactive mode is on, %FALSE otherwise.
 */
gboolean
gtk_terp_get_interactive(GtkTerp *terp)
{
    g_return_val_if_fail(terp != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    return terp->priv->interactive == 1;
}

/**
 * gtk_terp_set_protected:
 * @terp: a #GtkTerp
 * @interactive: %TRUE to set protected mode
 *
 * Sets whether the interpreter should be in protected mode. In protected
 * mode, commands that involve writing to a file (save, restore, script, etc.)
 * will not work.
 */
void
gtk_terp_set_protected(GtkTerp *terp, const gboolean protect)
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    terp->priv->protect = protect? 1 : 0;
    
    if(terp->priv->running == 1)
        g_return_if_fail(
          org_informfiction_garglk_set_protected(terp->priv->proxy, protect,
                                                 NULL));
}

/**
 * gtk_terp_get_protected:
 * @terp: a #GtkTerp
 *
 * Returns whether the interpreter is in protected mode (see
 * gtk_terp_set_protected()).
 *
 * Returns: %TRUE if protected mode is on, %FALSE otherwise.
 */
gboolean
gtk_terp_get_protected(GtkTerp *terp)
{
    g_return_val_if_fail(terp != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_TERP(terp), FALSE);
    return terp->priv->protect == 1;
}

/**
 * gtk_terp_set_minimum_size:
 * @terp: a #GtkTerp
 * @x: horizontal size in pixels
 * @y: vertical size in pixels
 *
 * Sets the minimum size of the interpreter, telling it to recalculate the
 * number of rows and columns and redisplay. If the interpreter is not running,
 * it does nothing.
 */
void
gtk_terp_set_minimum_size(GtkTerp *terp, const guint x, const guint y)
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    
    if(terp->priv->running == 1)
        g_return_if_fail(
          org_informfiction_garglk_set_minimum_size(terp->priv->proxy, x, y,
                                                    NULL));
}

/**
 * gtk_terp_feed_command:
 * @terp: a #GtkTerp
 * @command: a command to pass to the game. Does not include newline.
 *
 * Passes a command to the game as if the user had typed it in and pressed
 * enter. Use this to automatically enter a single line of input. Does nothing
 * if there is no game running. Note that this function will cause the
 * #command-received signal to be emitted, even if the command is an empty
 * string.
 *
 * The string to pass may not include a newline.
 */
void
gtk_terp_feed_command(GtkTerp *terp, const gchar *command)
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    g_return_if_fail(command != NULL);
    g_return_if_fail(terp->priv->running == 1);
    g_return_if_fail(strchr(command, '\n') == NULL);
    gchar *text = g_strconcat(command, "\n", NULL);
    g_return_if_fail(org_informfiction_garglk_feed_text(terp->priv->proxy,
                                                        text, NULL));
}

/**
 * gtk_terp_feed_text:
 * @terp: a #GtkTerp
 * @text: literal text to pass to the game.
 *
 * Passes text to the game as if the user had typed exactly that text. Use this
 * to automatically enter many lines at once, or less than one line. Does
 * nothing if there is no game running. Note that the #command-received signal
 * will only be emitted if the text contains a newline; it is possible to
 * trigger more than one #command-received signal.
 */
void
gtk_terp_feed_text(GtkTerp *terp, const gchar *text)
{
    g_return_if_fail(terp != NULL);
    g_return_if_fail(GTK_IS_TERP(terp));
    g_return_if_fail(text != NULL);
    g_return_if_fail(terp->priv->running == 1);
    g_return_if_fail(org_informfiction_garglk_feed_text(terp->priv->proxy,
                                                        text, NULL));
}

/**
 * gtk_terp_error_quark:
 *
 * Returns the error domain for #GtkTerp errors.
 *
 * Returns: a #GQuark identifying the #GtkTerp error domain
 */
GQuark
gtk_terp_error_quark(void)
{
    return g_quark_from_string("gtk-terp-error-quark");
}
