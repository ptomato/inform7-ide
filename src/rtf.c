/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <gnome.h>
#include <ctype.h>
#include <string.h>

/* The following two functions were adapted from GLib's g_strescape and
   g_strcompress */

static gchar *
rtf_escape(const gchar *source) 
{
    const guchar *p;
    gchar *dest;
    gchar *q;
    
    g_return_val_if_fail(source != NULL, NULL);

    p = (guchar *)source;
    /* Each source byte needs max four destination chars ("\tab")*/
    q = dest = g_malloc(strlen (source) * 4 + 1);

    while (*p) {
        switch (*p) {
        case '\t':
            *q++ = '\\';
            *q++ = 't';
            *q++ = 'a';
            *q++ = 'b';
            break;
        case '\n':
            *q++ = '\\';
            *q++ = '\n';
            break;
        case '\\':
            *q++ = '\\';
            *q++ = '\\';
            break;
        case '{':
            *q++ = '\\';
            *q++ = '{';
            break;
        case '}':
            *q++ = '\\';
            *q++ = '}';
            break;
        default:
            if ((*p < ' ') || (*p >= 0177)) {
                *q++ = '?';
                /* TODO: Convert other characters here */
            } else
                *q++ = *p;
            break;
        }
        p++;
    }
    *q = 0;
    return dest;
}

static gchar *
rtf_unescape(const gchar *source) 
{
    const gchar *p = source;
    gchar *dest = g_malloc(strlen(source) + 1);
    gchar *q = dest;

    /* Keywords supported:
    \tab \~ \{ \} \\ \
    \'xx is converted to ?
    All other keywords are ignored!
    Eventually support all keywords that stand for symbols such as \emdash */
    while (*p) {
        if (*p == '\\') {
            p++;
            
            /* Keywords that we do not ignore */
            /* BUG: This will also catch keywords that start with the same
            letters as a shorter keyword */
            if(!strncmp(p, "tab", 3))
                *q++ = '\t';
                /* let the regular keyword-handling code advance p */
            
            /* Keywords that are destinations; ignore everything in the group */
            if(*p == '*' || !strncmp(p, "fonttbl", 7)
              || !strncmp(p, "colortbl", 8)) {
                while(*p++ != '}')
                    ;
                /* A space after a group is ignored */
                if(isspace(*p))
                    p++;
                continue;
            }
            
            if(*p == '\'') {
                *q++ = '?'; /* Special character */
                p++;
                while(isxdigit(*p))
                    p++;
            } else if(*p == '~') { /* "Non-breaking" space */
                *q++ = ' ';
                p++;
            }
            else if(*p == '{' || *p == '}' || *p == '\\' || *p == '\n')
                *q++ = *p++;
            else if(*p == '-' || *p == '_' || *p == ':')
                p++;
            else {
                while(isalpha(*p))
                    p++;
                if(*p == '-')
                    p++;
                while(isdigit(*p))
                    p++;
            }
            /* A space after a keyword is part of the keyword */
            if(isspace(*p))
                p++;
        } else if(*p == '{' || *p == '}' || *p == '\n')
            p++;
        else
            *q++ = *p++;
    }
    *q = 0;
  
    return dest;
}

/* Convert a string of RTF to plain text and put it in the text buffer */
void
gtk_text_buffer_set_rtf_text(GtkTextBuffer *buffer, const gchar *rtf_text)
{
    gchar *text;
    if(strncmp(rtf_text, "{\\rtf", 5)) {
        g_warning(_("notes.rtf: Invalid RTF"));
        text = g_strdup("");
    } else
        text = rtf_unescape(rtf_text);
    gtk_text_buffer_set_text(buffer, text, -1);
    g_free(text);
}

/* Return a string of RTF corresponding to what Mac OS X would write */
gchar *
gtk_text_buffer_get_rtf_text(GtkTextBuffer *buffer, const GtkTextIter *start,
                             const GtkTextIter *end)
{
    gchar *text = gtk_text_buffer_get_text(buffer, start, end, FALSE);
    gchar *retval;
    
    /* Skeleton RTF with no text */
    if(strlen(text) == 0) {
        g_free(text);
        return g_strdup(
          "{\\rtf1\\mac\\ansicpg10000\\cocoartf824\\cocoasubrtf410\n"
          "{\\fonttbl}\n"
          "{\\colortbl;\\red255\\green255\\blue255;}\n"
          "}");
    }
    
    gchar *rtftext = rtf_escape(text);
    retval = g_strconcat(
      "{\\rtf1\\mac\\ansicpg10000\\cocoartf824\\cocoasubrtf410\n"
      "{\\fonttbl\\f0\\fswiss\\fcharset77 Helvetica;}\n"
      "{\\colortbl;\\red255\\green255\\blue255;}\n"
      "\\pard\\tx560\\tx1120\\tx1680\\tx2240\\tx2800\\tx3360\\tx3920\\tx4480"
      "\\tx5040\\tx5600\\tx6160\\tx6720\\ql\\qnatural\\pardirnatural\n"
      "\\f0\\fs24 \\cf0 ",
      rtftext,
      "}", NULL);
    g_free(text);
    g_free(rtftext);
    return retval;
}
