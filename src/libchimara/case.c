#include <gtk/gtk.h>
#include "glk.h"

/**
 * glk_char_to_lower:
 * @ch: A Latin-1 character.
 *
 * You can convert Latin-1 characters between upper and lower case with two Glk
 * utility functions, glk_char_to_lower() and glk_char_to_upper(). These have a
 * few advantages over the standard ANSI <function>tolower()</function> and
 * <function>toupper()</function> macros. They work for the entire Latin-1
 * character set, including accented letters; they behave consistently on all
 * platforms, since they're part of the Glk library; and they are safe for all
 * characters. That is, if you call glk_char_to_lower() on a lower-case
 * character, or a character which is not a letter, you'll get the argument
 * back unchanged.
 * 
 * The case-sensitive characters in Latin-1 are the ranges 0x41..0x5A,
 * 0xC0..0xD6, 0xD8..0xDE (upper case) and the ranges 0x61..0x7A, 0xE0..0xF6,
 * 0xF8..0xFE (lower case). These are arranged in parallel; so 
 * glk_char_to_lower() will add 0x20 to values in the upper-case ranges, and
 * glk_char_to_upper() will subtract 0x20 from values in the lower-case ranges. 
 *
 * Returns: A lowercase or non-letter Latin-1 character.
 */
unsigned char
glk_char_to_lower(unsigned char ch)
{
	if( (ch >= 0x41 && ch <= 0x5A) || (ch >= 0xC0 && ch <= 0xD6) || (ch >= 0xD8 && ch <= 0xDE) )
		return ch + 0x20;
	return ch;
}

/**
 * glk_char_to_upper:
 * @ch: A Latin-1 character.
 *
 * If @ch is a lowercase character in the Latin-1 character set, converts it to
 * uppercase. Otherwise, leaves it unchanged. See glk_char_to_lower().
 *
 * Returns: An uppercase or non-letter Latin-1 character.
 */
unsigned char
glk_char_to_upper(unsigned char ch)
{
	if( (ch >= 0x61 && ch <= 0x7A) || (ch >= 0xE0 && ch <= 0xF6) || (ch >= 0xF8 && ch <= 0xFE) )
		return ch - 0x20;
	return ch;
}

/**
 * glk_buffer_to_lower_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 *
 * Unicode character conversion is trickier, and must be applied to character
 * arrays, not single characters. These functions 
 * (glk_buffer_to_lower_case_uni(), glk_buffer_to_upper_case_uni(), and
 * glk_buffer_to_title_case_uni()) provide two length arguments because a
 * string of Unicode characters may expand when its case changes. The @len
 * argument is the available length of the buffer; @numchars is the number of
 * characters in the buffer initially. (So @numchars must be less than or equal
 * to @len. The contents of the buffer after @numchars do not affect the
 * operation.)
 *
 * The functions return the number of characters after conversion. If this is
 * greater than @len, the characters in the array will be safely truncated at
 * @len, but the true count will be returned. (The contents of the buffer after
 * the returned count are undefined.)
 *
 * The <code>lower_case</code> and <code>upper_case</code> functions do what
 * you'd expect: they convert every character in the buffer (the first @numchars
 * of them) to its upper or lower-case equivalent, if there is such a thing.
 * 
 * See the Unicode spec (chapter 3.13, chapter 4.2, etc) for the exact
 * definitions of upper, lower, and title-case mapping.
 * 
 * <note><para>
 *   Unicode has some strange case cases. For example, a combined character
 *   that looks like <quote>ss</quote> might properly be upper-cased into 
 *   <emphasis>two</emphasis> characters <quote>S</quote>. Title-casing is even
 *   stranger; <quote>ss</quote> (at the beginning of a word) might be 
 *   title-cased into a different combined character that looks like 
 *   <quote>Ss</quote>. The glk_buffer_to_title_case_uni() function is actually
 *   title-casing the first character of the buffer. If it makes a difference.
 * </para></note>
 *
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* GLib has a function that converts _one_ UCS-4 character to _one_
	lowercase UCS-4 character; so apparently we don't have to worry about the
	string length changing... */
	glui32 *ptr;
	for(ptr = buf; ptr < buf + numchars; ptr++)
		*ptr = g_unichar_tolower(*ptr);
	
	return numchars;
}

/**
 * glk_buffer_to_upper_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 *
 * Converts the first @numchars characters of @buf to their uppercase 
 * equivalents, if there is such a thing. See glk_buffer_to_lower_case_uni().
 *
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* GLib has a function that converts _one_ UCS-4 character to _one_
	uppercase UCS-4 character; so apparently we don't have to worry about the
	string length changing... */
	glui32 *ptr;
	for(ptr = buf; ptr < buf + numchars; ptr++)
		*ptr = g_unichar_toupper(*ptr);
	
	return numchars;
}

/**
 * glk_buffer_to_title_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 * @lowerrest: %TRUE if the rest of @buf should be lowercased, %FALSE 
 * otherwise.
 *
 * See glk_buffer_to_lower_case_uni(). The <code>title_case</code> function has
 * an additional (boolean) flag. Its basic function is to change the first
 * character of the buffer to upper-case, and leave the rest of the buffer
 * unchanged. If @lowerrest is true, it changes all the non-first characters to
 * lower-case (instead of leaving them alone.) 
 *
 * <note><para>
 *   Earlier drafts of this spec had a separate function which title-cased the
 *   first character of every <emphasis>word</emphasis> in the buffer. I took
 *   this out after reading Unicode Standard Annex &num;29, which explains how
 *   to divide a string into words. If you want it, feel free to implement it.
 * </para></note>
 * 
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_title_case_uni(glui32 *buf, glui32 len, glui32 numchars, glui32 lowerrest)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* GLib has a function that converts _one_ UCS-4 character to _one_
	titlecase UCS-4 character; so apparently we don't have to worry about the
	string length changing... */
	*buf = g_unichar_totitle(*buf);
	/* Call lowercase on the rest of the string */
	if(lowerrest)
		return glk_buffer_to_lower_case_uni(buf + 1, len - 1, numchars - 1) + 1;
	return numchars;
}

