#include "resource.h"

extern GPrivate *glk_data_key;

/**
 * giblorb_set_resource_map:
 * @file: The file stream to read the resource map from
 *
 * This function tells the library that the file is indeed the Blorby source
 * of all resource goodness. Whenever your program calls an image or sound
 * function, such as glk_image_draw(), the library will search this file for
 * the resource you request. 
 *
 * Do <emphasis>not</emphasis> close the stream after calling this function. 
 * The library is responsible for closing the stream at shutdown time.
 *
 * Returns: a Blorb error code.
 */
giblorb_err_t
giblorb_set_resource_map(strid_t file)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	giblorb_map_t *newmap; /* create map allocates memory */
	giblorb_err_t error = giblorb_create_map(file, &newmap);

	if(error != giblorb_err_None) {
		g_free(newmap);
		return error;
	}

	/* Check if there was already an existing resource map */
	if(glk_data->resource_map != NULL) {
		WARNING("Overwriting existing resource map.\n");
		giblorb_destroy_map(glk_data->resource_map);
		glk_stream_close(glk_data->resource_file, NULL);
	}

	glk_data->resource_map = newmap;
	glk_data->resource_file = file;

	//giblorb_print_contents(newmap);
	return giblorb_err_None;
}

/**
 * giblorb_get_resource_map:
 * 
 * This function returns the current resource map being used. Returns %NULL
 * if giblorb_set_resource_map() has not been called yet.
 *
 * Returns: a resource map, or %NULL.
 */
giblorb_map_t*
giblorb_get_resource_map()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	if(glk_data->resource_map == NULL) {
		WARNING("Resource map not set yet.\n");
	}

	return glk_data->resource_map;
}

/* giblorb_chunkdesc_t: Describes one chunk of the Blorb file. */
typedef struct giblorb_chunkdesc_struct {
    glui32 type;
    glui32 len;
    glui32 startpos; /* start of chunk header */
    glui32 datpos; /* start of data (either startpos or startpos+8) */
    
    void *ptr; /* pointer to malloc'd data, if loaded */
    int auxdatnum; /* entry in the auxsound/auxpict array; -1 if none.
        This only applies to chunks that represent resources;  */
    
} giblorb_chunkdesc_t;

/* giblorb_resdesc_t: Describes one resource in the Blorb file. */
typedef struct giblorb_resdesc_struct {
    glui32 usage;
    glui32 resnum;
    glui32 chunknum;
} giblorb_resdesc_t;

/* giblorb_map_t: Holds the complete description of an open Blorb file. */
struct giblorb_map_struct {
    glui32 inited; /* holds giblorb_Inited_Magic if the map structure is 
        valid */
    strid_t file;
    
    int numchunks;
    giblorb_chunkdesc_t *chunks; /* list of chunk descriptors */
    
    int numresources;
    giblorb_resdesc_t *resources; /* list of resource descriptors */
    giblorb_resdesc_t **ressorted; /* list of pointers to descriptors 
        in map->resources -- sorted by usage and resource number. */
};

void
giblorb_print_contents(giblorb_map_t *map)
{
	int i;
	for(i=0; i<map->numresources; i++) {
		giblorb_resdesc_t *resource = map->ressorted[i];
		printf("Resource #%d, chunknum: %d\n", resource->resnum, resource->chunknum);
	}	

	printf("\n-------\n");

	for(i=0; i<map->numresources; i++) {
		giblorb_chunkdesc_t chunk = map->chunks[i];
		printf("Chunk #%d, type: %d\n", i, chunk.type);
	}	
}

gchar*
giblorb_get_error_message(giblorb_err_t err)
{
	switch(err)
	{
		case giblorb_err_None:
			return "not BLORB's fault";
		case giblorb_err_CompileTime:
			return "BLORB compile time error";
		case giblorb_err_Alloc:
			return "memory allocation failed";
		case giblorb_err_Read:
			return "error during reading of file";
		case giblorb_err_NotAMap:
			return "invalid resource map supplied";
		case giblorb_err_Format:
			return "invalid format of resource";
		case giblorb_err_NotFound:
			return "resource not found";
		default:
			return "Unknown error code";
	}
}
