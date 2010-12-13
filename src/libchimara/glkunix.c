#include <string.h>
#include <stdlib.h>
#include <libchimara/glk.h>
#include <libchimara/glkstart.h>
#include "chimara-glk-private.h"
#include "magic.h"
#include "fileref.h"
#include "stream.h"

extern GPrivate *glk_data_key;

/**
 * glkunix_stream_open_pathname:
 * @pathname: A path to a file, in the system filename encoding. 
 * @textmode: Bitfield with one or more of the <code>fileusage_</code> constants.
 * @rock: The new stream's rock value.
 *
 * Opens an arbitrary file, in read-only mode. Note that this function is
 * <emphasis>only</emphasis> available during glkunix_startup_code(). It is 
 * inherently non-portable; it should not and cannot be called from inside 
 * glk_main().
 * 
 * Returns: A new stream, or %NULL if the file operation failed.
 */
strid_t
glkunix_stream_open_pathname(char *pathname, glui32 textmode, glui32 rock)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	if(!glk_data->in_startup)
		ILLEGAL("glkunix_stream_open_pathname() may only be called from "
				"glkunix_startup_code().");
	
	g_return_val_if_fail(pathname, NULL);
	g_return_val_if_fail(strlen(pathname) > 0, NULL);
	
	frefid_t fileref = fileref_new(pathname, rock, textmode, filemode_Read);
	return file_stream_new(fileref, filemode_Read, rock, FALSE);
}

/**
 * glkunix_set_base_file:
 * @filename: A path to a file, in the system filename encoding.
 *
 * Sets the library's idea of the <quote>current directory</quote> for the 
 * executing program. The argument should be the name of a file (not a 
 * directory). When this is set, glk_fileref_create_by_name() will create files 
 * in the same directory as that file, and glk_fileref_create_by_prompt() will 
 * base default filenames off of the file. If this is not called, the library 
 * works in the Unix current working directory, and picks reasonable default 
 * defaults.
 */
void 
glkunix_set_base_file(char *filename)
{
	g_return_if_fail(filename);
	g_return_if_fail(strlen(filename) > 0);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	gchar *dirname = g_path_get_dirname(filename);
	if(!g_file_test(dirname, G_FILE_TEST_IS_DIR))
	{
		WARNING_S("Not a directory", dirname);
		g_free(dirname);
		return;
	}
	
	glk_data->current_dir = dirname;
}

/* Internal function: parse the command line, getting only the arguments
 requested by the plugin in its glkunix_arguments structure. Algorithm copied
 from CheapGlk by Andrew Plotkin. */
gboolean
parse_command_line(glkunix_argumentlist_t glkunix_arguments[], int argc, char *argv[], glkunix_startup_t *data)
{
	GSList *arglist = NULL, *iter;
	int arg;
	
	/* Now some argument-parsing. This is probably going to hurt. */
    for(arg = 1; arg < argc; arg++) 
	{
        glkunix_argumentlist_t *argform;
        char *numptr;
        
        for(argform = glkunix_arguments; argform->argtype != glkunix_arg_End; argform++) 
		{
            if(argform->name[0] == '\0') 
			{
                if(((argform->argtype == glkunix_arg_ValueFollows ||
				    argform->argtype == glkunix_arg_ValueCanFollow) && 
				    argv[arg][0] != '-') ||
				    (argform->argtype == glkunix_arg_NumberValue &&
					(atoi(argv[arg]) != 0 || argv[arg][0] == '0')))
				{
                    arglist = g_slist_prepend(arglist, argv[arg]);
				}
				else
					continue;
            }
			
            else if((argform->argtype == glkunix_arg_NumberValue)
                && !strncmp(argv[arg], argform->name, strlen(argform->name))
                && (numptr = argv[arg] + strlen(argform->name))
                && (atoi(numptr) != 0 || numptr[0] == '0')) 
			{
                arglist = g_slist_prepend(arglist, argv[arg]);
			}
			
            else if(strcmp(argv[arg], argform->name) == 0) 
			{
                if(argform->argtype == glkunix_arg_ValueFollows) 
				{
                    if(arg + 1 >= argc) {
						g_slist_free(arglist);
						return FALSE; /* No more arguments, and this one is invalid */
					}
                    arglist = g_slist_prepend(arglist, argv[arg++]);
					arglist = g_slist_prepend(arglist, argv[arg]);
                }

				else if(argform->argtype == glkunix_arg_NoValue) 
                    arglist = g_slist_prepend(arglist, argv[arg]);

                else if(argform->argtype == glkunix_arg_ValueCanFollow) 
				{
					arglist = g_slist_prepend(arglist, argv[arg]);
                    if(arg + 1 < argc && argv[arg + 1][0] != '-') 
                        arglist = g_slist_prepend(arglist, argv[++arg]);
                }
                
				else if(argform->argtype == glkunix_arg_NumberValue) 
				{
                    if(arg + 1 >= argc || (atoi(argv[arg + 1]) == 0 && argv[arg + 1][0] != '0')) 
					{
						g_slist_free(arglist);
						return FALSE;
					}
                    arglist = g_slist_prepend(arglist, argv[arg++]);
					arglist = g_slist_prepend(arglist, argv[arg]);
                }
                else 
				{
					g_slist_free(arglist);
					return FALSE;
				}
            }
		}
	}
	
	data->argc = g_slist_length(arglist) + 1;
	data->argv = g_new0(char *, data->argc);
	arglist = g_slist_reverse(arglist);
	for(iter = arglist, arg = 1; iter; iter = g_slist_next(iter), arg++)
		data->argv[arg] = g_strdup(iter->data);
	g_slist_free(arglist);
	
	return TRUE;
}