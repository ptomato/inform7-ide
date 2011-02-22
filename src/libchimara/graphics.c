#include "graphics.h"
#include "chimara-glk-private.h"
#include "magic.h"

#define BUFFER_SIZE (1024)

extern GPrivate *glk_data_key;
void on_size_prepared(GdkPixbufLoader *loader, gint width, gint height, struct image_info *info);
void on_pixbuf_closed(GdkPixbufLoader *loader, gpointer data);
glui32 draw_image_common(winid_t win, GdkPixbuf *pixbuf, glsi32 val1, glsi32 val2);

static gboolean image_loaded;
static gboolean size_determined;

static struct image_info*
load_image_from_blorb(giblorb_result_t resource, glui32 image, gint width, gint height)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GError *pixbuf_error = NULL;
	guchar *buffer;

	struct image_info *info = g_new0(struct image_info, 1);
	info->resource_number = image;

	/* Load the resource */
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
	g_signal_connect( loader, "size-prepared", G_CALLBACK(on_size_prepared), info ); 
	g_signal_connect( loader, "closed", G_CALLBACK(on_pixbuf_closed), NULL ); 

	/* Scale image if necessary */
	if(width > 0 && height > 0) {
		gdk_pixbuf_loader_set_size(loader, width, height);
		info->scaled = TRUE;
	}

	glk_stream_set_position(glk_data->resource_file, resource.data.startpos, seekmode_Start);
	buffer = g_malloc( BUFFER_SIZE * sizeof(guchar) );

	guint32 total_read = 0;
	image_loaded = FALSE;
	while(total_read < resource.length && !image_loaded) {
		guint32 num_read = glk_get_buffer_stream(glk_data->resource_file, (char *) buffer, BUFFER_SIZE);

		if( !gdk_pixbuf_loader_write(loader, buffer, MIN(BUFFER_SIZE, num_read), &pixbuf_error) ) {
			WARNING_S("Cannot read image", pixbuf_error->message);
			giblorb_unload_chunk(glk_data->resource_map, image);
			gdk_pixbuf_loader_close(loader, &pixbuf_error);
			g_free(buffer);
			return NULL;
		}

		total_read += num_read;
	}
	gdk_pixbuf_loader_close(loader, &pixbuf_error);
	giblorb_unload_chunk(glk_data->resource_map, resource.chunknum);
	g_free(buffer);

	/* Wait for the PixbufLoader to finish loading the image */
	g_mutex_lock(glk_data->resource_lock);
	while(!image_loaded) {
		g_cond_wait(glk_data->resource_loaded, glk_data->resource_lock);
	}
	g_mutex_unlock(glk_data->resource_lock);

	info->pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	gdk_pixbuf_ref(info->pixbuf);

	g_object_unref(loader);
	return info;
}

static struct image_info *
load_image_from_file(const gchar *filename, glui32 image, gint width, gint height)
{
	GError *err = NULL;
	
	struct image_info *info = g_new0(struct image_info, 1);
	info->resource_number = image;
	
	if(width > 0 && height > 0) {
		info->scaled = TRUE;
		info->pixbuf = gdk_pixbuf_new_from_file_at_size(filename, width, height, &err);
	} else {
		info->pixbuf = gdk_pixbuf_new_from_file(filename, &err);
	}
	if(!info->pixbuf) {
		IO_WARNING("Error loading resource from alternative location", filename, err->message);
		g_error_free(err);
		g_free(info);
		return NULL;
	}
	gdk_pixbuf_ref(info->pixbuf);

	return info;
}

static struct image_info*
load_image_in_cache(glui32 image, gint width, gint height)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	struct image_info *info = NULL;
	
	/* Lookup the proper resource */
	if(!glk_data->resource_map) {
		if(!glk_data->resource_load_callback) {
			WARNING("No resource map has been loaded yet.");
			return NULL;
		}
		gchar *filename = glk_data->resource_load_callback(CHIMARA_RESOURCE_IMAGE, image, glk_data->resource_load_callback_data);
		if(!filename) {
			WARNING("Error loading resource from alternative location");
			return NULL;
		}
		info = load_image_from_file(filename, image, width, height);
		g_free(filename);
	} else {
		giblorb_result_t resource;
		giblorb_err_t blorb_error = giblorb_load_resource(glk_data->resource_map, giblorb_method_FilePos, &resource, giblorb_ID_Pict, image);
		if(blorb_error != giblorb_err_None) {
			WARNING_S( "Error loading resource", giblorb_get_error_message(blorb_error) );
			return NULL;
		}
		info = load_image_from_blorb(resource, image, width, height);
	}

	/* Store the image in the cache */
	gdk_threads_enter();

	if( g_slist_length(glk_data->image_cache) >= IMAGE_CACHE_MAX_NUM ) {
		struct image_info *head = (struct image_info*) glk_data->image_cache->data;
		gdk_pixbuf_unref(head->pixbuf);
		g_free(head);
		glk_data->image_cache = g_slist_remove_link(glk_data->image_cache, glk_data->image_cache);
	}
	info->width = gdk_pixbuf_get_width(info->pixbuf);
	info->height = gdk_pixbuf_get_height(info->pixbuf);
	glk_data->image_cache = g_slist_prepend(glk_data->image_cache, info);

	gdk_threads_leave();
	return info;
}

void
on_size_prepared(GdkPixbufLoader *loader, gint width, gint height, struct image_info *info)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	g_mutex_lock(glk_data->resource_lock);
	info->width = width;
	info->height = height;
	size_determined = TRUE;
	g_cond_broadcast(glk_data->resource_info_available);
	g_mutex_unlock(glk_data->resource_lock);
}

void
on_pixbuf_closed(GdkPixbufLoader *loader, gpointer data)
{
	gdk_threads_enter();

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	g_mutex_lock(glk_data->resource_lock);
	image_loaded = TRUE;
	g_cond_broadcast(glk_data->resource_loaded);
	g_mutex_unlock(glk_data->resource_lock);

	gdk_threads_leave();
}


void
clear_image_cache(struct image_info *data, gpointer user_data)
{
	gdk_pixbuf_unref(data->pixbuf);
	g_free(data);
}

static struct image_info*
image_cache_find(struct image_info* to_find)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GSList *link = glk_data->image_cache;

	gdk_threads_enter();

	/* Empty cache */
	if(link == NULL) {
		gdk_threads_leave();
		return NULL;
	}

	/* Iterate over the cache to find the correct image and size */
	struct image_info *match = NULL;
	do {
		struct image_info *info = (struct image_info*) link->data;
		if(info->resource_number == to_find->resource_number) {
			/* Check size: are we looking for a scaled version or the original one? */
			if(to_find->scaled) {

				if(info->width == to_find->width && info->height == to_find->height) {
					/* Prescaled image found */
					gdk_threads_leave();
					return info;
				}
				else if(info->width >= to_find->width && info->height >= to_find->height) {
					/* Larger image found, needs to be scaled down in order to work */
					gdk_threads_leave();
					match = info;
				}
			} else {
				if(!info->scaled) {
					gdk_threads_leave();
					return info; /* Found a match */
				}
			}
		}
	} while( (link = g_slist_next(link)) );

	gdk_threads_leave();

	return match;
}

/**
 * glk_image_get_info:
 * @image: An image resource number.
 * @width: Pointer to a location at which to store the image's width.
 * @height: Pointer to a location at which to store the image's height.
 *
 * This gets information about the image resource with the given identifier. It
 * returns %TRUE if there is such an image, and %FALSE if not. You can also pass
 * pointers to width and height variables; if the image exists, the variables
 * will be filled in with the width and height of the image, in pixels. (You can
 * pass %NULL for either width or height if you don't care about that 
 * information.)
 * 
 * <note><para>
 *   You should always use this function to measure the size of images when you 
 *   are creating your display. Do this even if you created the images, and you
 *   know how big they <quote>should</quote> be. This is because images may be 
 *   scaled in translating from one platform to another, or even from one 
 *   machine to another. A Glk library might display all images larger than 
 *   their original size, because of screen resolution or player preference. 
 *   Images will be scaled proportionally, but you still need to call 
 *   glk_image_get_info() to determine their absolute size.
 * </para></note>
 * 
 * Returns: %TRUE if @image is a valid identifier, %FALSE if not.
 */
glui32
glk_image_get_info(glui32 image, glui32 *width, glui32 *height)
{
	struct image_info *to_find = g_new0(struct image_info, 1);
	struct image_info *found;
	to_find->resource_number = image;
	to_find->scaled = FALSE; /* we want the original image size */

	if( !(found = image_cache_find(to_find)) ) {
		found = load_image_in_cache(image, 0, 0);
		if(found == NULL)
			return FALSE;
	}

	if(width != NULL)
		*width = found->width;
	if(height != NULL)
		*height = found->height;
	return TRUE;
}

/**
 * glk_image_draw:
 * @win: A graphics or text buffer window.
 * @image: An image resource number.
 * @val1: The x coordinate at which to draw the image (if @win is a graphics 
 * window); or, an <link linkend="chimara-imagealign-InlineUp">image 
 * alignment</link> constant (if @win is a text window).
 * @val2: The y coordinate at which to draw the image (if @win is a graphics
 * window); this parameter is ignored if @win is a text buffer window.
 *
 * This draws the given image resource in the given window. The position of the
 * image is given by @val1 and @val2, but their meaning varies depending on what
 * kind of window you are drawing in. See <link 
 * linkend="chimara-Graphics-in-Graphics-Windows">Graphics in Graphics 
 * Windows</link> and <link linkend="Graphics-in-Text-Buffer-Windows">Graphics 
 * in Text Buffer Windows</link>.
 * 
 * This function returns a flag indicating whether the drawing operation 
 * succeeded.
 * <note><para>
 *   A %FALSE result can occur for many reasons. The image data might be 
 *   corrupted; the library may not have enough memory to operate; there may be 
 *   no image with the given identifier; the window might not support image 
 *   display; and so on.
 * </para></note>
 *
 * Returns: %TRUE if the operation succeeded, %FALSE if not.
 */
glui32
glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2)
{
	VALID_WINDOW(win, return FALSE);
	g_return_val_if_fail(win->type == wintype_Graphics || win->type == wintype_TextBuffer, FALSE);

	struct image_info *to_find = g_new0(struct image_info, 1);
	struct image_info *info;

	/* Lookup the proper resource */
	to_find->resource_number = image;
	to_find->scaled = FALSE; /* we want the original image size */

	if( !(info = image_cache_find(to_find)) ) {
		info = load_image_in_cache(image, 0, 0);
		if(info == NULL)
			return FALSE;
	}

	return draw_image_common(win, info->pixbuf, val1, val2);
}

/**
 * glk_image_draw_scaled:
 * @win: A graphics or text buffer window.
 * @image: An image resource number.
 * @val1: The x coordinate at which to draw the image (if @win is a graphics 
 * window); or, an <link linkend="chimara-imagealign-InlineUp">image 
 * alignment</link> constant (if @win is a text window).
 * @val2: The y coordinate at which to draw the image (if @win is a graphics
 * window); this parameter is ignored if @win is a text buffer window.
 * @width: The width of the image.
 * @height: The height of the image.
 *
 * This is similar to glk_image_draw(), but it scales the image to the given 
 * @width and @height, instead of using the image's standard size. (You can 
 * measure the standard size with glk_image_get_info().)
 * 
 * If @width or @height is zero, nothing is drawn. Since those arguments are 
 * unsigned integers, they cannot be negative. If you pass in a negative number,
 * it will be interpreted as a very large positive number, which is almost 
 * certain to end badly. 
 *
 * Returns: %TRUE if the operation succeeded, %FALSE otherwise.
 */
glui32
glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
	VALID_WINDOW(win, return FALSE);
	g_return_val_if_fail(win->type == wintype_Graphics || win->type == wintype_TextBuffer, FALSE);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	struct image_info *to_find = g_new0(struct image_info, 1);
	struct image_info *info;
	struct image_info *scaled_info;

	/* Lookup the proper resource */
	to_find->resource_number = image;
	to_find->scaled = TRUE; /* any image size equal or larger than requested will do */

	if( !(info = image_cache_find(to_find)) ) {
		info = load_image_in_cache(image, width, height);
		if(info == NULL)
			return FALSE;
	}

	/* Scale the image if necessary */
	if(info->width != width || info->height != height) {
		GdkPixbuf *scaled = gdk_pixbuf_scale_simple(info->pixbuf, width, height, GDK_INTERP_BILINEAR);

		/* Add the scaled image into the image cache */
		scaled_info = g_new0(struct image_info, 1);
		scaled_info->resource_number = info->resource_number;
		scaled_info->width = gdk_pixbuf_get_width(scaled);
		scaled_info->height = gdk_pixbuf_get_width(scaled);
		scaled_info->pixbuf = scaled;
		scaled_info->scaled = TRUE;
		glk_data->image_cache = g_slist_prepend(glk_data->image_cache, scaled_info);

		/* Continue working with the scaled version */
		info = scaled_info;
	}

	return draw_image_common(win, info->pixbuf, val1, val2);
}

/* Internal function: draws a pixbuf to a graphics window of text buffer */
glui32
draw_image_common(winid_t win, GdkPixbuf *pixbuf, glsi32 val1, glsi32 val2)
{
	switch(win->type) {
	case wintype_Graphics:
	{
		GdkPixmap *canvas;

		gdk_threads_enter();

		gtk_image_get_pixmap( GTK_IMAGE(win->widget), &canvas, NULL );
		if(canvas == NULL) {
			WARNING("Could not get pixmap");
			return FALSE;
		}

		gdk_draw_pixbuf( GDK_DRAWABLE(canvas), NULL, pixbuf, 0, 0, val1, val2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0 );

		/* Update the screen */
		gtk_widget_queue_draw(win->widget);

		gdk_threads_leave();
	}
		break;

	case wintype_TextBuffer:
	{
		flush_window_buffer(win);

		gdk_threads_enter();

		GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		GtkTextIter end, start;
		gtk_text_buffer_get_end_iter(buffer, &end);

		gtk_text_buffer_insert_pixbuf(buffer, &end, pixbuf);
		start = end;
		gtk_text_iter_forward_char(&end);

		gint height = 0;
		switch(val1) {
		case imagealign_InlineDown:
			height -= win->unit_height;
			break;
		case imagealign_InlineCenter:
			height = -win->unit_height / 2;
			break;
		case imagealign_InlineUp:
		default:
			height = 0;
		}

		if(height != 0) {
			GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, "rise", PANGO_SCALE * (-height), NULL);
			gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
		}

		gdk_threads_leave();
	}
		break;
	}
	return TRUE;
}

/**
 * glk_window_set_background_color:
 * @win: A graphics window.
 * @color: a 32-bit RGB color value.
 *
 * This sets the window's background color. It does not change what is currently
 * displayed; it only affects subsequent clears and resizes. The initial 
 * background color of each window is white.
 * 
 * Colors are encoded in a 32-bit value: the top 8 bits must be zero, the next 8
 * bits are the red value, the next 8 bits are the green value, and the bottom 8
 * bits are the blue value. Color values range from 0 to 255.
 * <note><para>
 *   So <code>0x00000000</code> is black, <code>0x00FFFFFF</code> is white, and 
 *   <code>0x00FF0000</code> is bright red.
 * </para></note>
 * 
 * <note><para>
 *   This function may only be used with graphics windows. To set background 
 *   colors in a text window, use text styles with color hints; see <link 
 *   linkend="chimara-Styles">Styles</link>.
 * </para></note>
 */
void
glk_window_set_background_color(winid_t win, glui32 color) 
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type == wintype_Graphics);
	
	win->background_color = color;
}

/**
 * glk_window_fill_rect:
 * @win: A graphics window.
 * @color: A 32-bit RGB color value, see glk_window_set_background_color().
 * @left: The x coordinate of the top left corner of the rectangle.
 * @top: The y coordinate of the top left corner of the rectangle.
 * @width: The width of the rectangle.
 * @height: The height of the rectangle.
 *
 * This fills the given rectangle with the given color. It is legitimate for
 * part of the rectangle to fall outside the window. If width or height is zero,
 * nothing is drawn. 
 */
void
glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type == wintype_Graphics);

	gdk_threads_enter();

	GdkPixmap *map;
	gtk_image_get_pixmap( GTK_IMAGE(win->widget), &map, NULL );

	GdkGC *gc = gdk_gc_new(map);
	GdkColor gdkcolor;
	glkcolor_to_gdkcolor(color, &gdkcolor);
	gdk_gc_set_rgb_fg_color(gc, &gdkcolor);
	gdk_draw_rectangle( GDK_DRAWABLE(map), gc, TRUE, left, top, width, height);
	gtk_widget_queue_draw(win->widget);
	g_object_unref(gc);

	gdk_threads_leave();
}

/**
 * glk_window_erase_rect:
 * @win: A graphics window.
 * @left: The x coordinate of the top left corner of the rectangle.
 * @top: The y coordinate of the top left corner of the rectangle.
 * @width: The width of the rectangle.
 * @height: The height of the rectangle.
 *
 * This fills the given rectangle with the window's background color.
 * 
 * You can also fill an entire graphics window with its background color by 
 * calling glk_window_clear().
 */
void
glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
	glk_window_fill_rect(win, win->background_color, left, top, width, height);
}

/**
 * glk_window_flow_break:
 * @win: A window.
 *
 * You may wish to <quote>break</quote> the stream of text down below the 
 * current margin image. Since lines of text can be in any font and size, you 
 * cannot do this by counting newlines. Instead, use this function.
 *
 * If the current point in the text is indented around a margin-aligned image, 
 * this acts like the correct number of newlines to start a new line below the 
 * image. (If there are several margin-aligned images, it goes below all of 
 * them.) If the current point is not beside a margin-aligned image, this call 
 * has no effect.
 *
 * When a text buffer window is resized, a flow-break behaves cleverly; it may 
 * become active or inactive as necessary. You can consider this function to 
 * insert an invisible mark in the text stream. The mark works out how many 
 * newlines it needs to be whenever the text is formatted for display.
 * 
 * An example of the use of glk_window_flow_break(): If you display a 
 * left-margin image at the start of every line, they can stack up in a strange 
 * diagonal way that eventually squeezes all the text off the screen. 
 * <note><para>
 *   If you can't picture this, draw some diagrams. Make the margin images more 
 *   than one line tall, so that each line starts already indented around the 
 *   last image.
 * </para></note>
 * To avoid this problem, call glk_window_flow_break() immediately before 
 * glk_image_draw() for every margin-aligned image.
 * 
 * In all windows other than text buffers, glk_window_flow_break() has no 
 * effect. 
 *
 * <warning><para>
 *   This function is not implemented yet.
 * </para></warning>
 */
void glk_window_flow_break(winid_t win)
{
	VALID_WINDOW(win, return);
}

/*** Called when the graphics window is resized. Resize the backing pixmap if necessary ***/
void
on_graphics_size_allocate(GtkWidget *widget, GtkAllocation *allocation, winid_t win)
{ 
	GdkPixmap *oldmap;
	gtk_image_get_pixmap( GTK_IMAGE(widget), &oldmap, NULL );
	gint oldwidth = 0;
	gint oldheight = 0;
 
	/* Determine whether a pixmap exists with the correct size */
	gboolean needs_resize = FALSE;
	if(oldmap == NULL)
		needs_resize = TRUE;
	else {
		gdk_drawable_get_size( GDK_DRAWABLE(oldmap), &oldwidth, &oldheight );
		if(oldwidth != allocation->width || oldheight != allocation->height)
			needs_resize = TRUE;
	}

	if(needs_resize) {
		/* Create a new pixmap */
		GdkPixmap *newmap = gdk_pixmap_new(widget->window, allocation->width, allocation->height, -1);
		gdk_draw_rectangle( GDK_DRAWABLE(newmap), widget->style->white_gc, TRUE, 0, 0, allocation->width, allocation->height);

		/* Copy the contents of the old pixmap */
		if(oldmap != NULL)
			gdk_draw_drawable( GDK_DRAWABLE(newmap), widget->style->white_gc, GDK_DRAWABLE(oldmap), 0, 0, 0, 0, oldwidth, oldheight);
		
		/* Use the new pixmap */
		gtk_image_set_from_pixmap( GTK_IMAGE(widget), newmap, NULL );
		g_object_unref(newmap);
	}
}

