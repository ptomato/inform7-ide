/*  Copyright (C) 2014, 2015 P. F. Chimento
 *  This file is part of GNOME Inform 7.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourceview.h>
#include "text-buffer-excerpt.h"

#define EXCERPT_1 "quick brown fox jumps over the lazy dog"
#define EXCERPT_2 "lazy dog jumps under the quick brown fox"
#define SAMPLE_TEXT_1 "The " EXCERPT_1 "."
#define SAMPLE_TEXT_2 "The " EXCERPT_2 "."
#define SAMPLE_TEXT_3 "Friends, Romans, and countrymen, lend me your ears!"

typedef struct {
	GtkSourceBuffer *obuffer;  /* original */
	I7TextBufferExcerpt *buffer;  /* excerpt */
	GtkWidget *window;  /* not created in most tests */
	GtkWidget *view;  /* not created in most tests */
} Fixture;

static void
update_gui(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}

static void
setup(Fixture *fixture)
{
	fixture->obuffer = gtk_source_buffer_new(NULL);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(fixture->obuffer), SAMPLE_TEXT_1, -1);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(fixture->obuffer), FALSE);
	fixture->buffer = i7_text_buffer_excerpt_new(fixture->obuffer);
	update_gui();
}

static void
teardown(Fixture *fixture)
{
	g_object_unref(fixture->buffer);
	g_object_unref(fixture->obuffer);
	if (fixture->window)
		gtk_widget_destroy(fixture->window);
}

static void
test_create_and_destroy(Fixture *fixture)
{
	g_assert_nonnull(fixture->buffer);
	g_assert_true(I7_IS_TEXT_BUFFER_EXCERPT(fixture->buffer));
}

static void
it_references_original_buffer(void)
{
	GtkSourceBuffer *obuffer = gtk_source_buffer_new(NULL);
	I7TextBufferExcerpt *buffer = i7_text_buffer_excerpt_new(obuffer);
	g_object_unref(obuffer);
	g_assert_true(GTK_IS_SOURCE_BUFFER(obuffer));
	g_object_unref(buffer);
	g_assert_false(GTK_IS_SOURCE_BUFFER(obuffer));
}

static void
it_does_not_leak_original_buffer_from_getter(void)
{
	GtkSourceBuffer *obuffer = gtk_source_buffer_new(NULL);
	I7TextBufferExcerpt *buffer = i7_text_buffer_excerpt_new(obuffer);
	GtkSourceBuffer *test_buffer = i7_text_buffer_excerpt_get_original_buffer(buffer);

	g_assert_true(test_buffer == obuffer);
	g_object_unref(obuffer);
	g_object_unref(buffer);
	g_assert_false(GTK_IS_SOURCE_BUFFER(test_buffer));
}

/* Helper function */
static void
assert_buffer_text(gpointer buffer, char *expected_text)
{
	update_gui();
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
	char *text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, TRUE);
	g_assert_cmpstr(text, ==, expected_text);
	g_free(text);
}

static void
it_excerpts_whole_original_buffer_by_default(Fixture *fixture)
{
	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1);
}

static void
it_reflects_changes_in_original_buffer(Fixture *fixture)
{
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(fixture->obuffer), SAMPLE_TEXT_3, -1);
	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_3);
}

static void
it_propagates_changes_from_excerpt_to_original_buffer(Fixture *fixture)
{
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(fixture->buffer), SAMPLE_TEXT_3, -1);
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_3);
}

/* Helper function */
static void
get_smaller_range(Fixture *fixture, GtkTextIter *start, GtkTextIter *end)
{
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(fixture->obuffer), start, 4);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(fixture->obuffer), end);
	gtk_text_iter_backward_char(end);
}

/* Helper function */
static void
set_smaller_excerpt(Fixture *fixture)
{
	GtkTextIter start, end;
	get_smaller_range(fixture, &start, &end);
	i7_text_buffer_excerpt_set_range(fixture->buffer, &start, &end);
	update_gui();
}

static void
it_excerpts_a_smaller_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);
	assert_buffer_text(fixture->buffer, EXCERPT_1);
}

static void
it_reflects_changes_in_original_buffer_in_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	GtkTextIter start, end;
	get_smaller_range(fixture, &start, &end);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(fixture->obuffer), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(fixture->obuffer), &start, EXCERPT_2, -1);

	assert_buffer_text(fixture->buffer, EXCERPT_2);
}

static void
it_propagates_changes_to_original_buffer_in_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(fixture->buffer), EXCERPT_2, -1);
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_2);
}

static void
it_ignores_changes_to_original_buffer_outside_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(fixture->obuffer), &start);
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(fixture->obuffer), &end, 3);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(fixture->obuffer), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(fixture->obuffer), &start, "That", -1);

	assert_buffer_text(fixture->buffer, EXCERPT_1);
}

static void
it_goes_back_to_excerpting_the_whole_buffer(Fixture *fixture)
{
	set_smaller_excerpt(fixture);
	i7_text_buffer_excerpt_clear_range(fixture->buffer);
	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1);
}

static void
it_correctly_processes_changes_across_excerpt_boundaries(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(fixture->obuffer), &start, 2);
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(fixture->obuffer), &end, 10);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(fixture->obuffer), &start, &end);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(fixture->obuffer), &start);
	gtk_text_iter_backward_chars(&start, 5);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(fixture->obuffer), &end);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(fixture->obuffer), &start, &end);

	assert_buffer_text(fixture->buffer, "brown fox jumps over the lazy");
}

static void
it_gets_the_excerpted_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	GtkTextIter start, end, test_start, test_end;
	get_smaller_range(fixture, &start, &end);
	i7_text_buffer_excerpt_get_range(fixture->buffer, &test_start, &test_end);

	g_assert_true(gtk_text_iter_equal(&start, &test_start));
	g_assert_true(gtk_text_iter_equal(&end, &test_end));
}

static void
it_does_not_modify_either_buffer_at_start(Fixture *fixture)
{
	g_assert_false(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(fixture->buffer)));
	g_assert_false(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(fixture->obuffer)));
}

static void
it_does_not_modify_either_buffer_by_excerpting(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	g_assert_false(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(fixture->buffer)));
	g_assert_false(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(fixture->obuffer)));
}

static void
it_correctly_reports_iters_in_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	GtkTextIter start, end;
	get_smaller_range(fixture, &start, &end);
	g_assert_true(i7_text_buffer_excerpt_iters_in_range(fixture->buffer, &start, &end));
}

static void
it_correctly_reports_iters_not_in_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);

	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(fixture->obuffer), &start, &end);
	g_assert_false(i7_text_buffer_excerpt_iters_in_range(fixture->buffer, &start, &end));
}

static void
it_converts_original_iters_to_excerpt_iters(Fixture *fixture)
{
	set_smaller_excerpt(fixture);
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(fixture->obuffer), &start, 10);
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(fixture->obuffer), &end, 15);

	g_assert_true(i7_text_buffer_excerpt_iters_from_buffer_iters(fixture->buffer, &start, &end));
	g_assert_true(gtk_text_iter_get_buffer(&start) == GTK_TEXT_BUFFER(fixture->buffer));
	g_assert_true(gtk_text_iter_get_buffer(&end) == GTK_TEXT_BUFFER(fixture->buffer));
	g_assert_cmpint(gtk_text_iter_get_offset(&start), ==, 6);
	g_assert_cmpint(gtk_text_iter_get_offset(&end), ==, 11);
}

static void
it_coerces_excerpt_iters_if_not_in_range(Fixture *fixture)
{
	set_smaller_excerpt(fixture);
	GtkTextIter start, end, excerpt_start, excerpt_end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(fixture->obuffer), &start, &end);
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(fixture->buffer), &excerpt_start, &excerpt_end);

	g_assert_false(i7_text_buffer_excerpt_iters_from_buffer_iters(fixture->buffer, &start, &end));
	g_assert_true(gtk_text_iter_get_buffer(&start) == GTK_TEXT_BUFFER(fixture->buffer));
	g_assert_true(gtk_text_iter_get_buffer(&end) == GTK_TEXT_BUFFER(fixture->buffer));
	g_assert_true(gtk_text_iter_equal(&start, &excerpt_start));
	g_assert_true(gtk_text_iter_equal(&end, &excerpt_end));
}

static void
set_up_window_and_view(Fixture *fixture)
{
	fixture->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	fixture->view = gtk_source_view_new_with_buffer(GTK_SOURCE_BUFFER(fixture->buffer));
	gtk_container_add(GTK_CONTAINER(fixture->window), fixture->view);
	gtk_widget_show_all(fixture->window);
}

static void
it_undoes_action_on_original_buffer(Fixture *fixture)
{
	set_up_window_and_view(fixture);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);

	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1 "r");
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_1 "r");
	g_assert_true(gtk_source_buffer_can_undo(fixture->obuffer));

	gtk_source_buffer_undo(fixture->obuffer);

	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1);
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_1);
}

static void
it_groups_actions_for_undo_like_original_buffer_would(Fixture *fixture)
{
	set_up_window_and_view(fixture);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);

	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1 "rrrr");
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_1 "rrrr");
	g_assert_true(gtk_source_buffer_can_undo(fixture->obuffer));

	gtk_source_buffer_undo(fixture->obuffer);

	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1);
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_1);
}

static void
it_undoes_correctly_on_ctrl_z_keypress(Fixture *fixture)
{
	set_up_window_and_view(fixture);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	gtk_test_widget_send_key(fixture->view, GDK_KEY_r, 0);
	update_gui();
	gtk_test_widget_send_key(fixture->view, GDK_KEY_z, GDK_CONTROL_MASK);

	assert_buffer_text(fixture->buffer, SAMPLE_TEXT_1);
	assert_buffer_text(fixture->obuffer, SAMPLE_TEXT_1);
}

void
add_text_buffer_excerpt_tests(void)
{
#define ADD_TEST(path, func) \
	g_test_add(("/text-buffer-excerpt/" path), Fixture, NULL, \
		(void (*)(Fixture *, gconstpointer))setup, \
		(void (*)(Fixture *, gconstpointer))(func), \
		(void (*)(Fixture *, gconstpointer))teardown)

	ADD_TEST("create-and-destroy", test_create_and_destroy);
	g_test_add_func("/text-buffer-excerpt/references-original-buffer", it_references_original_buffer);
	g_test_add_func("/text-buffer-excerpt/does-not-leak-original-buffer-from-getter", it_does_not_leak_original_buffer_from_getter);
	ADD_TEST("excerpts-whole-original-buffer-by-default", it_excerpts_whole_original_buffer_by_default);
	ADD_TEST("reflects-changes-in-original-buffer", it_reflects_changes_in_original_buffer);
	ADD_TEST("propagates-changes-from-excerpt-to-original-buffer", it_propagates_changes_from_excerpt_to_original_buffer);
	ADD_TEST("excerpts-a-smaller-range", it_excerpts_a_smaller_range);
	ADD_TEST("reflects-changes-in-original-buffer-in-range", it_reflects_changes_in_original_buffer_in_range);
	ADD_TEST("propagates-changes-to-original-buffer-in-range", it_propagates_changes_to_original_buffer_in_range);
	ADD_TEST("ignores-changes-to-original-buffer-outside-range", it_ignores_changes_to_original_buffer_outside_range);
	ADD_TEST("goes-back-to-excerpting-the-whole-buffer", it_goes_back_to_excerpting_the_whole_buffer);
	ADD_TEST("correctly-processes-changes-across-excerpt-boundaries", it_correctly_processes_changes_across_excerpt_boundaries);
	ADD_TEST("gets-the-excerpted-range", it_gets_the_excerpted_range);
	ADD_TEST("does-not-modify-either-buffer-at-start", it_does_not_modify_either_buffer_at_start);
	ADD_TEST("does-not-modify-either-buffer-by-excerpting", it_does_not_modify_either_buffer_by_excerpting);
	ADD_TEST("correctly-reports-iters-in-range", it_correctly_reports_iters_in_range);
	ADD_TEST("correctly-reports-iters-not-in-range", it_correctly_reports_iters_not_in_range);
	ADD_TEST("converts-original-iters-to-excerpt-iters", it_converts_original_iters_to_excerpt_iters);
	ADD_TEST("coerces-excerpt-iters-if-not-in-range", it_coerces_excerpt_iters_if_not_in_range);
	ADD_TEST("undoes-action-on-original-buffer", it_undoes_action_on_original_buffer);
	ADD_TEST("groups-actions-for-undo-like-original-buffer-would", it_groups_actions_for_undo_like_original_buffer_would);
	ADD_TEST("undoes-correctly-on-ctrl-z-keypress", it_undoes_correctly_on_ctrl_z_keypress);

#undef ADD_TEST
}
