#include "event.h"
#include "magic.h"
#include "glk.h"
#include "window.h"
#include "input.h"
#include <string.h>

#include "chimara-glk.h"
#include "chimara-glk-private.h"

extern GPrivate *glk_data_key;

#define EVENT_TIMEOUT_MICROSECONDS (3000000)

/* Internal function: push an event onto the event queue. If the event queue is
full, wait for max three seconds and then drop the event. If the event queue is
NULL, i.e. freed, then fail silently. */
void
event_throw(ChimaraGlk *glk, glui32 type, winid_t win, glui32 val1, glui32 val2)
{
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	
	if(!priv->event_queue)
		return;

	GTimeVal timeout;
	g_get_current_time(&timeout);
	g_time_val_add(&timeout, EVENT_TIMEOUT_MICROSECONDS);

	g_mutex_lock(priv->event_lock);

	/* Wait for room in the event queue */
	while( g_queue_get_length(priv->event_queue) >= EVENT_QUEUE_MAX_LENGTH )
		if( !g_cond_timed_wait(priv->event_queue_not_full, priv->event_lock, &timeout) ) 
		{
			/* Drop the event after 3 seconds */
			g_mutex_unlock(priv->event_lock);
			return;
		}

	event_t *event = g_new0(event_t, 1);
	event->type = type;
	event->win = win;
	event->val1 = val1;
	event->val2 = val2;
	g_queue_push_head(priv->event_queue, event);

	/* Signal that there is an event */
	g_cond_signal(priv->event_queue_not_empty);

	g_mutex_unlock(priv->event_lock);
}

/* Helper function: Wait for an event in the event queue. If it is a forced
 * input event, but no windows have an input request of that type, then wait
 * for the next event and put the forced input event back on top of the queue.
 */
static void
get_appropriate_event(event_t *event)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	g_mutex_lock(glk_data->event_lock);

	event_t *retrieved_event = NULL;

	/* Wait for an event */
	if( g_queue_is_empty(glk_data->event_queue) )
		g_cond_wait(glk_data->event_queue_not_empty, glk_data->event_lock);

	retrieved_event = g_queue_pop_tail(glk_data->event_queue);

	/* Signal that the event queue is no longer full */
	g_cond_signal(glk_data->event_queue_not_full);
	
	g_mutex_unlock(glk_data->event_lock);
	
	if(retrieved_event->type == evtype_ForcedCharInput)
	{
		/* Check for forced character input in the queue */
		winid_t win;
		for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
			if(win->input_request_type == INPUT_REQUEST_CHARACTER || win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE)
				break;
		if(win)
		{
			force_char_input_from_queue(win, event);
			g_free(retrieved_event);
		}
		else
		{
			get_appropriate_event(event);
			g_mutex_lock(glk_data->event_lock);
			g_queue_push_tail(glk_data->event_queue, retrieved_event);
			g_cond_signal(glk_data->event_queue_not_empty);
			g_mutex_unlock(glk_data->event_lock);
		}
	}
	else if(retrieved_event->type == evtype_ForcedLineInput)
	{
		/* Check for forced line input in the queue */
		winid_t win;
		for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
			if(win->input_request_type == INPUT_REQUEST_LINE || win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
				break;
		if(win)
		{
			force_line_input_from_queue(win, event);
			g_free(retrieved_event);
		}
		else
		{
			get_appropriate_event(event);
			g_mutex_lock(glk_data->event_lock);
			g_queue_push_tail(glk_data->event_queue, retrieved_event);
			g_cond_signal(glk_data->event_queue_not_empty);
			g_mutex_unlock(glk_data->event_lock);
		}
	}
	else
	{
		if(retrieved_event == NULL)
		{
			WARNING("Retrieved NULL event from non-empty event queue");
			return;
		}
		memcpy(event, retrieved_event, sizeof(event_t));
		g_free(retrieved_event);
	}
}

/**
 * glk_select:
 * @event: Pointer to an #event_t.
 *
 * Causes the program to wait for an event, and then store it in the structure
 * pointed to by @event. Unlike most Glk functions that take pointers, the
 * argument of glk_select() may not be %NULL.
 *
 * Most of the time, you only get the events that you request. However, there
 * are some events which can arrive at any time. This is why you must always
 * call glk_select() in a loop, and continue the loop until you get the event
 * you really want.
 */
void
glk_select(event_t *event)
{
	g_return_if_fail(event != NULL);

	/* Flush all window buffers */
	winid_t win;
	for(win = glk_window_iterate(NULL, NULL); win != NULL; win = glk_window_iterate(win, NULL)) {
		if(win->type == wintype_TextBuffer || win->type == wintype_TextGrid)
			flush_window_buffer(win);
	}

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	get_appropriate_event(event);

	/* Check for interrupt */
	glk_tick();

	/* If the event was a line input event, the library must release the buffer */
	if(event->type == evtype_LineInput && glk_data->unregister_arr) 
	{
        if(event->win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
			(*glk_data->unregister_arr)(event->win->line_input_buffer_unicode, event->win->line_input_buffer_max_len, "&+#!Iu", event->win->buffer_rock);
		else
            (*glk_data->unregister_arr)(event->win->line_input_buffer, event->win->line_input_buffer_max_len, "&+#!Cn", event->win->buffer_rock);
    }
	
	/* If an abort event was generated, the thread should have exited by now */
	g_assert(event->type != evtype_Abort);
}

/**
 * glk_select_poll:
 * @event: Return location for an event.
 *
 * You can also inquire if an event is available, without stopping to wait for 
 * one to occur.
 * 
 * This checks if an internally-spawned event is available. If so, it stores it 
 * in the structure pointed to by @event. If not, it sets
 * <code>@event->type</code> to %evtype_None. Either way, it returns almost
 * immediately.
 * 
 * The first question you now ask is, what is an internally-spawned event?
 * glk_select_poll() does not check for or return %evtype_CharInput,
 * %evtype_LineInput, %evtype_MouseInput, or %evtype_Hyperlink events. It is
 * intended for you to test conditions which may have occurred while you are
 * computing, and not interfacing with the player. For example, time may pass
 * during slow computations; you can use glk_select_poll() to see if a 
 * %evtype_Timer event has occurred. (See <link 
 * linkend="chimara-Timer-Events">Timer Events</link>.)
 * 
 * At the moment, glk_select_poll() checks for %evtype_Timer, %evtype_Arrange,
 * %evtype_Redraw and %evtype_SoundNotify events. But see <link 
 * linkend="chimara-Other-Events">Other Events</link>.
 * 
 * The second question is, what does it mean that glk_select_poll() returns 
 * <quote>almost immediately</quote>? In some Glk libraries, text that you send 
 * to a window is buffered; it does not actually appear until you request player
 * input with glk_select(). glk_select_poll() attends to this buffer-flushing 
 * task in the same way. (Although it does not do the <quote><computeroutput>Hit
 * any key to scroll down</computeroutput></quote> waiting which may be done in
 * glk_select(); that's a player-input task.)
 * 
 * Similarly, on multitasking platforms, glk_select() may yield time to other
 * processes; and glk_select_poll() does this as well.
 * 
 * The upshot of this is that you should not call glk_select_poll() very often. 
 * If you are not doing much work between player inputs, you should not need to
 * call it at all.
 *
 * <note><para>
 *  For example, in a virtual machine interpreter, you should not call
 *  glk_select_poll() after every opcode.
 * </para></note>
 * 
 * However, if you are doing intense computation, you may wish to call
 * glk_select_poll() every so often to yield time to other processes. And if you
 * are printing intermediate results during this computation, you should
 * glk_select_poll() every so often, so that you can be certain your output will 
 * be displayed before the next glk_select().
 * 
 * <note><para>
 *  However, you should call glk_tick() often &mdash; once per opcode in a VM
 *  interpreter. See <link linkend="chimara-The-Tick-Thing">The Tick 
 *  Thing</link>.
 * </para></note>
 */
void
glk_select_poll(event_t *event)
{
	g_return_if_fail(event != NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	event->type = evtype_None;
	
	g_mutex_lock(glk_data->event_lock);
	
	if( !g_queue_is_empty(glk_data->event_queue) )
	{
		GList *link;
		int count;
		for(count = 0; (link = g_queue_peek_nth_link(glk_data->event_queue, count)) != NULL; count++)
		{
			glui32 type = ((event_t *)link->data)->type;
			if(type != evtype_CharInput && type != evtype_LineInput && type != evtype_MouseInput && type != evtype_Hyperlink)
			{
				memcpy(event, link->data, sizeof(event_t));
				g_free(link->data);
				g_queue_delete_link(glk_data->event_queue, link);
				g_cond_signal(glk_data->event_queue_not_full);
				break;
			}
		}
	}
	
	g_mutex_unlock(glk_data->event_lock);
	
	/* Check for interrupt */
	glk_tick();

	/* If an abort event was generated, the thread should have exited by now */
	g_assert(event->type != evtype_Abort);
}
