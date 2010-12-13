/* Copyright / licensing information here. */

#ifndef __CHIMARA_GLK_H__
#define __CHIMARA_GLK_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <pango/pango.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_GLK            (chimara_glk_get_type())
#define CHIMARA_GLK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_GLK, ChimaraGlk))
#define CHIMARA_GLK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_GLK, ChimaraGlkClass))
#define CHIMARA_IS_GLK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_GLK))
#define CHIMARA_IS_GLK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_GLK))
#define CHIMARA_GLK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_GLK, ChimaraGlkClass))

/**
 * ChimaraGlk:
 * 
 * This structure contains no public members.
 */
typedef struct _ChimaraGlk {
	GtkContainer parent_instance;
    
	/*< public >*/
} ChimaraGlk;

typedef struct _ChimaraGlkClass {
	GtkContainerClass parent_class;
	/* Signals */
	void(* stopped) (ChimaraGlk *self);
	void(* started) (ChimaraGlk *self);
	void(* waiting) (ChimaraGlk *self);
	void(* char_input) (ChimaraGlk *self, guint32 window_rock, guint keysym);
	void(* line_input) (ChimaraGlk *self, guint32 window_rock, gchar *text);
	void(* text_buffer_output) (ChimaraGlk *self, guint32 window_rock, gchar *text);
	void(* iliad_screen_update) (ChimaraGlk *self, gboolean typing);
} ChimaraGlkClass;

typedef enum {
	CHIMARA_GLK_TEXT_BUFFER,
	CHIMARA_GLK_TEXT_GRID
} ChimaraGlkWindowType;

/**
 * ChimaraError:
 * @CHIMARA_LOAD_MODULE_ERROR: There was an error opening the plugin containing 
 * the Glk program. The error message from <link 
 * linkend="g-module-error">g_module_error()</link> is appended to the <link
 * linkend="GError">GError</link> message.
 * @CHIMARA_NO_GLK_MAIN: The plugin containing the Glk program did not export a 
 * glk_main() function.
 * @CHIMARA_PLUGIN_NOT_FOUND: An appropriate interpreter plugin for the 
 * autodetected game file type could not be found.
 * @CHIMARA_PLUGIN_ALREADY_RUNNING: A plugin was opened while there was already
 * another plugin running in the widget.
 * 
 * Error codes returned by #ChimaraGlk widgets and subclasses.
 */
typedef enum _ChimaraError {
	CHIMARA_LOAD_MODULE_ERROR,
	CHIMARA_NO_GLK_MAIN,
	CHIMARA_PLUGIN_NOT_FOUND,
	CHIMARA_PLUGIN_ALREADY_RUNNING
} ChimaraError;

/**
 * ChimaraResourceType:
 * @CHIMARA_RESOURCE_SOUND: A sound file.
 * @CHIMARA_RESOURCE_IMAGE: An image file.
 *
 * The type of resource that the Glk program is requesting, passed to a
 * #ChimaraResourceLoadFunc.
 */
typedef enum _ChimaraResourceType {
	CHIMARA_RESOURCE_SOUND,
	CHIMARA_RESOURCE_IMAGE
} ChimaraResourceType;

/**
 * ChimaraResourceLoadFunc:
 *
 * The type of function passed to chimara_glk_set_resource_load_callback(). It
 * takes a #ChimaraResourceType constant, @usage, to indicate what sort of 
 * resource to look for; @resnum is the resource number to look for, and
 * @user_data is the user data provided along with the callback. The function
 * must return an allocated string containing the filename where the resource
 * can be found.
 */
typedef gchar * (*ChimaraResourceLoadFunc)(ChimaraResourceType usage, guint32 resnum, gpointer user_data);

/**
 * CHIMARA_ERROR:
 *
 * The domain of errors raised by Chimara widgets.
 */
#define CHIMARA_ERROR chimara_error_quark()

GQuark chimara_error_quark(void);
GType chimara_glk_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_glk_new(void);
void chimara_glk_set_interactive(ChimaraGlk *glk, gboolean interactive);
gboolean chimara_glk_get_interactive(ChimaraGlk *glk);
void chimara_glk_set_protect(ChimaraGlk *glk, gboolean protect);
gboolean chimara_glk_get_protect(ChimaraGlk *glk);
void chimara_glk_set_css_to_default(ChimaraGlk *glk);
gboolean chimara_glk_set_css_from_file(ChimaraGlk *glk, const gchar *filename, GError **error);
void chimara_glk_set_css_from_string(ChimaraGlk *glk, const gchar *css);
void chimara_glk_set_spacing(ChimaraGlk *glk, guint spacing);
guint chimara_glk_get_spacing(ChimaraGlk *glk);
gboolean chimara_glk_run(ChimaraGlk *glk, const gchar *plugin, int argc, char *argv[], GError **error);
void chimara_glk_stop(ChimaraGlk *glk);
void chimara_glk_wait(ChimaraGlk *glk);
gboolean chimara_glk_get_running(ChimaraGlk *glk);
void chimara_glk_feed_char_input(ChimaraGlk *glk, guint32 keyval);
void chimara_glk_feed_line_input(ChimaraGlk *glk, const gchar *text);
gboolean chimara_glk_is_char_input_pending(ChimaraGlk *glk);
gboolean chimara_glk_is_line_input_pending(ChimaraGlk *glk);
GtkTextTag *chimara_glk_get_tag(ChimaraGlk *glk, ChimaraGlkWindowType window, const gchar *name);
const gchar **chimara_glk_get_tag_names(ChimaraGlk *glk);
gint chimara_glk_get_num_tag_names(ChimaraGlk *glk);
void chimara_glk_update_style(ChimaraGlk *glk);
void chimara_glk_set_resource_load_callback(ChimaraGlk *glk, ChimaraResourceLoadFunc func, gpointer user_data);

G_END_DECLS

#endif /* __CHIMARA_GLK_H__ */
