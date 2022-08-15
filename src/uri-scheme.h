/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2022 Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <webkit2/webkit2.h>

G_BEGIN_DECLS

void i7_uri_scheme_register(WebKitWebContext *web_context);

G_END_DECLS
