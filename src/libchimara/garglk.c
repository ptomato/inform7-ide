#include <glib/gi18n.h>
#include <libchimara/glk.h>
#include "chimara-glk-private.h"
#include "stream.h"
#include "fileref.h"

extern GPrivate *glk_data_key;

/**
 * garglk_fileref_get_name:
 * @fref: A file reference.
 *
 * Gets the actual disk filename that @fref refers to, in the platform's
 * native filename encoding. The string is owned by @fref and must not be
 * changed or freed.
 *
 * Returns: a string in filename encoding.
 */
char * 
garglk_fileref_get_name(frefid_t fref)
{
	VALID_FILEREF(fref, return NULL);
	return fref->filename;
}

/**
 * garglk_set_program_name:
 * @name: Name of the Glk program that is running.
 *
 * This function is used to let the library know the name of the currently
 * running Glk program, in case it wants to display this information somewhere
 * &mdash; for example, in the title bar of a window. A typical use of this
 * function would be:
 * |[ garglk_set_program_name("SuperGlkFrotz 0.1"); ]|
 */
void 
garglk_set_program_name(const char *name)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	glk_data->program_name = g_strdup(name);
	g_object_notify(G_OBJECT(glk_data->self), "program-name");
}

/**
 * garglk_set_program_info:
 * @info: Information about the Glk program that is running.
 *
 * This function is used to provide the library with additional information
 * about the currently running Glk program, in case it wants to display this
 * information somewhere &mdash; for example, in an About box. A typical use of
 * this function would be:
 * |[ 
 * garglk_set_program_info("SuperGlkFrotz, version 0.1\n"
 *     "Original Frotz by Stefan Jokisch\n"
 *     "Unix port by Jim Dunleavy and David Griffith\n"
 *     "Glk port by Tor Andersson\n"
 *     "Animation, networking, and evil AI by Sven Metcalfe");
 * ]|
 */
void 
garglk_set_program_info(const char *info)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	glk_data->program_info = g_strdup(info);
	g_object_notify(G_OBJECT(glk_data->self), "program-info");
}

/**
 * garglk_set_story_name:
 * @name: Name of the story that the Glk program is currently interpreting.
 *
 * If the Glk program running is an interactive fiction interpreter, then this
 * function can be used to let the library know the name of the story currently
 * loaded in the interpreter, in case it wants to display this information
 * anywhere &mdash; for example, in the title bar of a window. A typical use of
 * this function would be:
 * |[ garglk_set_story_name("Lighan Ses Lion, el Zarf"); ]|
 */
void 
garglk_set_story_name(const char *name)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	glk_data->story_name = g_strdup(name);
	g_object_notify(G_OBJECT(glk_data->self), "story-name");
}

/**
 * garglk_set_line_terminators:
 * @win: A window.
 * @keycodes: An array of <code>keycode_</code> constants.
 * @numkeycodes: The length of @keycodes.
 *
 * Amends the current line input request of @win to include terminating key
 * codes. Any of the specified key codes will terminate the line input request 
 * (without printing a newline). 
 *
 * Usually, in the event structure returned from a line input request, @val2 is
 * zero, but if garglk_set_line_terminators() has been called during that input
 * request, @val2 will be filled in with the key code that terminated the input
 * request.
 *
 * This function only applies to one input request; any subsequent line input
 * requests on that window are treated normally.
 *
 * If @numkeycodes is zero, then any previous call to 
 * garglk_set_line_terminators() is cancelled and the input request is treated
 * normally.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void 
garglk_set_line_terminators(winid_t win, const glui32 *keycodes, glui32 numkeycodes)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);
	
	if(win->input_request_type != INPUT_REQUEST_LINE && win->input_request_type != INPUT_REQUEST_LINE_UNICODE) {
		ILLEGAL(_("Tried to set the line terminators on a window without a line input request."));
		return;
	}

	WARNING(_("Not implemented"));
}

/**
 * garglk_unput_string:
 * @str: a null-terminated string.
 *
 * Removes @str from the end of the current stream, if indeed it is there. The
 * stream's write count is decreased accordingly, and the stream's echo stream
 * is also modified, if it has one.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void 
garglk_unput_string(char *str)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);

	WARNING(_("Not implemented"));
}

/**
 * garglk_unput_string_uni:
 * @str: a zero-terminated array of Unicode code points.
 *
 * Like garglk_unput_string(), but for Unicode streams.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void 
garglk_unput_string_uni(glui32 *str)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	
	WARNING(_("Not implemented"));
}

/**
 * garglk_set_zcolors:
 * @fg: one of the <code>zcolor_</code> constants.
 * @bg: one of the <code>zcolor_</code> constants.
 *
 * Glk works with styles, not specific colors. This is not quite compatible with
 * the Z-machine, so this Glk extension implements Z-machine style colors.
 *
 * This function changes the foreground color of the current stream to @fg and 
 * the background color to @bg.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void 
garglk_set_zcolors(glui32 fg, glui32 bg)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	g_return_if_fail(glk_data->current_stream->window != NULL);
	
	WARNING(_("Not implemented"));
}

static void
apply_reverse_color(GtkTextTag *tag, gpointer data)
{
	g_object_set_data( G_OBJECT(tag), "reverse_color", data );
}

/**
 * garglk_set_reversevideo:
 * @reverse: nonzero for reverse colors, zero for normal colors.
 *
 * If @reverse is not zero, uses the foreground color of the current stream as
 * its background and vice versa. If @reverse is zero, changes the colors of the
 * current stream back to normal.
 */
void 
garglk_set_reversevideo(glui32 reverse)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	g_return_if_fail(glk_data->current_stream->window != NULL);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(glk_data->current_stream->window->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	gtk_text_tag_table_foreach( tags, apply_reverse_color, GINT_TO_POINTER(reverse) );
}
