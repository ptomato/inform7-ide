/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef FIND_REPLACE_H
#define FIND_REPLACE_H

#include <gnome.h>

#include "extension.h"
#include "story.h"

enum {
    FIND_CONTAINS,
    FIND_STARTS_WITH,
    FIND_FULL_WORD
};

void
after_find_dialog_realize              (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_find_text_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_find_next_clicked                   (GtkButton       *button,
                                        Story           *thestory);

void
on_xfind_next_clicked                  (GtkButton       *button,
                                        Extension       *ext);

void
on_find_previous_clicked               (GtkButton       *button,
                                        Story           *thestory);

void
on_xfind_previous_clicked              (GtkButton       *button,
                                        Extension       *ext);

void
on_find_replace_find_clicked           (GtkButton       *button,
                                        Story           *thestory);

void
on_xfind_replace_find_clicked          (GtkButton       *button,
                                        Extension       *ext);

void
on_find_replace_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_xfind_replace_clicked               (GtkButton       *button,
                                        gpointer         user_data);
                                        
void
on_find_replace_all_clicked            (GtkButton       *button,
                                        Story           *thestory);
                                        
void
on_xfind_replace_all_clicked           (GtkButton       *button,
                                        Extension       *ext);
  
#endif
