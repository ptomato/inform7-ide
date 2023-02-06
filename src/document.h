/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2008-2015, 2019 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "toast.h"

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
	gboolean (*find_text)();
	void (*set_spellcheck)();
	gboolean (*can_revert)();
	void (*revert)();
} I7DocumentClass;

typedef struct {
	GtkApplicationWindow parent_instance;

	GtkWidget *contents;
	GtkHeaderBar *titlebar;
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
	I7Toast *search_toast;
	/* "Search Files" dialog widgets */
	GtkWidget *search_files_dialog;
	GtkWidget *search_files_type;
	GtkWidget *search_files_entry;
	GtkWidget *search_files_project;
	GtkWidget *search_files_extensions;
	GtkWidget *search_files_documentation;
	GtkWidget *search_files_ignore_case;
	GtkWidget *search_files_find;
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
	I7_SEARCH_CONTAINS       = 0x00,
	I7_SEARCH_STARTS_WORD    = 0x01,
	I7_SEARCH_FULL_WORD      = 0x02,
	I7_SEARCH_ALGORITHM_MASK = 0x03, /* bottom two bits: type of search */
	I7_SEARCH_REVERSE        = 0x04,
	I7_SEARCH_RESTRICT       = 0x08, /* restrict to currently visible section */
	I7_SEARCH_IGNORE_CASE    = 0x10,
} I7SearchFlags;

typedef void (*I7DocumentExtensionDownloadCallback)(gboolean success, const char *id, gpointer);

/* Statusbar Contexts */
#define FILE_OPERATIONS    "File"

GType i7_document_get_type(void) G_GNUC_CONST;
GFile *i7_document_get_file(I7Document *self);
gchar *i7_document_get_display_name(I7Document *self);
void i7_document_set_file(I7Document *self, GFile *file);
GtkSourceBuffer *i7_document_get_buffer(I7Document *self);
GtkTextView *i7_document_get_default_view(I7Document *self);
void i7_document_set_source_text(I7Document *self, const char *text);
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

void i7_document_set_spellcheck(I7Document *self, gboolean spellcheck);
void i7_document_check_spelling(I7Document *self);

void i7_document_download_single_extension_async(I7Document *self, GFile *remote_file, const char *author, const char *title, GAsyncReadyCallback callback, void *data);
bool i7_document_download_single_extension_finish(I7Document *self, GAsyncResult *res);
void i7_document_download_multiple_extensions(I7Document *self, unsigned n_extensions, char * const *ids, GFile **remote_files, char * const *descriptions, I7DocumentExtensionDownloadCallback callback, void *data);

/* Search, document-search.c */
bool i7_document_find_text(I7Document *self, const char *text, I7SearchFlags flags);
void i7_document_unhighlight_quicksearch(I7Document *self);
void i7_document_set_highlighted_view(I7Document *self, GtkWidget *view);
GtkWidget *i7_document_get_highlighted_view(I7Document *self);
void i7_document_set_quicksearch_not_found(I7Document *self, gboolean not_found);
void i7_document_find_in_source(I7Document *self, const char *text, I7SearchFlags flags);

#endif /* _DOCUMENT_H_ */
