#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "fileref.h"
#include "magic.h"
#include "chimara-glk-private.h"
#include "gi_dispa.h"

extern GPrivate *glk_data_key;

/* Internal function: create a fileref using the given parameters. */
frefid_t
fileref_new(gchar *filename, glui32 rock, glui32 usage, glui32 orig_filemode)
{
	g_return_val_if_fail(filename != NULL, NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	frefid_t f = g_new0(struct glk_fileref_struct, 1);
	f->magic = MAGIC_FILEREF;
	f->rock = rock;
	if(glk_data->register_obj)
		f->disprock = (*glk_data->register_obj)(f, gidisp_Class_Fileref);
	
	f->filename = g_strdup(filename);
	f->usage = usage;
	f->orig_filemode = orig_filemode;
	
	/* Add it to the global fileref list */
	glk_data->fileref_list = g_list_prepend(glk_data->fileref_list, f);
	f->fileref_list = glk_data->fileref_list;
	
	return f;
}

static void
fileref_close_common(frefid_t fref)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	glk_data->fileref_list = g_list_delete_link(glk_data->fileref_list, fref->fileref_list);

	if(glk_data->unregister_obj)
	{
		(*glk_data->unregister_obj)(fref, gidisp_Class_Fileref, fref->disprock);
		fref->disprock.ptr = NULL;
	}
	
	if(fref->filename)
		g_free(fref->filename);
	
	fref->magic = MAGIC_FREE;
	g_free(fref);
}

/**
 * glk_fileref_iterate:
 * @fref: A file reference, or %NULL.
 * @rockptr: Return location for the next fileref's rock, or %NULL.
 *
 * Iterates through all the existing filerefs. See <link
 * linkend="chimara-Iterating-Through-Opaque-Objects">Iterating Through Opaque
 * Objects</link>.
 *
 * Returns: the next file reference, or %NULL if there are no more.
 */
frefid_t
glk_fileref_iterate(frefid_t fref, glui32 *rockptr)
{
	VALID_FILEREF_OR_NULL(fref, return NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GList *retnode;
	
	if(fref == NULL)
		retnode = glk_data->fileref_list;
	else
		retnode = fref->fileref_list->next;
	frefid_t retval = retnode? (frefid_t)retnode->data : NULL;
		
	/* Store the fileref's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_fileref_get_rock(retval);
		
	return retval;
}

/**
 * glk_fileref_get_rock:
 * @fref: A file reference.
 * 
 * Retrieves the file reference @fref's rock value. See <link 
 * linkend="chimara-Rocks">Rocks</link>.
 *
 * Returns: A rock value.
 */
glui32
glk_fileref_get_rock(frefid_t fref)
{
	VALID_FILEREF(fref, return 0);
	return fref->rock;
}

/**
 * glk_fileref_create_temp:
 * @usage: Bitfield with one or more of the <code>fileusage_</code> constants.
 * @rock: The new fileref's rock value.
 *
 * Creates a reference to a temporary file. It is always a new file (one which
 * does not yet exist). The file (once created) will be somewhere out of the
 * player's way.
 *
 * <note><para>
 *   This is why no name is specified; the player will never need to know it.
 * </para></note>
 *
 * A temporary file should never be used for long-term storage. It may be
 * deleted automatically when the program exits, or at some later time, say
 * when the machine is turned off or rebooted. You do not have to worry about
 * deleting it yourself.
 *
 * Returns: A new fileref, or #NULL if the fileref creation failed.
 */ 
frefid_t
glk_fileref_create_temp(glui32 usage, glui32 rock)
{
	/* Get a temp file */
	GError *error = NULL;
	gchar *filename = NULL;
	gint handle = g_file_open_tmp("glkXXXXXX", &filename, &error);
	if(handle == -1)
	{
		WARNING_S("Error creating temporary file", error->message);
		if(filename)
			g_free(filename);
		return NULL;
	}
	if(close(handle) == -1) /* There is no g_close() */
	{
		IO_WARNING( "Error closing temporary file", filename, g_strerror(errno) );
		if(filename)
			g_free(filename);
		return NULL;
	}
	
	frefid_t f = fileref_new(filename, rock, usage, filemode_Write);
	g_free(filename);
	return f;
}

/**
 * glk_fileref_create_by_prompt:
 * @usage: Bitfield with one or more of the <code>fileusage_</code> constants.
 * @fmode: File mode, contolling the dialog's behavior.
 * @rock: The new fileref's rock value.
 *
 * Creates a reference to a file by asking the player to locate it. The library
 * may simply prompt the player to type a name, or may use a platform-native
 * file navigation tool. (The prompt, if any, is inferred from the usage
 * argument.)
 *
 * <note><title>Chimara</title>
 * <para>
 * Chimara uses a <link 
 * linkend="GtkFileChooserDialog">GtkFileChooserDialog</link>. The default
 * starting location for the dialog may be set with glkunix_set_base_file().
 * </para></note>
 *
 * @fmode must be one of these values:
 * <variablelist>
 * <varlistentry>
 *   <term>%filemode_Read</term>
 *   <listitem><para>The file must already exist; and the player will be asked
 *   to select from existing files which match the usage.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%filemode_Write</term>
 *   <listitem><para>The file should not exist; if the player selects an
 *   existing file, he will be warned that it will be replaced.
 *   </para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%filemode_ReadWrite</term>
 *   <listitem><para>The file may or may not exist; if it already exists, the
 *   player will be warned that it will be modified.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%filemode_WriteAppend</term>
 *   <listitem><para>Same behavior as %filemode_ReadWrite.</para></listitem>
 * </varlistentry>
 * </variablelist>
 *
 * The @fmode argument should generally match the @fmode which will be used to
 * open the file.
 *
 * <note><para>
 *   It is possible that the prompt or file tool will have a 
 *   <quote>cancel</quote> option. If the player chooses this,
 *   glk_fileref_create_by_prompt() will return %NULL. This is a major reason
 *   why you should make sure the return value is valid before you use it.
 * </para></note>
 *
 * Returns: A new fileref, or #NULL if the fileref creation failed or the
 * dialog was canceled.
 */
frefid_t
glk_fileref_create_by_prompt(glui32 usage, glui32 fmode, glui32 rock)
{
	/* TODO: Remember current working directory and last used filename
	for each usage */
	GtkWidget *chooser;

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	gdk_threads_enter();

	switch(fmode)
	{
		case filemode_Read:
			chooser = gtk_file_chooser_dialog_new("Select a file to open", NULL,
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
			break;
		case filemode_Write:
			chooser = gtk_file_chooser_dialog_new("Select a file to save to", NULL,
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SAVE);

			/* COMPAT: */
#if GTK_CHECK_VERSION(2,8,0)
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser), TRUE);
#endif
			break;
		case filemode_ReadWrite:
		case filemode_WriteAppend:
			chooser = gtk_file_chooser_dialog_new("Select a file to save to", NULL,
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SAVE);
			break;
		default:
			ILLEGAL_PARAM("Unknown file mode: %u", fmode);
			gdk_threads_leave();
			return NULL;
	}
	
	if(glk_data->current_dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), glk_data->current_dir);
	
	if(gtk_dialog_run( GTK_DIALOG(chooser) ) != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_destroy(chooser);
		gdk_threads_leave();
		return NULL;
	}
	gchar *filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(chooser) );
	frefid_t f = fileref_new(filename, rock, usage, fmode);
	g_free(filename);
	gtk_widget_destroy(chooser);

	gdk_threads_leave();
	return f;
}

/**
 * glk_fileref_create_by_name:
 * @usage: Bitfield with one or more of the <code>fileusage_</code> constants.
 * @name: A filename.
 * @rock: The new fileref's rock value.
 *
 * This creates a reference to a file with a specific name. The file will be
 * in a fixed location relevant to your program, and visible to the player.
 *
 * <note><para>
 *   This usually means <quote>in the same directory as your program.</quote>
 * </para></note>
 * <note><title>Chimara</title>
 * <para>
 * In Chimara, the file is created in the directory last set by 
 * glkunix_set_base_file(), and otherwise in the current working directory.
 * </para></note>
 *
 * Since filenames are highly platform-specific, you should use
 * glk_fileref_create_by_name() with care. It is legal to pass any string in the
 * name argument. However, the library may have to mangle, transform, or
 * truncate the string to make it a legal native filename. 
 *
 * <note><para>
 *   For example, if you create two filerefs with the names <quote>File</quote>
 *   and <quote>FILE</quote>, they may wind up pointing to the same file; the
 *   platform may not support case distinctions in file names. Another example:
 *   on a platform where file type is specified by filename suffix, the library
 *   will add an appropriate suffix based on the usage; any suffix in the string
 *   will be overwritten or added to. For that matter, remember that the period
 *   is not a legal character in Acorn filenames...
 * </para></note>
 *
 * The most conservative approach is to pass a string of no more than 8
 * characters, consisting entirely of upper-case letters and numbers, starting
 * with a letter. You can then be reasonably sure that the resulting filename
 * will display all the characters you specify &mdash; in some form. 
 *
 * Returns: A new fileref, or %NULL if the fileref creation failed. 
 */
frefid_t
glk_fileref_create_by_name(glui32 usage, char *name, glui32 rock)
{
	g_return_val_if_fail(name != NULL && strlen(name) > 0, NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	/* Do any string-munging here to remove illegal Latin-1 characters from 
	filename. On ext3, the only illegal characters are '/' and '\0'. */
	g_strdelimit(name, "/", '_');
	
	/* Find out what encoding filenames are in */
	const gchar **charsets; /* Do not free */
	g_get_filename_charsets(&charsets);

	/* Convert name to that encoding */
	GError *error = NULL;
	gchar *osname = g_convert(name, -1, charsets[0], "ISO-8859-1", NULL, NULL,
		&error);
	if(osname == NULL)
	{
		WARNING_S("Error during latin1->filename conversion", error->message);
		return NULL;
	}
	
	gchar *path;
	if(glk_data->current_dir)
		path = g_build_filename(glk_data->current_dir, osname, NULL);
	else
		path = g_strdup(osname);
	g_free(osname);
	
	frefid_t f = fileref_new(path, rock, usage, filemode_ReadWrite);
	g_free(path);
	return f;
}

/**
 * glk_fileref_create_from_fileref:
 * @usage: Bitfield with one or more of the <code>fileusage_</code> constants.
 * @fref: Fileref to copy.
 * @rock: The new fileref's rock value.
 *
 * This copies an existing file reference @fref, but changes the usage. (The
 * original fileref is not modified.)
 *
 * The use of this function can be tricky. If you change the type of the fileref
 * (%fileusage_Data, %fileusage_SavedGame, etc), the new reference may or may
 * not point to the same actual disk file.
 *
 * <note><para>
 *   This generally depends on whether the platform uses suffixes to indicate
 *   file type.
 * </para></note>
 *
 * If you do this, and open both file references for writing, the results are
 * unpredictable. It is safest to change the type of a fileref only if it refers
 * to a nonexistent file.
 *
 * If you change the mode of a fileref (%fileusage_TextMode,
 * %fileusage_BinaryMode), but leave the rest of the type unchanged, the new
 * fileref will definitely point to the same disk file as the old one.
 * 
 * Obviously, if you write to a file in text mode and then read from it in
 * binary mode, the results are platform-dependent. 
 *
 * Returns: A new fileref, or %NULL if the fileref creation failed. 
 */
frefid_t
glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock)
{
	VALID_FILEREF(fref, return NULL);
	return fileref_new(fref->filename, rock, usage, fref->orig_filemode);
}

/**
 * glk_fileref_destroy:
 * @fref: Fileref to destroy.
 * 
 * Destroys a fileref which you have created. This does <emphasis>not</emphasis>
 * affect the disk file; it just reclaims the resources allocated by the
 * <code>glk_fileref_create...</code> function.
 *
 * It is legal to destroy a fileref after opening a file with it (while the
 * file is still open.) The fileref is only used for the opening operation,
 * not for accessing the file stream.
 */
void
glk_fileref_destroy(frefid_t fref)
{
	VALID_FILEREF(fref, return);
	fileref_close_common(fref);
}

/**
 * glk_fileref_delete_file:
 * @fref: A refrence to the file to delete.
 *
 * Deletes the file referred to by @fref. It does not destroy @fref itself.
 */
void
glk_fileref_delete_file(frefid_t fref)
{
	VALID_FILEREF(fref, return);
	if( glk_fileref_does_file_exist(fref) )
		if(g_unlink(fref->filename) == -1)
			IO_WARNING( "Error deleting file", fref->filename, g_strerror(errno) );
}

/**
 * glk_fileref_does_file_exist:
 * @fref: A fileref to check.
 *
 * Checks whether the file referred to by @fref exists.
 *
 * Returns: %TRUE (1) if @fref refers to an existing file, and %FALSE (0) if 
 * not.
 */
glui32
glk_fileref_does_file_exist(frefid_t fref)
{
	VALID_FILEREF(fref, return 0);
	if( g_file_test(fref->filename, G_FILE_TEST_EXISTS) )
		return 1;
	return 0;
}

