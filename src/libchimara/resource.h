#ifndef RESOURCE_H
#define RESOURCE_H

#include <glib.h>
#include "glk.h"
#include "gi_blorb.h"
#include "chimara-glk-private.h"
#include "magic.h"

void giblorb_print_contents(giblorb_map_t *map);
gchar* giblorb_get_error_message(giblorb_err_t err);

#endif
