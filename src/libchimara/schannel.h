#ifndef __SCHANNEL_H__
#define __SCHANNEL_H__

#include <glib.h>
#include "glk.h"
#include "gi_dispa.h"

struct glk_schannel_struct
{
	/*< private >*/
	glui32 magic, rock;
	gidispatch_rock_t disprock;
	/* Pointer to the list node in the global sound channel list that contains 
	 this sound channel */
	GList *schannel_list;
};

#endif