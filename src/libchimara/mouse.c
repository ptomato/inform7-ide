#include "mouse.h"
#include "magic.h"

/**
 * glk_request_mouse_event:
 * @win: Window on which to request a mouse input event.
 *
 * Requests mouse input on the window @win.
 */
void
glk_request_mouse_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextGrid || win->type == wintype_Graphics);

	g_signal_handler_unblock(win->widget, win->button_press_event_handler);
}

/**
 * glk_cancel_mouse_event:
 * @win: Window with a mouse input event pending.
 *
 * Cancels the pending mouse input request on @win.
 */
void 
glk_cancel_mouse_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextGrid || win->type == wintype_Graphics);

	g_signal_handler_block(win->widget, win->button_press_event_handler);
}

gboolean
on_window_button_press(GtkWidget *widget, GdkEventButton *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	switch(win->type)
	{
		case wintype_TextGrid:
			event_throw(glk, evtype_MouseInput, win, event->x/win->unit_width, event->y/win->unit_height);
			break;
		case wintype_Graphics:
			event_throw(glk, evtype_MouseInput, win, event->x, event->y);
			break;
		default:
            ILLEGAL_PARAM("Unknown window type: %u", win->type);
	}

	return TRUE;
}
