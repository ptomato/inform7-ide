#include "charset.h"
#include "magic.h"
#include "input.h"
#include "pager.h"
#include "chimara-glk-private.h"

extern GPrivate *glk_data_key;

/* Forward declarations */
static int finish_text_buffer_line_input(winid_t win, gboolean emit_signal);
static int finish_text_grid_line_input(winid_t win, gboolean emit_signal);
static void cancel_old_input_request(winid_t win);

/* Internal function: code common to both flavors of char event request */
void
request_char_event_common(winid_t win, gboolean unicode)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	cancel_old_input_request(win);

	flush_window_buffer(win);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	win->input_request_type = unicode? INPUT_REQUEST_CHARACTER_UNICODE : INPUT_REQUEST_CHARACTER;
	g_signal_handler_unblock( win->widget, win->char_input_keypress_handler );

	gdk_threads_enter();
	gtk_widget_grab_focus( GTK_WIDGET(win->widget) );
	gdk_threads_leave();

	/* Emit the "waiting" signal to let listeners know we are ready for input */
	g_signal_emit_by_name(glk_data->self, "waiting");
}

/**
 * glk_request_char_event:
 * @win: A window to request char events from.
 *
 * Request input of a Latin-1 character or special key. A window cannot have
 * requests for both character and line input at the same time. Nor can it have
 * requests for character input of both types (Latin-1 and Unicode). It is
 * illegal to call glk_request_char_event() if the window already has a pending
 * request for either character or line input.
 */
void
glk_request_char_event(winid_t win)
{
	request_char_event_common(win, FALSE);
}

/**
 * glk_request_char_event_uni:
 * @win: A window to request char events from.
 *
 * Request input of a Unicode character or special key. See
 * glk_request_char_event().
 */
void
glk_request_char_event_uni(winid_t win)
{
	request_char_event_common(win, TRUE);
}

/**
 * glk_cancel_char_event:
 * @win: A window to cancel the latest char event request on.
 *
 * This cancels a pending request for character input. (Either Latin-1 or
 * Unicode.) For convenience, it is legal to call glk_cancel_char_event() even
 * if there is no charcter input request on that window. Glk will ignore the
 * call in this case.
 */
void
glk_cancel_char_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	if(win->input_request_type == INPUT_REQUEST_CHARACTER || win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE)
	{
		win->input_request_type = INPUT_REQUEST_NONE;
		g_signal_handler_block( win->widget, win->char_input_keypress_handler );
	}
}

/* Internal function: Request either latin-1 or unicode line input, in a text grid window. */
static void
text_grid_request_line_event_common(winid_t win, glui32 maxlen, gboolean insert, gchar *inserttext)
{
	gdk_threads_enter();

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

    GtkTextMark *cursor = gtk_text_buffer_get_mark(buffer, "cursor_position");
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, cursor);

    /* Determine the maximum length of the line input */
    gint cursorpos = gtk_text_iter_get_line_offset(&start_iter);
    /* Odd; the Glk spec says the maximum input length is
    windowwidth - 1 - cursorposition. I say no, because if cursorposition is
    zero, then the input should fill the whole line. FIXME??? */
    win->input_length = MIN(win->width - cursorpos, win->line_input_buffer_max_len);
    end_iter = start_iter;
    gtk_text_iter_set_line_offset(&end_iter, cursorpos + win->input_length);

	/* If the buffer currently has a selection with one bound in the middle of
	the input field, then deselect it. Otherwise the input field gets trashed */
	GtkTextIter start_sel, end_sel;
	if( gtk_text_buffer_get_selection_bounds(buffer, &start_sel, &end_sel) )
	{
		if( gtk_text_iter_in_range(&start_sel, &start_iter, &end_iter) )
			gtk_text_buffer_place_cursor(buffer, &end_sel);
		if( gtk_text_iter_in_range(&end_sel, &start_iter, &end_iter) )
			gtk_text_buffer_place_cursor(buffer, &start_sel);
	}

    /* Erase the text currently in the input field and replace it with a GtkEntry */
    gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
    win->input_anchor = gtk_text_buffer_create_child_anchor(buffer, &start_iter);
    win->input_entry = gtk_entry_new();
	/* Set the entry's font to match that of the window */
    GtkRcStyle *style = gtk_widget_get_modifier_style(win->widget);	/* Don't free */
	gtk_widget_modify_font(win->input_entry, style->font_desc);
	/* Make the entry as small as possible to fit with the text */
	gtk_entry_set_has_frame(GTK_ENTRY(win->input_entry), FALSE);
	GtkBorder border = { 0, 0, 0, 0 };

	/* COMPAT: */
#if GTK_CHECK_VERSION(2,10,0)
	gtk_entry_set_inner_border(GTK_ENTRY(win->input_entry), &border);
#endif
    gtk_entry_set_max_length(GTK_ENTRY(win->input_entry), win->input_length);
    gtk_entry_set_width_chars(GTK_ENTRY(win->input_entry), win->input_length);

    /* Insert pre-entered text if needed */
    if(insert)
    	gtk_entry_set_text(GTK_ENTRY(win->input_entry), inserttext);

    /* Set background color of entry (TODO: implement as property) */
    GdkColor background;
	gdk_color_parse("grey", &background);
    gtk_widget_modify_base(win->input_entry, GTK_STATE_NORMAL, &background);

    g_signal_connect(win->input_entry, "activate", G_CALLBACK(on_input_entry_activate), win);
    g_signal_connect(win->input_entry, "key-press-event", G_CALLBACK(on_input_entry_key_press_event), win);
    win->line_input_entry_changed = g_signal_connect(win->input_entry, "changed", G_CALLBACK(on_input_entry_changed), win);

    gtk_widget_show(win->input_entry);
    gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(win->widget), win->input_entry, win->input_anchor);

	gtk_widget_grab_focus(win->input_entry);

	gdk_threads_leave();
}

/* Internal function: Request either latin-1 or unicode line input, in a text buffer window. */
static void
text_buffer_request_line_event_common(winid_t win, glui32 maxlen, gboolean insert, gchar *inserttext)
{
	flush_window_buffer(win);

	gdk_threads_enter();

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

    /* Move the input_position mark to the end of the window_buffer */
    GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_move_mark(buffer, input_position, &end_iter);

    /* Set the entire contents of the window_buffer as uneditable
     * (so input can only be entered at the end) */
    GtkTextIter start_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_remove_tag_by_name(buffer, "uneditable", &start_iter, &end_iter);
    gtk_text_buffer_apply_tag_by_name(buffer, "uneditable", &start_iter, &end_iter);

    /* Insert pre-entered text if needed */
    if(insert) {
        gtk_text_buffer_insert(buffer, &end_iter, inserttext, -1);
		gtk_text_buffer_get_end_iter(buffer, &end_iter); /* update after text insertion */
	}

	/* Apply the correct style to the input prompt */
	GtkTextIter input_iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &input_iter, input_position);
    gtk_text_buffer_apply_tag_by_name(buffer, "input", &input_iter, &end_iter);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), TRUE);

	g_signal_handler_unblock(buffer, win->insert_text_handler);
	gtk_widget_grab_focus(win->widget);

	gdk_threads_leave();
}

/**
 * glk_request_line_event:
 * @win: A text buffer or text grid window to request line input on.
 * @buf: A buffer of at least @maxlen bytes.
 * @maxlen: Length of the buffer.
 * @initlen: The number of characters in @buf to pre-enter.
 *
 * Requests input of a line of Latin-1 characters. A window cannot have requests
 * for both character and line input at the same time. Nor can it have requests
 * for line input of both types (Latin-1 and Unicode). It is illegal to call
 * glk_request_line_event() if the window already has a pending request for
 * either character or line input.
 *
 * The @buf argument is a pointer to space where the line input will be stored.
 * (This may not be %NULL.) @maxlen is the length of this space, in bytes; the
 * library will not accept more characters than this. If @initlen is nonzero,
 * then the first @initlen bytes of @buf will be entered as pre-existing input
 * &mdash; just as if the player had typed them himself. (The player can continue
 * composing after this pre-entered input, or delete it or edit as usual.)
 *
 * The contents of the buffer are undefined until the input is completed (either
 * by a line input event, or glk_cancel_line_event(). The library may or may not
 * fill in the buffer as the player composes, while the input is still pending;
 * it is illegal to change the contents of the buffer yourself.
 */
void
glk_request_line_event(winid_t win, char *buf, glui32 maxlen, glui32 initlen)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(buf);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);
	g_return_if_fail(initlen <= maxlen);

	cancel_old_input_request(win);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	/* Register the buffer */
	if(glk_data->register_arr)
        win->buffer_rock = (*glk_data->register_arr)(buf, maxlen, "&+#!Cn");

	win->input_request_type = INPUT_REQUEST_LINE;
	win->line_input_buffer = buf;
	win->line_input_buffer_max_len = maxlen;

	gchar *inserttext = (initlen > 0)? g_strndup(buf, initlen) : g_strdup("");
	switch(win->type)
	{
	    case wintype_TextBuffer:
	        text_buffer_request_line_event_common(win, maxlen, (initlen > 0), inserttext);
	        break;
	    case wintype_TextGrid:
	        text_grid_request_line_event_common(win, maxlen, (initlen > 0), inserttext);
	        break;
    }
	g_free(inserttext);
	g_signal_handler_unblock(win->widget, win->line_input_keypress_handler);

	/* Emit the "waiting" signal to let listeners know we are ready for input */
	g_signal_emit_by_name(glk_data->self, "waiting");
}

/**
 * glk_request_line_event_uni:
 * @win: A text buffer or text grid window to request line input on.
 * @buf: A buffer of at least @maxlen characters.
 * @maxlen: Length of the buffer.
 * @initlen: The number of characters in @buf to pre-enter.
 *
 * Request input of a line of Unicode characters. This works the same as
 * glk_request_line_event(), except the result is stored in an array of
 * <type>glui32</type> values instead of an array of characters, and the values
 * may be any valid Unicode code points.
 *
 * The result will be in Unicode Normalization Form C. This basically means that
 * composite characters will be single characters where possible, instead of
 * sequences of base and combining marks. See
 * <ulink url="http://www.unicode.org/reports/tr15/">Unicode Standard Annex
 * #15</ulink> for the details.
 */
void
glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(buf);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);
	g_return_if_fail(initlen <= maxlen);

	cancel_old_input_request(win);
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	/* Register the buffer */
	if(glk_data->register_arr)
        win->buffer_rock = (*glk_data->register_arr)(buf, maxlen, "&+#!Iu");



	win->input_request_type = INPUT_REQUEST_LINE_UNICODE;
	win->line_input_buffer_unicode = buf;
	win->line_input_buffer_max_len = maxlen;

	gchar *utf8;
	if(initlen > 0) {
		utf8 = convert_ucs4_to_utf8(buf, initlen);
		if(utf8 == NULL)
			return;
	}
	else
		utf8 = g_strdup("");

    switch(win->type)
	{
	    case wintype_TextBuffer:
	        text_buffer_request_line_event_common(win, maxlen, (initlen > 0), utf8);
	        break;
	    case wintype_TextGrid:
	        text_grid_request_line_event_common(win, maxlen, (initlen > 0), utf8);
	        break;
    }
    g_signal_handler_unblock(win->widget, win->line_input_keypress_handler);
	g_free(utf8);

	/* Emit the "waiting" signal to let listeners know we are ready for input */
	g_signal_emit_by_name(glk_data->self, "waiting");
}

/**
 * glk_cancel_line_event:
 * @win: A text buffer or text grid window to cancel line input on.
 * @event: Will be filled in if the user had already input something.
 *
 * This cancels a pending request for line input. (Either Latin-1 or Unicode.)
 *
 * The event pointed to by the event argument will be filled in as if the
 * player had hit <keycap>enter</keycap>, and the input composed so far will be
 * stored in the buffer; see below. If you do not care about this information,
 * pass %NULL as the @event argument. (The buffer will still be filled.)
 *
 * For convenience, it is legal to call glk_cancel_line_event() even if there
 * is no line input request on that window. The event type will be set to
 * %evtype_None in this case.
 */
void
glk_cancel_line_event(winid_t win, event_t *event)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	if(event != NULL) {
		event->type = evtype_None;
		event->win = win;
		event->val1 = 0;
		event->val2 = 0;
	}

	if(win->input_request_type != INPUT_REQUEST_LINE && win->input_request_type != INPUT_REQUEST_LINE_UNICODE)
		return;

	g_signal_handler_block( win->widget, win->line_input_keypress_handler );

	int chars_written = 0;

	gdk_threads_enter();
	if(win->type == wintype_TextGrid) {
		chars_written = finish_text_grid_line_input(win, FALSE);
	} else if(win->type == wintype_TextBuffer) {
		gtk_text_view_set_editable( GTK_TEXT_VIEW(win->widget), FALSE );
		GtkTextBuffer *window_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		g_signal_handler_block(window_buffer, win->insert_text_handler);
		chars_written = finish_text_buffer_line_input(win, FALSE);
	}
	gdk_threads_leave();

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	if(glk_data->unregister_arr)
	{
		if(win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
			(*glk_data->unregister_arr)(win->line_input_buffer_unicode, win->line_input_buffer_max_len, "&+#!Iu", win->buffer_rock);
		else
        	(*glk_data->unregister_arr)(win->line_input_buffer, win->line_input_buffer_max_len, "&+#!Cn", win->buffer_rock);
    }

	if(event != NULL && chars_written > 0) {
		event->type = evtype_LineInput;
		event->val1 = chars_written;
	}
}

/* Helper function: Turn off shutdown key-press-event signal handler */
static gboolean
turn_off_handler(GNode *node)
{
	winid_t win = node->data;
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);
	return FALSE; /* don't stop */
}

/* Internal function: Callback for signal key-press-event while waiting for shutdown. */
gboolean
on_shutdown_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(widget, CHIMARA_TYPE_GLK));
	g_assert(glk);
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	
	/* Turn off all the signal handlers */
	if(priv->root_window)
		g_node_traverse(priv->root_window, G_IN_ORDER, G_TRAVERSE_LEAVES, -1, (GNodeTraverseFunc)turn_off_handler, NULL);
	
	/* Signal the Glk library that it can shut everything down now */
	g_mutex_lock(priv->shutdown_lock);
	g_cond_signal(priv->shutdown_key_pressed);
	g_mutex_unlock(priv->shutdown_lock);
	
	return TRUE; /* block the event */
}

/* Internal function: General callback for signal key-press-event on a text buffer or text grid window. Used in character input on both text buffers and grids. Blocked when not in use. */
gboolean
on_char_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	/* Ignore modifier keys, otherwise the char input will already trigger on 
	the shift key when the user tries to type a capital letter */
	if(event->is_modifier)
		return FALSE; /* don't stop the event */

	/* All text up to the input position is now regarded as being read by the user */
	if(win->type == wintype_TextBuffer)
		pager_update(win);
	
	glui32 keycode = keyval_to_glk_keycode(event->keyval, win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE);

	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(widget, CHIMARA_TYPE_GLK));
	g_assert(glk);
	event_throw(glk, evtype_CharInput, win, keycode, 0);
	g_signal_emit_by_name(glk, "char-input", win->rock, event->keyval);

	/* Only one keypress will be handled */
	win->input_request_type = INPUT_REQUEST_NONE;
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);

	return TRUE;
}

gboolean
on_line_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	switch(win->type)
	{
		case wintype_TextBuffer:
			/* All text up to the input position is now regarded as being read by the user */
			pager_update(win);

			/* History up/down */
			if(event->keyval == GDK_Up || event->keyval == GDK_KP_Up
				|| event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
			{
				/* Prevent falling off the end of the history list */
				if(win->history == NULL)
					return TRUE;
				if( (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
					&& win->history_pos && win->history_pos->next == NULL)
					return TRUE;
				if( (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
					&& (win->history_pos == NULL || win->history_pos->prev == NULL) )
					return TRUE;

				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
				GtkTextIter start, end;
				/* Erase any text that was already typed */
				GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
				gtk_text_buffer_get_iter_at_mark(buffer, &start, input_position);
				gtk_text_buffer_get_end_iter(buffer, &end);

				if(win->history_pos == NULL) {
					gchar *current_input = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
					win->history = g_list_prepend(win->history, current_input);
					win->history_pos = win->history;
				}

				gtk_text_buffer_delete(buffer, &start, &end);

				if(event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
				{
					if(win->history_pos)
						win->history_pos = g_list_next(win->history_pos);
					else
						win->history_pos = win->history;
				}
				else /* down */
					win->history_pos = g_list_previous(win->history_pos);

				/* Insert the history item into the window */
				gtk_text_buffer_get_end_iter(buffer, &end);

				g_signal_handler_block(buffer, win->insert_text_handler);
				gtk_text_buffer_insert_with_tags_by_name(buffer, &end, win->history_pos->data, -1, "default", "input", NULL);
				g_signal_handler_unblock(buffer, win->insert_text_handler);
				return TRUE;
			}

			/* Move to beginning/end of input field */
			else if(event->keyval == GDK_Home) {
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
				GtkTextIter input_iter;
				GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
				gtk_text_buffer_get_iter_at_mark(buffer, &input_iter, input_position);
				gtk_text_buffer_place_cursor(buffer, &input_iter);
				return TRUE;
			}
			else if(event->keyval == GDK_End) {
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
				GtkTextIter end_iter;
				gtk_text_buffer_get_end_iter(buffer, &end_iter);
				gtk_text_buffer_place_cursor(buffer, &end_iter);
				return TRUE;
			}

			/* Handle the enter key, which could occur in the middle of the sentence. */
			else if(event->keyval == GDK_Return || event->keyval == GDK_KP_Enter) {
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
				GtkTextIter end_iter;
				gtk_text_buffer_get_end_iter(buffer, &end_iter);
				gtk_text_buffer_place_cursor(buffer, &end_iter);
				return FALSE; 
			}

			return FALSE;

		/* If this is a text grid window, then redirect the key press to the line input GtkEntry */
		case wintype_TextGrid:
		{
			if(event->keyval == GDK_Up || event->keyval == GDK_KP_Up
				|| event->keyval == GDK_Down || event->keyval == GDK_KP_Down
				|| event->keyval == GDK_Left || event->keyval == GDK_KP_Left
				|| event->keyval == GDK_Right || event->keyval == GDK_KP_Right
				|| event->keyval == GDK_Tab || event->keyval == GDK_KP_Tab
				|| event->keyval == GDK_Page_Up || event->keyval == GDK_KP_Page_Up
				|| event->keyval == GDK_Page_Down || event->keyval == GDK_KP_Page_Down
				|| event->keyval == GDK_Home || event->keyval == GDK_KP_Home
				|| event->keyval == GDK_End || event->keyval == GDK_KP_End)
				return FALSE; /* Don't redirect these keys */
			gtk_widget_grab_focus(win->input_entry);
			gtk_editable_set_position(GTK_EDITABLE(win->input_entry), -1);
			gboolean retval = TRUE;
			g_signal_emit_by_name(win->input_entry, "key-press-event", event, &retval);
			return retval; /* Block this key event if the entry handled it */
		}
	}
	return FALSE;
}

/* Internal function: finish handling a line input request, for both text grid and text buffer windows. */
static int
write_to_window_buffer(winid_t win, const gchar *inserted_text)
{
	int copycount = 0;

    /* Convert the string from UTF-8 to Latin-1 or Unicode */
    if(win->input_request_type == INPUT_REQUEST_LINE)
    {
        gsize bytes_written;
        gchar *latin1 = convert_utf8_to_latin1(inserted_text, &bytes_written);

        if(latin1 == NULL)
            return 0;

        /* Place input in the echo stream */
        if(win->echo_stream != NULL)
            glk_put_string_stream(win->echo_stream, latin1);

        /* Copy the string (bytes_written does not include the NULL at the end) */
        copycount = MIN(win->line_input_buffer_max_len, bytes_written);
        memcpy(win->line_input_buffer, latin1, copycount);
        g_free(latin1);
    }
    else if(win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
    {
        glong items_written;
        gunichar *unicode = convert_utf8_to_ucs4(inserted_text, &items_written);

        if(unicode == NULL)
            return 0;

        /* Place input in the echo stream */
        if(win->echo_stream != NULL)
            glk_put_string_stream_uni(win->echo_stream, unicode);

        /* Copy the string (but not the NULL at the end) */
        copycount = MIN(win->line_input_buffer_max_len, items_written);
        memcpy(win->line_input_buffer_unicode, unicode, copycount * sizeof(gunichar));
        g_free(unicode);
    }
    else
        WARNING("Wrong input request type");

    win->input_request_type = INPUT_REQUEST_NONE;
	return copycount;
}

/* Internal function: Retrieves the input of a TextBuffer window and stores it in the window buffer.
 * Returns the number of characters written, suitable for inclusion in a line input event. */
static int
finish_text_buffer_line_input(winid_t win, gboolean emit_signal)
{
	VALID_WINDOW(win, return 0);
	g_return_val_if_fail(win->type == wintype_TextBuffer, 0);

	GtkTextIter start_iter, end_iter, last_character;

	GtkTextBuffer *window_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
	gtk_text_buffer_get_iter_at_mark(window_buffer, &start_iter, input_position);
	gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
	gtk_text_buffer_get_end_iter(window_buffer, &last_character);
	gtk_text_iter_backward_cursor_position(&last_character);

	gchar* last_char = gtk_text_buffer_get_text(window_buffer, &last_character, &end_iter, FALSE);

	if( strchr(last_char, '\n') != NULL )
		gtk_text_iter_backward_cursor_position(&end_iter);

	gchar* inserted_text = gtk_text_buffer_get_text(window_buffer, &start_iter, &end_iter, FALSE);

	int chars_written = write_to_window_buffer(win, inserted_text);
	if(emit_signal)
	{
		ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
		g_assert(glk);
		g_signal_emit_by_name(glk, "line-input", win->rock, inserted_text);
	}

	/* Add the text to the window input history */
	if(win->history_pos != NULL)
	{
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
	}
	if(*inserted_text != 0)
		win->history = g_list_prepend(win->history, g_strdup(inserted_text));

	win->history_pos = NULL;

	g_free(inserted_text);

	return chars_written;
}

/* Internal function: Retrieves the input of a TextGrid window and stores it in the window buffer.
 * Returns the number of characters written, suitable for inclusion in a line input event. */
static int
finish_text_grid_line_input(winid_t win, gboolean emit_signal)
{
	VALID_WINDOW(win, return 0);
	g_return_val_if_fail(win->type == wintype_TextGrid, 0);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	gchar *text = g_strdup( gtk_entry_get_text(GTK_ENTRY(win->input_entry)) );
	/* Move the focus back into the text view */
	gtk_widget_grab_focus(win->widget);
	/* Remove entry widget from text view */
	/* Should be ok even though this is the widget's own signal handler */
	gtk_container_remove( GTK_CONTAINER(win->widget), GTK_WIDGET(win->input_entry) );
	win->input_entry = NULL;
	/* Delete the child anchor */
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_child_anchor(buffer, &start, win->input_anchor);
	end = start;
	gtk_text_iter_forward_char(&end); /* Point after the child anchor */
	gtk_text_buffer_delete(buffer, &start, &end);
	win->input_anchor = NULL;

    gchar *spaces = g_strnfill(win->input_length - g_utf8_strlen(text, -1), ' ');
    gchar *text_to_insert = g_strconcat(text, spaces, NULL);
	g_free(spaces);
    gtk_text_buffer_insert(buffer, &start, text_to_insert, -1);
    g_free(text_to_insert);

    int chars_written = write_to_window_buffer(win, text);
    if(emit_signal)
    {
		ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
		g_assert(glk);
		g_signal_emit_by_name(glk, "line-input", win->rock, text);
    }

	/* Add the text to the window input history */
	if(win->history_pos != NULL)
	{
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
	}
	if(*text != 0)
		win->history = g_list_prepend(win->history, g_strdup(text));
	win->history_pos = NULL;

	g_free(text);
	return chars_written;
}

/* Internal function: Callback for signal insert-text on a text buffer window.
Runs after the default handler has already inserted the text.
FIXME: This function assumes that newline was the last character typed into the
window. That assumption is wrong if, for example, text containing a newline was
pasted into the window. */
void
after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, winid_t win)
{
	GtkTextBuffer *window_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	/* Set the history position to NULL and erase the text we were already editing */
	if(win->history_pos != NULL)
	{
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
		win->history_pos = NULL;
	}
	if( strchr(text, '\n') != NULL )
	{
		/* Remove signal handlers */
		g_signal_handler_block(window_buffer, win->insert_text_handler);
		g_signal_handler_block(win->widget, win->line_input_keypress_handler);

		/* Make the window uneditable again and retrieve the text that was input */
        gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), FALSE);

        int chars_written = finish_text_buffer_line_input(win, TRUE);
        ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
		event_throw(glk, evtype_LineInput, win, chars_written, 0);
	}

	/* Apply the 'input' style to the text that was entered */
	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
	GtkTextIter input_iter;
	GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
	gtk_text_buffer_get_iter_at_mark(window_buffer, &input_iter, input_position);
	gtk_text_buffer_apply_tag_by_name(window_buffer, "input", &input_iter, &end_iter);
}

/* Internal function: Callback for signal activate on the line input GtkEntry
in a text grid window. */
void
on_input_entry_activate(GtkEntry *input_entry, winid_t win)
{
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);

	int chars_written = finish_text_grid_line_input(win, TRUE);
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	event_throw(glk, evtype_LineInput, win, chars_written, 0);
}

/* Internal function: Callback for signal key-press-event on the line input
GtkEntry in a text grid window. */
gboolean
on_input_entry_key_press_event(GtkEntry *input_entry, GdkEventKey *event, winid_t win)
{
	if(event->keyval == GDK_Up || event->keyval == GDK_KP_Up
		|| event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		/* Prevent falling off the end of the history list */
		if( (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
			&& win->history_pos && win->history_pos->next == NULL)
			return TRUE;
		if( (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
			&& (win->history_pos == NULL || win->history_pos->prev == NULL) )
			return TRUE;

		if(win->history_pos == NULL)
		{
			const gchar *current_input = gtk_entry_get_text(input_entry);
			win->history = g_list_prepend(win->history, g_strdup(current_input));
			win->history_pos = win->history;
		}

		if(event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
		{
			if(win->history_pos)
				win->history_pos = g_list_next(win->history_pos);
			else
				win->history_pos = win->history;
		}
		else /* down */
			win->history_pos = g_list_previous(win->history_pos);

		/* Insert the history item into the window */
		g_signal_handler_block(input_entry, win->line_input_entry_changed);
		gtk_entry_set_text(input_entry, win->history_pos->data);
		g_signal_handler_unblock(input_entry, win->line_input_entry_changed);
		return TRUE;
	}
	return FALSE;
}

void
on_input_entry_changed(GtkEditable *editable, winid_t win)
{
	/* Set the history position to NULL and erase the text we were already editing */
	if(win->history_pos != NULL)
	{
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
		win->history_pos = NULL;
	}
}

glui32
keyval_to_glk_keycode(guint keyval, gboolean unicode)
{
	glui32 keycode;
	switch(keyval) {
		case GDK_Up:
		case GDK_KP_Up: return keycode_Up;
		case GDK_Down:
		case GDK_KP_Down: return keycode_Down;
		case GDK_Left:
		case GDK_KP_Left: return keycode_Left;
		case GDK_Right:
		case GDK_KP_Right: return keycode_Right;
		case GDK_Linefeed:
		case GDK_Return:
		case GDK_KP_Enter: return keycode_Return;
		case GDK_Delete:
		case GDK_BackSpace:
		case GDK_KP_Delete: return keycode_Delete;
		case GDK_Escape: return keycode_Escape;
		case GDK_Tab:
		case GDK_KP_Tab: return keycode_Tab;
		case GDK_Page_Up:
		case GDK_KP_Page_Up: return keycode_PageUp;
		case GDK_Page_Down:
		case GDK_KP_Page_Down: return keycode_PageDown;
		case GDK_Home:
		case GDK_KP_Home: return keycode_Home;
		case GDK_End:
		case GDK_KP_End: return keycode_End;
		case GDK_F1:
		case GDK_KP_F1: return keycode_Func1;
		case GDK_F2:
		case GDK_KP_F2: return keycode_Func2;
		case GDK_F3:
		case GDK_KP_F3: return keycode_Func3;
		case GDK_F4:
		case GDK_KP_F4: return keycode_Func4;
		case GDK_F5: return keycode_Func5;
		case GDK_F6: return keycode_Func6;
		case GDK_F7: return keycode_Func7;
		case GDK_F8: return keycode_Func8;
		case GDK_F9: return keycode_Func9;
		case GDK_F10: return keycode_Func10;
		case GDK_F11: return keycode_Func11;
		case GDK_F12: return keycode_Func12;
		default:
			keycode = gdk_keyval_to_unicode(keyval);
			/* If keycode is 0, then keyval was not recognized; also return
			unknown if Latin-1 input was requested and the character is not in
			Latin-1 */
			if(keycode == 0 || (!unicode && keycode > 255))
				return keycode_Unknown;
			return keycode;
	}
}

void
force_char_input_from_queue(winid_t win, event_t *event)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	guint keyval = GPOINTER_TO_UINT(g_async_queue_pop(glk_data->char_input_queue));

	glk_cancel_char_event(win);

	gdk_threads_enter();
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);
	g_signal_emit_by_name(glk, "char-input", win->rock, keyval);
	gdk_threads_leave();

	event->type = evtype_CharInput;
	event->win = win;
	event->val1 = keyval_to_glk_keycode(keyval, win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE);
	event->val2 = 0;
}

void
force_line_input_from_queue(winid_t win, event_t *event)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	const gchar *text = g_async_queue_pop(glk_data->line_input_queue);
	glui32 chars_written = 0;

	gdk_threads_enter();
	if(win->type == wintype_TextBuffer)
	{
		GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		GtkTextIter start, end;

		/* Remove signal handlers so the line input doesn't get picked up again */
		g_signal_handler_block(buffer, win->insert_text_handler);
		g_signal_handler_block(win->widget, win->line_input_keypress_handler);

		/* Erase any text that was already typed */
		GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
		gtk_text_buffer_get_iter_at_mark(buffer, &start, input_position);
		gtk_text_buffer_get_end_iter(buffer, &end);
		gtk_text_buffer_delete(buffer, &start, &end);

		/* Make the window uneditable again */
		gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), FALSE);

		/* Insert the forced input into the window */
		gtk_text_buffer_get_end_iter(buffer, &end);
		gchar *text_to_insert = g_strconcat(text, "\n", NULL);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &end, text_to_insert, -1, "default", "input", NULL);
		chars_written = finish_text_buffer_line_input(win, TRUE);
	}
	else if(win->type == wintype_TextGrid)
	{
		/* Remove signal handlers so the line input doesn't get picked up again */
		g_signal_handler_block(win->widget, win->char_input_keypress_handler);

		/* Insert the forced input into the window */
		gtk_entry_set_text(GTK_ENTRY(win->input_entry), text);
		chars_written = finish_text_grid_line_input(win, TRUE);
	}
	gdk_threads_leave();

	event->type = evtype_LineInput;
	event->win = win;
	event->val1 = chars_written;
	event->val2 = 0;
}

/*** Internal function: cancels any pending input requests on the window and presents a warning if not INPUT_REQUEST_NONE ***/
void
cancel_old_input_request(winid_t win)
{
	switch(win->input_request_type) {
	case INPUT_REQUEST_NONE:
		break; /* All is well */
	case INPUT_REQUEST_CHARACTER:
	case INPUT_REQUEST_CHARACTER_UNICODE:
		glk_cancel_char_event(win);
		WARNING("Cancelling pending char event");
		break;
	case INPUT_REQUEST_LINE:
	case INPUT_REQUEST_LINE_UNICODE:
		glk_cancel_line_event(win, NULL);
		WARNING("Cancelling pending line event");
		break;
	default:
		WARNING("Could not cancel pending input request: unknown input request");
	}
}
