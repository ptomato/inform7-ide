#ifndef INPUT_H
#define INPUT_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "window.h"
#include "event.h"
#include "strio.h"

G_GNUC_INTERNAL gboolean on_shutdown_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL gboolean on_char_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL gboolean on_line_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, winid_t win);
G_GNUC_INTERNAL void on_input_entry_activate(GtkEntry *input_entry, winid_t win);
G_GNUC_INTERNAL gboolean on_input_entry_key_press_event(GtkEntry *input_entry, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void on_input_entry_changed(GtkEditable *editable, winid_t win);
G_GNUC_INTERNAL glui32 keyval_to_glk_keycode(guint keyval, gboolean unicode);
G_GNUC_INTERNAL void force_char_input_from_queue(winid_t win, event_t *event);
G_GNUC_INTERNAL void force_line_input_from_queue(winid_t win, event_t *event);

#endif
