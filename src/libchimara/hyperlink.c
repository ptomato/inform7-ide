#include <config.h>
#include <glib/gi18n-lib.h>
#include "hyperlink.h"
#include "chimara-glk-private.h"
#include "magic.h"

extern GPrivate *glk_data_key;

/**
 * glk_set_hyperlink:
 * @linkval: Set to nonzero to initiate hyperlink mode. Set to zero to disengage.
 *
 * This call sets the current link value in the current output stream. See
 * glk_set_hyperlink_stream().
 */
void 
glk_set_hyperlink(glui32 linkval)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_set_hyperlink_stream(glk_data->current_stream, linkval);
}

/**
 * glk_set_hyperlink_stream:
 * @str: The stream to set the hyperlink mode on.
 * @linkval: Set to non-zero to initiate hyperlink mode. Set to zero to 
 * disengage.
 *
 * This call sets the current link value on the specified output stream. A link
 * value is any non-zero integer; zero indicates no link. Subsequent text output
 * is considered to make up the body of the link, which continues until the link
 * value is changed (or set to zero).
 *
 * Note that it is almost certainly useless to change the link value of a stream
 * twice with no intervening text. The result will be a zero-length link, which 
 * the player probably cannot see or select; the library may optimize it out 
 * entirely.
 *
 * Setting the link value of a stream to the value it already has, has no 
 * effect.
 *
 * If the library supports images, they take on the current link value as they 
 * are output, just as text does. The player can select an image in a link just 
 * as he does text. (This includes margin-aligned images, which can lead to some 
 * peculiar situations, since a right-margin image may not appear directly 
 * adjacent to the text it was output with.)
 * 
 * The library will attempt to display links in some distinctive way (and it 
 * will do this whether or not hyperlink input has actually been requested for 
 * the window). Naturally, blue underlined text is most likely. Link images may 
 * not be distinguished from non-link images, so it is best not to use a 
 * particular image both ways. 
 */
void 
glk_set_hyperlink_stream(strid_t str, glui32 linkval)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->type == STREAM_TYPE_WINDOW);
	g_return_if_fail(str->window != NULL);
	g_return_if_fail(str->window->type == wintype_TextBuffer || str->window->type == wintype_TextGrid);

	flush_window_buffer(str->window);

	if(linkval == 0) {
		/* Turn off hyperlink mode */
		str->hyperlink_mode = FALSE;
		str->window->current_hyperlink = NULL;
		return;
	}

	/* Check whether a tag with the needed value already exists */
	hyperlink_t *new_hyperlink = g_hash_table_lookup(str->window->hyperlinks, &linkval);
	if(new_hyperlink == NULL) {
		/* Create a new hyperlink with the requested value */
		new_hyperlink = g_new0(struct hyperlink, 1);
		new_hyperlink->value = linkval;
		new_hyperlink->tag = gtk_text_tag_new(NULL);
		new_hyperlink->event_handler = g_signal_connect( new_hyperlink->tag, "event", G_CALLBACK(on_hyperlink_clicked), new_hyperlink );
		if(!str->window->hyperlink_event_requested)
			g_signal_handler_block(new_hyperlink->tag, new_hyperlink->event_handler);
		new_hyperlink->window = str->window;

		/* Add the new tag to the tag table of the textbuffer */
		GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(str->window->widget) );
		GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(textbuffer);
		gtk_text_tag_table_add(tags, new_hyperlink->tag);

		gint *linkval_pointer = g_new0(gint, 1);
		*linkval_pointer = linkval;
		g_hash_table_insert(str->window->hyperlinks, linkval_pointer, new_hyperlink);
	}

	str->hyperlink_mode = TRUE;
	str->window->current_hyperlink = new_hyperlink;
}

/* Internal function used to iterate over all the hyperlinks, unblocking the event handler */
void
hyperlink_unblock_event_handler(gpointer key, gpointer value, gpointer user_data)
{
	hyperlink_t *link = (hyperlink_t *) value;
	g_signal_handler_unblock(link->tag, link->event_handler);
}

/* Internal function used to iterate over all the hyperlinks, blocking the event handler */
void
hyperlink_block_event_handler(gpointer key, gpointer value, gpointer user_data)
{
	hyperlink_t *link = (hyperlink_t *) value;
	g_signal_handler_block(link->tag, link->event_handler);
}

/**
 * glk_request_hyperlink_event:
 * @win: The window to request a hyperlink event on.
 *
 * This call works like glk_request_char_event(), glk_request_line_event() and
 * glk_request_mouse_event(). A pending request on a window remains pending 
 * until the player selects a link, or the request is cancelled.
 *
 * A window can have hyperlink input and mouse, character, or line input pending
 * at the same time. However, if hyperlink and mouse input are requested at the
 * same time, the library may not provide an intuitive way for the player to 
 * distingish which a mouse click represents. Therefore, this combination should
 * be avoided.
 *
 * When a link is selected in a window with a pending request, glk_select() will
 * return an event of type %evtype_Hyperlink. In the event structure, @win tells
 * what window the event came from, and @val1 gives the (non-zero) link value.
 * 
 * If no hyperlink request is pending in a window, the library will ignore 
 * attempts to select a link. No %evtype_Hyperlink event will be generated 
 * unless it has been requested. 
 */
void 
glk_request_hyperlink_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextBuffer || win->type == wintype_TextGrid);

	if(win->hyperlink_event_requested) {
		WARNING(_("Tried to request a hyperlink event on a window that already had a hyperlink request"));
		return;
	}

	win->hyperlink_event_requested = TRUE;
	g_hash_table_foreach(win->hyperlinks, hyperlink_unblock_event_handler, NULL);

}

/**
 * glk_cancel_hyperlink_event:
 * @win: The window in which to cancel the hyperlink event request.
 *
 * This call works like glk_cancel_char_event(), glk_cancel_line_event(), and
 * glk_cancel_mouse_event(). See glk_request_hyperlink_event().
 */
void 
glk_cancel_hyperlink_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextBuffer || win->type == wintype_TextGrid);

	if(!win->hyperlink_event_requested) {
		WARNING(_("Tried to cancel a nonexistent hyperlink request"));
		return;
	}

	win->hyperlink_event_requested = FALSE;
	g_hash_table_foreach(win->hyperlinks, hyperlink_block_event_handler, NULL);
}

gboolean
on_hyperlink_clicked(GtkTextTag *tag, GObject *object, GdkEvent *event, GtkTextIter *iter, hyperlink_t *link)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(link->window->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	if(event->type == GDK_BUTTON_PRESS) {
		link->window->hyperlink_event_requested = FALSE;
		g_hash_table_foreach(link->window->hyperlinks, hyperlink_block_event_handler, NULL);
		event_throw(glk, evtype_Hyperlink, link->window, link->value, 0);
	}

	return FALSE;
}
