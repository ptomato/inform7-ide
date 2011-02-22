#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <libchimara/glk.h>
#ifdef GSTREAMER_SOUND
#include <gst/gst.h>
#endif
#include "magic.h"
#include "schannel.h"
#include "chimara-glk-private.h"
#include "gi_dispa.h"
#include "gi_blorb.h"
#include "resource.h"
#include "event.h"

extern GPrivate *glk_data_key;

#ifdef GSTREAMER_SOUND
/* Stop any currently playing sound on this channel, and remove any
 format-specific GStreamer elements from the channel. */
static void
clean_up_after_playing_sound(schanid_t chan)
{
	if(!gst_element_set_state(chan->pipeline, GST_STATE_NULL))
		WARNING_S(_("Could not set GstElement state to"), "NULL");
	if(chan->demux)
	{
		gst_bin_remove(GST_BIN(chan->pipeline), chan->demux);
		chan->demux = NULL;
	}
	if(chan->decode)
	{
		gst_bin_remove(GST_BIN(chan->pipeline), chan->decode);
		chan->decode = NULL;
	}
}

/* This signal is thrown whenever the GStreamer pipeline generates a message.
 Most messages are harmless. */
static void
on_pipeline_message(GstBus *bus, GstMessage *message, schanid_t s)
{
	/* g_printerr("Got %s message\n", GST_MESSAGE_TYPE_NAME(message)); */

	GError *err;
	gchar *debug_message;
	
	switch(GST_MESSAGE_TYPE(message)) {
	case GST_MESSAGE_ERROR: 
	{
		gst_message_parse_error(message, &err, &debug_message);
		IO_WARNING(_("GStreamer error"), err->message, debug_message);
		g_error_free(err);
		g_free(debug_message);
		clean_up_after_playing_sound(s);
	}
		break;
	case GST_MESSAGE_WARNING:
	{
		gst_message_parse_warning(message, &err, &debug_message);
		IO_WARNING(_("GStreamer warning"), err->message, debug_message);
		g_error_free(err);
		g_free(debug_message);
	}
		break;
	case GST_MESSAGE_INFO:
	{
		gst_message_parse_info(message, &err, &debug_message);
		g_message("GStreamer info \"%s\": %s", err->message, debug_message);
		g_error_free(err);
		g_free(debug_message);
	}
		break;
	case GST_MESSAGE_EOS: /* End of stream */
		/* Decrease repeats if not set to forever */
		if(s->repeats != (glui32)-1)
			s->repeats--;
		if(s->repeats > 0) {
			if(!gst_element_seek_simple(s->pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 0)) {
				WARNING(_("Could not execute GStreamer seek"));
				clean_up_after_playing_sound(s);
			}
		} else {
			clean_up_after_playing_sound(s);
			/* Sound ended normally, send a notification if requested */
			if(s->notify)
				event_throw(s->glk, evtype_SoundNotify, NULL, s->resource, s->notify);
		}
		break;
	default:
		/* unhandled message */
		break;
	}
}

/* This signal is thrown when the OGG demuxer element has decided what kind of
 outputs it will output. We connect the decoder element dynamically. */
static void
on_ogg_demuxer_pad_added(GstElement *demux, GstPad *pad, schanid_t s)
{
	GstPad *sinkpad;
	
	/* We can now link this pad with the vorbis-decoder sink pad */
	sinkpad = gst_element_get_static_pad(s->decode, "sink");
	if(gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
		WARNING(_("Could not link OGG demuxer with Vorbis decoder"));
	gst_object_unref(sinkpad);
}

/* This signal is thrown when the typefinder element has found the type of its
 input. Now that we know what kind of input stream we have, we can connect the
 proper demuxer/decoder elements. */
static void
on_type_found(GstElement *typefind, guint probability, GstCaps *caps, schanid_t s)
{
	gchar *type = gst_caps_to_string(caps);
	if(strcmp(type, "application/ogg") == 0) {
		s->demux = gst_element_factory_make("oggdemux", NULL);
		s->decode = gst_element_factory_make("vorbisdec", NULL);
		if(!s->demux || !s->decode) {
			WARNING(_("Could not create one or more GStreamer elements"));
			goto finally;
		}
		gst_bin_add_many(GST_BIN(s->pipeline), s->demux, s->decode, NULL);
		if(!gst_element_link(s->typefind, s->demux) || !gst_element_link(s->decode, s->convert)) {
			WARNING(_("Could not link GStreamer elements"));
			goto finally;
		}
		/* We link the demuxer and decoder together dynamically, since the
		 demuxer doesn't know what source pads it will have until it starts
		 demuxing the stream */
		g_signal_connect(s->demux, "pad-added", G_CALLBACK(on_ogg_demuxer_pad_added), s);
	} else if(strcmp(type, "audio/x-aiff") == 0) {
		s->decode = gst_element_factory_make("aiffparse", NULL);
		if(!s->decode) {
			WARNING(_("Could not create 'aiffparse' GStreamer element"));
			goto finally;
		}
		gst_bin_add(GST_BIN(s->pipeline), s->decode);
		if(!gst_element_link_many(s->typefind, s->decode, s->convert, NULL)) {
			WARNING(_("Could not link GStreamer elements"));
			goto finally;
		}
	} else if(strcmp(type, "audio/x-mod") == 0) {
		s->decode = gst_element_factory_make("modplug", NULL);
		if(!s->decode) {
			WARNING(_("Could not create 'modplug' GStreamer element"));
			goto finally;
		}
		gst_bin_add(GST_BIN(s->pipeline), s->decode);
		if(!gst_element_link_many(s->typefind, s->decode, s->convert, NULL)) {
			WARNING(_("Could not link GStreamer elements"));
			goto finally;
		}
	} else {
		WARNING_S(_("Unexpected audio type in blorb"), type);
	}

finally:
	g_free(type);
}
#endif /* GSTREAMER_SOUND */

/**
 * glk_schannel_create:
 * @rock: The rock value to give the new sound channel.
 *
 * This creates a sound channel, about as you'd expect.
 *
 * Remember that it is possible that the library will be unable to create a new
 * channel, in which case glk_schannel_create() will return %NULL.
 *
 * Returns: A new sound channel, or %NULL.
 */
schanid_t 
glk_schannel_create(glui32 rock)
{
#ifdef GSTREAMER_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	schanid_t s = g_new0(struct glk_schannel_struct, 1);
	s->magic = MAGIC_SCHANNEL;
	s->rock = rock;
	if(glk_data->register_obj)
		s->disprock = (*glk_data->register_obj)(s, gidisp_Class_Schannel);

	/* Add it to the global sound channel list */
	glk_data->schannel_list = g_list_prepend(glk_data->schannel_list, s);
	s->schannel_list = glk_data->schannel_list;

	/* Add a pointer to the ChimaraGlk widget, for convenience */
	s->glk = glk_data->self;

	/* Create a GStreamer pipeline for the sound channel */
	gchar *pipeline_name = g_strdup_printf("pipeline-%p", s);
	s->pipeline = gst_pipeline_new(pipeline_name);
	g_free(pipeline_name);

	/* Watch for messages from the pipeline */
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(s->pipeline));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message", G_CALLBACK(on_pipeline_message), s);
	gst_object_unref(bus);

	/* Create GStreamer elements to put in the pipeline */
	s->source = gst_element_factory_make("giostreamsrc", NULL);
	s->typefind = gst_element_factory_make("typefind", NULL);
	s->convert = gst_element_factory_make("audioconvert", NULL);
	s->filter = gst_element_factory_make("volume", NULL);
	s->sink = gst_element_factory_make("autoaudiosink", NULL);
	if(!s->source || !s->typefind || !s->convert || !s->filter || !s->sink) {
		WARNING(_("Could not create one or more GStreamer elements"));
		goto fail;
	}

	/* Put the elements in the pipeline and link as many together as we can
	 without knowing the type of the audio stream */
	gst_bin_add_many(GST_BIN(s->pipeline), s->source, s->typefind, s->convert, s->filter, s->sink, NULL);
	/* Link elements: Source -> typefinder -> ??? -> Converter -> Volume filter -> Sink */
	if(!gst_element_link(s->source, s->typefind) || !gst_element_link_many(s->convert, s->filter, s->sink, NULL)) {
		WARNING(_("Could not link GStreamer elements"));
		goto fail;
	}
	g_signal_connect(s->typefind, "have-type", G_CALLBACK(on_type_found), s);
	
	return s;

fail:
	glk_schannel_destroy(s);
	return NULL;
#else
	return NULL;
#endif /* GSTREAMER_SOUND */
}

/**
 * glk_schannel_destroy:
 * @chan: The sound channel to destroy.
 *
 * Destroys the channel. If the channel is playing a sound, the sound stops 
 * immediately (with no notification event).
 */
void 
glk_schannel_destroy(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);

#ifdef GSTREAMER_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	if(!gst_element_set_state(chan->pipeline, GST_STATE_NULL))
		WARNING_S(_("Could not set GstElement state to"), "NULL");
	
	glk_data->schannel_list = g_list_delete_link(glk_data->schannel_list, chan->schannel_list);

	if(glk_data->unregister_obj)
	{
		(*glk_data->unregister_obj)(chan, gidisp_Class_Schannel, chan->disprock);
		chan->disprock.ptr = NULL;
	}

	/* This also frees all the objects inside the pipeline */
	if(chan->pipeline)
		gst_object_unref(chan->pipeline);
	
	chan->magic = MAGIC_FREE;
	g_free(chan);
#endif
}

/**
 * glk_schannel_iterate:
 * @chan: A sound channel, or %NULL.
 * @rockptr: Return location for the next sound channel's rock, or %NULL.
 *
 * This function can be used to iterate through the list of all open channels.
 * See <link linkend="chimara-Iterating-Through-Opaque-Objects">Iterating 
 * Through Opaque Objects</link>.
 *
 * As that section describes, the order in which channels are returned is 
 * arbitrary.
 *
 * Returns: the next sound channel, or %NULL if there are no more.
 */
schanid_t 
glk_schannel_iterate(schanid_t chan, glui32 *rockptr)
{
	VALID_SCHANNEL_OR_NULL(chan, return NULL);

#ifdef GSTREAMER_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GList *retnode;
	
	if(chan == NULL)
		retnode = glk_data->schannel_list;
	else
		retnode = chan->schannel_list->next;
	schanid_t retval = retnode? (schanid_t)retnode->data : NULL;
		
	/* Store the sound channel's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_schannel_get_rock(retval);
		
	return retval;
#else
	return NULL;
#endif /* GSTREAMER_SOUND */
}

/**
 * glk_schannel_get_rock:
 * @chan: A sound channel.
 * 
 * Retrieves the channel's rock value. See <link 
 * linkend="chimara-Rocks">Rocks</link>.
 *
 * Returns: A rock value.
 */
glui32 
glk_schannel_get_rock(schanid_t chan)
{
	VALID_SCHANNEL(chan, return 0);
	return chan->rock;
}

/**
 * glk_schannel_play:
 * @chan: Channel to play the sound in.
 * @snd: Resource number of the sound to play.
 *
 * Begins playing the given sound on the channel. If the channel was already
 * playing a sound (even the same one), the old sound is stopped (with no
 * notification event.
 *
 * This returns 1 if the sound actually started playing, and 0 if there was any
 * problem.
 * <note><para>
 *   The most obvious problem is if there is no sound resource with the given
 *   identifier. But other problems can occur. For example, the MOD-playing 
 *   facility in a library might be unable to handle two MODs at the same time,
 *   in which case playing a MOD resource would fail if one was already playing.
 * </para></note>
 *
 * Returns: 1 on success, 0 on failure.
 */
glui32 
glk_schannel_play(schanid_t chan, glui32 snd)
{
	return glk_schannel_play_ext(chan, snd, 1, 0);
}

/**
 * glk_schannel_play_ext:
 * @chan: Channel to play the sound in.
 * @snd: Resource number of the sound to play.
 * @repeats: Number of times to repeat the sound.
 * @notify: If nonzero, requests a notification when the sound is finished.
 *
 * This works the same as glk_schannel_play(), but lets you specify additional 
 * options. <code>glk_schannel_play(chan, snd)</code> is exactly equivalent to 
 * <code>glk_schannel_play_ext(chan, snd, 1, 0)</code>.
 * 
 * The @repeats value is the number of times the sound should be repeated. A 
 * repeat value of -1 (or rather 0xFFFFFFFF) means that the sound should repeat 
 * forever. A repeat value of 0 means that the sound will not be played at all; 
 * nothing happens. (Although a previous sound on the channel will be stopped, 
 * and the function will return 1.)
 * 
 * The @notify value should be nonzero in order to request a sound notification
 * event. If you do this, when the sound is completed, you will get an event 
 * with type %evtype_SoundNotify. The @window will be %NULL, @val1 will be the 
 * sound's resource id, and @val2 will be the nonzero value you passed as 
 * @notify.
 * 
 * If you request sound notification, and the repeat value is greater than one, 
 * you will get the event only after the last repetition. If the repeat value is
 * 0 or -1, you will never get a notification event at all. Similarly, if the 
 * sound is stopped or interrupted, or if the channel is destroyed while the 
 * sound is playing, there will be no notification event.
 *
 * Not all libraries support sound notification. You should test the
 * %gestalt_SoundNotify selector before you rely on it; see <link
 * linkend="chimara-Testing-for-Sound-Capabilities">Testing for Sound 
 * Capabilities</link>.
 * 
 * Returns: 1 on success, 0 on failure.
 */
glui32 
glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify)
{
	VALID_SCHANNEL(chan, return 0);
	g_printerr("Play sound %d with repeats %d and notify %d\n", snd, repeats, notify);
#ifdef GSTREAMER_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GInputStream *stream;

	/* Stop the previous sound */
	clean_up_after_playing_sound(chan);

	/* Don't play if repeats = 0 */
	if(repeats == 0) {
		chan->repeats = 0;
		return 1;
	}

	/* Load the sound into a GInputStream, by whatever method */
	if(!glk_data->resource_map) {
		if(!glk_data->resource_load_callback) {
			WARNING(_("No resource map has been loaded yet."));
			return 0;
		}
		gchar *filename = glk_data->resource_load_callback(CHIMARA_RESOURCE_SOUND, snd, glk_data->resource_load_callback_data);
		if(!filename) {
			WARNING(_("Error loading resource from alternative location."));
			return 0;
		}

		GError *err = NULL;
		GFile *file = g_file_new_for_path(filename);
		stream = G_INPUT_STREAM(g_file_read(file, NULL, &err));
		if(!stream) {
			IO_WARNING(_("Error loading resource from file"), filename, err->message);
			g_free(filename);
			g_object_unref(file);
			return 0;
		}
		g_free(filename);
		g_object_unref(file);
	} else {
		giblorb_result_t resource;
		giblorb_err_t result = giblorb_load_resource(glk_data->resource_map, giblorb_method_Memory, &resource, giblorb_ID_Snd, snd);
		if(result != giblorb_err_None) {
			WARNING_S( _("Error loading resource"), giblorb_get_error_message(result) );
			return 0;
		}
		stream = g_memory_input_stream_new_from_data(resource.data.ptr, resource.length, NULL);
	}

	chan->repeats = repeats;
	chan->resource = snd;
	chan->notify = notify;
	g_object_set(chan->source, "stream", stream, NULL);
	g_object_unref(stream); /* Now owned by GStreamer element */
	
	if(!gst_element_set_state(chan->pipeline, GST_STATE_PLAYING)) {
		WARNING_S(_("Could not set GstElement state to"), "PLAYING");
		return 0;
	}
	return 1;
#else
	return 0;
#endif
}

/**
 * glk_schannel_stop:
 * @chan: Channel to silence.
 *
 * Stops any sound playing in the channel. No notification event is generated,
 * even if you requested one. If no sound is playing, this has no effect.
 */
void 
glk_schannel_stop(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);
#ifdef GSTREAMER_SOUND
	clean_up_after_playing_sound(chan);
#endif
}

/**
 * glk_schannel_set_volume:
 * @chan: Channel to set the volume of.
 * @vol: Integer representing the volume; 0x10000 is 100&percnt;.
 *
 * Sets the volume in the channel. When you create a channel, it has full 
 * volume, represented by the value 0x10000. Half volume would be 0x8000, 
 * three-quarters volume would be 0xC000, and so on. A volume of zero represents
 * silence, although the sound is still considered to be playing.
 *
 * You can call this function between sounds, or while a sound is playing. The 
 * effect is immediate.
 * 
 * You can overdrive the volume of a channel by setting a volume greater than 
 * 0x10000. However, this is not recommended; the library may be unable to 
 * increase the volume past full, or the sound may become distorted. You should 
 * always create sound resources with the maximum volume you will need, and then
 * call glk_schannel_set_volume() to reduce the volume when appropriate.
 *
 * Not all libraries support this function. You should test the
 * %gestalt_SoundVolume selector before you rely on it; see <link
 * linkend="chimara-Testing-for-Sound-Capabilities">Testing for Sound
 * Capabilities</link>.
 *
 * <note><title>Chimara</title>
 *   <para>Chimara supports volumes from 0 to 1000&percnt;, that is, values of
 *   @vol up to 0xA0000.</para>
 * </note>
 */
void 
glk_schannel_set_volume(schanid_t chan, glui32 vol)
{
	VALID_SCHANNEL(chan, return);
#ifdef GSTREAMER_SOUND
	gdouble volume_gst = (gdouble)vol / 0x10000;
	g_printerr("Volume set to: %f\n", volume_gst);
	g_object_set(chan->filter, "volume", CLAMP(volume_gst, 0.0, 10.0), NULL);
#endif
}

/**
 * glk_sound_load_hint:
 * @snd: Resource number of a sound.
 * @flag: Nonzero to tell the library to load the sound, zero to tell the
 * library to unload it.
 *
 * This gives the library a hint about whether the given sound should be loaded
 * or not. If the @flag is nonzero, the library may preload the sound or do
 * other initialization, so that glk_schannel_play() will be faster. If the
 * @flag is zero, the library may release memory or other resources associated
 * with the sound. Calling this function is always optional, and it has no
 * effect on what the library actually plays.
 */
void 
glk_sound_load_hint(glui32 snd, glui32 flag)
{
#ifdef GSTREAMER_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	giblorb_result_t resource;
	giblorb_err_t result;

	/* Sound load hints only work for Blorb resource maps */
	if(!glk_data->resource_map)
		return;

	if(flag) {
		/* The sound load hint simply loads the resource from the resource map;
		 loading a chunk more than once does nothing */
		result = giblorb_load_resource(glk_data->resource_map, giblorb_method_Memory, &resource, giblorb_ID_Snd, snd);
		if(result != giblorb_err_None) {
			WARNING_S( _("Error loading resource"), giblorb_get_error_message(result) );
			return;
		}
	} else {
		/* Get the Blorb chunk number by loading the resource with
		 method_DontLoad, then unload that chunk - has no effect if the chunk
		 isn't loaded */
		result = giblorb_load_resource(glk_data->resource_map, giblorb_method_DontLoad, &resource, giblorb_ID_Snd, snd);
		if(result != giblorb_err_None) {
			WARNING_S( _("Error loading resource"), giblorb_get_error_message(result) );
			return;
		}
		result = giblorb_unload_chunk(glk_data->resource_map, resource.chunknum);
		if(result != giblorb_err_None) {
			WARNING_S( _("Error unloading chunk"), giblorb_get_error_message(result) );
			return;
		}
	}
#endif /* GSTREAMER_SOUND */
}
