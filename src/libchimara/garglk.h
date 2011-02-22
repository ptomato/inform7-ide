#ifndef __GARGLK_H__
#define __GARGLK_H__

/**
 * GARGLK:
 *
 * To test at compile time whether the Gargoyle extensions are available, you
 * can perform a preprocessor test for the existence of %GARGLK. If this
 * macro is defined, then so are all the Gargoyle extensions. If not, not.
 *
 * <note><title>Chimara</title><para>
 *   Currently, in Chimara, the functions are defined, but most of them are
 *   not implemented. That is, you can call them, but they do nothing.
 * </para></note>
 */
#define GARGLK 1

extern char* garglk_fileref_get_name(frefid_t fref);

extern void garglk_set_program_name(const char *name);
extern void garglk_set_program_info(const char *info);
extern void garglk_set_story_name(const char *name);

/*
 This function is not implemented even in Gargoyle. Looks like it was planned, 
 but never added.
extern void garglk_set_config(const char *name);
*/

/* JM: functions added to support Z-machine features that aren't in the Glk standard */

extern void garglk_set_line_terminators(winid_t win, const glui32 *keycodes, glui32 numkeycodes);

/* garglk_unput_string - removes the specified string from the end of the output buffer, if
 * indeed it is there. */
extern void garglk_unput_string(char *str);
extern void garglk_unput_string_uni(glui32 *str);

/* TODO document */
#define zcolor_Transparent   (-4)
/* TODO document */
#define zcolor_Cursor        (-3)
/**
 * zcolor_Current:
 *
 * Z-machine color constant representing the current color.
 */
#define zcolor_Current       (-2)
/**
 * zcolor_Default:
 *
 * Z-machine color constant representing the default color.
 */
#define zcolor_Default       (-1)

extern void garglk_set_zcolors(glui32 fg, glui32 bg);
extern void garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg);
extern void garglk_set_reversevideo(glui32 reverse);
extern void garglk_set_reversevideo_stream(strid_t str, glui32 reverse);

/* non standard keycodes */
/**
 * keycode_Erase:
 *
 * Since %keycode_Delete represents either the <keycap>Delete</keycap> or 
 * <keycap>Backspace</keycap> key, Gargoyle defines a separate constant
 * %keycode_Erase to represent <emphasis>only</emphasis> the 
 * <keycap>Delete</keycap> key. In character input, <keycap>Delete</keycap> is
 * still reported as %keycode_Delete, but the two are distinguished in 
 * garglk_set_line_terminators().
 */
#define keycode_Erase    (0xffffef7f)
/* TODO document */
#define keycode_MouseWheelUp        (0xffffeffe)
/* TODO document */
#define keycode_MouseWheelDown      (0xffffefff)

#endif /* __GARGLK_H__ */
