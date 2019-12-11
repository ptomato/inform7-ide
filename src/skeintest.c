/*
 * Compile me with:
gcc -o skeintest skeintest.c skein.c node.c `pkg-config --cflags --libs gtk+-2.0 gdk-pixbuf-2.0 libxml-2.0 cairo goocanvas`
 */

#include <gtk/gtk.h>
#include <goocanvas.h>
#include "skein.h"
#include "skein-view.h"

#define _(x) x

typedef struct {
	GtkWidget *view;
	I7Skein *skein;
} Widgets;

typedef struct {
	I7Node *node;
	I7Skein *skein;
	I7SkeinView *view;
} CallbackData;

static void
play_to_here(I7Skein *skein, I7Node *node)
{
	gchar *text = i7_node_get_command(node);
	g_print("Playing to: %s\n", text);
	g_free(text);
}

static void
hspacing_changed(GtkRange *hspacing, Widgets *w)
{
	g_object_set(w->skein, "horizontal-spacing", gtk_range_get_value(hspacing), NULL);
}

static void
vspacing_changed(GtkRange *vspacing, Widgets *w)
{
	g_object_set(w->skein, "vertical-spacing", gtk_range_get_value(vspacing), NULL);
}

static void
on_node_activate(I7Skein *skein, I7Node *node, I7SkeinView *view)
{
	play_to_here(skein, node);
}

static void
on_differs_badge_activate(I7Skein *skein, I7Node *node, I7SkeinView *view)
{
	g_printerr("Differs badge activated! Of node: ");
	play_to_here(skein, node);
}

static void
on_popup_menu_play_to_here(GtkMenuItem *menuitem, CallbackData *data)
{
	play_to_here(data->skein, data->node);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_edit(GtkMenuItem *menuitem, CallbackData *data)
{
	i7_skein_view_edit_node(data->view, data->node);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_edit_label(GtkMenuItem *menuitem, CallbackData *data)
{
	i7_skein_view_edit_label(data->view, data->node);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_lock(GtkMenuItem *menuitem, CallbackData *data)
{
	if(i7_node_get_locked(data->node))
		i7_skein_unlock(data->skein, data->node);
	else
		i7_skein_lock(data->skein, data->node);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_lock_thread(GtkMenuItem *menuitem, CallbackData *data)
{
	if(i7_node_get_locked(data->node))
		i7_skein_unlock(data->skein, i7_skein_get_thread_top(data->skein, data->node));
	else
		i7_skein_lock(data->skein, i7_skein_get_thread_bottom(data->skein, data->node));
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_new_thread(GtkMenuItem *menuitem, CallbackData *data)
{
	I7Node *newnode = i7_skein_add_new(data->skein, data->node);
	i7_skein_view_edit_node(data->view, newnode);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_insert_knot(GtkMenuItem *menuitem, CallbackData *data)
{
	I7Node *newnode = i7_skein_add_new_parent(data->skein, data->node);
	i7_skein_view_edit_node(data->view, newnode);
	g_slice_free(CallbackData, data);
}

static gboolean
can_remove(I7Skein *skein, I7Node *node/*, I7Story *story */)
{
	if(/*game_is_running(story) &&*/ i7_skein_is_node_in_current_thread(skein, node)) {
		GtkWidget *dialog = gtk_message_dialog_new_with_markup(NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			_("<b>Unable to delete the active branch in the skein</b>"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			_("It is not possible to delete the branch of the skein that leads "
			"to the current position in the game. To delete this branch, either"
			" stop or restart the game."));
		/* GTK bug #632511 */
		gtk_widget_show(dialog);
		gtk_window_present(GTK_WINDOW(dialog));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return FALSE;
	}

	if(!i7_node_get_locked(node))
		return TRUE;

	GtkWidget *dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		 _("This knot has been locked to preserve it. Do you really want to "
		 "delete it? (This cannot be undone.)"));
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
	int response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return (response == GTK_RESPONSE_YES);
}

static void
on_popup_menu_delete(GtkMenuItem *menuitem, CallbackData *data)
{
	if(can_remove(data->skein, data->node))
		i7_skein_remove_single(data->skein, data->node);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_delete_below(GtkMenuItem *menuitem, CallbackData *data)
{
	if(can_remove(data->skein, data->node))
		i7_skein_remove_all(data->skein, data->node);
	g_slice_free(CallbackData, data);
}

static void
on_popup_menu_delete_thread(GtkMenuItem *menuitem, CallbackData *data)
{
	I7Node *topnode = i7_skein_get_thread_top(data->skein, data->node);
	if(can_remove(data->skein, topnode))
		i7_skein_remove_all(data->skein, topnode);
	g_slice_free(CallbackData, data);
}

static void
on_node_popup(I7Skein *skein, I7Node *node, I7SkeinView *view)
{
	CallbackData *data = g_slice_new0(CallbackData);
	data->node = node;
	data->skein = skein;
	data->view = view;

	GtkWidget *menu = gtk_menu_new();

	GtkWidget *menuitem = gtk_menu_item_new_with_label("Play to Here");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_play_to_here), data);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	if(!i7_node_is_root(node)) {
		menuitem = gtk_menu_item_new_with_label("Edit");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_edit), data);

		menuitem = gtk_menu_item_new_with_label(i7_node_has_label(node)? "Edit Label" : "Add Label");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_edit_label), data);
	}

	menuitem = gtk_menu_item_new_with_label("Show in Transcript");
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	if(!i7_node_is_root(node)) {
		menuitem = gtk_menu_item_new_with_label(i7_node_get_locked(node)? "Unlock" : "Lock");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_lock), data);

		menuitem = gtk_menu_item_new_with_label(i7_node_get_locked(node)? "Unlock This Thread" : "Lock This Thread");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_lock_thread), data);
	}

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("New Thread");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_new_thread), data);

	if(!i7_node_is_root(node)) {
		menuitem = gtk_menu_item_new_with_label("Insert Knot");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_insert_knot), data);

		menuitem = gtk_menu_item_new_with_label("Delete");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_delete), data);

		menuitem = gtk_menu_item_new_with_label("Delete all Below");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_delete_below), data);

		menuitem = gtk_menu_item_new_with_label("Delete all in Thread");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(on_popup_menu_delete_thread), data);
	}

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Save Transcript to here...");
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);

	GdkRectangle rect;
	if (i7_node_get_command_coordinates(node, &rect, GOO_CANVAS(view)))
		gtk_menu_popup_at_rect(GTK_MENU(menu), gtk_widget_get_window(GTK_WIDGET(view)), &rect,
			GDK_GRAVITY_NORTH_EAST, GDK_GRAVITY_NORTH_WEST, NULL);
	else
		gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

int
main(int argc, char **argv)
{
	GError *error = NULL;

	gtk_init(&argc, &argv);

	/* Create widgets */
	Widgets *w = g_slice_new0(Widgets);
	w->skein = i7_skein_new();
	w->view = i7_skein_view_new();
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	GtkWidget *hspacing = gtk_hscale_new_with_range(20.0, 100.0, 1.0);
	GtkWidget *vspacing = gtk_hscale_new_with_range(20.0, 100.0, 1.0);

	/* Configure widgets */
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 600);
	gtk_range_set_value(GTK_RANGE(hspacing), 40.0);
	gtk_range_set_value(GTK_RANGE(vspacing), 75.0);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(hspacing, "value-changed", G_CALLBACK(hspacing_changed), w);
	g_signal_connect(vspacing, "value-changed", G_CALLBACK(vspacing_changed), w);
	g_object_set(w->skein,
		"vertical-spacing", 75.0,
		NULL);
	g_signal_connect(w->skein, "node-activate", G_CALLBACK(on_node_activate), w->view);
	g_signal_connect(w->skein, "node-menu-popup", G_CALLBACK(on_node_popup), w->view);
	g_signal_connect(w->skein, "differs-badge-activate", G_CALLBACK(on_differs_badge_activate), w->view);

	/* Assemble widgets */
	gtk_box_pack_start(GTK_BOX(hbox), hspacing, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vspacing, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(scroll), w->view);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Read skein */
	if(!i7_skein_load(w->skein, "/home/fliep/Documents/writing/if/Blood.inform/Skein.skein", &error)) {
		g_printerr("Error: %s\n", error->message);
		g_error_free(error);
	}

	i7_skein_view_set_skein(I7_SKEIN_VIEW(w->view), w->skein);

	/* Display widgets */
	gtk_widget_show_all(window);
	gtk_main();

	g_object_unref(w->skein);
	g_slice_free(Widgets, w);

	return 0;
}
