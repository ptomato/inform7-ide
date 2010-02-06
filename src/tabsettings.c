/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <gnome.h>

#include "support.h"

#include "story.h"
#include "tabsettings.h"

void
on_z5_button_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(togglebutton));
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    if(active)
        thestory->story_format = FORMAT_Z5;

    /* When the one changes, change the other */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "z5_button_l")), active);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "z5_button_r")), active);
}

void
on_z6_button_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(togglebutton));
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    if(active)
        thestory->story_format = FORMAT_Z6;

    /* When the one changes, change the other */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "z6_button_l")), active);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "z6_button_r")), active);
}

void
on_z8_button_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(togglebutton));
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    if(active)
        thestory->story_format = FORMAT_Z8;

    /* When the one changes, change the other */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "z8_button_l")), active);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "z8_button_r")), active);
}

void
on_glulx_button_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(togglebutton));
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    if(active)
        thestory->story_format = FORMAT_GLULX;

    /* When the one changes, change the other */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "glulx_button_l")), active);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "glulx_button_r")), active);
}

void
on_blorb_button_toggled(GtkToggleButton *togglebutton, gpointer data)
{
    Story *thestory = get_story(GTK_WIDGET(togglebutton));
    thestory->make_blorb = gtk_toggle_button_get_active(togglebutton);

    /* When the one changes, change the other */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "blorb_button_l")), thestory->make_blorb);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "blorb_button_r")), thestory->make_blorb);
}

void
on_random_button_toggled(GtkToggleButton *togglebutton, gpointer data)
{
	Story *thestory = get_story(GTK_WIDGET(togglebutton));
	thestory->nobble_rng = gtk_toggle_button_get_active(togglebutton);

	/* When one changes, change the other */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "random_button_l")), 
	    thestory->nobble_rng);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
        GTK_WIDGET(togglebutton), "random_button_r")), 
	    thestory->nobble_rng);
}

/* Select all the right buttons according to the story settings */
void update_settings(Story *thestory) {
    switch(thestory->story_format) {
    case FORMAT_Z6:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
          thestory->window, "z6_button_l")), TRUE);
        break;
    case FORMAT_Z8:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
          thestory->window, "z8_button_l")), TRUE);
        break;
    case FORMAT_GLULX:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
          thestory->window, "glulx_button_l")), TRUE);
        break;
    case FORMAT_Z5:
    default:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(
          thestory->window, "z5_button_l")), TRUE);
    }
    gtk_toggle_button_set_active(
      GTK_TOGGLE_BUTTON(lookup_widget(thestory->window, "blorb_button_l")),
      thestory->make_blorb);
	gtk_toggle_button_set_active(
	    GTK_TOGGLE_BUTTON(lookup_widget(thestory->window, "random_button_l")),
	    thestory->nobble_rng);
    /* The callbacks ensure that we don't have to manually set the other ones */
}
