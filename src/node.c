#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <goocanvas.h>
#include <cairo.h>

#include "node.h"
#include "skein.h"

#define DIFFERS_BADGE_WIDTH 8.0

enum {
    PROP_0,
    PROP_COMMAND,
    PROP_LABEL,
    PROP_TRANSCRIPT_TEXT,
    PROP_EXPECTED_TEXT,
	PROP_CHANGED,
	PROP_BLESSED,
    PROP_LOCKED,
	PROP_PLAYED,
    PROP_SCORE
};

enum {
	NODE_UNPLAYED_UNBLESSED,
	NODE_UNPLAYED_BLESSED,
	NODE_PLAYED_UNBLESSED,
	NODE_PLAYED_BLESSED,
	NODE_NUM_PATTERNS
};
#define SELECT_PATTERN(played,blessed) (((played? 1:0) << 1) | (blessed? 1:0))

typedef struct _I7NodePrivate {
	gchar *id;
	gchar *command;
    gchar *label;
    gchar *transcript_text;
    gchar *expected_text;
	gboolean changed;
	gboolean blessed;
    gboolean played;
    gboolean locked;
    gint score;
    
    /* Graphical goodness */
   	cairo_pattern_t *label_pattern;
   	cairo_pattern_t *node_pattern[NODE_NUM_PATTERNS];
    GooCanvasItemModel *command_item;
    GooCanvasItemModel *label_item;
    GooCanvasItemModel *badge_item;
    GooCanvasItemModel *command_shape_item;
    GooCanvasItemModel *label_shape_item;

	/* x-coordinate */
	gdouble x;
	
	/* Cached values; initialize to -1 */
	gdouble command_width;
	gdouble command_height;
	gdouble label_width;
	gdouble label_height;
} I7NodePrivate;

#define I7_NODE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), I7_TYPE_NODE, I7NodePrivate))
#define I7_NODE_USE_PRIVATE I7NodePrivate *priv = I7_NODE_PRIVATE(self)

G_DEFINE_TYPE(I7Node, i7_node, GOO_TYPE_CANVAS_GROUP_MODEL);

/* STATIC FUNCTIONS */

static void
update_node_background(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	g_object_set(priv->command_shape_item, 
		"fill-pattern", priv->node_pattern[SELECT_PATTERN(priv->played, priv->blessed)], 
	    NULL);
}

static void
transcript_modified(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	
	gboolean old_changed_status = priv->changed;
	priv->changed = (strcmp(priv->transcript_text? priv->transcript_text : "", 
	                        priv->expected_text? priv->expected_text : "") != 0);
	if(priv->changed != old_changed_status)
		g_object_notify(G_OBJECT(self), "changed");
    
    /* TODO clear diffs */
    
    if(priv->changed && priv->expected_text && strlen(priv->expected_text))
        /* TODO new diffs */;

	update_node_background(self);
}

gboolean
on_node_button_press(GooCanvasItem *item, GooCanvasItem *target_item, GdkEventButton *event, I7Node *self)
{
	I7Skein *skein = I7_SKEIN(goo_canvas_item_model_get_parent(GOO_CANVAS_ITEM_MODEL(self)));
	if(event->type == GDK_2BUTTON_PRESS && event->button == 1) {
		g_signal_emit_by_name(skein, "node-activate", self);
		return TRUE;
	} else if(event->type == GDK_BUTTON_PRESS && event->button == 3) {
		GooCanvas *view = goo_canvas_item_get_canvas(target_item);
		g_signal_emit_by_name(view, "node-menu-popup", self);
		return TRUE;
	}
	return FALSE;
}

gboolean
on_differs_badge_button_press(GooCanvasItem *item, GooCanvasItem *target_item, GdkEventButton *event, GooCanvasItemModel *model)
{
	I7Node *self = I7_NODE(goo_canvas_item_model_get_parent(GOO_CANVAS_ITEM_MODEL(model)));
	I7Skein *skein = I7_SKEIN(goo_canvas_item_model_get_parent(GOO_CANVAS_ITEM_MODEL(self)));
	if(event->type == GDK_2BUTTON_PRESS && event->button == 1) {
		g_signal_emit_by_name(skein, "differs-badge-activate", self);
		return TRUE;
	}
	return FALSE;
}

static void
i7_node_set_expected_text(I7Node *self, const gchar *text)
{
    I7_NODE_USE_PRIVATE;

    g_free(priv->expected_text);
	priv->expected_text = g_strdup(text? text : ""); /* silently accept NULL */
	
	/* Change newline separators to \n */
	if(strstr(priv->expected_text, "\r\n")) {
		gchar **lines = g_strsplit(priv->expected_text, "\r\n", 0);
		g_free(priv->expected_text);
		priv->expected_text = g_strjoinv("\n", lines);
		g_strfreev(lines);
	}
	priv->expected_text = g_strdelimit(priv->expected_text, "\r", '\n');

	if(strlen(priv->expected_text) == 0)
		priv->blessed = FALSE;

	transcript_modified(self);
	
	g_object_notify(G_OBJECT(self), "expected-text");
}

/* GENERAL STATIC FUNCTIONS */

static cairo_pattern_t *
create_node_pattern(double r, double g, double b)
{
	cairo_pattern_t *retval = cairo_pattern_create_radial(0.0, -0.33, 0.0, 0.0, 0.0, 1.0);
	cairo_pattern_add_color_stop_rgb(retval, 0.0, MAX(1.0, r * 1.5), MAX(1.0, g * 1.5), MAX(1.0, b * 1.5));
	cairo_pattern_add_color_stop_rgb(retval, 0.5, r, g, b);
	cairo_pattern_add_color_stop_rgb(retval, 0.75, r * 0.5, g * 0.5, b * 0.5);
	cairo_pattern_add_color_stop_rgb(retval, 1.0, r, g, b);
	return retval;
}

/* TYPE SYSTEM */

static void
i7_node_init(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	priv->id = g_strdup_printf("node-%p", self);
	self->gnode = NULL;
	self->tree_item = NULL;
    
    /* TODO diffs */
    
    /* Create the cairo gradients */
    /* Label */
    priv->label_pattern = cairo_pattern_create_linear(0.0, 0.0, 0.0, -1.0);
    cairo_pattern_add_color_stop_rgba(priv->label_pattern, 0.0, 0.0, 0.33, 0.0, 0.1);
    cairo_pattern_add_color_stop_rgba(priv->label_pattern, 0.33, 0.2, 0.5, 0.2, 0.5);
    cairo_pattern_add_color_stop_rgba(priv->label_pattern, 0.67, 0.73, 0.84, 0.73, 0.5);
    cairo_pattern_add_color_stop_rgba(priv->label_pattern, 1.0, 0.5, 0.85, 0.5, 0.16);
    /* Node, unplayed, without blessed transcript text */
    priv->node_pattern[NODE_UNPLAYED_UNBLESSED] = create_node_pattern(0.26, 0.56, 0.26);
    /* Node, unplayed, with blessed transcript text */
    priv->node_pattern[NODE_UNPLAYED_BLESSED] = create_node_pattern(0.33, 0.7, 0.33);
    /* Node, played, without blessed transcript text */
    priv->node_pattern[NODE_PLAYED_UNBLESSED] = create_node_pattern(0.56, 0.51, 0.17);
    /* Node, played, with blessed transcript text */
    priv->node_pattern[NODE_PLAYED_BLESSED] = create_node_pattern(0.72, 0.64, 0.21);
	
    /* Create the canvas items, though some of them can't be drawn yet */
    priv->command_shape_item = goo_canvas_path_model_new(GOO_CANVAS_ITEM_MODEL(self), "", 
    	"stroke-pattern", NULL,
	    "fill-pattern", priv->node_pattern[NODE_UNPLAYED_UNBLESSED],
    	NULL);
    priv->label_shape_item = goo_canvas_path_model_new(GOO_CANVAS_ITEM_MODEL(self), "", 
    	"stroke-pattern", NULL, 
    	"fill-pattern", priv->label_pattern, 
    	NULL);
    priv->command_item = goo_canvas_text_model_new(GOO_CANVAS_ITEM_MODEL(self), "", 0.0, 0.0, -1, GTK_ANCHOR_CENTER, NULL);
	priv->label_item = goo_canvas_text_model_new(GOO_CANVAS_ITEM_MODEL(self), "", 0.0, 0.0, -1, GTK_ANCHOR_CENTER, NULL);
	priv->badge_item = goo_canvas_path_model_new(GOO_CANVAS_ITEM_MODEL(self), "",
	  "fill-color", "red",
	  "line-width", 0,
	  "visibility", GOO_CANVAS_ITEM_HIDDEN,
	  NULL);
	g_object_set_data(G_OBJECT(priv->badge_item), "path-drawn", NULL);
	g_object_set_data(G_OBJECT(priv->badge_item), "node-part", GINT_TO_POINTER(I7_NODE_PART_DIFFERS_BADGE));
	/* Avoid drawing the differs badges unless they're actually needed, otherwise
	it really slows down the story startup */

	priv->x = 0.0;
	priv->command_width = -1.0;
	priv->command_height = -1.0;
	priv->label_width = -1.0;
	priv->label_height = -1.0;
}

static void
i7_node_set_property(GObject *self, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	I7_NODE_USE_PRIVATE;
	
    switch(prop_id) {
		case PROP_COMMAND:
			i7_node_set_command(I7_NODE(self), g_value_get_string(value));
			break;
		case PROP_LABEL:
			i7_node_set_label(I7_NODE(self), g_value_get_string(value));
			break;
		case PROP_TRANSCRIPT_TEXT:
			i7_node_set_transcript_text(I7_NODE(self), g_value_get_string(value));
			break;
		case PROP_EXPECTED_TEXT: /* Construct only */
			i7_node_set_expected_text(I7_NODE(self), g_value_get_string(value));
			break;
		case PROP_LOCKED:
			i7_node_set_locked(I7_NODE(self), g_value_get_boolean(value));
			break;
		case PROP_PLAYED:
			i7_node_set_played(I7_NODE(self), g_value_get_boolean(value));
			break;
		case PROP_SCORE: /* Construct only */
			priv->score = g_value_get_int(value);
			g_object_notify(self, "score");
			break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
i7_node_get_property(GObject *self, guint prop_id, GValue *value, GParamSpec *pspec)
{
    I7_NODE_USE_PRIVATE;
    
    switch(prop_id) {
		case PROP_COMMAND:
			g_value_set_string(value, priv->command);
			break;
		case PROP_LABEL:
			g_value_set_string(value, priv->label);
			break;
		case PROP_TRANSCRIPT_TEXT:
			g_value_set_string(value, priv->transcript_text);
			break;
		case PROP_EXPECTED_TEXT:
			g_value_set_string(value, priv->expected_text);
			break;
		case PROP_CHANGED:
			g_value_set_boolean(value, priv->changed);
			break;
		case PROP_BLESSED:
			g_value_set_boolean(value, priv->blessed);
			break;
		case PROP_LOCKED:
			g_value_set_boolean(value, priv->locked);
			break;
		case PROP_PLAYED:
			g_value_set_boolean(value, priv->played);
			break;
		case PROP_SCORE:
			g_value_set_int(value, priv->score);
			break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
    }
}

static void
unref_node(GNode *gnode)
{
	g_object_unref(gnode->data);
}

static void
i7_node_finalize(GObject *self)
{
	I7_NODE_USE_PRIVATE;
	
	cairo_pattern_destroy(priv->label_pattern);
	cairo_pattern_destroy(priv->node_pattern[NODE_UNPLAYED_UNBLESSED]);
	cairo_pattern_destroy(priv->node_pattern[NODE_UNPLAYED_BLESSED]);
	cairo_pattern_destroy(priv->node_pattern[NODE_PLAYED_UNBLESSED]);
	cairo_pattern_destroy(priv->node_pattern[NODE_PLAYED_BLESSED]);
	g_free(priv->command);
    g_free(priv->label);
    g_free(priv->transcript_text);
    g_free(priv->expected_text);
	g_free(priv->id);
    
    /* recurse */
	g_node_children_foreach(I7_NODE(self)->gnode, G_TRAVERSE_ALL, (GNodeForeachFunc)unref_node, NULL);
    /* free the node itself */
    g_node_destroy(I7_NODE(self)->gnode);
	
	G_OBJECT_CLASS(i7_node_parent_class)->finalize(self);
}

static void
i7_node_class_init(I7NodeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = i7_node_set_property;
	object_class->get_property = i7_node_get_property;
	object_class->finalize = i7_node_finalize;

	/* Install properties */
	GParamFlags flags = G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB;
	/* SUCKY DEBIAN replace with G_PARAM_STATIC_STRINGS */
	g_object_class_install_property(object_class, PROP_COMMAND,
		g_param_spec_string("command", _("Command"),
		    _("The command entered in the game"),
		    "", flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class, PROP_LABEL,
		g_param_spec_string("label", _("Label"),
		    _("The text this node has been labelled with"),
		    "", flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class, PROP_TRANSCRIPT_TEXT,
	    g_param_spec_string("transcript-text", _("Transcript text"),
		    _("The text produced by the command on the last play-through"),
		    "", flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class, PROP_EXPECTED_TEXT,
	    g_param_spec_string("expected-text", _("Expected text"),
		    _("The text this command should produce"),
		    "", flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class, PROP_CHANGED,
	    g_param_spec_boolean("changed", _("Changed"),
		    _("Whether the transcript text and expected text differ in this node"),
		    FALSE, flags | G_PARAM_READABLE));
	g_object_class_install_property(object_class, PROP_BLESSED,
	    g_param_spec_boolean("blessed", _("Blessed"),
		    _("Whether this node has expected text"),
		    FALSE, flags | G_PARAM_READABLE));
	g_object_class_install_property(object_class, PROP_LOCKED,
	    g_param_spec_boolean("locked", _("Locked"),
		    _("Whether this node is locked"),
		    FALSE, flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class, PROP_PLAYED,
	    g_param_spec_boolean("played", _("Played"),
		    _("Whether this node is in the currently playing thread"),
		    FALSE, flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(object_class, PROP_SCORE,
	    g_param_spec_int("score", _("Score"),
		    _("This node's score, used for cleaning up the skein"),
		    G_MININT16, G_MAXINT16, 0, flags | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	
    /* Private data */
    g_type_class_add_private(klass, sizeof(I7NodePrivate));
}

I7Node *
i7_node_new(const gchar *command, const gchar *label, const gchar *transcript, 
    const gchar *expected, gboolean played, gboolean locked, int score, 
    GooCanvasItemModel *skein)
{
	I7Node *self = g_object_new(I7_TYPE_NODE, 
	    "command", command, 
	    "label", label, 
	    "transcript-text", transcript,
	    "expected-text", expected,
	    "locked", locked,
	    "played", played,
	    "score", score,
	    NULL);
	g_object_set(self, "parent", skein, NULL);
	self->gnode = g_node_new(self);
	return self;
}

gchar *
i7_node_get_command(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return g_strdup(priv->command);
}

void
i7_node_set_command(I7Node *self, const gchar *command)
{
    I7_NODE_USE_PRIVATE;
	g_free(priv->command);
	priv->command = g_strdup(command? command : ""); /* silently accept NULL */

	/* Update the graphics */
	g_object_set(priv->command_item, "text", priv->command, NULL);
	priv->command_width = priv->command_height = -1.0;
	
	g_object_notify(G_OBJECT(self), "command");
}

/* Free after use */
gchar *
i7_node_get_label(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return g_strdup(priv->label);
}

void
i7_node_set_label(I7Node *self, const gchar *label)
{
    I7_NODE_USE_PRIVATE;
    g_free(priv->label);
	priv->label = g_strdup(label? label : ""); /* silently accept NULL */

	/* Update the graphics */
	
	g_object_set(priv->label_item, "text", priv->label, NULL);
	priv->label_width = priv->label_height = -1.0;
	priv->command_width = priv->command_height = -1.0;
	
	g_object_notify(G_OBJECT(self), "label");
}

gboolean
i7_node_has_label(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return (self->gnode->parent != NULL) && priv->label && (strlen(priv->label) > 0);
}

gchar *
i7_node_get_transcript_text(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return g_strdup(priv->transcript_text);
}

void
i7_node_set_transcript_text(I7Node *self, const gchar *transcript)
{
    I7_NODE_USE_PRIVATE;
    g_free(priv->transcript_text);
    priv->transcript_text = g_strdup(transcript? transcript : ""); /* silently accept NULL */

	/* Change newline separators to \n */
	if(strstr(priv->transcript_text, "\r\n")) {
		gchar **lines = g_strsplit(priv->transcript_text, "\r\n", 0);
		g_free(priv->transcript_text);
		priv->transcript_text = g_strjoinv("\n", lines);
		g_strfreev(lines);
	}
	priv->transcript_text = g_strdelimit(priv->transcript_text, "\r", '\n');

	transcript_modified(self);
	
	g_object_notify(G_OBJECT(self), "transcript-text");
}

gchar *
i7_node_get_expected_text(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return g_strdup(priv->expected_text);
}

gboolean
i7_node_get_changed(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return priv->changed;
}

gboolean
i7_node_get_locked(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return priv->locked;
}

void
i7_node_set_locked(I7Node *self, gboolean locked)
{
	I7_NODE_USE_PRIVATE;
    priv->locked = locked;
	g_object_notify(G_OBJECT(self), "locked");
}

gboolean
i7_node_get_played(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	return priv->played;
}

void
i7_node_set_played(I7Node *self, gboolean played)
{
	I7_NODE_USE_PRIVATE;
    priv->played = played;
	update_node_background(self);
	g_object_notify(G_OBJECT(self), "played");
}

gboolean
i7_node_get_blessed(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	return priv->blessed;
}

void
i7_node_bless(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    i7_node_set_expected_text(self, priv->transcript_text);
}

gint i7_node_get_score(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	return priv->score;
}

void i7_node_set_score(I7Node *self, gint score)
{
	I7_NODE_USE_PRIVATE;
	priv->score = score;
	g_object_notify(G_OBJECT(self), "score");
}

gdouble
i7_node_get_tree_width(I7Node *self, GooCanvasItemModel *skein, GooCanvas *canvas)
{
	I7_NODE_USE_PRIVATE;
	gdouble spacing;
	g_object_get(skein, "horizontal-spacing", &spacing, NULL);
	
    /* Get the tree width of all children */
    int i;
    gdouble total = 0.0;
    for(i = 0; i < g_node_n_children(self->gnode); i++) {
        total += i7_node_get_tree_width(g_node_nth_child(self->gnode, i)->data, skein, canvas);
        if(i > 0)
            total += spacing;
    }
	/* Return whichever is larger, that or the node width */
	if(priv->command_width < 0.0)
		i7_node_calculate_size(self, skein, canvas);
    gdouble width = MAX(priv->command_width, priv->label_width);
    return MAX(total, width);
}    

const gchar *
i7_node_get_unique_id(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
    return priv->id;
}

gboolean
i7_node_in_thread(I7Node *self, I7Node *endnode)
{
    return (endnode == self) || g_node_is_ancestor(self->gnode, endnode->gnode);
}

gboolean
i7_node_is_root(I7Node *self)
{
	return self->gnode->parent == NULL;
}

static void
write_child_pointer(GNode *gnode, GString *string)
{
	g_string_append_printf(string, "      <child nodeId=\"%s\"/>\n", I7_NODE_PRIVATE(gnode->data)->id);
}

gchar *
i7_node_get_xml(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	
    /* Escape the following strings if necessary */
    gchar *command = g_markup_escape_text(priv->command, -1);
    gchar *transcript_text = g_markup_escape_text(priv->transcript_text, -1);
    gchar *expected_text = g_markup_escape_text(priv->expected_text, -1);
    gchar *label = g_markup_escape_text(priv->label, -1);

	GString *string = g_string_new("");
	g_string_append_printf(string, "  <item nodeId=\"%s\">\n", priv->id);
	g_string_append_printf(string, "    <command xml:space=\"preserve\">%s</command>\n", command);
	g_string_append_printf(string, "    <result xml:space=\"preserve\">%s</result>\n", transcript_text);
	g_string_append_printf(string, "    <commentary xml:space=\"preserve\">%s</commentary>\n", expected_text);
    g_string_append_printf(string, "    <played>%s</played>\n", priv->played? "YES" : "NO");
	g_string_append_printf(string, "    <changed>%s</changed>\n", priv->changed? "YES" : "NO");
	g_string_append_printf(string, "    <temporary score=\"%d\">%s</temporary>\n", priv->score, priv->locked? "NO" : "YES");
	
    if(label)
        g_string_append_printf(string, "    <annotation xml:space=\"preserve\">%s</annotation>\n", label);
    if(self->gnode->children) {
        g_string_append(string, "    <children>\n");
        g_node_children_foreach(self->gnode, G_TRAVERSE_ALL, (GNodeForeachFunc)write_child_pointer, string);
        g_string_append(string, "    </children>\n");
    }
    g_string_append(string, "  </item>\n");
    /* Free strings if necessary */
    g_free(command);
	g_free(transcript_text);
    g_free(expected_text);
    g_free(label);
	
	return g_string_free(string, FALSE); /* return cstr */
}

gdouble
i7_node_get_x(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	return priv->x;
}

void
i7_node_layout(I7Node *self, GooCanvasItemModel *skein, GooCanvas *canvas, gdouble x)
{
	I7_NODE_USE_PRIVATE;
	
	gdouble hspacing, vspacing;
	g_object_get(skein,
	    "horizontal-spacing", &hspacing,
	    "vertical-spacing", &vspacing,
	    NULL);
    
	if(g_node_n_children(self->gnode) == 1)
		i7_node_layout(self->gnode->children->data, skein, canvas, x);
	else {
		/* Find the total width of all descendant nodes */
    	gdouble total = i7_node_get_tree_width(self, skein, canvas);
		/* Lay out each child node */
		GNode *child; 
		gdouble child_x = 0.0;

		for(child = self->gnode->children; child; child = child->next) {
		    gdouble treewidth = i7_node_get_tree_width(child->data, skein, canvas);
		    i7_node_layout(child->data, skein, canvas, x - total * 0.5 + child_x + treewidth * 0.5);
		    child_x += treewidth + hspacing;
		}
	}

	/* Move the node's group to its proper place */
	gdouble y = (gdouble)(g_node_depth(self->gnode) - 1.0) * vspacing;
	/* SUCKY DEBIAN set_simple_transform -> x,y properties */
	goo_canvas_item_model_set_simple_transform(GOO_CANVAS_ITEM_MODEL(self), x, y, 1.0, 0.0);

	/* Cache the x coordinate */
	priv->x = x;
}

void
i7_node_calculate_size(I7Node *self, GooCanvasItemModel *skein, GooCanvas *canvas)
{
	I7_NODE_USE_PRIVATE;	
	GooCanvasBounds size;
    GooCanvasItem *item;
    gdouble width, height;
	gchar *path;
	cairo_matrix_t matrix;
	gboolean size_changed = FALSE;

	/* Move this node's item models to their proper places, now that we can use
    the canvas to calculate them */
	item = goo_canvas_get_item(canvas, priv->command_item);
    goo_canvas_item_get_bounds(item, &size);
    width = size.x2 - size.x1;
    height = size.y2 - size.y1;

	if((width != 0.0 && priv->command_width != width) || (height != 0.0 && priv->command_height != height)) {
		/* Move the label, its background, and the differs badge */
		/* SUCKY DEBIAN we have to do this with set_simple_transform, because x and 
		y properties don't yet exist */
		goo_canvas_item_model_set_simple_transform(priv->label_item, 0.0, -height, 1.0, 0.0);
		goo_canvas_item_model_set_simple_transform(priv->label_shape_item, 0.0, -height, 1.0, 0.0);
		goo_canvas_item_model_set_simple_transform(priv->badge_item, width / 2 + DIFFERS_BADGE_WIDTH, height / 2, DIFFERS_BADGE_WIDTH, 0.0);
		
		/* Calculate the scale for the pattern gradients */
		cairo_matrix_init_scale(&matrix, 0.5 / width, 1.0 / height);
		cairo_pattern_set_matrix(priv->node_pattern[NODE_UNPLAYED_UNBLESSED], &matrix);
		cairo_pattern_set_matrix(priv->node_pattern[NODE_UNPLAYED_BLESSED], &matrix);
		cairo_pattern_set_matrix(priv->node_pattern[NODE_PLAYED_UNBLESSED], &matrix);
		cairo_pattern_set_matrix(priv->node_pattern[NODE_PLAYED_BLESSED], &matrix);
	
		/* Draw the text background */
		path = g_strdup_printf("M %.1f -%.1f "
			"a %.1f,%.1f 0 0,1 0,%.1f "
			"h -%.1f "
			"a %.1f,%.1f 0 0,1 0,-%.1f "
			"Z",
			width / 2, height / 2, height / 2, height / 2, height, width,
			height / 2, height / 2, height);
		g_object_set(priv->command_shape_item, "data", path, NULL);
		g_free(path);

		/* If the size of the node has changed, we need to relayout the skein */
		size_changed = TRUE;
		priv->command_width = width;
		priv->command_height = height;
	}
	
    /* Draw the label background */
    if(priv->label && *priv->label) {
		item = goo_canvas_get_item(canvas, priv->label_item);
		goo_canvas_item_get_bounds(item, &size);
		width = size.x2 - size.x1;
		height = size.y2 - size.y1;

		if((width != 0.0 && priv->label_width != width) || (height != 0.0 && priv->label_height != height)) {
			path = g_strdup_printf("M %.1f,%.1f "
				"a %.1f,%.1f 0 0,0 -%.1f,-%.1f "
				"h -%.1f "
				"a %.1f,%.1f 0 0,0 -%.1f,%.1f "
				"Z",
				width / 2 + height, height / 2, height, height, height, height,
				width, height, height, height, height);
			cairo_pattern_set_matrix(priv->label_pattern, &matrix);
			g_object_set(priv->label_shape_item, 
				"data", path, 
				"visibility", GOO_CANVAS_ITEM_VISIBLE,
				NULL);
			g_free(path);

			/* Again, if the label size has changed, we need to relayout the skein */
			size_changed = TRUE;
			priv->label_width = width;
			priv->label_height = height;
		}
    } else {
    	g_object_set(priv->label_shape_item, 
    		"data", "", 
    		"visibility", GOO_CANVAS_ITEM_HIDDEN,
    		NULL);
    }
    
    /* Show or hide the differs badge */
    if(priv->changed && priv->expected_text && *priv->expected_text) {
    	if(g_object_get_data(G_OBJECT(priv->badge_item), "path-drawn") == NULL) {
    		/* if the differs badge hasn't been drawn yet, draw it */
    		g_object_set(priv->badge_item, "data",
    		"M 1.0,0.0 0.691,0.112 0.949,0.317 0.62,0.325 0.799,0.601 "
			"0.485,0.505 0.568,0.823 0.3,0.632 0.278,0.961 0.084,0.695 -0.04,0.999 "
			"-0.14,0.686 -0.355,0.935 -0.35,0.606 -0.632,0.775 -0.524,0.464 "
			"-0.845,0.534 -0.644,0.274 -0.971,0.239 -0.698,0.056 -0.997,-0.08 "
			"-0.68,-0.168 -0.92,-0.392 -0.592,-0.374 -0.749,-0.663 -0.443,-0.542 "
			"-0.5,-0.866 -0.248,-0.655 -0.2,-0.98 -0.028,-0.699 0.121,-0.993 "
			"0.195,-0.672 0.429,-0.903 0.398,-0.576 0.693,-0.721 0.56,-0.421 "
			"0.885,-0.465 0.664,-0.222 0.987,-0.16 0.7,-0.0 Z",
			"visibility", GOO_CANVAS_ITEM_VISIBLE,
			NULL);
			/* That SVG code is generated with this Python code:
			import numpy as N
			angles = N.linspace(0, 2 * N.pi, 40)
			radii = N.ones_like(angles)
			radii[1::2] *= 0.7
			xs = radii * N.cos(angles)
			ys = radii * N.sin(angles)
			print "M",
			for x, y in zip(xs, ys):
				print "{0:.3},{1:.3}".format(round(x, 3), round(y, 3)),
			print "Z" */
			g_object_set_data(G_OBJECT(priv->badge_item), "path-drawn", GINT_TO_POINTER(1));
		} else
    		g_object_set(priv->badge_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
    } else
    	g_object_set(priv->badge_item, "visibility", GOO_CANVAS_ITEM_HIDDEN, NULL);

	if(size_changed)
		g_signal_emit_by_name(skein, "needs-layout");
}

void 
i7_node_invalidate_size(I7Node *self)
{
	I7_NODE_USE_PRIVATE;
	priv->command_width = -1.0;
	priv->command_height = -1.0;
	priv->label_width = -1.0;
	priv->label_height = -1.0;
}

static gboolean
i7_goo_canvas_item_get_onscreen_coordinates(GooCanvasItem *item, GooCanvas *canvas, gint *x, gint *y)
{
	GooCanvasBounds bounds;
	GtkAllocation allocation;
	gdouble canvas_x, canvas_y;
	gdouble top, bottom, left, right, item_x, item_y;

	/* Find out the size and coordinates of the current viewport */
	goo_canvas_get_bounds(canvas, &canvas_x, &canvas_y, NULL, NULL);
	GtkWidget *scrolled_window = gtk_widget_get_parent(GTK_WIDGET(canvas));
	g_assert(GTK_IS_SCROLLED_WINDOW(scrolled_window));
	GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
	left = canvas_x + gtk_adjustment_get_value(adj);
	right = left + gtk_adjustment_get_page_size(adj);
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
	top = canvas_y + gtk_adjustment_get_value(adj);
	bottom = top + gtk_adjustment_get_page_size(adj);

	/* Make sure item is currently displayed */
	goo_canvas_item_get_bounds(item, &bounds);
	if(bounds.x1 > right || bounds.x2 < left || bounds.y1 > bottom || bounds.y2 < top) {
		g_warning("Node not onscreen in canvas");
		return FALSE;
	}

	/* Find out the onscreen coordinates of the canvas viewport */
	gtk_widget_get_allocation(GTK_WIDGET(canvas), &allocation);
		
	if(x) {
		item_x = bounds.x1;
		*x = (gint)(item_x - left) + allocation.x;
	}
	if(y) {
		item_y = bounds.y1;
		*y = (gint)(item_y - top) + allocation.y;
	}
	return TRUE;
}

gboolean
i7_node_get_command_coordinates(I7Node *self, gint *x, gint *y, GooCanvas *canvas)
{
	g_return_val_if_fail(self || I7_IS_NODE(self), FALSE);
	g_return_val_if_fail(canvas || GOO_IS_CANVAS(canvas), FALSE);

	I7_NODE_USE_PRIVATE;

	return i7_goo_canvas_item_get_onscreen_coordinates(goo_canvas_get_item(canvas, GOO_CANVAS_ITEM_MODEL(priv->command_item)), canvas, x, y);
}

gboolean
i7_node_get_label_coordinates(I7Node *self, gint *x, gint *y, GooCanvas *canvas)
{
	g_return_val_if_fail(self || I7_IS_NODE(self), FALSE);
	g_return_val_if_fail(canvas || GOO_IS_CANVAS(canvas), FALSE);

	I7_NODE_USE_PRIVATE;

	return i7_goo_canvas_item_get_onscreen_coordinates(goo_canvas_get_item(canvas, GOO_CANVAS_ITEM_MODEL(priv->label_item)), canvas, x, y);
}
