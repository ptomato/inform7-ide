#ifndef __CHIMARA_GLK_PRIVATE_H__
#define __CHIMARA_GLK_PRIVATE_H__

#include <glib.h>
#include <gmodule.h>
#include <pango/pango.h>
#include "glk.h"
#include "style.h"
#include "gi_blorb.h"
#include "gi_dispa.h"
#include "chimara-glk.h"

G_BEGIN_DECLS

typedef struct _ChimaraGlkPrivate ChimaraGlkPrivate;

struct _ChimaraGlkPrivate {
    /* Pointer back to the widget itself for use in thread */
    ChimaraGlk *self;

	/* *** Widget properties *** */
    /* Whether user input is expected */
    gboolean interactive;
    /* Whether file operations are allowed */
    gboolean protect;
	/* Spacing between Glk windows */
	guint spacing;
	/* The CSS file to read style defaults from */
	gchar *css_file;
	/* Hashtable containing the current styles set by CSS and GLK */
	struct StyleSet *styles;
	struct StyleSet *glk_styles;
	PangoAttrList *pager_attr_list;
	/* Final message displayed when game exits */
	gchar *final_message;
	/* Image cache */
	GSList *image_cache;

	/* *** Threading data *** */
	/* Whether program is running */
	gboolean running;
    /* Glk program loaded in widget */
    GModule *program;
    /* Thread in which Glk program is run */
    GThread *thread;
    /* Event queue and threading stuff */
    GQueue *event_queue;
    GMutex *event_lock;
    GCond *event_queue_not_empty;
    GCond *event_queue_not_full;
    /* Abort mechanism */
    GMutex *abort_lock;
    gboolean abort_signalled;
	/* Key press after shutdown mechanism */
	GMutex *shutdown_lock;
	GCond *shutdown_key_pressed;
	/* Window arrangement locks */
	GMutex *arrange_lock;
	GCond *rearranged;
	gboolean needs_rearrange;
	gboolean ignore_next_arrange_event;
	/* Input queues */
	GAsyncQueue *char_input_queue;
	GAsyncQueue *line_input_queue;
	/* Resource loading locks */
	GMutex *resource_lock;
	GCond *resource_loaded;
	GCond *resource_info_available;
	guint32 resource_available;

	/* *** Glk library data *** */
	/* Info about current plugin */
	gchar *program_name;
	gchar *program_info;
	gchar *story_name;
    /* User-defined interrupt handler */
    void (*interrupt_handler)(void);
    /* Global tree of all windows */
    GNode *root_window;
    /* List of filerefs currently in existence */
    GList *fileref_list;
    /* Current stream */
    strid_t current_stream;
    /* List of streams currently in existence */
    GList *stream_list;
	/* List of sound channels currently in existence */
	GList *schannel_list;
	/* Current timer */
	guint timer_id;
	/* Current resource blorb map */
	giblorb_map_t *resource_map;
	/* File stream pointing to the blorb used as current resource map */
	strid_t resource_file;
	/* Optional callback for loading resource data */
	ChimaraResourceLoadFunc resource_load_callback;
	gpointer resource_load_callback_data;
	/* Callbacks for registering and unregistering dispatch objects */
	gidispatch_rock_t (*register_obj)(void *, glui32);
	void (*unregister_obj)(void *, glui32, gidispatch_rock_t);
	gidispatch_rock_t (*register_arr)(void *, glui32, char *);
	void (*unregister_arr)(void *, glui32, char *, gidispatch_rock_t);

	/* *** Platform-dependent Glk library data *** */
	/* Flag for functions to find out if they are being called from startup code */
	gboolean in_startup;
	/* "Current directory" for creating filerefs */
	gchar *current_dir;
};

#define CHIMARA_GLK_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHIMARA_TYPE_GLK, ChimaraGlkPrivate))
#define CHIMARA_GLK_USE_PRIVATE(o, n) ChimaraGlkPrivate *n = CHIMARA_GLK_PRIVATE(o)

G_END_DECLS

#endif /* __CHIMARA_GLK_PRIVATE_H__ */
