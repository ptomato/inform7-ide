#ifndef __PLACEHOLDER_ENTRY_H__
#define __PLACEHOLDER_ENTRY_H__

#include <glib-object.h>

#define I7_TYPE_PLACEHOLDER_ENTRY         (i7_placeholder_entry_get_type())
#define I7_PLACEHOLDER_ENTRY(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), I7_TYPE_PLACEHOLDER_ENTRY, I7PlaceholderEntry))
#define I7_IS_PLACEHOLDER_ENTRY(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), I7_TYPE_PLACEHOLDER_ENTRY))
#define I7_PLACEHOLDER_ENTRY_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), I7_TYPE_PLACEHOLDER_ENTRY, I7PlaceholderEntryClass))
#define I7_IS_PLACEHOLDER_ENTRY_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), I7_TYPE_PLACEHOLDER_ENTRY))
#define I7_PLACEHOLDER_ENTRY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), I7_TYPE_PLACEHOLDER_ENTRY, I7PlaceholderEntryClass))

typedef struct _I7PlaceholderEntry I7PlaceholderEntry;
typedef struct _I7PlaceholderEntryClass I7PlaceholderEntryClass;

struct _I7PlaceholderEntry {
	GtkEntry parent_instance;
};

struct _I7PlaceholderEntryClass {
	GtkEntryClass parent_class;
};

GType i7_placeholder_entry_get_type(void) G_GNUC_CONST;
GtkWidget *i7_placeholder_entry_new(const gchar *placeholder_text);
void i7_placeholder_entry_set_placeholder_text(I7PlaceholderEntry *self, const gchar *placeholder_text);
gchar *i7_placeholder_entry_get_placeholder_text(I7PlaceholderEntry *self);
G_CONST_RETURN const gchar *i7_placeholder_entry_get_text(I7PlaceholderEntry *self);

#endif /* __PLACEHOLDER_ENTRY_H__ */
