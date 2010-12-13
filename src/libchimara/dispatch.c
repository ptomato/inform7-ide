#include <libchimara/glk.h>
#include "chimara-glk-private.h"
#include "window.h"
#include "stream.h"
#include "fileref.h"

extern GPrivate *glk_data_key;

/**
 * gidispatch_set_object_registry:
 * @regi: Function to call whenever an opaque object is created.
 * @unregi: Function to call whenever an opaque object is destroyed.
 *
 * The Glk API refers to opaque objects by pointer; but a VM probably cannot 
 * store pointers to native memory. Therefore, a VM program will want to keep a
 * VM-accessible collection of opaque objects.
 * 
 * <note><para>
 *   For example, it might keep a hash table for each opaque object class,
 *   mapping integer identifiers to object pointers.
 * </para></note>
 * 
 * To make this possible, a Glk library must implement 
 * gidispatch_set_object_registry().
 * 
 * Your program calls gidispatch_set_object_registry() early (before it begins
 * actually executing VM code.) You pass in two function pointers, matching the
 * following prototypes:
 * |[
 * gidispatch_rock_t my_vm_reg_object(void *obj, glui32 objclass);
 * void my_vm_unreg_object(void *obj, glui32 objclass, gidispatch_rock_t objrock);
 * ]|
 * 
 * Whenever the Glk library creates an object, it will call 
 * <function>my_vm_reg_object&lpar;&rpar;</function>. It will pass the object
 * pointer and the class number (from 0 to <inlineequation><mathphrase>N - 
 * 1</mathphrase><alt>N - 1</alt></inlineequation>, where N is the value 
 * returned by gidispatch_count_classes().)
 * 
 * You can return any value in the #gidispatch_rock_t object; the library will
 * stash this away inside the object.
 * 
 * <note><para>
 *   Note that this is entirely separate from the regular Glk rock, which is
 *   always a #glui32 and can be set independently.
 * </para></note>
 * 
 * Whenever the Glk library destroys an object, it will call
 * <function>my_vm_unreg_object&lpar;&rpar;</function>. It passes you the object
 * pointer, class number, and the object rock.
 *
 * One significant detail: It is possible that some Glk objects will already
 * exist when your glk_main() function is called.
 * 
 * <note><para>
 *   For example, MacGlk can open a stream when the user double-clicks a file;
 *   this occurs before glk_main().
 * </para></note>
 * 
 * So when you call gidispatch_set_object_registry(), it may immediately call
 * your <function>my_vm_reg_object&lpar;&rpar;</function> callback, notifying 
 * you of the existing objects. You must be prepared for this possibility.
 * 
 * <note><para>
 *   If you are keeping hash tables, for example, create them before you call
 *   gidispatch_set_object_registry().
 * </para></note>
 */
void 
gidispatch_set_object_registry(gidispatch_rock_t (*regi)(void *obj, glui32 objclass), void (*unregi)(void *obj, glui32 objclass, gidispatch_rock_t objrock))
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	winid_t win;
    strid_t str;
    frefid_t fref;
    
    glk_data->register_obj = regi;
    glk_data->unregister_obj = unregi;
    
    if(glk_data->register_obj) 
	{
        /* It's now necessary to go through all existing objects, and register them. */
        for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
            win->disprock = (*glk_data->register_obj)(win, gidisp_Class_Window);
        for(str = glk_stream_iterate(NULL, NULL); str; str = glk_stream_iterate(str, NULL))
            str->disprock = (*glk_data->register_obj)(str, gidisp_Class_Stream);
        for(fref = glk_fileref_iterate(NULL, NULL); fref; fref = glk_fileref_iterate(fref, NULL))
            fref->disprock = (*glk_data->register_obj)(fref, gidisp_Class_Fileref);
    }
}

/**
 * gidispatch_get_objrock:
 * @obj: An opaque object.
 * @objclass: One of #gidisp_Class_Window, #gidisp_Class_Stream,
 * #gidisp_Class_Fileref, or #gidisp_Class_Schannel.
 *
 * You can, at any time, get the object rock of an object. The library
 * implements this function.
 * 
 * With this and your two callbacks, you can maintain (say) a hash table for
 * each object class, and easily convert back and forth between hash table keys
 * and Glk object pointers. A more sophisticated run-time system (such as Java)
 * could create a typed VM object for every Glk object, thus allowing VM code to
 * manipulate Glk objects intelligently.
 */
gidispatch_rock_t 
gidispatch_get_objrock(void *obj, glui32 objclass)
{
	g_return_val_if_fail(obj, (gidispatch_rock_t)NULL);
	
	switch(objclass) 
	{
		case gidisp_Class_Window:
			return ((winid_t)obj)->disprock;
		case gidisp_Class_Stream:
			return ((strid_t)obj)->disprock;
		case gidisp_Class_Fileref:
			return ((frefid_t)obj)->disprock;
		default: 
		{
			gidispatch_rock_t dummy;
			dummy.num = 0;
			return dummy;
		}
	}
}

/**
 * gidispatch_set_retained_registry:
 * @regi: Function to call whenever the Glk library assumes ownership of an
 * array.
 * @unregi: Function to call whenever the Glk library releases ownership of an
 * array.
 *
 * A few Glk functions take an array and hold onto it. The memory is 
 * <quote>owned</quote> by the library until some future Glk call releases it.
 * While the library retains the array, your program should not read, write,
 * move, or deallocate it. When the library releases it, the contents are in
 * their final form, and you can copy them out (if appropriate) and dispose of
 * the memory as you wish.
 * 
 * To allow this, the library implements gidispatch_set_retained_registry().
 * 
 * Again, you pass in two function pointers:
 * |[
 * gidispatch_rock_t my_vm_reg_array(void *array, glui32 len, char *typecode);
 * void my_vm_unreg_array(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock);
 * ]|
 *
 * Whenever a Glk function retains an array, it will call 
 * <function>my_vm_reg_array&lpar;&rpar;</function>. This occurs only if you 
 * pass an array to an argument with the <code>"#!"</code> prefix.
 *
 * <note><para>
 *   But not in every such case. Wait for the
 *   <function>my_vm_reg_array&lpar;&rpar;</function> call to confirm it.
 * </para></note>
 *
 * The library passes the array and its length, exactly as you put them in the
 * #gluniversal_t array. It also passes the string which describes the argument.
 *
 * <note><para>
 *   Currently, the only calls that retain arrays are glk_request_line_event(),
 *   glk_stream_open_memory(), glk_request_line_event_uni(), and
 *   glk_stream_open_memory_uni(). The first two of these use arrays of
 *   characters, so the string is <code>"&+#!Cn"</code>. The latter two use
 *   arrays of #glui32, so the string is <code>"&+#!Iu"</code>.
 * </para></note>
 * 
 * You can return any value in the #gidispatch_rock_t object; the library will
 * stash this away with the array.
 * 
 * When a Glk function releases a retained array, it will call
 * <function>my_vm_unreg_array&lpar;&rpar;</function>. It passes back the same
 * @array, @len, and @typecode parameters, as well as the #gidispatch_rock_t you
 * returned from <function>my_vm_reg_array&lpar;&rpar;</function>.
 * 
 * With these callbacks, you can maintain a collection of retained arrays. You
 * can use this to copy data from C arrays to your own data structures, or keep
 * relocatable memory locked, or prevent a garbage-collection system from
 * deallocating an array while Glk is writing to it.
 */
void 
gidispatch_set_retained_registry(gidispatch_rock_t (*regi)(void *array, glui32 len, char *typecode), void (*unregi)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock))
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	glk_data->register_arr = regi;
	glk_data->unregister_arr = unregi;
}
