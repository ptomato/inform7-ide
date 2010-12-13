#ifndef CHARSET_H
#define CHARSET_H

#include <glib.h>

#define PLACEHOLDER '?'
#define PLACEHOLDER_STRING "?"
/* Our placeholder character is '?'; other options are possible, like printing "0x7F" or something */

G_GNUC_INTERNAL gchar *convert_latin1_to_utf8(const gchar *s, const gsize len);
G_GNUC_INTERNAL gchar *convert_latin1_to_ucs4be_string(const gchar *s, const gsize len);
G_GNUC_INTERNAL gchar *convert_utf8_to_latin1(const gchar *s, gsize *bytes_written);
G_GNUC_INTERNAL gunichar *convert_utf8_to_ucs4(const gchar *s, glong *items_written);
G_GNUC_INTERNAL gchar *convert_ucs4_to_utf8(const gunichar *buf, const glong len);
G_GNUC_INTERNAL gchar *convert_ucs4_to_latin1_binary(const gunichar *buf, const glong len);
G_GNUC_INTERNAL gchar *convert_ucs4_to_ucs4be_string(const gunichar *buf, const glong len);

#endif /* CHARSET_H */
