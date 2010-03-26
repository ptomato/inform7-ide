/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <gnome.h>
#include <ctype.h>
#include <gtkhtml/gtkhtml.h>
#include <gtkhtml/gtkhtml-stream.h>

#include "support.h"

#include "appwindow.h"
#include "datafile.h"
#include "error.h"
#include "extwindow.h"
#include "file.h"
#include "history.h"
#include "html.h"
#include "prefs.h"
#include "story.h"
#include "tabsource.h"

/* GtkHTML ignores and erases <script> tags; so, when we load an URL we keep the
source and store it in this table, so we can refer back to the original
source. Stupid and ugly, but necessary. */
static GSList *source_table = NULL;

/* Structure representing an entry in the HTML source table */
struct source_entry {
    GtkHTML *html;
    gchar *source;
};

/* Add the source of a HTML page to the table, with the GtkHTML widget as key*/
static void 
source_table_add(GtkHTML *html, gchar *source) 
{
    g_return_if_fail(html != NULL);
    g_return_if_fail(source != NULL);
    
    struct source_entry *entry = g_malloc(sizeof(struct source_entry));
    entry->html = html;
    entry->source = g_strdup(source);
    source_table = g_slist_prepend(source_table, (gpointer)entry);
}

/* Remove an entry from the HTML source table */
static void 
source_table_remove(GtkHTML *html) 
{
    g_return_if_fail(html != NULL);
    
    GSList *iter;
    for(iter = source_table; iter != NULL; iter = g_slist_next(iter))
        if(((struct source_entry *)(iter->data))->html == html) {
            g_free(((struct source_entry *)(iter->data))->source);
            source_table = g_slist_remove(source_table,
              (gconstpointer)(iter->data));
        }
}

/* The GtkHTML widget is now displaying another HTML file, change it in the
table */
static void 
source_table_update(GtkHTML *html, gchar *source) 
{
    g_return_if_fail(html != NULL);
    g_return_if_fail(source != NULL);
    
    GSList *iter;
    for(iter = source_table; iter != NULL; iter = g_slist_next(iter))
        if(((struct source_entry *)(iter->data))->html == html) {
            g_free(((struct source_entry *)(iter->data))->source);
            ((struct source_entry *)(iter->data))->source = g_strdup(source);
        }
}

/* Get the HTML source from the table */
static gchar *
source_table_get(GtkHTML *html) 
{
    g_return_val_if_fail(html != NULL, NULL);
    
    GSList *iter;
    for(iter = source_table; iter != NULL; iter = g_slist_next(iter))
        if(((struct source_entry *)(iter->data))->html == html)
            return ((struct source_entry *)(iter->data))->source;
    return NULL;
}

/* Get the base path of an URL, for relative paths */
static gchar *
get_base_name(const gchar *url) 
{
    g_return_val_if_fail(url != NULL, NULL);
    
    gchar *newbase = g_malloc(strlen(url));
    strcpy(newbase, "");

    /* here we realize a minuscule improvement in performance by removing
    redundant directory changes in the base URL; oh, and also we remove the
    filename from the end */
    gchar **parts = g_strsplit(url, G_DIR_SEPARATOR_S, 0);
    gchar **ptr;
    for(ptr = parts ; *(ptr + 1) ; ptr++) { /*do not copy the last element*/
        if(strcmp(*ptr, "..")
          && strcmp(*ptr, ".")
          && !strcmp(*(ptr + 1), ".."))
            /* (if this element is not '.' or '..' and the next one is '..') */
            ptr++; /* skip them */
        else {
            g_strlcat(newbase, *ptr, strlen(url));
            g_strlcat(newbase, G_DIR_SEPARATOR_S, strlen(url));
        }
    }
    g_strfreev(parts);
    return newbase;
}

/* Change escapes of the type [=0xNNNN=] to their UTF8 character equivalents.
Returns a newly-allocated string */
static gchar *
unescape_unicode(const gchar *source)
{
    const gchar *p = source;
    gchar *dest = g_malloc(strlen(source) + 1); 
    /* no need to allocate more, because every ten-character escape code only 
    translates to at most six utf8 characters */
    gchar *q = dest;
  
    while(*p) {
        if(*p == '[' && *(p+1) == '=') {
            gunichar code;
            if(sscanf(p, "[=0x%4X=]", &code) != 1) {
                g_warning(_("unescape_unicode: Incorrect character escape!"));
                *q = '\0';
                return dest;
            }
            p += 10; /* length of [=0xNNNN=] */
            gchar buffer[6];
            gint length = g_unichar_to_utf8(code, buffer);
            int foo;
            for(foo = 0; foo < length; foo++)
                *q++ = buffer[foo];
        } else
            *q++ = *p++;
    }
    *q = '\0';
    return dest;
}

/* Find the code to be pasted within one of the pasteCode134, etc. javascript
functions */
static gchar *
javascript_find_paste_code(const gchar *source, const gchar *function_call) 
{
    g_return_val_if_fail(source != NULL, NULL);
    g_return_val_if_fail(function_call != NULL, NULL);
    
    gchar *retval;
    gchar *function_name = g_strdup(function_call);
    
    /* erase everything after the first parenthesis as arguments */
    gchar *args;
    if((args = strchr(function_name, '(')))
        *args = '\0';

    gchar *buf = g_strdup(source);
    gchar *beginptr = strstr(buf, "<script language=\"JavaScript\">");
    if(beginptr == NULL)
        return NULL;
    beginptr += strlen("<script language=\"JavaScript\">");
    gchar *endptr = strstr(beginptr, "</script>");
    ptrdiff_t length = endptr - beginptr;
    gchar *result = g_strndup(beginptr, length);
    
    if(strstr(result, function_name)) {
        gchar *temp = g_strdup(strstr(result, "pasteCode('")
          + strlen("pasteCode('"));
        *(strstr(temp, "');")) = '\0';
        retval = unescape_unicode(temp);
        g_free(temp);
    } else
        retval = javascript_find_paste_code(endptr, function_call);
    
    g_free(result);
    g_free(function_name);
    g_free(buf);
    return retval;
}

/* This is the function responsible for getting the data from the URLs. There is
already a stream opened at this point, so we do not handle anything that does
not involve data being written to the GtkHTML widget. That should already have
been done in on_link_clicked. */
static void 
on_url_requested(GtkHTML *html, const gchar *url, GtkHTMLStream *handle, 
                 gpointer data) 
{
    g_return_if_fail(html != NULL);
    g_return_if_fail(url != NULL);
    g_return_if_fail(handle != NULL);
    
    GError *err = NULL;
    gchar *buf;
    gchar *anchor;
    gsize length;
    gchar *file = g_strdup(url);
    
    /* Get the anchor if there is a # in the URL, and remove it from the URL */
    if((anchor = strchr(file, '#')))
        *(anchor++) = '\0';

    /* inform: protocol can mean a file in any one of several locations;
    find it and save it in the variable "file" */
    if(g_str_has_prefix(url, "inform:/")) {
        gchar *real_file = NULL;
        if(check_datafile(url + 8))
            real_file = get_datafile_path(url + 8);
        
        gchar *checkfile = g_build_filename("Documentation", url + 8, NULL);
        if(check_datafile(checkfile))
            real_file = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("Documentation","doc_images", url+8, NULL);
        if(check_datafile(checkfile))
            real_file = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("Documentation","Sections", url + 8, NULL);
        if(check_datafile(checkfile))
            real_file = get_datafile_path(checkfile);
        g_free(checkfile);
        
        if(!real_file) {
            error_dialog(NULL, NULL, _("Error opening file %s."), url + 8);
            gtk_html_end(html, handle, GTK_HTML_STREAM_ERROR);
            return;
        }
        
        g_free(file);
        file = g_strdup(real_file);
        g_free(real_file);
    }
    
    /* Check if it's a datafile */
    gchar *temp = g_path_get_basename(file);
    if(check_datafile(temp)) {
        g_free(file);
        file = get_datafile_path(temp);
        g_free(temp);
    }
    /* Check if it's the "file://" protocol */
    if(g_str_has_prefix(file, "file://")) {
        temp = g_strdup(file + 7);
        g_free(file);
        file = temp;
    }
    
    /* Open "file" and write it to the html widget */
    if(!g_file_get_contents(file, &buf, &length, &err)) {
        error_dialog(NULL, err, _("Error opening file %s: "), file);
        gtk_html_end(html, handle, GTK_HTML_STREAM_ERROR);
        g_free(file);
        return;
    }
    gtk_html_write(html, handle, buf, length);
    gtk_html_end(html, handle, GTK_HTML_STREAM_OK);
    g_free(buf);
    g_free(file);
    return;
}

/* This function is called when the user clicks on a link. It handles the
different protocols used by the Inform help browser and, if necessary,
eventually passes control to on_url_requested */
static void
on_link_clicked(GtkHTML *html, const gchar *requested_url, gpointer data)
{
    g_return_if_fail(html != NULL);
    g_return_if_fail(requested_url != NULL);
    
    Story *thestory = get_story(GTK_WIDGET(html));
    int side = -1;
    if(GTK_WIDGET(html) == lookup_widget(GTK_WIDGET(html), "docs_l"))
        side = LEFT;
    else if(GTK_WIDGET(html) == lookup_widget(GTK_WIDGET(html), "docs_r"))
        side = RIGHT;
    
    GError *err = NULL;
    gchar *anchor;
    gchar *real_url = NULL;
    gchar *url = g_strdup(requested_url); /* make a copy so we can mess w/ it */
    
    /* Check if there is an anchor we have to jump to later and remove it from
    the regular URL */
    if((anchor = strchr(url, '#'))) {
        *(anchor++) = '\0';
        if(!strcmp(url, gtk_html_get_base(html))) {
            /* if we are only jumping to an anchor on the same page */
            gtk_html_jump_to_anchor(html, anchor);
            if(side != -1) {
                gchar *last_url;
                if(thestory->current[side]->page)
                    last_url = g_strdup(thestory->current[side]->page);
                else {
                    last_url = history_get_last_docpage(thestory, side);
                    gchar *old_anchor;
                    if((old_anchor = strchr(last_url, '#')))
                        *old_anchor = '\0';
                }
                gchar *history_url = g_strconcat(last_url, "#", anchor, NULL);
                history_push_docpage(thestory, side, history_url);
                g_free(history_url);
                g_free(last_url);
            }
            g_free(url);
            return;
        }
    }
    
    /* If there is no ':' (no protocol) then it is just a file */
    if(!strchr(url, ':')) {
        real_url = g_strdup(url);
    /* file: protocol */
    } else if(g_str_has_prefix(url, "file:")) {
        real_url = g_strdup(url + 5); /* Shove the pointer past the prefix */
    /* inform: protocol can mean one of several locations */
    } else if(g_str_has_prefix(url, "inform://Extensions")) {
        real_url = g_build_filename(g_get_home_dir(), "Inform", "Documentation",
          url + 19, NULL);
		/* When clicking on a link to extension documentation, this is a hack to
        get it to open in the documentation tab */
        int right = get_current_notebook_side(GTK_WIDGET(html));
        html = GTK_HTML(lookup_widget(GTK_WIDGET(html), 
          right? "docs_r" : "docs_l"));
        gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(html), right),
          TAB_DOCUMENTATION);
    } else if(g_str_has_prefix(url, "inform:/")) {
        if(check_datafile(url + 8))
            real_url = get_datafile_path(url + 8);
        
        gchar *checkfile = g_build_filename("Documentation", url + 8, NULL);
        if(check_datafile(checkfile))
            real_url = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("Documentation","doc_images", url+8, NULL);
        if(check_datafile(checkfile))
            real_url = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("Documentation","Sections", url + 8, NULL);
        if(check_datafile(checkfile))
            real_url = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename(g_get_home_dir(), "Inform",
          "Documentation", url + 8, NULL);
        if(g_file_test(checkfile, G_FILE_TEST_EXISTS))
            real_url = g_strdup(checkfile);
        g_free(checkfile);
        
        if(!real_url) {
            g_free(url);
            return;
        }
        
        /* When clicking on a link to the documentation, this is a hack to
        get it to open in the documentation tab */
        int right = get_current_notebook_side(GTK_WIDGET(html));
        html = GTK_HTML(lookup_widget(GTK_WIDGET(html), 
          right? "docs_r" : "docs_l"));
        gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(html), right),
          TAB_DOCUMENTATION);
    /* http: protocol, open in the default web browser */    
    } else if(g_str_has_prefix(url, "http:")
      || g_str_has_prefix(url, "mailto:")) {
        if(!gnome_url_show(url, &err)) {
            error_dialog(NULL, err, _("Error opening external viewer for %s: "),
              url);
        }
        g_free(url);
        return;
    /* javascript: protocol, damned if I'm going to write an entire javascript
    interpreter, so here we just recognize the functions that are used in the
    Inform docs */
    } else if(g_str_has_prefix(url, "javascript:")) {
        gchar *function_call = g_strdup(url + 11);
        /* Handle the function pasteCode('...') which pastes its argument
        directly into the source */
        if(g_str_has_prefix(function_call, "pasteCode('")) {
            *(strrchr(function_call, '\'')) = '\0';
            paste_code(get_story(GTK_WIDGET(html))->buffer, function_call + 11);
            g_free(function_call);
            g_free(url);
            return;
        }
        /* Now the name of the function is something else, like "pasteCode134()"
        and we need to look the function body up in the source cache in order to
        find what text to paste */
        gchar *source = source_table_get(html);
        gchar *code = javascript_find_paste_code(source, function_call);
        paste_code(get_story(GTK_WIDGET(html))->buffer, code);
        g_free(code);
        g_free(function_call);
        g_free(url);
        return;
    /* source: protocol, a link to somewhere in the source file or an 
    extension */
    } else if(g_str_has_prefix(url, "source:")) {
		/* If it links to the source file, just jump to the line */
		if(strcmp(url + 7, "story.ni") == 0) {
		    gint line;
		    if(sscanf(anchor, "line%d", &line))
		        jump_to_line(GTK_WIDGET(html), line);
		    g_free(url);
		    return;
		}
		/* Otherwise it's a link to an extension */
		gchar *filename = get_case_insensitive_extension(g_strdup(url + 7));
		Extension *ext = open_extension(filename);
		if(ext != NULL) {
			gtk_widget_show(ext->window);
			gint line;
		    if(sscanf(anchor, "line%d", &line))
		        jump_to_line_ext(ext->window, line);
		}
		g_free(filename);
		g_free(url);
		return;
    } else {
        g_warning(_("Unrecognized protocol: %s\n"), url);
        g_free(url);
        return;
    }

    /* This is the actual filename of the page we are loading, store it in the
    history */
    if(side != -1)
        history_push_docpage(thestory, side, real_url);
    
    GtkHTMLStream *handle = gtk_html_begin(html);
    gchar *newbase = get_base_name(real_url);
    gtk_html_set_base(html, newbase);
    g_free(newbase);
    
    /* Read the file and store the contents in the source table */
    gchar *buf;
    gsize length;
    if(!g_file_get_contents(real_url, &buf, &length, &err)) {
        error_dialog(NULL, err, "Error opening file %s: ", real_url);
        gtk_html_end(html, handle, GTK_HTML_STREAM_ERROR);
        g_free(url);
        g_free(real_url);
        return;
    }
    source_table_update(html, buf);
    g_free(buf);
    
    /* Do the actual loading of the file into the html widget */
    on_url_requested(html, real_url, handle, data);
    /* Jump to an anchor if there is one */
    if(anchor)
        gtk_html_jump_to_anchor(html, anchor);
    g_free(real_url);
    g_free(url);
}

/* Destructor which removes the entry from the source table */
static void
on_html_destroy(GtkObject *widget, gpointer data)
{
    source_table_remove(GTK_HTML(widget));
}

/* Create a GtkHTML widget and do all the standard stuff that we do to all our
GtkHTML widgets */
GtkWidget *
create_html(gchar *widget_name, gchar *string1, gchar *string2, gint int1, 
            gint int2)
{
    GtkWidget *html = gtk_html_new();
    g_signal_connect(html, "url_requested", G_CALLBACK(on_url_requested), NULL);
    g_signal_connect(html, "link_clicked", G_CALLBACK(on_link_clicked), NULL);
    g_signal_connect(html, "destroy", G_CALLBACK(on_html_destroy), NULL);
    source_table_add(GTK_HTML(html), "");
    update_font_size(html);
    return html;
}

/* Have the html widget display the HTML file in filename */
void 
html_load_file(GtkHTML *html, const gchar *filename) 
{
    g_return_if_fail(html != NULL);
    g_return_if_fail(filename != NULL || strlen(filename));
    
    GError *err = NULL;
    gchar *buf;

    /* Set the base path for relative paths in the HTML file */
    gchar *newbase = get_base_name(filename);
    gtk_html_set_base(html, newbase);
    g_free(newbase);

    /* Open a stream and write the contents of the file to it */
    GtkHTMLStream *stream = gtk_html_begin(html);
    if(g_file_get_contents(filename, &buf, NULL, &err)) {
        source_table_update(html, buf);
        gtk_html_write(html, stream, buf, strlen(buf));
        gtk_html_end(html, stream, GTK_HTML_STREAM_OK);
        g_free(buf);
        return;
    }
    gtk_html_end(html, stream, GTK_HTML_STREAM_ERROR);
    error_dialog(NULL, err, _("Error opening HTML file '%s': "), filename);
}

/* Blank the GtkHTML widget */
void 
html_load_blank(GtkHTML *html) 
{
    g_return_if_fail(html != NULL);
    
    GtkHTMLStream *stream = gtk_html_begin(html);
    gtk_html_end(html, stream, GTK_HTML_STREAM_OK);
    source_table_update(html, "");
}

/* Reload the current page from the "cache" */
void 
html_refresh(GtkHTML *html) 
{
    g_return_if_fail(html != NULL);
    
    GtkHTMLStream *stream = gtk_html_begin(html);
    gchar *buf = source_table_get(html);
    gtk_html_write(html, stream, buf, strlen(buf));  
    gtk_html_end(html, stream, GTK_HTML_STREAM_OK);
}

struct literal {
	const gchar *name;
	int len;
	gchar replace;
};

static struct literal literals[] = {
	{"quot;", 5, '\"'},
	{"nbsp;", 5, ' '},
	{"lt;",   3, '<'},
	{"gt;",   3, '>'}
};

#define NUM_LITERALS (sizeof(literals) / sizeof(literals[0]))

/* Convert HTML text to plain text by removing tags and substituting literals */
gchar *
html_to_plain_text(const gchar *htmltext)
{
    /* Reserve space for the main text */
    int len = strlen(htmltext);
    GString *text = g_string_sized_new(len);

    /* Scan the text, removing markup */
    gboolean white = FALSE;
    const gchar *p1 = htmltext;
    const gchar *p2 = p1 + len;
    while(p1 < p2) {
        /* Look for an HTML tag */
        if(*p1 == '<') {
            if(*(p1 + 1) == '!') {
                p1 = strstr(p1, "-->");
                if(p1 != NULL)
                    p1 += 2;
                else
                    p1 = p2;
            } else {
                while(p1 < p2 && *p1 != '>')
                    p1++;
            }
            g_assert(*p1 == '>');
            
            /* Add a carriage return for appropriate markup */
            if(p1 >= htmltext + 2 && (strncmp(p1, "<p", 2) == 0
               || strncmp(p1, "<P", 2) == 0)) {
                g_string_append_c(text, '\n');
                white = TRUE;
            }
        } else if(*p1 == '&') {
            /* Scan for a known literal */
            gboolean found = FALSE;
            int i = 0;
            while(!found && i < NUM_LITERALS) {
                if(strncmp(p1 + 1, literals[i].name, literals[i].len) == 0)
                    found = TRUE;
                if(!found)
                    i++;
            }
			/* Replace the literal */
            if(found) {
                g_string_append_c(text, literals[i].replace);
                p1 += literals[i].len;
            } else
                g_string_append_c(text, *p1);
            white = FALSE;
        } else if(isspace(*p1)) {
            if(!white)
                g_string_append_c(text, ' ');
            white = TRUE;
        } else {
            g_string_append_c(text, *p1);
            white = FALSE;
        }
        p1++;
    }

    gchar *retval = g_strdup(text->str);
    g_string_free(text, TRUE);
    return retval;
}
