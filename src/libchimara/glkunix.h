#ifndef __GLKUNIX_H__
#define __GLKUNIX_H__

#include <glib.h>
#include <libchimara/glkstart.h>

G_GNUC_INTERNAL gboolean parse_command_line(glkunix_argumentlist_t glkunix_arguments[], int argc, char *argv[], glkunix_startup_t *data);

#endif