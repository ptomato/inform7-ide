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

#include "appwindow.h"
#include "datafile.h"
#include "history.h"
#include "html.h"
#include "story.h"

/* Names of the button widgets, for speed */
static gchar *left_back_buttons[TAB_LAST + 1] = {
    "source_back_l", "errors_back_l", "index_back_l", "skein_back_l",
    "transcript_back_l", "game_back_l", "docs_back_l", "settings_back_l"
};
static gchar *right_back_buttons[TAB_LAST + 1] = {
    "source_back_r", "errors_back_r", "index_back_r", "skein_back_r",
    "transcript_back_r", "game_back_r", "docs_back_r", "settings_back_r"
};
static gchar *left_forward_buttons[TAB_LAST + 1] = {
    "source_forward_l", "errors_forward_l", "index_forward_l",
    "skein_forward_l", "transcript_forward_l", "game_forward_l",
    "docs_forward_l", "settings_forward_l"
};
static gchar *right_forward_buttons[TAB_LAST + 1] = {
    "source_forward_r", "errors_forward_r", "index_forward_r",
    "skein_forward_r", "transcript_forward_r", "game_forward_r",
    "docs_forward_r", "settings_forward_r"
};

/* Run through the list of buttons and change their 'sensitive' status */
static void
set_buttons_sensitive(GtkWidget *window, gchar **buttons, gboolean sensitive)
{
    int foo;
    for(foo = TAB_FIRST; foo <= TAB_LAST; foo++)
        gtk_widget_set_sensitive(lookup_widget(window, buttons[foo]),sensitive);
}
    
/* Empty the list of pages to go forward to */
static void
empty_forward_queue(Story *thestory, int side)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    History *foo;
    while((foo = g_queue_pop_head(thestory->forward[side])) != NULL) {
        if(foo->page)
            g_free(foo->page);
        g_free(foo);
    }
    
    set_buttons_sensitive(thestory->window, side == LEFT? left_forward_buttons
                          : right_forward_buttons, FALSE);
}

/* Go to the location pointed to by the "current" history pointer */
static void
goto_current(Story *thestory, int side)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    history_block_handlers(thestory, side);
    if(thestory->current[side]->tab == TAB_ERRORS)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(
                                      thestory->window, side == LEFT?
                                      "errors_notebook_l" :
                                      "errors_notebook_r")),
                                      thestory->current[side]->subtab);
    else if(thestory->current[side]->tab == TAB_INDEX)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(
                                      thestory->window, side == LEFT?
                                      "index_notebook_l" : "index_notebook_r")),
                                      thestory->current[side]->subtab);
    else if(thestory->current[side]->tab == TAB_DOCUMENTATION) {
        if(thestory->current[side]->page)
            html_load_file(GTK_HTML(lookup_widget(thestory->window,
                           side == LEFT? "docs_l" : "docs_r")),
                           thestory->current[side]->page);
        if(thestory->current[side]->anchor)
            gtk_html_jump_to_anchor(GTK_HTML(lookup_widget(thestory->window,
                                    side == LEFT? "docs_l" : "docs_r")),
                                    thestory->current[side]->anchor);
    }
    
    gtk_notebook_set_current_page(GTK_NOTEBOOK(lookup_widget(thestory->window,
                                  side == LEFT? "notebook_l" : "notebook_r")),
                                  thestory->current[side]->tab);
    history_unblock_handlers(thestory, side);
}

/* Set a tab as the current location on one side of the main window, and push
the previous location into the back queue */
void
history_push_tab(Story *thestory, int side, int tab)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    if(g_queue_is_empty(thestory->back[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_back_buttons : right_back_buttons, TRUE);
    
    g_queue_push_head(thestory->back[side], thestory->current[side]);
    thestory->current[side] = g_new0(History, 1);
    thestory->current[side]->tab = tab;
    
    empty_forward_queue(thestory, side);
}

/* Set a combination of tab and subtab as the current location on one side of
the main window, and push the previous location into the back queue */
void
history_push_subtab(Story *thestory, int side, int tab, int subtab)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    if(g_queue_is_empty(thestory->back[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_back_buttons : right_back_buttons, TRUE);
    
    g_queue_push_head(thestory->back[side], thestory->current[side]);
    thestory->current[side] = g_new0(History, 1);
    thestory->current[side]->tab = tab;
    thestory->current[side]->subtab = subtab;
    
    empty_forward_queue(thestory, side);
}

/* Set a documentation page as the current location on one side of the main
window, and push the previous location into the back queue */
void
history_push_docpage(Story *thestory, int side, const gchar *page)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    if(g_queue_is_empty(thestory->back[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_back_buttons : right_back_buttons, TRUE);
    
    g_queue_push_head(thestory->back[side], thestory->current[side]);
    thestory->current[side] = g_new0(History, 1);
    thestory->current[side]->tab = TAB_DOCUMENTATION;
    gchar *anchor = NULL;
    if((anchor = strchr(page, '#')))
        *(anchor++) = '\0';
    thestory->current[side]->page = g_strdup(page);
    thestory->current[side]->anchor = g_strdup(anchor);
    
    empty_forward_queue(thestory, side);
}

/* User clicked on the "back" button */
void
go_back(Story *thestory, int side)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    if(g_queue_is_empty(thestory->forward[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_forward_buttons : right_forward_buttons,
                              TRUE);
    
    g_queue_push_head(thestory->forward[side], thestory->current[side]);
    thestory->current[side] = g_queue_pop_head(thestory->back[side]);
    goto_current(thestory, side);
    
    if(g_queue_is_empty(thestory->back[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_back_buttons : right_back_buttons, FALSE);
}

/* User clicked on the "forward" button */
void
go_forward(Story *thestory, int side)
{
    g_return_if_fail(side == LEFT || side == RIGHT);
    
    if(g_queue_is_empty(thestory->back[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_back_buttons : right_back_buttons, TRUE);
    
    g_queue_push_head(thestory->back[side], thestory->current[side]);
    thestory->current[side] = g_queue_pop_head(thestory->forward[side]);
    goto_current(thestory, side);
    
    if(g_queue_is_empty(thestory->forward[side]))
        set_buttons_sensitive(thestory->window, side == LEFT?
                              left_forward_buttons : right_forward_buttons,
                              FALSE);
}

/* Temporarily block the handlers that push stuff to the "back" queue */
void
history_block_handlers(Story *thestory, int side)
{
    g_signal_handler_block(lookup_widget(thestory->window, side == LEFT?
                           "notebook_l" : "notebook_r"),
                           thestory->handler_notebook_change[side]);
    g_signal_handler_block(lookup_widget(thestory->window, side == LEFT?
                           "errors_notebook_l" : "errors_notebook_r"),
                           thestory->handler_errors_change[side]);
    g_signal_handler_block(lookup_widget(thestory->window, side == LEFT?
                           "index_notebook_l" : "index_notebook_r"),
                           thestory->handler_index_change[side]);
}

/* Unblock the handlers that push stuff to the "back" queue */
void
history_unblock_handlers(Story *thestory, int side)
{
    g_signal_handler_unblock(lookup_widget(thestory->window, side == LEFT?
                             "notebook_l" : "notebook_r"),
                             thestory->handler_notebook_change[side]);
    g_signal_handler_unblock(lookup_widget(thestory->window, side == LEFT?
                             "errors_notebook_l" : "errors_notebook_r"),
                             thestory->handler_errors_change[side]);
    g_signal_handler_unblock(lookup_widget(thestory->window, side == LEFT?
                             "index_notebook_l" : "index_notebook_r"),
                             thestory->handler_index_change[side]);
}

/* Get the page last displayed in the documentation window 
Returns a newly-allocated string. */
gchar *
history_get_last_docpage(Story *thestory, int side)
{
    int foo;
    for(foo = 0; foo < g_queue_get_length(thestory->back[side]); foo++) {
        History *data = (History *)g_queue_peek_nth(thestory->back[side], foo);
        if(data->tab == TAB_DOCUMENTATION)
            return g_strdup(data->page);
    }
    return get_datafile_path_va("Documentation", "index.html", NULL);
}
