#ifndef __MAGIC_H__
#define __MAGIC_H__

#include <gtk/gtk.h>
#include "glk.h"

#define MAGIC_FREE     0x46524545 /* "FREE" */
#define MAGIC_NULL     0x4E554C4C /* "NULL" */
#define MAGIC_WINDOW   0x57494E44 /* "WIND" */
#define MAGIC_STREAM   0x53545245 /* "STRE" */
#define MAGIC_FILEREF  0x46494C45 /* "FILE" */
#define MAGIC_SCHANNEL 0x53434841 /* "SCHA" */

G_GNUC_INTERNAL gboolean magic_is_valid_or_null(const glui32 goodmagic, const glui32 realmagic, const gchar *function);
G_GNUC_INTERNAL gboolean magic_is_valid(const void *obj, const glui32 goodmagic, const glui32 realmagic, const gchar *function);

#define VALID_MAGIC(obj, goodmagic, die) \
	if( !magic_is_valid(obj, goodmagic, obj->magic, G_STRFUNC) ) die
#define VALID_MAGIC_OR_NULL(obj, goodmagic, die) \
	if( !magic_is_valid_or_null(goodmagic, obj? obj->magic : MAGIC_NULL, G_STRFUNC) ) die

#define VALID_WINDOW(o, d)           VALID_MAGIC(o, MAGIC_WINDOW, d)
#define VALID_WINDOW_OR_NULL(o, d)   VALID_MAGIC_OR_NULL(o, MAGIC_WINDOW, d)
#define VALID_STREAM(o, d)           VALID_MAGIC(o, MAGIC_STREAM, d)
#define VALID_STREAM_OR_NULL(o, d)   VALID_MAGIC_OR_NULL(o, MAGIC_STREAM, d)
#define VALID_FILEREF(o, d)          VALID_MAGIC(o, MAGIC_FILEREF, d)
#define VALID_FILEREF_OR_NULL(o, d)  VALID_MAGIC_OR_NULL(o, MAGIC_FILEREF, d)
#define VALID_SCHANNEL(o, d)         VALID_MAGIC(o, MAGIC_SCHANNEL, d)
#define VALID_SCHANNEL_OR_NULL(o, d) VALID_MAGIC_OR_NULL(o, MAGIC_SCHANNEL, d)

/* This works with string variables as well as literal strings */
#define ILLEGAL(str)               g_critical("%s: %s", G_STRFUNC, str)
/* This only works with literal strings */
#define ILLEGAL_PARAM(str, param)  g_critical("%s: " str, G_STRFUNC, param)

#define WARNING(msg)         g_warning("%s: %s", G_STRFUNC, msg)
#define WARNING_S(msg, str)  g_warning("%s: %s: %s", G_STRFUNC, msg, str)
#define IO_WARNING(msg, str, errmsg) \
	g_warning("%s: %s \"%s\": %s", G_STRFUNC, msg, str, errmsg)

#endif /* __MAGIC_H__ */
