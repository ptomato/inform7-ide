#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>
#ifdef GSTREAMER_SOUND
#include <gst/gst.h>
#endif

static gboolean chimara_initialized = FALSE;

/* This function is called at every entry point of the library, to set up
threads and gettext. It is NOT called from Glk functions. */
void
chimara_init(void)
{
	if( G_UNLIKELY(!chimara_initialized) )
	{
		/* Setup gettext */
		bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
		bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
		
		/* Make sure threads have been initialized */
		if(!g_thread_supported())
			g_error(_("In order to use the Chimara library, you must initialize"
				" the thread system by calling g_threads_init() and "
				"gdk_threads_init() BEFORE the initial call to gtk_init() in "
				"your main program."));

#ifdef GSTREAMER_SOUND
		/* Make sure GStreamer has been initialized if it hasn't been already;
		in particular, if you want your program to parse GStreamer command line
		options then you should do it yourself, before gtk_init(). */
		
		/* SUCKY DEBIAN: gst_is_initialized() supported from 0.10.30 onward */
		/*if( !gst_is_initialized() )*/
			gst_init(NULL, NULL);
#endif

		/* Initialize thread-private data */
		extern GPrivate *glk_data_key;
		glk_data_key = g_private_new(NULL);
	}
}

