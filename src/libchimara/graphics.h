#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glib.h>
#include <gtk/gtk.h>

#include "glk.h"
#include "gi_blorb.h"
#include "resource.h"
#include "window.h"
#include "style.h"
#include "strio.h"

#define IMAGE_CACHE_MAX_NUM 10
#define IMAGE_CACHE_MAX_SIZE 5242880

struct image_info {
	guint32 resource_number;
	gint width, height;
	GdkPixbuf* pixbuf;
	gboolean scaled;
};

void on_graphics_size_allocate(GtkWidget *widget, GtkAllocation *allocation, winid_t win);
void clear_image_cache(struct image_info *data, gpointer user_data);

#endif
