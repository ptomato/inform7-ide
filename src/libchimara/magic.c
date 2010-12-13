#include <gtk/gtk.h>
#include "glk.h"
#include "magic.h"

/* The "magic" mechanism was stolen from Evin Robertson's GtkGlk. */

static gchar *
magic_to_string(glui32 magic)
{
	switch(magic)
	{
		case MAGIC_WINDOW:
			return "winid_t";
		case MAGIC_STREAM:
			return "strid_t";
		case MAGIC_FILEREF:
			return "frefid_t";
		case MAGIC_SCHANNEL:
			return "schanid_t";
		default:
			g_return_val_if_reached("unknown");
	}
}

/* Internal function: check the object's magic number to make sure it is the
 right type, and not freed. */
gboolean
magic_is_valid_or_null(const glui32 goodmagic, const glui32 realmagic, const gchar *function)
{
	if(realmagic != MAGIC_NULL)
	{
		if(realmagic != goodmagic)
		{
			if(realmagic == MAGIC_FREE)
				g_critical("%s: Using a freed object", function);
			else
				g_critical( "%s: %s object not a %s", function, magic_to_string(realmagic), magic_to_string(goodmagic) );
			return FALSE;
		}
	}
	return TRUE;
}
 

/* Internal function: check the object's magic number to make sure it is
 not NULL, the right type, and not freed. */
gboolean 
magic_is_valid(const void *obj, const glui32 goodmagic, const glui32 realmagic, const gchar *function)
{
	if(obj == NULL)
	{
		g_critical( "%s: NULL %s pointer", function, magic_to_string(goodmagic) );
		return FALSE;
	}
	return magic_is_valid_or_null(goodmagic, realmagic, function);
}
