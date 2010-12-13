#ifndef TIMER_H
#define TIMER_H

#include <glib.h>
#include "event.h"
#include "chimara-glk.h"
#include "chimara-glk-private.h"

G_GNUC_INTERNAL gboolean push_timer_event(ChimaraGlk *glk);

#endif
