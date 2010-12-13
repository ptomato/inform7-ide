#include "charset.h"
#include "magic.h"
#include <glib.h>

/* Internal function: change illegal (control) characters in a string to a
placeholder character. Must free returned string afterwards. */
static gchar *
remove_latin1_control_characters(const unsigned char *s, const gsize len)
{
	/* If len == 0, then return an empty string, not NULL */
	if(len == 0)
		return g_strdup("");
			
	gchar *retval = g_new0(gchar, len);
	int i;
	for(i = 0; i < len; i++)
		if( (s[i] < 32 && s[i] != 10) || (s[i] >= 127 && s[i] <= 159) )
			retval[i] = PLACEHOLDER;
		else
			retval[i] = s[i];
	return retval;
}

/* Internal function: convert a Latin-1 string to a UTF-8 string, replacing
Latin-1 control characters by a placeholder first. The UTF-8 string must be
freed afterwards. Returns NULL on error. */
gchar *
convert_latin1_to_utf8(const gchar *s, const gsize len)
{
	GError *error = NULL;
	gchar *canonical = remove_latin1_control_characters( (unsigned char *)s, len);
	gchar *retval = g_convert(canonical, len, "UTF-8", "ISO-8859-1", NULL, NULL, &error);
	g_free(canonical);
	
	if(retval == NULL)
		IO_WARNING("Error during latin1->utf8 conversion of string", s, error->message);
	
	return retval;
}

/* Internal function: convert a Latin-1 string to a four-byte-per-character
big-endian string of gchars. The string must be freed afterwards. */
gchar *
convert_latin1_to_ucs4be_string(const gchar *s, const gsize len)
{
	/* "UCS-4BE" is also a conversion type in g_convert()... but this may be more efficient */
	gchar *retval = g_new0(gchar, len * 4);
	int i;
	for(i = 0; i < len; i++)
		retval[i * 4 + 3] = s[i];
	return retval;
}

/* Internal function: convert a null-terminated UTF-8 string to a 
null-terminated Latin-1 string, replacing characters that cannot be represented 
in Latin-1 by a placeholder. If bytes_written is not NULL it will be filled with
the number of bytes returned, not counting the NULL terminator. The returned
string must be freed afterwards. Returns NULL on error. */
gchar *
convert_utf8_to_latin1(const gchar *s, gsize *bytes_written)
{
	GError *error = NULL;
	gchar *retval = g_convert_with_fallback(s, -1, "ISO-8859-1", "UTF-8", PLACEHOLDER_STRING, NULL, bytes_written, &error);
	
	if(retval == NULL)
		IO_WARNING("Error during utf8->latin1 conversion of string", s, error->message);

	return retval;
}

/* Internal function: convert a null-terminated UTF-8 string to a
null-terminated UCS4 string. If items_written is not NULL it will be filled with
the number of code points returned, not counting the terminator. The returned
string must be freed afterwards. Returns NULL on error. */
gunichar *
convert_utf8_to_ucs4(const gchar *s, glong *items_written)
{
	gunichar *retval = g_utf8_to_ucs4_fast(s, -1, items_written);
	
	if(retval == NULL)
		WARNING_S("Error during utf8->unicode conversion of string", s);

	return retval;
}

/* Internal function: Convert a Unicode buffer to a null-terminated UTF-8 
string. The returned string must be freed afterwards. Returns NULL on error. */
gchar *
convert_ucs4_to_utf8(const gunichar *buf, const glong len)
{
	GError *error = NULL;
	gchar *retval = g_ucs4_to_utf8(buf, len, NULL, NULL, &error);
		
	if(retval == NULL)
		WARNING_S("Error during unicode->utf8 conversion", error->message);
		
	return retval;
}

/* Internal function: Convert a Unicode buffer to a Latin-1 string. Do not do
any character processing, just return values > 255 as the placeholder character.
The returned string must be freed afterwards.*/
gchar *
convert_ucs4_to_latin1_binary(const gunichar *buf, const glong len)
{
	gchar *retval = g_new0(gchar, len);
	int foo;
	for(foo = 0; foo < len; foo++)
		retval[foo] = (buf[foo] > 255)? PLACEHOLDER : buf[foo];
	return retval;
}

/* Internal function: convert a Unicode buffer to a four-byte-per-character
big-endian string of gchars. The string must be freed afterwards. */
gchar *
convert_ucs4_to_ucs4be_string(const gunichar *buf, const glong len)
{
	gchar *retval = g_new0(gchar, len * 4);
	int i;
	for(i = 0; i < len; i++)
	{
		retval[i * 4]     = buf[i] >> 24       ;
		retval[i * 4 + 1] = buf[i] >> 16 & 0xFF;
		retval[i * 4 + 2] = buf[i] >> 8  & 0xFF;
		retval[i * 4 + 3] = buf[i]       & 0xFF;
	}
	return retval;
}
