#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "placeholder-entry.h"

enum {
	PROP_0,
	PROP_PLACEHOLDER_TEXT
};

typedef struct _I7PlaceholderEntryPrivate {
	gchar *placeholder;
	gboolean is_default;
} I7PlaceholderEntryPrivate;

#define I7_PLACEHOLDER_ENTRY_PRIVATE(o) G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_PLACEHOLDER_ENTRY, I7PlaceholderEntryPrivate)
#define I7_PLACEHOLDER_ENTRY_USE_PRIVATE(o,n) I7PlaceholderEntryPrivate *n = I7_PLACEHOLDER_ENTRY_PRIVATE(o) 

G_DEFINE_TYPE(I7PlaceholderEntry, i7_placeholder_entry, GTK_TYPE_ENTRY);

static void
set_normal(GtkWidget *self)
{
	GdkColor black;
	gtk_entry_set_text(GTK_ENTRY(self), "");
	gdk_color_parse("black", &black);
	gtk_widget_modify_text(self, GTK_STATE_NORMAL, &black);
}

static void
set_placeholder(GtkWidget *self)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(self, priv);
	GdkColor gray;
	gtk_entry_set_text(GTK_ENTRY(self), priv->placeholder);
	gdk_color_parse("gray", &gray);
	gtk_widget_modify_text(self, GTK_STATE_NORMAL, &gray);
}

static gboolean
on_focus_in_event(GtkWidget *widget, GdkEventFocus *event)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(widget, priv);
	if(priv->is_default)
		set_normal(widget);
	return FALSE; /* Propagate event further */
}

static gboolean
on_focus_out_event(GtkWidget *widget, GdkEventFocus *event)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(widget, priv);
	
	if(strcmp(gtk_entry_get_text(GTK_ENTRY(widget)), "") == 0) {
		set_placeholder(widget);
		priv->is_default = TRUE;
	} else
		priv->is_default = FALSE;
	return FALSE; /* Propagate event further */
}

static void
i7_placeholder_entry_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(object, priv);
	
	switch(prop_id) {
		case PROP_PLACEHOLDER_TEXT:
			priv->placeholder = g_strdup(g_value_get_string(value));
			if(priv->is_default)
				gtk_entry_set_text(GTK_ENTRY(object), priv->placeholder);
			g_object_notify(object, "placeholder-text");
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
i7_placeholder_entry_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(object, priv);

	switch(prop_id) {
		case PROP_PLACEHOLDER_TEXT:
			g_value_set_string(value, g_strdup(priv->placeholder));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
i7_placeholder_entry_finalize(GObject *self)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(self, priv);
	g_free(priv->placeholder);
	
	/* Chain up */
	G_OBJECT_CLASS(i7_placeholder_entry_parent_class)->finalize(self);
}

static void
i7_placeholder_entry_class_init(I7PlaceholderEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = i7_placeholder_entry_set_property;
	object_class->get_property = i7_placeholder_entry_get_property;
	object_class->finalize = i7_placeholder_entry_finalize;
	
	/* Install properties */
	GParamSpec *pspec = g_param_spec_string("placeholder-text",  
		_("Placeholder text"), 
		_("Text to use as a placeholder when no text has been entered"),
		"", G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(object_class, PROP_PLACEHOLDER_TEXT, pspec);
	
	g_type_class_add_private(klass, sizeof(I7PlaceholderEntryPrivate));
}

static void
i7_placeholder_entry_init(I7PlaceholderEntry *self)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(self, priv);
	priv->placeholder = NULL;
	priv->is_default = TRUE;
	
	g_signal_connect(self, "focus-in-event", G_CALLBACK(on_focus_in_event), NULL);
	g_signal_connect(self, "focus-out-event", G_CALLBACK(on_focus_out_event), NULL);
}

GtkWidget *
i7_placeholder_entry_new(const gchar *placeholder_text)
{
	GtkWidget *retval = (GtkWidget *)g_object_new(I7_TYPE_PLACEHOLDER_ENTRY, 
		"placeholder-text", placeholder_text, NULL);
	set_placeholder(retval);
	return retval;
}

G_CONST_RETURN const gchar *
i7_placeholder_entry_get_text(I7PlaceholderEntry *self)
{
	I7_PLACEHOLDER_ENTRY_USE_PRIVATE(self, priv);
	if(priv->is_default)
		return NULL;
	return gtk_entry_get_text(GTK_ENTRY(self));
}
