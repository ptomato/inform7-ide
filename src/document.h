/* Copyright (C) 2008–2015, 2019 P. F. Chimento
 * This file is part of GNOME Inform 7.
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

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#define I7_TYPE_DOCUMENT            (i7_document_get_type())
#define I7_DOCUMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_DOCUMENT, I7Document))
#define I7_DOCUMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_DOCUMENT, I7DocumentClass))
#define I7_IS_DOCUMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_DOCUMENT))
#define I7_IS_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_DOCUMENT))
#define I7_DOCUMENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_DOCUMENT, I7DocumentClass))

typedef struct {
	GtkApplicationWindowClass parent_class;

	/* Private pure virtual */
	gchar * (*extract_title)();
	void (*set_contents_display)();
	/* Public pure virtual */
	GtkTextView * (*get_default_view)();
	gboolean (*save)();
	void (*save_as)();
	GFile *(*run_save_dialog)();
	void (*scroll_to_selection)();
	void (*update_tabs)();
	void (*update_fonts)();
	void (*update_font_sizes)();
	void (*expand_headings_view)();
	gboolean (*highlight_search)();
	void (*set_spellcheck)();
	void (*check_spelling)();
	void (*set_elastic_tabstops)();
	gboolean (*can_revert)();
	void (*revert)();
} I7DocumentClass;

typedef struct {
	GtkApplicationWindow parent_instance;

	GtkWidget *box;
	GtkWidget *toolbar;
	GtkWidget *statusline;
	GtkWidget *statusbar;
	GtkWidget *progressbar;
	GtkWidget *findbar;
	GtkWidget *findbar_entry;
	/* "Find and Replace" dialog widgets */
	GtkWidget *find_dialog;
	GtkWidget *search_type;
	GtkWidget *find_entry;
	GtkWidget *replace_entry;
	GtkWidget *ignore_case;
	GtkWidget *reverse;
	GtkWidget *restrict_search;
	GtkWidget *find_button;
	GtkWidget *replace_button;
	GtkWidget *replace_all_button;
	/* "Search Files" dialog widgets */
	GtkWidget *search_files_dialog;
	GtkWidget *search_files_type;
	GtkWidget *search_files_entry;
	GtkWidget *search_files_project;
	GtkWidget *search_files_extensions;
	GtkWidget *search_files_documentation;
	GtkWidget *search_files_ignore_case;
	GtkWidget *search_files_find;
	/* "Multi Download" dialog widgets */
	GtkWidget *multi_download_dialog;
	GtkWidget *download_label;
	GtkWidget *download_progress;
} I7Document;

typedef enum  {
	I7_HEADINGS_TITLE,
	I7_HEADINGS_LINE,
	I7_HEADINGS_DEPTH,
	I7_HEADINGS_SECTION_NUMBER,
	I7_HEADINGS_SECTION_NAME,
	I7_HEADINGS_BOLD,
	I7_HEADINGS_NUM_COLUMNS
} I7HeadingsColumns;

typedef enum  {
	I7_HEADING_NONE = -1,
	I7_HEADING_VOLUME,
	I7_HEADING_BOOK,
	I7_HEADING_PART,
	I7_HEADING_CHAPTER,
	I7_HEADING_SECTION
} I7Heading;

typedef enum {
	I7_SEARCH_CONTAINS,
	I7_SEARCH_STARTS_WORD,
	I7_SEARCH_FULL_WORD
} I7SearchType;

typedef void (*I7DocumentExtensionDownloadCallback)(gboolean success, const char *id, gpointer);

/* Statusbar Contexts */
#define FILE_OPERATIONS    "File"
#define PRINT_OPERATIONS   "Print"
#define SEARCH_OPERATIONS  "Search"
#define COMPILE_OPERATIONS "Compile"
#define INDEX_TABS         "Index"

GType i7_document_get_type(void) G_GNUC_CONST;
GFile *i7_document_get_file(I7Document *self);
gchar *i7_document_get_display_name(I7Document *self);
void i7_document_set_file(I7Document *self, GFile *file);
GtkSourceBuffer *i7_document_get_buffer(I7Document *self);
GtkTextView *i7_document_get_default_view(I7Document *self);
void i7_document_set_source_text(I7Document *self, gchar *text);
gchar *i7_document_get_source_text(I7Document *self);
gboolean i7_document_get_modified(I7Document *self);
void i7_document_set_modified(I7Document *self, gboolean modified);
GtkTreeModel *i7_document_get_headings(I7Document *self);
GtkTreePath *i7_document_get_child_path(I7Document *self, GtkTreePath *path);

void i7_document_monitor_file(I7Document *self, GFile *file);
void i7_document_stop_file_monitor(I7Document *self);
gboolean i7_document_save(I7Document *self);
void i7_document_save_as(I7Document *self, GFile *file);
GFile *i7_document_run_save_dialog(I7Document *self, GFile *default_file);
gboolean i7_document_verify_save(I7Document *self);
gboolean i7_document_can_revert(I7Document *self);
void i7_document_revert(I7Document *self);
void i7_document_close(I7Document *self);
void i7_document_scroll_to_selection(I7Document *self);
void i7_document_jump_to_line(I7Document *self, guint lineno);

void i7_document_update_tabs(I7Document *self);
void i7_document_update_fonts(I7Document *self);
void i7_document_update_font_sizes(I7Document *self);
void i7_document_update_font_styles(I7Document *self);
void i7_document_refresh_elastic_tabstops(I7Document *self);
void i7_document_update_indent_tags(I7Document *self, GtkTextIter *orig_start, GtkTextIter *orig_end);
gboolean i7_document_iter_is_invisible(I7Document *self, GtkTextIter *iter);

void i7_document_expand_headings_view(I7Document *self);
void i7_document_set_headings_filter_level(I7Document *self, gint depth);
void i7_document_reindex_headings(I7Document *self);
void i7_document_show_heading(I7Document *self, GtkTreePath *path);
GtkTreePath *i7_document_get_previous_heading(I7Document *self);
GtkTreePath *i7_document_get_next_heading(I7Document *self);
GtkTreePath *i7_document_get_shallower_heading(I7Document *self);
GtkTreePath *i7_document_get_deeper_heading(I7Document *self);
GtkTreePath *i7_document_get_deepest_heading(I7Document *self);
void i7_document_show_entire_source(I7Document *self);

void i7_document_display_status_message(I7Document *self, const char *message, const char *context);
void i7_document_remove_status_message(I7Document *self, const char *context);
void i7_document_flash_status_message(I7Document *self, const char *message, const char *context);
void i7_document_display_progress_busy(I7Document *self);
void i7_document_display_progress_percentage(I7Document *self, double fraction);
void i7_document_display_progress_message(I7Document *self, const char *message);
void i7_document_clear_progress(I7Document *self);

void i7_document_set_spellcheck(I7Document *self, gboolean spellcheck);
void i7_document_check_spelling(I7Document *self);
void i7_document_set_elastic_tabstops(I7Document *self, gboolean elastic);

gboolean i7_document_download_single_extension(I7Document *self, GFile *remote_file, const char *author, const char *title);
void i7_document_download_multiple_extensions(I7Document *self, unsigned n_extensions, char * const *ids, GFile **remote_files, char * const *authors, char * const *titles, char * const *versions, I7DocumentExtensionDownloadCallback callback, void *data);

/* Search, document-search.c */
gboolean i7_document_highlight_quicksearch(I7Document *self, const char *text, gboolean forward);
void i7_document_unhighlight_quicksearch(I7Document *self);
void i7_document_set_highlighted_view(I7Document *self, GtkWidget *view);
GtkWidget *i7_document_get_highlighted_view(I7Document *self);
void i7_document_set_quicksearch_not_found(I7Document *self, gboolean not_found);
void i7_document_find(I7Document *self, const char *text, gboolean forward, gboolean ignore_case, gboolean restrict_search, I7SearchType search_type);

#endif /* _DOCUMENT_H_ */
