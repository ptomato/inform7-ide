#include <glib.h>
#include "window.h"
#include "magic.h"
#include "chimara-glk-private.h"
#include "gi_dispa.h"
#include "pager.h"

extern GPrivate *glk_data_key;

static winid_t
window_new_common(glui32 rock)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	winid_t win = g_new0(struct glk_window_struct, 1);
	
	win->magic = MAGIC_WINDOW;
	win->rock = rock;
	if(glk_data->register_obj)
		win->disprock = (*glk_data->register_obj)(win, gidisp_Class_Window);
	
	win->window_node = g_node_new(win);
	
	/* Every window has a window stream, but printing to it might have no effect */
	win->window_stream = stream_new_common(0);
	win->window_stream->file_mode = filemode_Write;
	win->window_stream->type = STREAM_TYPE_WINDOW;
	win->window_stream->window = win;
	win->window_stream->style = "normal";
	win->window_stream->glk_style = "normal";

	win->echo_stream = NULL;
	win->input_request_type = INPUT_REQUEST_NONE;
	win->line_input_buffer = NULL;
	win->line_input_buffer_unicode = NULL;
	win->history = NULL;

	/* Initialise the buffer */
	win->buffer = g_string_sized_new(1024);

	/* Initialise hyperlink table */
	win->hyperlinks = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, g_free);
	
	return win;
}

/* Internal function: do all the stuff necessary to close a window. Call only
 from Glk thread. */
static void
window_close_common(winid_t win, gboolean destroy_node)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	if(glk_data->unregister_obj) 
	{
        (*glk_data->unregister_obj)(win, gidisp_Class_Window, win->disprock);
        win->disprock.ptr = NULL;
    }
	
	if(destroy_node)
		g_node_destroy(win->window_node);
	
	win->magic = MAGIC_FREE;
	
	g_list_foreach(win->history, (GFunc)g_free, NULL);
	g_list_free(win->history);
	
	g_string_free(win->buffer, TRUE);
	g_hash_table_destroy(win->hyperlinks);
	g_free(win->current_hyperlink);

	if(win->pager_layout)
		g_object_unref(win->pager_layout);

	g_free(win);
}

/**
 * glk_window_iterate:
 * @win: A window, or %NULL.
 * @rockptr: Return location for the next window's rock, or %NULL.
 *
 * This function can be used to iterate through the list of all open windows
 * (including pair windows.) See <link 
 * linkend="chimara-Iterating-Through-Opaque-Objects">Iterating Through Opaque
 * Objects</link>.
 *
 * As that section describes, the order in which windows are returned is
 * arbitrary. The root window is not necessarily first, nor is it necessarily
 * last.
 *
 * Returns: the next window, or %NULL if there are no more.
 */
winid_t
glk_window_iterate(winid_t win, glui32 *rockptr)
{
	VALID_WINDOW_OR_NULL(win, return NULL);
	
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GNode *retnode;
	
	if(win == NULL)
		retnode = glk_data->root_window;
	else
	{
		GNode *node = win->window_node;
		if( G_NODE_IS_LEAF(node) )
		{
			while(node && node->next == NULL)
				node = node->parent;
			if(node)
				retnode = node->next;
			else
				retnode = NULL;
		}
		else
			retnode = g_node_first_child(node);
	}
	winid_t retval = retnode? (winid_t)retnode->data : NULL;
		
	/* Store the window's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_window_get_rock(retval);
		
	return retval;
}

/**
 * glk_window_get_rock:
 * @win: A window.
 * 
 * Returns @win's rock value. Pair windows always have rock 0; all other windows
 * return whatever rock value you created them with.
 *
 * Returns: A rock value.
 */
glui32
glk_window_get_rock(winid_t win)
{
	VALID_WINDOW(win, return 0);
	return win->rock;
}

/**
 * glk_window_get_type:
 * @win: A window.
 *
 * Returns @win's type, one of %wintype_Blank, %wintype_Pair,
 * %wintype_TextBuffer, %wintype_TextGrid, or %wintype_Graphics.
 *
 * Returns: The window's type.
 */
glui32
glk_window_get_type(winid_t win)
{
	VALID_WINDOW(win, return 0);
	return win->type;
}

/**
 * glk_window_get_parent:
 * @win: A window.
 *
 * Returns the window which is the parent of @win. If @win is the root window,
 * this returns %NULL, since the root window has no parent. Remember that the
 * parent of every window is a pair window; other window types are always
 * childless.
 *
 * Returns: A window, or %NULL.
 */
winid_t
glk_window_get_parent(winid_t win)
{
	VALID_WINDOW(win, return NULL);

	/* Value will also be NULL if win is the root window */
	if(win->window_node->parent == NULL)
		return NULL;

	return (winid_t)win->window_node->parent->data;
}

/**
 * glk_window_get_sibling:
 * @win: A window.
 *
 * Returns the other child of @win's parent. If @win is the root window, this
 * returns %NULL.
 *
 * Returns: A window, or %NULL.
 */
winid_t
glk_window_get_sibling(winid_t win)
{
	VALID_WINDOW(win, return NULL);
	
	if(G_NODE_IS_ROOT(win->window_node))
		return NULL;
	if(win->window_node->next)
		return (winid_t)win->window_node->next;
	return (winid_t)win->window_node->prev;
}

/**
 * glk_window_get_root:
 * 
 * Returns the root window. If there are no windows, this returns %NULL.
 *
 * Returns: A window, or %NULL.
 */
winid_t
glk_window_get_root()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	if(glk_data->root_window == NULL)
		return NULL;
	return (winid_t)glk_data->root_window->data;
}

/**
 * glk_window_open:
 * @split: The window to split to create the new window. Must be 0 if there
 * are no windows yet.
 * @method: Position of the new window and method of size computation. One of
 * %winmethod_Above, %winmethod_Below, %winmethod_Left, or %winmethod_Right
 * OR'ed with %winmethod_Fixed or %winmethod_Proportional. If @wintype is
 * %wintype_Blank, then %winmethod_Fixed is not allowed.
 * @size: Size of the new window, in percentage points if @method is
 * %winmethod_Proportional, otherwise in characters if @wintype is 
 * %wintype_TextBuffer or %wintype_TextGrid, or pixels if @wintype is
 * %wintype_Graphics.
 * @wintype: Type of the new window. One of %wintype_Blank, %wintype_TextGrid,
 * %wintype_TextBuffer, or %wintype_Graphics.
 * @rock: The new window's rock value.
 *
 * Creates a new window. If there are no windows, the first three arguments are
 * meaningless. @split <emphasis>must</emphasis> be 0, and @method and @size
 * are ignored. @wintype is the type of window you're creating, and @rock is
 * the rock (see <link linkend="chimara-Rocks">Rocks</link>).
 *
 * If any windows exist, new windows must be created by splitting existing
 * ones. @split is the window you want to split; this <emphasis>must 
 * not</emphasis> be zero. @method is a mask of constants to specify the
 * direction and the split method (see below). @size is the size of the split.
 * @wintype is the type of window you're creating, and @rock is the rock.
 *
 * Remember that it is possible that the library will be unable to create a new
 * window, in which case glk_window_open() will return %NULL.
 * 
 * <note><para>
 *   It is acceptable to gracefully exit, if the window you are creating is an
 *   important one &mdash; such as your first window. But you should not try to
 *   perform any window operation on the id until you have tested to make sure
 *   it is non-zero.
 * </para></note>
 * 
 * The examples we've seen so far have the simplest kind of size control. (Yes,
 * this is <quote>below</quote>.) Every pair is a percentage split, with 
 * <inlineequation>
 *   <alt>X</alt>
 *   <mathphrase>X</mathphrase>
 * </inlineequation>
 * percent going to one side, and 
 * <inlineequation>
 *   <alt>(100-X)</alt>
 *   <mathphrase>(100 - X)</mathphrase>
 * </inlineequation> 
 * percent going to the other side. If the player resizes the window, the whole
 * mess expands, contracts, or stretches in a uniform way.
 * 
 * As I said above, you can also make fixed-size splits. This is a little more
 * complicated, because you have to know how this fixed size is measured.
 * 
 * Sizes are measured in a way which is different for each window type. For
 * example, a text grid window is measured by the size of its fixed-width font.
 * You can make a text grid window which is fixed at a height of four rows, or
 * ten columns. A text buffer window is measured by the size of its font.
 * 
 * <note><para>
 *   Remember that different windows may use different size fonts. Even two
 *   text grid windows may use fixed-size fonts of different sizes.
 * </para></note>
 *
 * Graphics windows are measured in pixels, not characters. Blank windows
 * aren't measured at all; there's no meaningful way to measure them, and
 * therefore you can't create a blank window of a fixed size, only of a
 * proportional (percentage) size.
 * 
 * So to create a text buffer window which takes the top 40% of the original
 * window's space, you would execute
 * |[ newwin = glk_window_open(win, winmethod_Above | winmethod_Proportional, 40, wintype_TextBuffer, 0); ]|
 *
 * To create a text grid which is always five lines high, at the bottom of the
 * original window, you would do
 * |[ newwin = glk_window_open(win, winmethod_Below | winmethod_Fixed, 5, wintype_TextGrid, 0); ]|
 * 
 * Note that the meaning of the @size argument depends on the @method argument.
 * If the method is %winmethod_Fixed, it also depends on the @wintype argument.
 * The new window is then called the <quote>key window</quote> of this split,
 * because its window type determines how the split size is computed.
 * 
 * <note><para>
 *   For %winmethod_Proportional splits, you can still call the new window the
 *   <quote>key window</quote>. But the key window is not important for
 *   proportional splits, because the size will always be computed as a simple
 *   ratio of the available space, not a fixed size of one child window.
 * </para></note>
 * 
 * This system is more or less peachy as long as all the constraints work out.
 * What happens when there is a conflict? The rules are simple. Size control
 * always flows down the tree, and the player is at the top. Let's bring out an
 * example:
 * <informaltable frame="none"><tgroup cols="2"><tbody><row>
 * <entry><mediaobject><imageobject><imagedata fileref="fig5-7a.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *      O
 *     / \
 *    O   B
 *   / \
 *  A   C
 * </literallayout></textobject></mediaobject></entry>
 * </row></tbody></tgroup></informaltable>
 * 
 * First we split A into A and B, with a 50% proportional split. Then we split
 * A into A and C, with C above, C being a text grid window, and C gets a fixed
 * size of two rows (as measured in its own font size). A gets whatever remains
 * of the 50% it had before.
 * 
 * Now the player stretches the window vertically.
 * <informalfigure><mediaobject><imageobject><imagedata fileref="fig6.png"/>
 * </imageobject></mediaobject></informalfigure>
 * 
 * The library figures: the topmost split, the original A/B split, is 50-50. So
 * B gets half the screen space, and the pair window next to it (the lower
 * <quote>O</quote>) gets the other half. Then it looks at the lower 
 * <quote>O</quote>. C gets two rows; A gets the rest. All done.
 * 
 * Then the user maliciously starts squeezing the window down, in stages:
 * <informaltable xml:id="chimara-Figure-Squeezing-Window" frame="none">
 * <tgroup cols="5"><tbody><row valign="top">
 * <entry><mediaobject><imageobject><imagedata fileref="fig5-7a.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><imageobject><imagedata fileref="fig7b.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><imageobject><imagedata fileref="fig7c.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><imageobject><imagedata fileref="fig7d.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><imageobject><imagedata fileref="fig7e.png"/>
 * </imageobject></mediaobject></entry>
 * </row></tbody></tgroup></informaltable>
 * 
 * The logic remains the same. B always gets half the space. At stage 3,
 * there's no room left for A, so it winds up with zero height. Nothing
 * displayed in A will be visible. At stage 4, there isn't even room in the
 * upper 50% to give C its two rows; so it only gets one. Finally, C is
 * squashed out of existence as well.
 * 
 * When a window winds up undersized, it remembers what size it should be. In
 * the example above, A remembers that it should be two rows; if the user
 * expands the window to the original size, it would return to the original
 * layout.
 * 
 * The downward flow of control is a bit harsh. After all, in stage 4, there's
 * room for C to have its two rows if only B would give up some of its 50%. But
 * this does not happen.
 * 
 * <note><para>
 *   This makes life much easier for the Glk library. To determine the
 *   configuration of a window, it only needs to look at the window's
 *   ancestors, never at its descendants. So window layout is a simple
 *   recursive algorithm, no backtracking.
 * </para></note>
 * 
 * What happens when you split a fixed-size window? The resulting pair window
 * &mdash; that is, the two new parts together &mdash; retain the same size
 * constraint as the original window that was split. The key window for the
 * original split is still the key window for that split, even though it's now
 * a grandchild instead of a child.
 * 
 * The easy, and correct, way to think about this is that the size constraint
 * is stored by a window's parent, not the window itself; and a constraint
 * consists of a pointer to a key window plus a size value.
 * 
 * <informaltable frame="none"><tgroup cols="6"><tbody><row>
 * <entry><mediaobject><imageobject><imagedata fileref="fig8a.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *  A   
 * </literallayout></textobject></mediaobject></entry>
 * <entry><mediaobject><imageobject><imagedata fileref="fig8b.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *    O1  
 *   / \  
 *  A   B 
 * </literallayout></textobject></mediaobject></entry> 
 * <entry><mediaobject><imageobject><imagedata fileref="fig8c.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *      O1  
 *     / \  
 *    O2  B 
 *   / \    
 *  A   C   
 * </literallayout></textobject></mediaobject></entry> 
 * </row></tbody></tgroup></informaltable>
 * After the first split, the new pair window (O1, which covers the whole
 * screen) knows that its first child (A) is above the second, and gets 50% of
 * its own area. (A is the key window for this split, but a proportional split
 * doesn't care about key windows.)
 * 
 * After the second split, all this remains true; O1 knows that its first child
 * gets 50% of its space, and A is O1's key window. But now O1's first child is
 * O2 instead of A. The newer pair window (O2) knows that its first child (C)
 * is above the second, and gets a fixed size of two rows. (As measured in C's
 * font, because C is O2's key window.)
 * 
 * If we split C, now, the resulting pair will still be two C-font rows high
 * &mdash; that is, tall enough for two lines of whatever font C displays. For
 * the sake of example, we'll do this vertically.
 * <informaltable frame="none"><tgroup cols="2"><tbody><row>
 * <entry><mediaobject><imageobject><imagedata fileref="fig9.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *      O1
 *     / \
 *    O2  B
 *   / \
 *  A   O3
 *     / \
 *    C   D
 * </literallayout></textobject></mediaobject></entry> 
 * </row></tbody></tgroup></informaltable>
 * 
 * O3 now knows that its children have a 50-50 left-right split. O2 is still
 * committed to giving its upper child, O3, two C-font rows. Again, this is
 * because C is O2's key window. 
 *
 * <note><para>
 *   This turns out to be a good idea, because it means that C, the text grid
 *   window, is still two rows high. If O3 had been a upper-lower split, things
 *   wouldn't work out so neatly. But the rules would still apply. If you don't
 *   like this, don't do it.
 * </para></note>
 *
 * Returns: the new window, or %NULL on error.
 */
winid_t
glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, 
                glui32 rock)
{
	VALID_WINDOW_OR_NULL(split, return NULL);
	g_return_val_if_fail(method == (method & (winmethod_DirMask | winmethod_DivisionMask)), NULL);
	g_return_val_if_fail(!(((method & winmethod_DivisionMask) == winmethod_Proportional) && size > 100), NULL);	

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	if(split == NULL && glk_data->root_window != NULL)
	{
		ILLEGAL("Tried to open a new root window, but there is already a root window");
		return NULL;
	}
	
	gdk_threads_enter();
	
	/* Create the new window */
	winid_t win = window_new_common(rock);
	win->type = wintype;

	switch(wintype)
	{
		case wintype_Blank:
		{
			/* A blank window will be a label without any text */
			GtkWidget *label = gtk_label_new("");
			gtk_widget_show(label);
			
			win->widget = label;
			win->frame = label;
			/* A blank window has no size */
			win->unit_width = 0;
			win->unit_height = 0;
		}
			break;
		
		case wintype_TextGrid:
		{
		    GtkWidget *textview = gtk_text_view_new();
			GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(textview) );

		    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(textview), GTK_WRAP_NONE );
		    gtk_text_view_set_editable( GTK_TEXT_VIEW(textview), FALSE );
			gtk_widget_show(textview);
		    		
			/* Create the styles available to the window stream */
			style_init_textgrid(textbuffer);
			gtk_widget_modify_font( textview, get_current_font(wintype) );
		    
		    win->widget = textview;
		    win->frame = textview;
			
			/* Determine the size of a "0" character in pixels */
			PangoLayout *zero = gtk_widget_create_pango_layout(textview, "0");
			pango_layout_set_font_description( zero, get_current_font(wintype) );
			pango_layout_get_pixel_size(zero, &(win->unit_width), &(win->unit_height));
			g_object_unref(zero);
			/* width and height are set later */
			
			/* Connect signal handlers */
			win->char_input_keypress_handler = g_signal_connect(textview, "key-press-event", G_CALLBACK(on_char_input_key_press_event), win);
			g_signal_handler_block(textview, win->char_input_keypress_handler);
			win->line_input_keypress_handler = g_signal_connect(textview, "key-press-event", G_CALLBACK(on_line_input_key_press_event), win);
			g_signal_handler_block(textview, win->line_input_keypress_handler);
			win->shutdown_keypress_handler = g_signal_connect(textview, "key-press-event", G_CALLBACK(on_shutdown_key_press_event), win);
			g_signal_handler_block(textview, win->shutdown_keypress_handler);
			win->button_press_event_handler = g_signal_connect( textview, "button-press-event", G_CALLBACK(on_window_button_press), win );
			g_signal_handler_block(textview, win->button_press_event_handler);
		}
		    break;
		
		case wintype_TextBuffer:
		{
			GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
			GtkWidget *textview = gtk_text_view_new();
			GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(textview) );

			gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
			
			gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR );
			gtk_text_view_set_editable( GTK_TEXT_VIEW(textview), FALSE );
			gtk_text_view_set_pixels_inside_wrap( GTK_TEXT_VIEW(textview), 3 );
			gtk_text_view_set_left_margin( GTK_TEXT_VIEW(textview), 20 );
			gtk_text_view_set_right_margin( GTK_TEXT_VIEW(textview), 20 );

			gtk_container_add( GTK_CONTAINER(scrolledwindow), textview );
			gtk_widget_show_all(scrolledwindow);

			win->widget = textview;
			win->frame = scrolledwindow;
			
			/* Create the styles available to the window stream */
			style_init_textbuffer(textbuffer);
			style_init_more_prompt(win);
			gtk_widget_modify_font( textview, get_current_font(wintype) );
			
			/* Determine the size of a "0" character in pixels */
			PangoLayout *zero = gtk_widget_create_pango_layout(textview, "0");
			pango_layout_set_font_description( zero, get_current_font(wintype) );
			pango_layout_get_pixel_size(zero, &(win->unit_width), &(win->unit_height));
			g_object_unref(zero);

			/* Connect signal handlers */
			
			/* Pager */
			g_signal_connect_after( textview, "size-request", G_CALLBACK(pager_after_size_request), win );
			win->pager_expose_handler = g_signal_connect_after( textview, "expose-event", G_CALLBACK(pager_on_expose), win );
			g_signal_handler_block(textview, win->pager_expose_handler);
			win->pager_keypress_handler = g_signal_connect( textview, "key-press-event", G_CALLBACK(pager_on_key_press_event), win );
			g_signal_handler_block(textview, win->pager_keypress_handler);
			GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledwindow));
			win->pager_adjustment_handler = g_signal_connect_after(adj, "value-changed", G_CALLBACK(pager_after_adjustment_changed), win);

			/* Char and line input */
			win->char_input_keypress_handler = g_signal_connect( textview, "key-press-event", G_CALLBACK(on_char_input_key_press_event), win );
			g_signal_handler_block(textview, win->char_input_keypress_handler);
			win->line_input_keypress_handler = g_signal_connect( textview, "key-press-event", G_CALLBACK(on_line_input_key_press_event), win );
			g_signal_handler_block(textview, win->line_input_keypress_handler);
			win->insert_text_handler = g_signal_connect_after( textbuffer, "insert-text", G_CALLBACK(after_window_insert_text), win );
			g_signal_handler_block(textbuffer, win->insert_text_handler);

			/* Shutdown key press */
			win->shutdown_keypress_handler = g_signal_connect( textview, "key-press-event", G_CALLBACK(on_shutdown_key_press_event), win );
			g_signal_handler_block(textview, win->shutdown_keypress_handler);			

			/* Create an editable tag to indicate uneditable parts of the window
			(for line input) */
			gtk_text_buffer_create_tag(textbuffer, "uneditable", "editable", FALSE, "editable-set", TRUE, NULL);

			/* Mark the position where the user will input text and the end mark */
			GtkTextIter end;
			gtk_text_buffer_get_end_iter(textbuffer, &end);
			gtk_text_buffer_create_mark(textbuffer, "input_position", &end, TRUE);
			gtk_text_buffer_create_mark(textbuffer, "end_position", &end, FALSE);

			/* Create the pager position mark; it stands for the last character in the buffer
			 that has been on-screen */
			gtk_text_buffer_create_mark(textbuffer, "pager_position", &end, TRUE);
		}
			break;

		case wintype_Graphics:
		{
		    GtkWidget *image = gtk_image_new_from_pixmap(NULL, NULL);
			gtk_widget_show(image);

			win->unit_width = 1;
			win->unit_height = 1;
		    win->widget = image;
		    win->frame = image;
			win->background_color = 0x00FFFFFF;
		    		
			/* Connect signal handlers */
			win->button_press_event_handler = g_signal_connect(image, "button-press-event", G_CALLBACK(on_window_button_press), win);
			g_signal_handler_block(image, win->button_press_event_handler);
			win->shutdown_keypress_handler = g_signal_connect(image, "key-press-event", G_CALLBACK(on_shutdown_key_press_event), win);
			g_signal_handler_block(image, win->shutdown_keypress_handler);			
			win->size_allocate_handler = g_signal_connect(image, "size-allocate", G_CALLBACK(on_graphics_size_allocate), win);
		}
		    break;
			
		default:
			gdk_threads_leave();
			ILLEGAL_PARAM("Unknown window type: %u", wintype);
			g_free(win);
			g_node_destroy(glk_data->root_window);
			glk_data->root_window = NULL;
			return NULL;
	}

	/* Set the minimum size to "as small as possible" so it doesn't depend on
	 the size of the window contents */
	gtk_widget_set_size_request(win->widget, 0, 0);
	gtk_widget_set_size_request(win->frame, 0, 0);
	
	if(split)
	{
		/* When splitting, construct a new parent window
		 * copying most characteristics from the window that is being split */
		winid_t pair = window_new_common(0);
		pair->type = wintype_Pair;

		/* The pair window must know about its children's split method */
		pair->key_window = win;
		pair->split_method = method;
		pair->constraint_size = size;
		
		/* Insert the new window into the window tree */
		if(split->window_node->parent == NULL)
			glk_data->root_window = pair->window_node;
		else 
		{
			if( split->window_node == g_node_first_sibling(split->window_node) )
				g_node_prepend(split->window_node->parent, pair->window_node);
			else
				g_node_append(split->window_node->parent, pair->window_node);
			g_node_unlink(split->window_node);
		}
		/* Place the windows in the correct order */
		switch(method & winmethod_DirMask)
		{
			case winmethod_Left:
			case winmethod_Above:
				g_node_append(pair->window_node, win->window_node);
				g_node_append(pair->window_node, split->window_node);
				break;
			case winmethod_Right:
			case winmethod_Below:
				g_node_append(pair->window_node, split->window_node);
				g_node_append(pair->window_node, win->window_node);
				break;
		}

	} else {
		/* Set the window as root window */
		glk_data->root_window = win->window_node;
	}

	/* Set the window as a child of the Glk widget, don't trigger an arrange event */
	g_mutex_lock(glk_data->arrange_lock);
	glk_data->needs_rearrange = TRUE;
	glk_data->ignore_next_arrange_event = TRUE;
	g_mutex_unlock(glk_data->arrange_lock);
	gtk_widget_set_parent(win->frame, GTK_WIDGET(glk_data->self));
	gtk_widget_queue_resize(GTK_WIDGET(glk_data->self));
	
    /* For text grid windows, fill the buffer with blanks. */
    if(wintype == wintype_TextGrid)
    {
        /* Create the cursor position mark */
        GtkTextIter begin;
        GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
        gtk_text_buffer_get_start_iter(buffer, &begin);
        gtk_text_buffer_create_mark(buffer, "cursor_position", &begin, TRUE);
	}

	gdk_threads_leave();
    glk_window_clear(win);
	return win;
}

/* Internal function: if node's key window is closing_win or one of its
 children, set node's key window to NULL. */
static gboolean 
remove_key_windows(GNode *node, winid_t closing_win)
{
	winid_t win = (winid_t)node->data;
	if(win->key_window && (win->key_window == closing_win || g_node_is_ancestor(closing_win->window_node, win->key_window->window_node)))
		win->key_window = NULL;
	return FALSE; /* Don't stop the traversal */
}

/* Internal function: destroy this window's GTK widgets, window streams, 
 and those of all its children. GDK threads must be locked. */
static void
destroy_windows_below(winid_t win, stream_result_t *result)
{
	switch(win->type)
	{
		case wintype_Blank:
	    case wintype_TextGrid:
		case wintype_TextBuffer:
		case wintype_Graphics:
			gtk_widget_unparent(win->frame);
			break;

		case wintype_Pair:
			destroy_windows_below(win->window_node->children->data, NULL);
			destroy_windows_below(win->window_node->children->next->data, NULL);
			break;

		default:
			ILLEGAL_PARAM("Unknown window type: %u", win->type);
			return;
	}
	stream_close_common(win->window_stream, result);
}

/* Internal function: free the winid_t structure of this window and those of all its children */
static void
free_winids_below(winid_t win)
{
	if(win->type == wintype_Pair) {
		free_winids_below(win->window_node->children->data);
		free_winids_below(win->window_node->children->next->data);
	}
	window_close_common(win, FALSE);
}

/**
 * glk_window_close:
 * @win: Window to close.
 * @result: Pointer to a #stream_result_t in which to store the write count.
 *
 * Closes @win, which is pretty much exactly the opposite of opening a window.
 * It is legal to close all your windows, or to close the root window (which is
 * the same thing.) 
 *
 * The @result argument is filled with the output character count of the window
 * stream. See <link linkend="chimara-Streams">Streams</link> and <link
 * linkend="chimara-Closing-Streams">Closing Streams</link>.
 * 
 * When you close a window (and it is not the root window), the other window
 * in its pair takes over all the freed-up area. Let's close D, in the current
 * example:
 * <informaltable frame="none"><tgroup cols="2"><tbody><row>
 * <entry><mediaobject><imageobject><imagedata fileref="fig10.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *      O1
 *     / \
 *    O2  B
 *   / \
 *  A   C
 * </literallayout></textobject></mediaobject></entry> 
 * </row></tbody></tgroup></informaltable>
 * 
 * Notice what has happened. D is gone. O3 is gone, and its 50-50 left-right
 * split has gone with it. The other size constraints are unchanged; O2 is
 * still committed to giving its upper child two rows, as measured in the font
 * of O2's key window, which is C. Conveniently, O2's upper child is C, just as
 * it was before we created D. In fact, now that D is gone, everything is back
 * to the way it was before we created D.
 * 
 * But what if we had closed C instead of D? We would have gotten this:
 * <informaltable frame="none"><tgroup cols="2"><tbody><row>
 * <entry><mediaobject><imageobject><imagedata fileref="fig11.png"/>
 * </imageobject></mediaobject></entry>
 * <entry><mediaobject><textobject><literallayout class="monospaced">
 *      O1
 *     / \
 *    O2  B
 *   / \
 *  A   D
 * </literallayout></textobject></mediaobject></entry> 
 * </row></tbody></tgroup></informaltable>
 * 
 * Again, O3 is gone. But D has collapsed to zero height. This is because its
 * height is controlled by O2, and O2's key window was C, and C is now gone. O2
 * no longer has a key window at all, so it cannot compute a height for its
 * upper child, so it defaults to zero.
 * 
 * <note><para>
 *   This may seem to be an inconvenient choice. That is deliberate. You should
 *   not leave a pair window with no key, and the zero-height default reminds
 *   you not to. You can use glk_window_set_arrangement() to set a new split
 *   measurement and key window. See <link 
 *   linkend="chimara-Changing-Window-Constraints">Changing Window
 *   Constraints</link>.
 * </para></note>
 */
void
glk_window_close(winid_t win, stream_result_t *result)
{
	VALID_WINDOW(win, return);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	gdk_threads_enter(); /* Prevent redraw while we're trashing the window */
	
	/* If any pair windows have this window or its children as a key window,
	 set their key window to NULL */
	g_node_traverse(glk_data->root_window, G_IN_ORDER, G_TRAVERSE_NON_LEAVES, -1, (GNodeTraverseFunc)remove_key_windows, win);
	
	/* Close all the window streams and destroy the widgets of this window
	 and below, before trashing the window tree */
	destroy_windows_below(win, result);
	
	/* Then free the winid_t structures below this node, but not this one itself */
	if(win->type == wintype_Pair) {
		free_winids_below(win->window_node->children->data);
		free_winids_below(win->window_node->children->next->data);
	}
	/* So now we should be left with a skeleton tree hanging off this node */	
	
	/* Parent window changes from a split window into the sibling window */
	/* The parent of any window is either a pair window or NULL */
	GNode *pair_node = win->window_node->parent;
	/* If win was not the root window: */
	if(pair_node != NULL)
	{
		gboolean new_child_on_left = ( pair_node == g_node_first_sibling(pair_node) );

		/* Lookup our sibling */
		GNode *sibling_node = pair_node->children;
		if(sibling_node == win->window_node)
			sibling_node = sibling_node->next;

		GNode *new_parent_node = pair_node->parent;
		g_node_unlink(pair_node);
		g_node_unlink(sibling_node);
		/* pair_node and sibling_node should now be totally unconnected to the tree */
		
		if(new_parent_node == NULL)
		{
			glk_data->root_window = sibling_node;
		} 
		else 
		{
			if(new_child_on_left)
				g_node_prepend(new_parent_node, sibling_node);
			else
				g_node_append(new_parent_node, sibling_node);
		}

		stream_close_common( ((winid_t) pair_node->data)->window_stream, NULL );
		window_close_common( (winid_t) pair_node->data, TRUE);
	} 
	else /* it was the root window */
	{
		glk_data->root_window = NULL;
	}

	window_close_common(win, FALSE);

	/* Schedule a redraw */
	g_mutex_lock(glk_data->arrange_lock);
	glk_data->needs_rearrange = TRUE;
	glk_data->ignore_next_arrange_event = TRUE;
	g_mutex_unlock(glk_data->arrange_lock);
	gtk_widget_queue_resize( GTK_WIDGET(glk_data->self) );
	gdk_threads_leave();
}

/**
 * glk_window_clear:
 * @win: A window.
 *
 * Erases @win. The meaning of this depends on the window type.
 * <variablelist>
 * <varlistentry>
 *  <term>Text buffer</term>
 *  <listitem><para>
 *   This may do any number of things, such as delete all text in the window, or
 *   print enough blank lines to scroll all text beyond visibility, or insert a
 *   page-break marker which is treated specially by the display part of the
 *   library.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry>
 *  <term>Text grid</term>
 *  <listitem><para>
 *   This will clear the window, filling all positions with blanks. The window
 *   cursor is moved to the top left corner (position 0,0).
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry>
 *  <term>Graphics</term>
 *  <listitem><para>
 *   Clears the entire window to its current background color. See <link
 *   linkend="chimara-The-Types-of-Windows&num;wintype-Graphics">Graphics 
 *   Windows</link>.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry>
 *  <term>Other window types</term>
 *  <listitem><para>No effect.</para></listitem>
 * </varlistentry>
 * </variablelist>
 *
 * It is illegal to erase a window which has line input pending. 
 */
void
glk_window_clear(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->input_request_type != INPUT_REQUEST_LINE && win->input_request_type != INPUT_REQUEST_LINE_UNICODE);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	switch(win->type)
	{
		case wintype_Blank:
		case wintype_Pair:
			/* do nothing */
			break;
		
		case wintype_TextGrid:
		    /* fill the buffer with blanks */
		{
			/* Wait for the window's size to be updated */
			g_mutex_lock(glk_data->arrange_lock);
			if(glk_data->needs_rearrange)
				g_cond_wait(glk_data->rearranged, glk_data->arrange_lock);
			g_mutex_unlock(glk_data->arrange_lock);
			
		    gdk_threads_enter();
		    
            /* Manually put newlines at the end of each row of characters in the buffer; manual newlines make resizing the window's grid easier. */
            gchar *blanks = g_strnfill(win->width, ' ');
            gchar **blanklines = g_new0(gchar *, win->height + 1);
            int count;
            for(count = 0; count < win->height; count++)
                blanklines[count] = blanks;
            blanklines[win->height] = NULL;
            gchar *text = g_strjoinv("\n", blanklines);
            g_free(blanklines); /* not g_strfreev() */
            g_free(blanks);
            
            GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
            gtk_text_buffer_set_text(textbuffer, text, -1);
            g_free(text);
            
            GtkTextIter begin;
            gtk_text_buffer_get_start_iter(textbuffer, &begin);
            gtk_text_buffer_move_mark_by_name(textbuffer, "cursor_position", &begin);
		    
		    gdk_threads_leave();
		}
		    break;
		
		case wintype_TextBuffer:
			/* delete all text in the window */
		{
			gdk_threads_enter();

			GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
			GtkTextIter start, end;
			gtk_text_buffer_get_bounds(buffer, &start, &end);
			gtk_text_buffer_delete(buffer, &start, &end);

			gdk_threads_leave();
		}
			break;

		case wintype_Graphics:
		{
			/* Wait for the window's size to be updated */
			g_mutex_lock(glk_data->arrange_lock);
			if(glk_data->needs_rearrange)
				g_cond_wait(glk_data->rearranged, glk_data->arrange_lock);
			g_mutex_unlock(glk_data->arrange_lock);

			glk_window_erase_rect(win, 0, 0, win->widget->allocation.width, win->widget->allocation.height);
		}
			break;
		
		default:
			ILLEGAL_PARAM("Unknown window type: %d", win->type);
	}
}

/**
 * glk_set_window:
 * @win: A window, or %NULL.
 *
 * Sets the current stream to @win's window stream. It is exactly equivalent to
 * |[ glk_stream_set_current(glk_window_get_stream(win)) ]| 
 * See <link linkend="chimara-Streams">Streams</link>.
 *
 * <note><title>Chimara</title>
 * <para>
 *   Although this is not mentioned in the specification, @win may also be 
 *   %NULL, in which case the current stream is also set to %NULL.
 * </para></note>
 */
void
glk_set_window(winid_t win)
{
	VALID_WINDOW_OR_NULL(win, return);
	if(win)
		glk_stream_set_current( glk_window_get_stream(win) );
	else
		glk_stream_set_current(NULL);
}

/**
 * glk_window_get_stream:
 * @win: A window.
 *
 * Returns the stream which is associated with @win. (See <link 
 * linkend="chimara-Window-Streams">Window Streams</link>.) Every window has a
 * stream which can be printed to, but this may not be useful, depending on the
 * window type.
 * 
 * <note><para>
 *   For example, printing to a blank window's stream has no effect.
 * </para></note>
 *
 * Returns: A window stream.
 */
strid_t glk_window_get_stream(winid_t win)
{
	VALID_WINDOW(win, return NULL);
	return win->window_stream;
}

/**
 * glk_window_set_echo_stream:
 * @win: A window.
 * @str: A stream to attach to the window, or %NULL.
 *
 * Sets @win's echo stream to @str, which can be any valid output stream. You
 * can reset a window to stop echoing by calling 
 * <code>#glk_window_set_echo_stream(@win, %NULL)</code>.
 *
 * It is illegal to set a window's echo stream to be its 
 * <emphasis>own</emphasis> window stream. That would create an infinite loop,
 * and is nearly certain to crash the Glk library. It is similarly illegal to
 * create a longer loop (two or more windows echoing to each other.)
 */
void
glk_window_set_echo_stream(winid_t win, strid_t str)
{
	VALID_WINDOW(win, return);
	VALID_STREAM_OR_NULL(str, return);
	
	/* Test for an infinite loop */
	strid_t next = str;
	for(; next && next->type == STREAM_TYPE_WINDOW; next = next->window->echo_stream)
	{
		if(next == win->window_stream)
		{
			ILLEGAL("Infinite loop detected");
			win->echo_stream = NULL;
			return;
		}
	}
	
	win->echo_stream = str;
}

/**
 * glk_window_get_echo_stream:
 * @win: A window.
 *
 * Returns the echo stream of window @win. Initially, a window has no echo
 * stream, so <code>#glk_window_get_echo_stream(@win)</code> will return %NULL.
 *
 * Returns: A stream, or %NULL.
 */
strid_t
glk_window_get_echo_stream(winid_t win)
{
	VALID_WINDOW(win, return NULL);
	return win->echo_stream;
}

/**
 * glk_window_get_size:
 * @win: A window.
 * @widthptr: Pointer to a location to store the window's width, or %NULL.
 * @heightptr: Pointer to a location to store the window's height, or %NULL.
 *
 * Simply returns the actual size of the window, in its measurement system.
 * As described in <link linkend="chimara-Other-API-Conventions">Other API 
 * Conventions</link>, either @widthptr or @heightptr can be %NULL, if you
 * only want one measurement. 
 *
 * <note><para>Or, in fact, both, if you want to waste time.</para></note>
 */
void
glk_window_get_size(winid_t win, glui32 *widthptr, glui32 *heightptr)
{
	VALID_WINDOW(win, return);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
    switch(win->type)
    {
        case wintype_Blank:
		case wintype_Pair:
            if(widthptr != NULL)
                *widthptr = 0;
            if(heightptr != NULL)
                *heightptr = 0;
            break;
            
        case wintype_TextGrid:
			/* Wait until the window's size is current */
			g_mutex_lock(glk_data->arrange_lock);
			if(glk_data->needs_rearrange)
				g_cond_wait(glk_data->rearranged, glk_data->arrange_lock);
			g_mutex_unlock(glk_data->arrange_lock);
			
			gdk_threads_enter();
			/* Cache the width and height */
			win->width = (glui32)(win->widget->allocation.width / win->unit_width);
		    win->height = (glui32)(win->widget->allocation.height / win->unit_height);
            gdk_threads_leave();
			
            if(widthptr != NULL)
                *widthptr = win->width;
            if(heightptr != NULL)
                *heightptr = win->height;
            break;
            
        case wintype_TextBuffer:
            /* Wait until the window's size is current */
			g_mutex_lock(glk_data->arrange_lock);
			if(glk_data->needs_rearrange)
				g_cond_wait(glk_data->rearranged, glk_data->arrange_lock);
			g_mutex_unlock(glk_data->arrange_lock);
			
            gdk_threads_enter();
            if(widthptr != NULL)
                *widthptr = (glui32)(win->widget->allocation.width / win->unit_width);
            if(heightptr != NULL)
                *heightptr = (glui32)(win->widget->allocation.height / win->unit_height);
            gdk_threads_leave();
            
            break;

		case wintype_Graphics:
			g_mutex_lock(glk_data->arrange_lock);
			if(glk_data->needs_rearrange)
				g_cond_wait(glk_data->rearranged, glk_data->arrange_lock);
			g_mutex_unlock(glk_data->arrange_lock);
			
            gdk_threads_enter();
            if(widthptr != NULL)
                *widthptr = (glui32)(win->widget->allocation.width);
            if(heightptr != NULL)
                *heightptr = (glui32)(win->widget->allocation.height);
            gdk_threads_leave();
            
            break;
            
        default:
            ILLEGAL_PARAM("Unknown window type: %u", win->type);
    }
}

/**
 * glk_window_set_arrangement:
 * @win: a pair window to rearrange.
 * @method: new method of size computation. One of %winmethod_Above, 
 * %winmethod_Below, %winmethod_Left, or %winmethod_Right OR'ed with 
 * %winmethod_Fixed or %winmethod_Proportional.
 * @size: new size constraint, in percentage points if @method is
 * %winmethod_Proportional, otherwise in characters if @win's type is 
 * %wintype_TextBuffer or %wintype_TextGrid, or pixels if @win's type is
 * %wintype_Graphics.
 * @keywin: new key window, or %NULL to leave the key window unchanged.
 *
 * Changes the size of an existing split &mdash; that is, it changes the 
 * constraint of a given pair window.
 * 
 * Consider the example above, where D has collapsed to zero height. Say D was a
 * text buffer window. You could make a more useful layout by doing
 * |[
 * winid_t o2;
 * o2 = glk_window_get_parent(d);
 * glk_window_set_arrangement(o2, winmethod_Above | winmethod_Fixed, 3, d);
 * ]|
 * That would set D (the upper child of O2) to be O2's key window, and give it a
 * fixed size of 3 rows.
 * 
 * If you later wanted to expand D, you could do
 * |[ glk_window_set_arrangement(o2, winmethod_Above | winmethod_Fixed, 5, NULL); ]|
 * That expands D to five rows. Note that, since O2's key window is already set 
 * to D, it is not necessary to provide the @keywin argument; you can pass %NULL
 * to mean <quote>leave the key window unchanged.</quote>
 * 
 * If you do change the key window of a pair window, the new key window 
 * <emphasis>must</emphasis> be a descendant of that pair window. In the current
 * example, you could change O2's key window to be A, but not B. The key window
 * also cannot be a pair window itself.
 * 
 * |[ glk_window_set_arrangement(o2, winmethod_Below | winmethod_Fixed, 3, NULL); ]|
 * This changes the constraint to be on the <emphasis>lower</emphasis> child of 
 * O2, which is A. The key window is still D; so A would then be three rows high
 * as measured in D's font, and D would get the rest of O2's space. That may not
 * be what you want. To set A to be three rows high as measured in A's font, you
 * would do
 * |[ glk_window_set_arrangement(o2, winmethod_Below | winmethod_Fixed, 3, a); ]|
 * 
 * Or you could change O2 to a proportional split:
 * |[ glk_window_set_arrangement(o2, winmethod_Below | winmethod_Proportional, 30, NULL); ]|
 * or
 * |[ glk_window_set_arrangement(o2, winmethod_Above | winmethod_Proportional, 70, NULL); ]|
 * These do exactly the same thing, since 30&percnt; above is the same as 
 * 70&percnt; below. You don't need to specify a key window with a proportional
 * split, so the @keywin argument is %NULL. (You could actually specify either A
 * or D as the key window, but it wouldn't affect the result.)
 * 
 * Whatever constraint you set, glk_window_get_size() will tell you the actual 
 * window size you got.
 * 
 * Note that you can resize windows, but you can't flip or rotate them. You 
 * can't move A above D, or change O2 to a vertical split where A is left or 
 * right of D. 
 * <note><para>
 *   To get this effect you could close one of the windows, and re-split the 
 *   other one with glk_window_open().
 * </para></note>
 */
void
glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin)
{
	VALID_WINDOW(win, return);
	VALID_WINDOW_OR_NULL(keywin, return);
	g_return_if_fail(win->type == wintype_Pair);
	if(keywin)
	{
		g_return_if_fail(keywin->type != wintype_Pair);
		g_return_if_fail(g_node_is_ancestor(win->window_node, keywin->window_node));
	}
	g_return_if_fail(method == (method & (winmethod_DirMask | winmethod_DivisionMask)));
	g_return_if_fail(!(((method & winmethod_DivisionMask) == winmethod_Proportional) && size > 100));

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	win->split_method = method;
	win->constraint_size = size;
	if(keywin)
		win->key_window = keywin;

	/* Tell GTK to rearrange the windows */
	gdk_threads_enter();
	g_mutex_lock(glk_data->arrange_lock);
	glk_data->needs_rearrange = TRUE;
	glk_data->ignore_next_arrange_event = TRUE;
	g_mutex_unlock(glk_data->arrange_lock);
	gtk_widget_queue_resize(GTK_WIDGET(glk_data->self));
	gdk_threads_leave();
}

/**
 * glk_window_get_arrangement:
 * @win: a pair window.
 * @methodptr: return location for the constraint flags of @win, or %NULL.
 * @sizeptr: return location for the constraint size of @win, or %NULL.
 * @keywinptr: return location for the key window of @win, or %NULL.
 *
 * Queries the constraint of a given pair window.
 */
void
glk_window_get_arrangement(winid_t win, glui32 *methodptr, glui32 *sizeptr, winid_t *keywinptr)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type == wintype_Pair);
	
	if(methodptr)
		*methodptr = win->split_method;
	if(sizeptr)
		*sizeptr = win->constraint_size;
	if(keywinptr)
		*keywinptr = win->key_window;
}

/**
 * glk_window_move_cursor:
 * @win: A text grid window.
 * @xpos: Horizontal cursor position.
 * @ypos: Vertical cursor position.
 * 
 * Sets the cursor position. If you move the cursor right past the end of a 
 * line, it wraps; the next character which is printed will appear at the
 * beginning of the next line.
 * 
 * If you move the cursor below the last line, or when the cursor reaches the
 * end of the last line, it goes <quote>off the screen</quote> and further
 * output has no effect. You must call glk_window_move_cursor() or
 * glk_window_clear() to move the cursor back into the visible region.
 * 
 * <note><para>
 *  Note that the arguments of glk_window_move_cursor() are <type>unsigned 
 *  int</type>s. This is okay, since there are no negative positions. If you try
 *  to pass a negative value, Glk will interpret it as a huge positive value,
 *  and it will wrap or go off the last line.
 * </para></note>
 *
 * <note><para>
 *  Also note that the output cursor is not necessarily visible. In particular,
 *  when you are requesting line or character input in a grid window, you cannot
 *  rely on the cursor position to prompt the player where input is indicated.
 *  You should print some character prompt at that spot &mdash; a 
 *  <quote>&gt;</quote> character, for example.
 * </para></note>
 */
void
glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type == wintype_TextGrid);

	flush_window_buffer(win);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	/* Wait until the window's size is current */
	g_mutex_lock(glk_data->arrange_lock);
	if(glk_data->needs_rearrange)
		g_cond_wait(glk_data->rearranged, glk_data->arrange_lock);
	g_mutex_unlock(glk_data->arrange_lock);

	/* Don't do anything if the window is shrunk down to nothing */
	if(win->width == 0 || win->height == 0)
		return;
	
	/* Calculate actual position if cursor is moved past the right edge */
	if(xpos >= win->width)
	{
	    ypos += xpos / win->width;
	    xpos %= win->width;
	}

	/* Go to the end if the cursor is moved off the bottom edge */
	if(ypos >= win->height)
	{
	    xpos = win->width - 1;
	    ypos = win->height - 1;
	}
	
	gdk_threads_enter();
	
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter newpos;
	/* There must actually be a character at xpos, or the following function will choke */
	gtk_text_buffer_get_iter_at_line_offset(buffer, &newpos, ypos, xpos);
	gtk_text_buffer_move_mark_by_name(buffer, "cursor_position", &newpos);
	
	gdk_threads_leave();
}
