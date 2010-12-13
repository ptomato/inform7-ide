#include <stddef.h> /* Surprisingly, the only symbol needed is NULL */
#include "glk.h"

/* Version of the Glk specification implemented by this library */
#define MAJOR_VERSION 0
#define MINOR_VERSION 7
#define SUB_VERSION   0

/**
 * glk_gestalt:
 * @sel: A selector, representing which capability to request information 
 * about.
 * @val: Extra information, depending on the value of @sel.
 *
 * Calls the gestalt system to request information about selector @sel, without
 * passing an array to store extra information in (see glk_gestalt_ext()).
 *
 * Returns: an integer, depending on what selector was called.
 */
glui32
glk_gestalt(glui32 sel, glui32 val)
{
	return glk_gestalt_ext(sel, val, NULL, 0);
}

/**
 * glk_gestalt_ext:
 * @sel: A selector, representing which capability to request information
 * about.
 * @val: Extra information, depending on the value of @sel.
 * @arr: Location of an array to store extra information in, or %NULL.
 * @arrlen: Length of @arr, or 0 if @arr is %NULL.
 *
 * Calls the gestalt system to request information about the capabilities of the
 * API. The selector @sel tells which capability you are requesting information
 * about; the other three arguments are additional information, which may or may
 * not be meaningful. The @arr and @arrlen arguments of glk_gestalt_ext() are
 * always optional; you may always pass %NULL and 0, if you do not want whatever
 * information they represent. glk_gestalt() is simply a shortcut for this;
 * <code>#glk_gestalt(x, y)</code> is exactly the same as 
 * <code>#glk_gestalt_ext(x, y, %NULL, 0)</code>.
 *
 * The critical point is that if the Glk library has never heard of the selector
 * @sel, it will return 0. It is <emphasis>always</emphasis> safe to call 
 * <code>#glk_gestalt(x, y)</code> (or <code>#glk_gestalt_ext(x, y, %NULL, 
 * 0)</code>). Even if you are using an old library, which was compiled before
 * the given capability was imagined, you can test for the capability by calling
 * glk_gestalt(); the library will correctly indicate that it does not support
 * it, by returning 0.
 *
 * (It is also safe to call <code>#glk_gestalt_ext(x, y, z, zlen)</code> for an
 * unknown selector <code>x</code>, where <code>z</code> is not %NULL, as long
 * as <code>z</code> points at an array of at least <code>zlen</code> elements.
 * The selector will be careful not to write beyond that point in the array, if
 * it writes to the array at all.)
 *
 * (If a selector does not use the second argument, you should always pass 0; do
 * not assume that the second argument is simply ignored. This is because the
 * selector may be extended in the future. You will continue to get the current
 * behavior if you pass 0 as the second argument, but other values may produce
 * other behavior.)
 *
 * Returns: an integer, depending on what selector was called.
 */
glui32
glk_gestalt_ext(glui32 sel, glui32 val, glui32 *arr, glui32 arrlen)
{
	switch(sel)
	{
		/* Version number */
		case gestalt_Version:
			return (MAJOR_VERSION << 16) + (MINOR_VERSION << 8) + SUB_VERSION;
		
		/* Which characters can the player type in line input? */
		case gestalt_LineInput:
			/* Does not accept control chars */
			if( val < 32 || (val >= 127 && val <= 159) )
				return 0;
			return 1;
			
		/* Which characters can the player type in char input? */
		case gestalt_CharInput:
			/* Does not accept control chars or unknown */
			if( val < 32 || (val >= 127 && val <= 159) || val == keycode_Unknown )
				return 0;
			return 1;
		
		/* Which characters can we print? */	
		case gestalt_CharOutput:
			/* All characters are printed as one character, in any case */
			if(arr && arrlen > 0)
				*arr = 1;
			/* Cannot print control chars except \n */
			if( (val < 32 && val != 10) || (val >= 127 && val <= 159) )
				return gestalt_CharOutput_CannotPrint;
			/* Can print all other characters */
			return gestalt_CharOutput_ExactPrint;
		
		/* Unicode capabilities present */
		case gestalt_Unicode:
			return 1;

		/* Timer capabilities present */
		case gestalt_Timer:
			return 1;

		/* Hyperlink capabilities present */
		case gestalt_Hyperlinks:
			return 1;

		/* Hyperlinks supported on textbuffers and textgrids */
		case gestalt_HyperlinkInput:
			return val == wintype_TextBuffer || val == wintype_TextGrid;

		/* Mouse support present in textgrids */
		case gestalt_MouseInput:
			return val == wintype_TextGrid;

		case gestalt_Graphics:
			return 1;

		case gestalt_DrawImage:
			return val == wintype_Graphics || val == wintype_TextBuffer;

		case gestalt_GraphicsTransparency:
			return 1;
			
		/* Unsupported capabilities */
		case gestalt_Sound:
		case gestalt_SoundVolume:
		case gestalt_SoundNotify:
		case gestalt_SoundMusic:
		/* Selector not supported */	
		default:
			return 0;
	}
}

