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
 These functions are not implemented even in Gargoyle. Looks like they were
 planned, but never added.
extern void garglk_set_config(const char *name);

#define garglk_font_Roman           (0)
#define garglk_font_Italic          (1)
#define garglk_font_Bold            (2)
#define garglk_font_BoldItalic      (3)
#define garglk_font_MonoRoman       (4)
#define garglk_font_MonoItalic      (5)
#define garglk_font_MonoBold        (6)
#define garglk_font_MonoBoldItalic  (7)

#define garglk_color_White          (0)
#define garglk_color_Red            (1)
#define garglk_color_Green          (2)
#define garglk_color_Blue           (3)
#define garglk_color_Cyan           (4)
#define garglk_color_Magenta        (5)
#define garglk_color_Yellow         (6)
#define garglk_color_Black          (7)

extern void garglk_set_style_font(glui32 font);
extern void garglk_set_style_stream_font(strid_t str, glui32 font);
extern void garglk_set_style_color(glui32 bg, glui32 fg);
extern void garglk_set_style_stream_color(strid_t str, glui32 bg, glui32 fg);
*/

/* JM: functions added to support Z-machine features that aren't in the Glk standard */

extern void garglk_set_line_terminators(winid_t win, const glui32 *keycodes, glui32 numkeycodes);

extern void garglk_unput_string(char *str);
extern void garglk_unput_string_uni(glui32 *str);

/**
 * zcolor_Current:
 *
 * Z-machine color constant representing the current color.
 */
#define zcolor_Current      (0)
/**
 * zcolor_Default:
 *
 * Z-machine color constant representing the default color.
 */
#define zcolor_Default      (1)
/**
 * zcolor_Black:
 *
 * Z-machine color constant representing black (0x000000).
 */
#define zcolor_Black        (2)
/**
 * zcolor_Red:
 *
 * Z-machine color constant representing red (0x0000E8).
 */
#define zcolor_Red          (3)
/**
 * zcolor_Green:
 *
 * Z-machine color constant representing green (0x00D000).
 */
#define zcolor_Green        (4)
/**
 * zcolor_Yellow:
 *
 * Z-machine color constant representing yellow (0x00E8E8).
 */
#define zcolor_Yellow       (5)
/**
 * zcolor_Blue:
 *
 * Z-machine color constant representing blue (0xB06800).
 */
#define zcolor_Blue         (6)
/**
 * zcolor_Magenta:
 *
 * Z-machine color constant representing magenta (0xFF00FF).
 */
#define zcolor_Magenta      (7)
/**
 * zcolor_Cyan:
 *
 * Z-machine color constant representing cyan (0xE8E800).
 */
#define zcolor_Cyan         (8)
/**
 * zcolor_White:
 *
 * Z-machine color constant representing white (0xFFFFFF).
 */
#define zcolor_White        (9)
/**
 * zcolor_LightGrey:
 *
 * Z-machine color constant representing light grey (0xB0B0B0).
 */
#define zcolor_LightGrey    (10)
/**
 * zcolor_MediumGrey:
 *
 * Z-machine color constant representing grey (0x888888).
 */
#define zcolor_MediumGrey   (11)
/**
 * zcolor_DarkGrey:
 *
 * Z-machine color constant representing dark grey (0x585858).
 */
#define zcolor_DarkGrey     (12)
#define zcolor_NUMCOLORS    (13)

extern void garglk_set_zcolors(glui32 fg, glui32 bg);
extern void garglk_set_reversevideo(glui32 reverse);

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

#endif /* __GARGLK_H__ */
