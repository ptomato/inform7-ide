#include <gtk/gtk.h>

#include "glk.h"
#include "abort.h"
#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "gi_blorb.h"
#include "window.h"

G_GNUC_INTERNAL GPrivate *glk_data_key = NULL;

/**
 * glk_exit:
 * 
 * If you want to shut down your program in the middle of your <function>
 * glk_main()</function> function, you can call glk_exit().
 *
 * This function does not return.
 *
 * If you print some text to a window and then shut down your program, you can
 * assume that the player will be able to read it. Most likely the Glk library
 * will give a <quote><computeroutput>Hit any key to 
 * exit</computeroutput></quote> prompt. (There are other possiblities, however.
 * A terminal-window version of Glk might simply exit and leave the last screen
 * state visible in the terminal window.)
 *
 * <note><para>
 * You should only shut down your program with glk_exit() or by returning from
 * your <function>glk_main()</function> function. If you call the ANSI 
 * <function>exit()</function> function, bad things may happen. Some versions of
 * the Glk library may be designed for multiple sessions, for example, and you
 * would be cutting off all the sessions instead of just yours. You would 
 * probably also prevent final text from being visible to the player.
 * </para></note>
 * <note><title>Chimara</title>
 * <para>
 * If there are any windows open at the time glk_exit() is called, then Chimara
 * will leave them open. This way, the final text remains visible. Note that bad  
 * things most definitely <emphasis>will</emphasis> happen if you use the ANSI
 * <function>exit()</function>.
 * </para></note>
 */
void
glk_exit(void)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	shutdown_glk_pre();
	
	/* Find the biggest text buffer window */
	winid_t win, largewin = NULL;
	glui32 largearea = 0;
	for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL)) {
		if(win->type == wintype_TextBuffer) {
			glui32 w, h;
			if(!largewin) {
				largewin = win;
				glk_window_get_size(largewin, &w, &h);
				largearea = w * h;
			} else {
				glk_window_get_size(win, &w, &h);
				if(w * h > largearea) {
					largewin = win;
					largearea = w * h;
				}
			}
		}
	}
	if(largewin) {
		glk_set_window(largewin);
		glk_set_style(style_Alert);
		glk_put_string("\n");
		glk_put_string(glk_data->final_message);
		glk_put_string("\n");
		flush_window_buffer(largewin);
	}
	
	g_mutex_lock(glk_data->shutdown_lock);
	for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL)) {
		if(win->type == wintype_TextGrid || win->type == wintype_TextBuffer)
			g_signal_handler_unblock(win->widget, win->shutdown_keypress_handler);
	}
	g_cond_wait(glk_data->shutdown_key_pressed, glk_data->shutdown_lock);
	g_mutex_unlock(glk_data->shutdown_lock);
	
	shutdown_glk_post();

	g_signal_emit_by_name(glk_data->self, "stopped");
	
	g_thread_exit(NULL);
}

/**
 * glk_tick:
 *
 * Carries out platform-dependent actions such as yielding time to the operating
 * system and checking for interrupts. glk_tick() should be called every so 
 * often when there is a long interval between calls of glk_select() or 
 * glk_select_poll(). This call is fast; in fact, on average, it does nothing at
 * all. So you can call it often.
 *
 * <note><para>
 *   In a virtual machine interpreter, once per opcode is appropriate. In a
 *   program with lots of computation, pick a comparable rate.
 * </para></note>
 * 
 * glk_tick() does not try to update the screen, or check for player input, or
 * any other interface task. For that, you should call glk_select() or 
 * glk_select_poll(). See <link linkend="chimara-Events">Events</link>.
 * 
 * <note>
 *   <para>Captious critics have pointed out that in the sample program
 *   <filename>model.c</filename>, I do not call glk_tick() at all. This is
 *   because <filename>model.c</filename> has no heavy loops. It does a bit of
 *   work for each command, and then cycles back to the top of the event loop.
 *   The glk_select() call, of course, blocks waiting for input, so it does all
 *   the yielding and interrupt-checking one could imagine.
 *   </para>
 *   <para>Basically, you must ensure there's some fixed upper bound on the
 *   amount of computation that can occur before a glk_tick() (or glk_select())
 *   occurs. In a VM interpreter, where the VM code might contain an infinite
 *   loop, this is critical. In a C program, you can often eyeball it.
 *   </para>
 *   <para>But the next version of <filename>model.c</filename> will have a
 *   glk_tick() in the ornate printing loop of 
 *   <function>verb_yada&lpar;&rpar;</function>. Just to make the point.
 *   </para>
 * </note>
 */
void
glk_tick()
{
	check_for_abort();
	
	/* Do one iteration of the main loop if there are any events */
	gdk_threads_enter();
	if(gtk_events_pending())
		gtk_main_iteration();
	gdk_threads_leave();
}

