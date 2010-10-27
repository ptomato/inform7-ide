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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include "error.h"
#include "prefs.h"
#include "skein.h"

#define SKEIN_FILE "Skein.skein"
#define max(a, b) (((a) > (b))? (a) : (b))
#define MIN_TEXT_WIDTH 25.0

typedef struct _SkeinPrivate SkeinPrivate;
struct _SkeinPrivate
{
	GNode * root;
	GNode * current;
	GNode * played;
	gboolean layout;
    gboolean modified;
};

#define I7_SKEIN_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), I7_TYPE_SKEIN, SkeinPrivate))

enum
{
	TREE_CHANGED,
	THREAD_CHANGED,
	NODE_TEXT_CHANGED,
	NODE_COLOR_CHANGED,
	LOCK_CHANGED,
	TRANSCRIPT_THREAD_CHANGED,
	SHOW_NODE,

	LAST_SIGNAL
};

static GObjectClass* parent_class = NULL;
static guint skein_signals[LAST_SIGNAL] = { 0 };

static void
skein_init(Skein *object)
{
    I7_SKEIN_PRIVATE(object)->root = node_create(_("- start -"), "", "", "", 
	  FALSE, FALSE, FALSE, 0);
    I7_SKEIN_PRIVATE(object)->current = I7_SKEIN_PRIVATE(object)->root;
    I7_SKEIN_PRIVATE(object)->played = I7_SKEIN_PRIVATE(object)->root;
    I7_SKEIN_PRIVATE(object)->layout = FALSE;
    I7_SKEIN_PRIVATE(object)->modified = TRUE;
}

static void
skein_finalize(GObject *object)
{
	G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
skein_class_init(SkeinClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));

	g_type_class_add_private (klass, sizeof (SkeinPrivate));

	object_class->finalize = skein_finalize;

	skein_signals[TREE_CHANGED] =
		g_signal_new ("tree-changed",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_NO_RECURSE,
		              G_STRUCT_OFFSET (SkeinClass, tree_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	skein_signals[THREAD_CHANGED] =
		g_signal_new ("thread-changed",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_NO_RECURSE,
		              G_STRUCT_OFFSET (SkeinClass, thread_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	skein_signals[NODE_TEXT_CHANGED] =
		g_signal_new ("node-text-changed",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_NO_RECURSE,
		              G_STRUCT_OFFSET (SkeinClass, node_text_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	skein_signals[NODE_COLOR_CHANGED] =
		g_signal_new ("node-color-changed",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_NO_RECURSE,
		              G_STRUCT_OFFSET (SkeinClass, node_color_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	skein_signals[LOCK_CHANGED] =
		g_signal_new ("lock-changed",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_NO_RECURSE,
		              G_STRUCT_OFFSET (SkeinClass, lock_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	skein_signals[TRANSCRIPT_THREAD_CHANGED] =
		g_signal_new ("transcript-thread-changed",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_NO_RECURSE,
		              G_STRUCT_OFFSET (SkeinClass, transcript_thread_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	skein_signals[SHOW_NODE] =
		g_signal_new ("show-node",
		              G_OBJECT_CLASS_TYPE (klass),
		              0,
		              G_STRUCT_OFFSET (SkeinClass, show_node),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__UINT_POINTER,
		              G_TYPE_NONE, 2,
		              G_TYPE_UINT, G_TYPE_POINTER);
}

GType
skein_get_type(void)
{
	static GType our_type = 0;

	if(our_type == 0)
	{
		static const GTypeInfo our_info =
		{
			sizeof (SkeinClass), /* class_size */
			(GBaseInitFunc) NULL, /* base_init */
			(GBaseFinalizeFunc) NULL, /* base_finalize */
			(GClassInitFunc) skein_class_init, /* class_init */
			(GClassFinalizeFunc) NULL, /* class_finalize */
			NULL /* class_data */,
			sizeof (Skein), /* instance_size */
			0, /* n_preallocs */
			(GInstanceInitFunc) skein_init, /* instance_init */
			NULL /* value_table */
		};

		our_type = g_type_register_static (G_TYPE_OBJECT, "Skein",
		                                   &our_info, 0);
	}

	return our_type;
}

Skein *
skein_new(void)
{
	return g_object_new(I7_TYPE_SKEIN, NULL);
}

void
skein_free(Skein *skein)
{
    node_destroy(I7_SKEIN_PRIVATE(skein)->root);
    skein_finalize(G_OBJECT(skein));
}

GNode *
skein_get_root_node(Skein *skein)
{
    return I7_SKEIN_PRIVATE(skein)->root;
}

GNode *
skein_get_current_node(Skein *skein)
{
    return I7_SKEIN_PRIVATE(skein)->current;
}

void
skein_set_current_node(Skein *skein, GNode *node)
{
    I7_SKEIN_PRIVATE(skein)->current = node;
    g_signal_emit(skein, skein_signals[THREAD_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

gboolean
in_current_thread(Skein *skein, GNode *node)
{
    return node_in_thread(node, I7_SKEIN_PRIVATE(skein)->current);
}

GNode *
skein_get_played_node(Skein *skein)
{
    return I7_SKEIN_PRIVATE(skein)->played;
}

/* Search an xmlNode's properties for a certain one and return its content.
String must be freed. Return NULL if not found */
static gchar *
get_property_from_node(xmlNode *node, gchar *propertyname)
{
    xmlAttr *props;
    for(props = node->properties; props; props = props->next)
        if(strcmp((gchar *)props->name, propertyname) == 0)
            return g_strdup((gchar *)props->children->content);
    return NULL;
}

/* Read the ObjectiveC "YES" and "NO" into a boolean */
static gboolean
get_boolean_from_content(xmlNode *node, gboolean default_val)
{
    if(strcmp((gchar *)node->children->content, "YES") == 0)
        return TRUE;
    else if(strcmp((gchar *)node->children->content, "NO") == 0)
        return FALSE;
    else
        return default_val;
}

static gchar *
get_text_content_from_node(xmlNode *node)
{
    if(node->children)
        if(strcmp((gchar *)node->children->name, "text") == 0)
            return g_strdup((gchar *)node->children->content);
    return NULL;
}

void
skein_load(Skein *skein, const gchar *path)
{
    gchar *filename = g_build_filename(path, SKEIN_FILE, NULL);
    xmlDoc *xmldoc = xmlParseFile(filename);
    g_free(filename);
    
    if(!xmldoc) {
        error_dialog(NULL, NULL, 
		  _("This project's Skein was not found, or it was unreadable."));
        return;
    }
    
    /* Get the top XML node */
    xmlNode *top = xmlDocGetRootElement(xmldoc);
    if(strcmp((gchar *)top->name, "Skein") != 0) {
        error_dialog(NULL, NULL, _("This project's Skein was unreadable."));
        xmlFreeDoc(xmldoc);
        return;
    }
    
    /* Get the ID of the root node */
    gchar *root_id = get_property_from_node(top, "rootNode");
    if(!root_id) {
        error_dialog(NULL, NULL, _("This project's Skein was unreadable."));
        xmlFreeDoc(xmldoc);
        return;
    }

    /* Get all the item nodes and the ID of the active node */
    GHashTable *nodetable = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                  g_free, NULL);
    gchar *active_id = NULL;
    xmlNode *item;
    /* Create a node object for each of the XML item nodes */
    for(item = top->children; item; item = item->next) {
        if(strcmp((gchar *)item->name, "activeNode") == 0)
            active_id = get_property_from_node(item, "nodeId");
        else if(strcmp((gchar *)item->name, "item") == 0) {
            gchar *id, *command = NULL, *label = NULL;
            gchar *result = NULL, *commentary = NULL;
            gboolean played = FALSE, changed = FALSE, temp = TRUE;
            int score = 0;
            
            xmlNode *child;
            for(child = item->children; child; child = child->next) {
                if(strcmp((gchar *)child->name, "command") == 0)
                    command = get_text_content_from_node(child);
                else if(strcmp((gchar *)child->name, "annotation") == 0)
                    label = get_text_content_from_node(child);
                else if(strcmp((gchar *)child->name, "result") == 0)
                    result = get_text_content_from_node(child);
                else if(strcmp((gchar *)child->name, "commentary") == 0)
                    commentary = get_text_content_from_node(child);
                else if(strcmp((gchar *)child->name, "played") == 0)
                    played = get_boolean_from_content(child, FALSE);
                else if(strcmp((gchar *)child->name, "changed") == 0)
                    changed = get_boolean_from_content(child, FALSE);
                else if(strcmp((gchar *)child->name, "temporary") == 0) {
                    temp = get_boolean_from_content(child, TRUE);
                    gchar *trash = get_property_from_node(child, "score");
                    sscanf(trash, "%d", &score);
                    if(trash) g_free(trash);
                }
            }
            id = get_property_from_node(item, "nodeId"); /* freed by table */
                
            GNode *skein_node = node_create(command, label, result, commentary,
                                            played, changed, temp, score);
            g_hash_table_insert(nodetable, id, skein_node);
            if(command)    g_free(command);
            if(label)      g_free(label);
            if(result)     g_free(result);
            if(commentary) g_free(commentary);
        }
    }
    
    /* Loop through the item nodes again, adding the children to each parent */
    for(item = top->children; item; item = item->next) {
        if(strcmp((gchar *)item->name, "item") != 0)
            continue;
        gchar *id = get_property_from_node(item, "nodeId");
        GNode *parent_node = g_hash_table_lookup(nodetable, id);
        xmlNode *child;
        for(child = item->children; child; child = child->next) {
            if(strcmp((gchar *)child->name, "children") != 0)
                continue;
            xmlNode *list;
            for(list = child->children; list; list = list->next) {
                if(strcmp((gchar *)list->name, "child") != 0)
                    continue;
                gchar *child_id = get_property_from_node(list, "nodeId");
                g_node_append(parent_node, 
                              (GNode *)g_hash_table_lookup(nodetable,child_id));
                g_free(child_id);
            }
        }
        g_free(id);
    }

    /* Discard the current skein and replace with the new */
    node_destroy(I7_SKEIN_PRIVATE(skein)->root);
    I7_SKEIN_PRIVATE(skein)->root = (GNode *)g_hash_table_lookup(nodetable,
                                                                 root_id);
    I7_SKEIN_PRIVATE(skein)->current = (GNode *)g_hash_table_lookup(nodetable,
                                                                    active_id);
    I7_SKEIN_PRIVATE(skein)->played = I7_SKEIN_PRIVATE(skein)->root;
    I7_SKEIN_PRIVATE(skein)->layout = FALSE;
    g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = FALSE;
    
    g_free(root_id);
    g_free(active_id);    
    g_hash_table_destroy(nodetable);
    xmlFreeDoc(xmldoc);
}

/* Hacked up routine to remove &, < and > from text destined for XML strings */
static gchar *
escape_xml(const gchar *string)
{
    /* Check whether we have to go through this */
    if(!string)
        return NULL;
    if(!(strchr(string, '&') || strchr(string, '<') || strchr(string, '>')))
        return g_strdup(string);
	
    /* Reserve space for the main text */
    int len = strlen(string);
    GString *text = g_string_sized_new(5 * len);

    /* Scan the text, replacing offending characters */
    const gchar *p1 = string;
    const gchar *p2 = p1 + len;
    while(p1 < p2) {
        switch(*p1) {
        case '&':
            g_string_append(text, "&amp;");
            break;
        case '<':
            g_string_append(text, "&lt;");
            break;
        case '>':
			g_string_append(text, "&gt;");
            break;
        default:
            g_string_append_c(text, *p1);
        }
        p1++;
    }

    gchar *retval = g_strdup(text->str);
    g_string_free(text, TRUE);
    return retval;
}

static gboolean
node_write_xml(GNode *node, FILE *fp)
{
    NodeData *data = (NodeData *)node->data;
    /* Escape the following strings if necessary */
    gchar *line = escape_xml(data->line);
    gchar *text_transcript = escape_xml(data->text_transcript);
    gchar *text_expected = escape_xml(data->text_expected);
    gchar *label = escape_xml(data->label);
    
    fprintf(fp,
            "  <item nodeId=\"%s\">\n"
            "    <command xml:space=\"preserve\">%s</command>\n"
            "    <result xml:space=\"preserve\">%s</result>\n"
            "    <commentary xml:space=\"preserve\">%s</commentary>\n"
            "    <played>%s</played>\n"
            "    <changed>%s</changed>\n"
            "    <temporary score=\"%d\">%s</temporary>\n",
            data->id? data->id : "",
            line? line : "",
            text_transcript? text_transcript : "",
            text_expected? text_expected : "",
            data->played? "YES" : "NO",
            data->changed? "YES" : "NO",
            data->score,
            data->temp? "YES" : "NO");
    if(label && strlen(label))
        fprintf(fp, "    <annotation xml:space=\"preserve\">%s</annotation>\n",
                data->label);
    if(node->children) {
        fprintf(fp, "    <children>\n");
        GNode *child;
        for(child = node->children; child; child = child->next)
            fprintf(fp, "      <child nodeId=\"%s\"/>\n",
                    ((NodeData *)child->data)->id);
        fprintf(fp, "    </children>\n");
    }
    fprintf(fp, "  </item>\n");
    /* Free strings if necessary */
    if(line) 
        g_free(line);
    if(text_transcript)
        g_free(text_transcript);
    if(text_expected)
        g_free(text_expected);
    if(label)
        g_free(label);
    return FALSE; /* Do not stop the traversal */
}
    
void
skein_save(Skein *skein, const gchar *path)
{
    GError *err = NULL;
    gchar *filename = g_build_filename(path, SKEIN_FILE, NULL);
    FILE *skeinfile = fopen(filename, "w");
    g_free(filename);
    if(!skeinfile) {
        error_dialog(NULL, err, _("Error saving file '%s': %s"), filename,
          g_strerror(errno));
        return;
    }
    
    fprintf(skeinfile,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<Skein rootNode=\"%s\" "
            "xmlns=\"http://www.logicalshift.org.uk/IF/Skein\">\n"
            "  <generator>GNOME Inform 7</generator>\n"
            "  <activeNode nodeId=\"%s\"/>\n",
            node_get_unique_id(I7_SKEIN_PRIVATE(skein)->root),
            node_get_unique_id(I7_SKEIN_PRIVATE(skein)->current));
    g_node_traverse(I7_SKEIN_PRIVATE(skein)->root, G_PRE_ORDER, G_TRAVERSE_ALL,
                    -1, (GNodeTraverseFunc)node_write_xml, skeinfile);
    fprintf(skeinfile, "</Skein>\n");
    fclose(skeinfile);
    I7_SKEIN_PRIVATE(skein)->modified = FALSE;
}

void
skein_reset(Skein *skein, gboolean current)
{
    if(current)
        I7_SKEIN_PRIVATE(skein)->current = I7_SKEIN_PRIVATE(skein)->root;
    I7_SKEIN_PRIVATE(skein)->played = I7_SKEIN_PRIVATE(skein)->root;
    g_signal_emit(skein, skein_signals[THREAD_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

static void
node_layout(GNode *node, PangoLayout *layout, double x, double spacing)
{
    /* Store the centre x coordinate for this node */
    ((NodeData *)node->data)->x = x;
    
    if(g_node_n_children(node) == 1) {
    	node_layout(g_node_nth_child(node, 0), layout, x, spacing);
    	return;
    }
    
    /* Find the total width of all descendant nodes */
    double total = node_get_tree_width(node, layout, spacing);
    int i;
    /* Lay out each child node */
    double child_x = 0.0;
    for(i = 0; i < g_node_n_children(node); i++) {
        GNode *child = g_node_nth_child(node, i);
        double treewidth = node_get_tree_width(child, layout, spacing);
        node_layout(child, layout, x - total*0.5 + child_x + treewidth*0.5,
                    spacing);
        child_x += treewidth + spacing;
    }
}  
            
void
skein_layout(Skein *skein, double spacing)
{
    if(I7_SKEIN_PRIVATE(skein)->layout == FALSE) {
        PangoFontDescription *font = get_font_description();
        PangoContext *context = gdk_pango_context_get();
        PangoLayout *layout = pango_layout_new(context);
        pango_layout_set_font_description(layout, font);
        node_layout(I7_SKEIN_PRIVATE(skein)->root, layout, 0.0, spacing);
        g_object_unref(G_OBJECT(layout));
        g_object_unref(G_OBJECT(context));
        pango_font_description_free(font);
    }
    I7_SKEIN_PRIVATE(skein)->layout = TRUE;
}

void
skein_invalidate_layout(Skein *skein)
{
	I7_SKEIN_PRIVATE(skein)->layout = FALSE;
    g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
}

void
skein_new_line(Skein *skein, const gchar *line)
{
    gboolean node_added = FALSE;
    gchar *nodeline = g_strescape(line, "\"");

    /* Is there a child node with the same line? */
    GNode *node = I7_SKEIN_PRIVATE(skein)->current->children;
    while(node != NULL) {
        /* Special case: NULL is treated as "" */
        if(((NodeData *)(node->data))->line == NULL) {
            if(line == NULL || strlen(line) == 0)
                break;
        } else if(strcmp(((NodeData *)(node->data))->line, line) == 0)
            break;
        node = node->next;
    }
    if(node == NULL) {
        node = node_create(nodeline, "", "", "", TRUE, FALSE, TRUE, 0);
        g_node_append(I7_SKEIN_PRIVATE(skein)->current, node);
        node_added = TRUE;
    }
    g_free(nodeline);
    
    /* Make this the new current node */
    I7_SKEIN_PRIVATE(skein)->current = node;
    I7_SKEIN_PRIVATE(skein)->played = node;

    /* Send signals */
    if(node_added) {
        I7_SKEIN_PRIVATE(skein)->layout = FALSE;
        g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
    } else
        g_signal_emit(skein, skein_signals[THREAD_CHANGED], 0);
    g_signal_emit(skein, skein_signals[SHOW_NODE], 0, GOT_LINE, (gpointer)node);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

/* Find the next node to use. Return TRUE if found, and if so, store a
newly-allocated copy of its command text in line.*/
gboolean
skein_next_line(Skein *skein, gchar **line)
{
    if(!g_node_is_ancestor(I7_SKEIN_PRIVATE(skein)->played,
                          I7_SKEIN_PRIVATE(skein)->current))
        return FALSE;

    GNode *next = I7_SKEIN_PRIVATE(skein)->current;
    while(next->parent != I7_SKEIN_PRIVATE(skein)->played)
        next = next->parent;
    I7_SKEIN_PRIVATE(skein)->played = next;
    *line = g_strcompress(node_get_line(next));
    g_signal_emit(skein, skein_signals[THREAD_CHANGED], 0);
    g_signal_emit(skein, skein_signals[SHOW_NODE], 0, GOT_LINE, (gpointer)next);
    return TRUE;
}

/* Get a list of the commands from the play pointer to the current pointer,
without resetting either of them */
GSList *
skein_get_commands(Skein *skein)
{
    GSList *commands = NULL;
    gchar *skeinline = NULL;
    GNode *pointer = I7_SKEIN_PRIVATE(skein)->played;
    while(g_node_is_ancestor(pointer, I7_SKEIN_PRIVATE(skein)->current)) {
        GNode *next = I7_SKEIN_PRIVATE(skein)->current;
        while(next->parent != pointer)
            next = next->parent;
        pointer = next;
        skeinline = g_strcompress(node_get_line(next));
        commands = g_slist_prepend(commands, g_strdup(skeinline));
        g_free(skeinline);
        g_signal_emit(skein, skein_signals[SHOW_NODE], 0, GOT_LINE,
                      (gpointer)next);
    }
    commands = g_slist_reverse(commands);
    return commands;
}

/* Update the status of the last played node */
void
skein_update_after_playing(Skein *skein, const gchar *transcript)
{
    node_set_played(I7_SKEIN_PRIVATE(skein)->played);
    g_signal_emit(skein, skein_signals[NODE_COLOR_CHANGED], 0);
    if(strlen(transcript)) {
        node_new_transcript_text(I7_SKEIN_PRIVATE(skein)->played, transcript);
        g_signal_emit(skein, skein_signals[SHOW_NODE], 0, GOT_TRANSCRIPT,
          (gpointer)I7_SKEIN_PRIVATE(skein)->played);
    }
}

gboolean
skein_get_line_from_history(Skein *skein, gchar **line, int history)
{
    /* Find the node to return the line from */
    GNode *node = I7_SKEIN_PRIVATE(skein)->current;
    while(--history > 0) {
        node = node->parent;
        if(node == NULL)
            return FALSE;
    }
    /* Don't return the root node */
    if(G_NODE_IS_ROOT(node))
        return FALSE;
    
    *line = g_strdelimit(g_strdup(node_get_line(node)), "\b\f\n\r\t", ' ');
    return TRUE;
}

/* Add an empty node as the child of node */
GNode *
skein_add_new(Skein *skein, GNode *node)
{
    GNode *newnode = node_create("", "", "", "", FALSE, FALSE, TRUE, 0);
    g_node_append(node, newnode);
    
    I7_SKEIN_PRIVATE(skein)->layout = FALSE;
    g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
    
    return newnode;
}

GNode *
skein_add_new_parent(Skein *skein, GNode *node)
{
    GNode *newnode = node_create("", "", "", "", FALSE, FALSE, TRUE, 0);
    g_node_insert(node->parent, g_node_child_position(node->parent, node),
                  newnode);
    g_node_unlink(node);
    g_node_append(newnode, node);
    
    I7_SKEIN_PRIVATE(skein)->layout = FALSE;
    g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
    
    return newnode;
}

gboolean
skein_remove_all(Skein *skein, GNode *node, gboolean notify)
{
    if(!G_NODE_IS_ROOT(node)) {
        gboolean in_current = in_current_thread(skein, node);
        node_destroy(node);
        
        if(in_current) {
            I7_SKEIN_PRIVATE(skein)->current = I7_SKEIN_PRIVATE(skein)->root;
            I7_SKEIN_PRIVATE(skein)->played = I7_SKEIN_PRIVATE(skein)->root;
        }
        
        I7_SKEIN_PRIVATE(skein)->layout = FALSE;
        if(notify)
            g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
        I7_SKEIN_PRIVATE(skein)->modified = TRUE;
        
        return TRUE;
    }
    return FALSE;
}

gboolean
skein_remove_single(Skein *skein, GNode *node)
{
    if(!G_NODE_IS_ROOT(node)) {
        gboolean in_current = in_current_thread(skein, node);

		if(!G_NODE_IS_LEAF(node)) {
			int i;
        	for(i = g_node_n_children(node) - 1; i >= 0; i--) {
				GNode *iter = g_node_nth_child(node, i);
				g_node_unlink(iter);
				g_node_insert_after(node->parent, node, iter);
			}
		}	
        node_destroy(node);
		
        if(in_current) {
            I7_SKEIN_PRIVATE(skein)->current = I7_SKEIN_PRIVATE(skein)->root;
            I7_SKEIN_PRIVATE(skein)->played = I7_SKEIN_PRIVATE(skein)->root;
        }
        
        I7_SKEIN_PRIVATE(skein)->layout = FALSE;
        g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
        I7_SKEIN_PRIVATE(skein)->modified = TRUE;
        return TRUE;
    }
    return FALSE;
}

void
skein_set_line(Skein *skein, GNode *node, const gchar *line)
{
    node_set_line(node, line);
    I7_SKEIN_PRIVATE(skein)->layout = FALSE;
    g_signal_emit(skein, skein_signals[NODE_TEXT_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

void
skein_set_label(Skein *skein, GNode *node, const gchar *label)
{
    node_set_label(node, label);
    I7_SKEIN_PRIVATE(skein)->layout = FALSE;
    g_signal_emit(skein, skein_signals[NODE_TEXT_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

void
skein_lock(Skein *skein, GNode *node)
{
    while(node) {
        node_set_temporary(node, FALSE);
        node = node->parent;
    }
    g_signal_emit(skein, skein_signals[LOCK_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

void
skein_unlock(Skein *skein, GNode *node, gboolean notify)
{
    node_set_temporary(node, TRUE);
    for(node = node->children; node != NULL; node = node->next)
        skein_unlock(skein, node, FALSE);
    
    if(notify)
        g_signal_emit(skein, skein_signals[LOCK_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

void
skein_trim(Skein *skein, GNode *node, int minScore, gboolean notify)
{
    int i = 0;
    while(i < g_node_n_children(node)) {
        GNode *child = g_node_nth_child(node, i);
        if(node_get_temporary(child)) {
            if(in_current_thread(skein, child)) {
                I7_SKEIN_PRIVATE(skein)->current =
                  I7_SKEIN_PRIVATE(skein)->root;
                I7_SKEIN_PRIVATE(skein)->played = I7_SKEIN_PRIVATE(skein)->root;
            }
            
            if(skein_remove_all(skein, child, FALSE) == FALSE)
                i++;
        } else {
            skein_trim(skein, child, -1, FALSE);
            i++;
        }
    }
    
    if(notify)
        g_signal_emit(skein, skein_signals[TREE_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}



static void
get_labels(GNode *node, GSList **labels)
{
	NodeData *data = (NodeData *)node->data;
	if(data->label && strlen(data->label)) {
		NodeLabel *nodelabel = g_new0(NodeLabel, 1);
		nodelabel->label = g_strdup(data->label);
		nodelabel->node = node;
		*labels = g_slist_prepend(*labels, (gpointer)nodelabel);
	}
	for(node = node->children; node != NULL; node = node->next)
		get_labels(node, labels);
}

GSList *
skein_get_labels(Skein *skein)
{
	GSList *labels = NULL;
	get_labels(I7_SKEIN_PRIVATE(skein)->root, &labels);
	labels = g_slist_sort_with_data(labels, (GCompareDataFunc)strcmp, NULL);
	return labels;
}

static gboolean
has_labels(GNode *node)
{
    NodeData *data = (NodeData *)node->data;
    if(data->label && strlen(data->label))
        return TRUE;
    for(node = node->children; node != NULL; node = node->next)
        if(has_labels(node))
            return TRUE;
    return FALSE;
}

gboolean
skein_has_labels(Skein *skein)
{
    return has_labels(I7_SKEIN_PRIVATE(skein)->root);
}

void
skein_bless(Skein *skein, GNode *node, gboolean all)
{
    while(node != NULL) {
        node_bless(node);
        node = all? node->parent : NULL;
    }
    g_signal_emit(skein, skein_signals[NODE_COLOR_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

gboolean
skein_can_bless(Skein *skein, GNode *node, gboolean all)
{
    gboolean can_bless = FALSE;
    while(node != NULL) {
        can_bless |= ((NodeData *)(node->data))->changed;
        node = all? node->parent : NULL;
    }
    return can_bless;
}

void
skein_set_expected_text(Skein *skein, GNode *node, const gchar *text)
{
    node_set_expected_text(node, text);
    g_signal_emit(skein, skein_signals[NODE_COLOR_CHANGED], 0);
    I7_SKEIN_PRIVATE(skein)->modified = TRUE;
}

GNode *
skein_get_thread_top(Skein *skein, GNode *node)
{
    while(TRUE) {
        if(G_NODE_IS_ROOT(node)) {
            g_assert_not_reached();
            return node;
        }
        if(g_node_n_children(node->parent) != 1)
            return node;
        if(G_NODE_IS_ROOT(node->parent))
            return node;
        node = node->parent;
    }
}

GNode *
skein_get_thread_bottom(Skein *skein, GNode *node)
{
    while(TRUE) {
        if(g_node_n_children(node) != 1)
            return node;
        node = node->children;
    }
}

/*
skein_get_blessed_thread_ends(Skein *skein, ...

void save_transcript(Skein *skein, GNode *node, const gchar *path)
*/

/* Returns whether the skein was modified since last save or load */
gboolean
skein_get_modified(Skein *skein)
{
    return I7_SKEIN_PRIVATE(skein)->modified;
}

GNode *
node_create(const gchar *line, const gchar *label, const gchar *transcript,
            const gchar *expected, gboolean played, gboolean changed,
            gboolean temp, int score)
{
    NodeData *data = g_new0(NodeData, 1);
    data->line = g_strdup(line);
    data->label = g_strdup(label);
    data->text_transcript = g_strdup(transcript);
    data->text_expected = g_strdup(expected);
    data->played = played;
    data->changed = changed;
    data->temp = temp;
    data->score = score;
    data->width = -1.0;
    data->linewidth = -1.0;
    data->labelwidth = 0.0;
    data->x = 0.0;
    
    data->id = g_strdup_printf("node-%p", data);
    /* diffs */
    
    return g_node_new(data);
}

static void
free_node_data(GNode *node, gpointer foo)
{
    NodeData *data = (NodeData *)(node->data);
    if(data->line)            g_free(data->line);
    if(data->label)           g_free(data->label);
    if(data->text_transcript) g_free(data->text_transcript);
    if(data->text_expected)   g_free(data->text_expected);
    if(data->id)              g_free(data->id);
    g_free(data);
    
    /* recurse */
    g_node_children_foreach(node, G_TRAVERSE_ALL, free_node_data, NULL);
    /* free the node itself */
    g_node_destroy(node);
}

void
node_destroy(GNode *node)
{
    free_node_data(node, NULL);
}

const gchar *
node_get_line(GNode *node)
{
    return ((NodeData *)node->data)->line;
}

void
node_set_line(GNode *node, const gchar *line)
{
    NodeData *data = (NodeData *)(node->data);
    if(data->line)
        g_free(data->line);
    data->line = g_strdup(line);
    data->width = -1.0;
    data->linewidth = -1.0;
    data->labelwidth = -1.0;
}

const gchar *
node_get_label(GNode *node)
{
    return ((NodeData *)node->data)->label;
}

void
node_set_label(GNode *node, const gchar *label)
{
    NodeData *data = (NodeData *)(node->data);
    if(data->label)
        g_free(data->label);
    data->label = g_strdup(label);
    data->width = -1.0;
    data->linewidth = -1.0;
    data->labelwidth = -1.0;
}

gboolean
node_has_label(GNode *node)
{
    NodeData *data = (NodeData *)node->data;
    return (node->parent != NULL) && data->label && (strlen(data->label) > 0);
}

const gchar *
node_get_expected_text(GNode *node)
{
    return ((NodeData *)node->data)->text_expected;
}

gboolean
node_get_changed(GNode *node)
{
    return ((NodeData *)(node->data))->changed;
}

gboolean
node_get_temporary(GNode *node)
{
    return ((NodeData *)node->data)->temp;
}

void
node_set_played(GNode *node)
{
    ((NodeData *)(node->data))->played = TRUE;
}

void
node_set_temporary(GNode *node, gboolean temp)
{
    ((NodeData *)node->data)->temp = temp;
}

void
node_new_transcript_text(GNode *node, const gchar *transcript)
{
    NodeData *data = (NodeData *)(node->data);
    if(data->text_transcript)
        g_free(data->text_transcript);
    data->text_transcript = g_strdup(transcript);
    g_strdelimit(data->text_transcript, "\r", '\n');
    data->changed = (strcmp(data->text_transcript, data->text_expected) != 0);
    
    /* clear diffs */
    if(data->changed && data->text_expected && strlen(data->text_expected))
        /* new diffs */;
}

void
node_bless(GNode *node)
{
    NodeData *data = (NodeData *)(node->data);
    if(data->text_expected)
        g_free(data->text_expected);
    data->text_expected = g_strdup(data->text_transcript);
    data->changed = FALSE;
    
    /* clear diffs */
}

void
node_set_expected_text(GNode *node, const gchar *text)
{
    NodeData *data = (NodeData *)(node->data);
    if(data->text_expected)
        g_free(data->text_expected);
    data->text_expected = g_strdup(text);
    g_strdelimit(data->text_transcript, "\r", '\n');
    data->changed = (strcmp(data->text_transcript, data->text_expected) != 0);
    
    /* clear diffs */
    
    if(data->changed && data->text_expected && strlen(data->text_expected))
        /* new diffs */;
}

static double
text_pixel_width(PangoLayout *layout, const gchar *text)
{
    int textwidth = 0;
	if(text && strlen(text)) {
    	pango_layout_set_text(layout, text, -1);
    	pango_layout_get_pixel_size(layout, &textwidth, NULL);
	}
    return (double)textwidth;
}

double
node_get_line_width(GNode *node, PangoLayout *layout)
{
    NodeData *data = (NodeData *)node->data;
    if(data->width < 0.0) {
    	if(layout) {
		    double size = text_pixel_width(layout, data->line);
		    data->width = size;
		    data->linewidth = size;
		    
		    if(data->label && strlen(data->label) > 0)
		        data->labelwidth = text_pixel_width(layout, data->label);
		    else
		        data->labelwidth = 0.0;
        }
        if(data->width < MIN_TEXT_WIDTH)
            data->width = MIN_TEXT_WIDTH;
        if(data->linewidth < MIN_TEXT_WIDTH)
            data->linewidth = MIN_TEXT_WIDTH;
        if(data->labelwidth < MIN_TEXT_WIDTH)
            data->labelwidth = MIN_TEXT_WIDTH;
    }
    return data->width;
}

double
node_get_line_text_width(GNode *node)
{
    return ((NodeData *)node->data)->linewidth;
}

double
node_get_label_text_width(GNode *node)
{
    return ((NodeData *)node->data)->labelwidth;
}

double
node_get_tree_width(GNode *node, PangoLayout *layout, double spacing)
{
    /* Get the tree width of all children */
    int i;
    double total = 0.0;
    for(i = 0; i < g_node_n_children(node); i++) {
        total += node_get_tree_width(g_node_nth_child(node,i), layout, spacing);
        if(i > 0)
            total += spacing;
    }
    /* Return the largest of the above, the width of this node's line and the
    width of this node's label */
    double linewidth = node_get_line_width(node, layout);
    double labelwidth = node_get_label_text_width(node);
    if(total > linewidth && total > labelwidth)
        return total;
    return max(linewidth, labelwidth);
}    

const gchar *
node_get_unique_id(GNode *node)
{
    return ((NodeData *)node->data)->id;
}

double
node_get_x(GNode *node)
{
    return ((NodeData *)node->data)->x;
}

gboolean
node_in_thread(GNode *node, GNode *endnode)
{
    return (endnode == node) || g_node_is_ancestor(node, endnode);
}

/* DEBUG */
static void 
dump_node_data(GNode *node, gpointer foo) 
{
    g_printerr("(%s)", ((NodeData *)(node->data))->line);
    if(g_node_n_children(node)) {
        g_printerr("->(");
        g_node_children_foreach(node, G_TRAVERSE_ALL, dump_node_data, foo);
        g_printerr(")");
    }
}

void 
skein_dump(Skein *skein) 
{
    dump_node_data(I7_SKEIN_PRIVATE(skein)->root, NULL);
    g_printerr("\n");
}
