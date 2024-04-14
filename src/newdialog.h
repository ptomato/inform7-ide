/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <gtk/gtk.h>

typedef enum _I7NewDialogAction {
	I7_NEW_DIALOG_CHOOSE_TYPE,
	I7_NEW_DIALOG_CREATE_STORY,
	I7_NEW_DIALOG_CREATE_EXTENSION,
} I7NewDialogAction;

GtkWidget *create_new_dialog(I7NewDialogAction action);
