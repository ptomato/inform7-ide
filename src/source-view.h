#ifndef _SOURCE_VIEW_H_
#define _SOURCE_VIEW_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#define I7_TYPE_SOURCE_VIEW             (i7_source_view_get_type())
#define I7_SOURCE_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_SOURCE_VIEW, I7SourceView))
#define I7_SOURCE_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_SOURCE_VIEW, I7SourceView))
#define I7_IS_SOURCE_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_SOURCE_VIEW))
#define I7_IS_SOURCE_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_SOURCE_VIEW))
#define I7_SOURCE_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_SOURCE_VIEW, I7SourceViewClass))

/* The names of the sub tabs in "Source" */
typedef enum {
	I7_SOURCE_VIEW_TAB_CONTENTS = 0,
	I7_SOURCE_VIEW_TAB_SOURCE,
	I7_SOURCE_VIEW_NUM_TABS
} I7SourceViewTab;

typedef enum {
	I7_CONTENTS_NORMAL = 0,
	I7_CONTENTS_TOO_SHALLOW,
	I7_CONTENTS_NO_HEADINGS
} I7ContentsDisplay;

/* Different than I7Heading (in document.h) -- these are floats corresponding to
 the slider values */
#define I7_DEPTH_VOLUMES_ONLY        0.0
#define I7_DEPTH_VOLUMES_AND_BOOKS   1.0
#define I7_DEPTH_PARTS_AND_HIGHER    2.0
#define I7_DEPTH_CHAPTERS_AND_HIGHER 3.0
#define I7_DEPTH_ALL_HEADINGS        4.0

typedef struct {
	GtkFrame parent_instance;
	/* Public pointers to widgets for convenience */
	GtkWidget *notebook;
	GtkWidget *source;
	GtkWidget *headings;
	GtkWidget *heading_depth;
	GtkWidget *message;
	GtkWidget *previous;
	GtkWidget *next;
} I7SourceView;

typedef struct {
	GtkFrameClass parent_class;
} I7SourceViewClass;

GType i7_source_view_get_type() G_GNUC_CONST;
GtkWidget *i7_source_view_new();
void i7_source_view_set_contents_display(I7SourceView *self, I7ContentsDisplay display);
void i7_source_view_jump_to_line(I7SourceView *self, guint line);
void i7_source_view_set_spellcheck(I7SourceView *self, gboolean spellcheck);
void i7_source_view_check_spelling(I7SourceView *self);
void i7_source_view_set_elastic_tabs(I7SourceView *self, gboolean elastic);

#endif /* _SOURCE_VIEW_H_ */
