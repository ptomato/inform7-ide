/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2010, 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include "config.h"

#include <gtk/gtk.h>

typedef enum _I7NewDialogAction {
	I7_NEW_DIALOG_CHOOSE_TYPE,
	I7_NEW_DIALOG_CREATE_STORY,
	I7_NEW_DIALOG_CREATE_EXTENSION,
} I7NewDialogAction;

GtkWidget *create_new_dialog(I7NewDialogAction action);

#endif
