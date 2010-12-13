#ifndef ABORT_H
#define ABORT_H

#include <glib.h>

G_GNUC_INTERNAL void check_for_abort(void);
G_GNUC_INTERNAL void shutdown_glk_pre(void);
G_GNUC_INTERNAL void shutdown_glk_post(void);

#endif

