/* ---------------------------------------------------------------------------
   cBlorb: a perl script for creating Blorb files 
   (c) Graham Nelson 1998, 2005 
   --------------------------------------------------------------------------- */

#define PLATFORM_UNIX

#ifdef PLATFORM_MACOSX
#define TEMP_PREFIX "Build" /* Prefix for location of temporary directory */
#define SEP_CHAR '/'
#endif
#ifdef PLATFORM_WINDOWS
#define TEMP_PREFIX "Build" /* Prefix for location of temporary directory */
#define SEP_CHAR '\\'
#endif
#ifdef PLATFORM_UNIX
#define TEMP_PREFIX "Build" /* Prefix for location of temporary directory */
#define SEP_CHAR '/'
#endif

#define VERSION "cBlorb 1.07"

char *output_filename = "story.zblorb";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

time_t the_present;
struct tm *here_and_now;

char blorbdate[128];
char blurb_filename[1024];
char temp_dir[1024];
char stupidly_big[102400];
char *filename_space;

typedef struct web_variable {
	char varname[128];
	char vartext[2048];
} web_variable;
int no_wv = 0;
#define MAX_WV 50
web_variable thewvs[MAX_WV];

typedef struct auxiliary_file {
	char filename[512];
	char description[512];
	char format[128];
} auxiliary_file;
int no_aux = 0;
#define MAX_AUX 50
auxiliary_file theauxes[MAX_AUX];

#define MAX_CHUNKS 500
char *chunk_filename_array[MAX_CHUNKS];
int chunk_indexed_array[MAX_CHUNKS];
char *chunk_id_array[MAX_CHUNKS];
int chunk_size_array[MAX_CHUNKS];
int chunk_number_array[MAX_CHUNKS];
int chunk_offset_array[MAX_CHUNKS];
int picture_numbering[MAX_CHUNKS];
int sound_numbering[MAX_CHUNKS];

int blurb_line = 0;

int chunk_opened = 0;
int chunk_count = 0;
int indexed_count = 0;
int total_size = 0;
int past_idx_offset = 0;
int max_resource_num = 0;
int scalables = 0;
int repeaters = 0;
int next_pnum = 1;
int next_snum = 3;
int cover_exists = FALSE;

int r_stdx = 600, r_stdy = 400;
int r_minx = 0, r_maxx = 0;
int r_miny = 0, r_maxy = 0;
int resolution_on = 0;

int iff_size = 0, pcount = 0, scount = 0;

int error_count = 0;

FILE *CHUNK = NULL;

/* ---------- */

/*
void ensure_tempdir(void)
{   if (opendir(TDIR, temp_dir))
    {   closedir(TDIR);
    }
    else
    {   if (mkdir(temp_dir, 0) == 0)
        { die("Fatal error: unable to create working directory temp_dir");
        }
    }
}

void remove_tempdir(void)
{   if (opendir(TDIR, temp_dir))
    {   @allfiles = grep !/^\./, readdir TDIR;
        closedir(TDIR);

        foreach $file (@allfiles)
        {   $fullfile = sprintf("%s%s%s", temp_dir, $SEP_CHAR, $file);
            unlink $fullfile;
        }

        rmdir(temp_dir);
    }
}
*/

/* ---------- */

void error(char *erm) {
    fprintf(stderr, "line %d: Error: %s\n", blurb_line, erm);
    error_count++;
}

void fatal(char *erm) {
    fprintf(stderr, "line %d: Fatal error: %s\n", blurb_line, erm);
    exit(1);
}

void fatal_fs(char *erm, char *fn) {
    fprintf(stderr, "line %d: Fatal error: %s\nFilename: %s\n",
    	blurb_line, erm, fn);
    exit(1);
}

/* ---------- */

void four_word(FILE *F, int n) {
	fputc((n / 0x1000000)%0x100, F);
	fputc((n / 0x10000)%0x100, F);
	fputc((n / 0x100)%0x100, F);
	fputc((n)%0x100, F);
#ifdef TRACE_CBLORB
	printf("Writing %08x to file\n", n);
#endif
}

void two_word(int n) {
#ifdef TRACE_CBLORB
	printf("Size of CHUNK is %d\n", ftell(CHUNK));
#endif
	fputc((n / 0x100)%0x100, CHUNK);
	fputc((n)%0x100, CHUNK);
#ifdef TRACE_CBLORB
	printf("Size of CHUNK became %d\n", ftell(CHUNK));
#endif
}

void one_byte(int n) {
    fprintf(CHUNK, "%c", n);
}

void begin_chunk(char *id, int cnum, char *chunk_filename) {

    if (cnum > max_resource_num) { max_resource_num = cnum; }

    if (chunk_filename == NULL) {
    	sprintf(filename_space, "%s%cC%d.chunk", temp_dir, SEP_CHAR, chunk_count);
    	chunk_filename = filename_space;
    	filename_space += strlen(filename_space)+1;
           
		CHUNK = fopen(chunk_filename, "wb");
        if (CHUNK == NULL)
        	fatal_fs("unable to create temporary file", chunk_filename);
    	chunk_opened = 1;
    } else {
    	sprintf(filename_space, "%s", chunk_filename);
    	chunk_filename = filename_space;
    	filename_space += strlen(filename_space)+1;
    }

	if (chunk_count >= MAX_CHUNKS) fatal("exceeded maximum number of chunks");

    chunk_filename_array[chunk_count] = chunk_filename;
    chunk_indexed_array[chunk_count] = 0;

    if ((strcmp(id, "Pict")==0) || (strcmp(id, "Snd1")==0) || (strcmp(id, "Snd2")==0)
        || (strcmp(id, "Snd3")==0) || (strcmp(id, "Exec")==0)) {
    	chunk_indexed_array[chunk_count] = 1;
        indexed_count = indexed_count + 1;        
    }

    chunk_id_array[chunk_count] = id;
    chunk_number_array[chunk_count] = cnum;
    chunk_offset_array[chunk_count] = total_size;

#ifdef TRACE_CBLORB
	printf("Begun chunk %d, %s: fn is <%s>\n", chunk_count, id, chunk_filename);
#endif
}

void end_chunk(void) {
	int size = -1; char *chunk_filename;
	FILE *COUNTUP;

    if (chunk_opened == 1) {
    	size = (int) (ftell(CHUNK));
    	fclose(CHUNK);
	    chunk_opened = 0;
	}

    chunk_filename = chunk_filename_array[chunk_count];

	if (size < 0) {
		COUNTUP = fopen(chunk_filename, "rb");
		if (COUNTUP == NULL) fatal_fs("unable to open chunk for size counting",
			chunk_filename_array[chunk_count]);
		fseek(COUNTUP, 0, SEEK_END);
		size = (int) (ftell(COUNTUP));

/*    while (1==1) {
    	if (fgetc(CHUNK) == EOF) break;
    	size++;
    } */
	    fclose(COUNTUP);
	}

#ifdef TRACE_CBLORB
	printf("Ended chunk %d (innate size %d): fn is <%s>\n",
		chunk_count, size, chunk_filename);
#endif

    if (strcmp(chunk_id_array[chunk_count], "Snd1") != 0) size += 8;

    chunk_size_array[chunk_count] = size;

    if (size % 2 == 1) size++;

    total_size += size;

    chunk_count++;
}

void author_chunk(char *t) {
    begin_chunk("AUTH", 0, NULL);
    fprintf(CHUNK, "%s", t);
#ifdef TRACE_CBLORB
    printf("Author: <%s>\n", t);
 	printf("Size of CHUNK became %d\n", ftell(CHUNK));   
#endif
    end_chunk();
}

void copyright_chunk(char *t) {
    begin_chunk("(c) ", 0, NULL);
    fprintf(CHUNK, "%s", t);
    end_chunk();
}

void release_chunk(int rn) {
    begin_chunk("RelN", 0, NULL);
    two_word(rn);
    end_chunk();
}

void palette_simple_chunk(int p) {
    begin_chunk("Plte", 0, NULL);
    one_byte(p);
    end_chunk();
}

void picture_chunk(int n, char *fn) {
    begin_chunk("Pict", n, fn);
    end_chunk();
}

void sound1_chunk(int n, char *fn) {
    begin_chunk("Snd1", n, fn);
    end_chunk();
}

void sound2_chunk(int n, char *fn) {
    begin_chunk("Snd2", n, fn);
    end_chunk();
}

void sound3_chunk(int n, char *fn) {
    begin_chunk("Snd3", n, fn);
    end_chunk();
}

void executable_chunk(char *fn) {
    begin_chunk("Exec", 0, fn);
    end_chunk();
}

void metadata_chunk(char *fn) {
    begin_chunk("IFmd", 0, fn);
    end_chunk();
}

void set_variable_to(char *var, char *text) {
	int s;
	for (s=0; s<no_wv; s++) {		
		if (strcmp(thewvs[s].varname, var) == 0) {
			strcpy(thewvs[s].vartext, text);
			return;
		}
	}
	if (no_wv >= MAX_WV) { error("too many web variables"); return; }
	strcpy(thewvs[no_wv].varname, var);
	strcpy(thewvs[no_wv].vartext, text);
	no_wv++;
}

void download_link(FILE *COPYTO, char *desc, char *fn, char *form) {
	int k, size_up = FALSE; char *original_fn = fn;
	if (strcmp(form, "link") != 0) {
		for (k=strlen(fn)-1; (k>0) && (fn[k]!=SEP_CHAR); k--) ;
		if (k>0) fn+=k+1;
		size_up = TRUE;
	}
	if (strcmp(form, "Blorb") == 0) size_up = FALSE;
	fprintf(COPYTO, "<a href=\"%s\">%s</a> <small>(%s",
		fn, desc, form);
	if (size_up) {
		long int file_size;
		FILE *TEST_FILE;
		TEST_FILE = fopen(original_fn, "rb");
		if (TEST_FILE) {
			if (fseek(TEST_FILE, 0, SEEK_END) == 0) {
				file_size = ftell(TEST_FILE);
				if (file_size != -1L) {
					char *units = " bytes";
					if (file_size > 1024L) { file_size /= 1024L; units = "KB"; }
					if (file_size > 1024L) { file_size /= 1024L; units = "MB"; }
					if (file_size > 1024L) { file_size /= 1024L; units = "GB"; }
					if (file_size > 1024L) { file_size /= 1024L; units = "TB"; }
					fprintf(COPYTO, ", %d%s", (int) file_size, units);
				} else fprintf(stderr, "Warning: ftell failed on linked file:\n  %s\n", original_fn);
			} else fprintf(stderr, "Warning: fseek failed on linked file:\n  %s\n", original_fn);
			fclose(TEST_FILE);
		} else fprintf(stderr, "Warning: Could not open linked file to count size:\n  %s\n", original_fn);
	}
	fprintf(COPYTO, ")</small>");
}

void copy_variable_to(char *var, FILE *COPYTO) {
	int s;
	if (strcmp(var, "COVER") == 0) {
		if (cover_exists == FALSE) return;
		fprintf(COPYTO, "<a href=\"Cover.jpg\"><img src=\"Small Cover.jpg\" border=\"0\"></a>");
		return;
	}
	if (strcmp(var, "DOWNLOAD") == 0) {
		download_link(COPYTO, "Story File", "story.zblorb", "Blorb");
		fprintf(COPYTO, "<p>");
		return;
	}
	if (strcmp(var, "AUXILIARY") == 0) {
		for (s=0; s<no_aux; s++) {
			download_link(COPYTO, theauxes[s].description,
				theauxes[s].filename, theauxes[s].format);
			fprintf(COPYTO, "<p>");
		}
		return;
	}
	for (s=0; s<no_wv; s++) {		
		if (strcmp(thewvs[s].varname, var) == 0) {
			int i; char *p = thewvs[s].vartext;
			for (i=0; p[i]; i++) {
				if ((p[i] == '\x0a') || (p[i] == '\x0d') || (p[i] == '\x7f'))
					fprintf(COPYTO, "<p>");
				else fprintf(COPYTO, "%c", p[i]);
			}
			return;
		}
	}
	error("no such substitution in web site source");
	fprintf(stderr, "Substitution: [%s]\n", var);
}

void copy_html_line(char *line, FILE *COPYTO) {
	char substitution[10240];
	int i;
	for (i=0; line[i]; i++) {
		if (line[i] == '[') {
			int j;
			for (j=i+1; (line[j] && line[j]!=']'); j++) ;
			if (line[j] == ']') {
				line[j] = 0;
				strcpy(substitution, line+i+1);
				copy_variable_to(substitution, COPYTO);
				i = j;
				continue;
			}
		}
		fprintf(COPYTO, "%c", line[i]);
	}
	fprintf(COPYTO, "\n");
}

void web_copy(char *from, char *to) {
	FILE *COPYFROM, *COPYTO; int i, c; char line[10240];
	COPYFROM = fopen(from, "r");
	if (COPYFROM == NULL) {
		error("unable to open source for web site");
		return;
	}
	COPYTO = fopen(to, "w");
	if (COPYTO == NULL) {
		error("unable to open destination for web site");
		return;
	}

	i=0; c = ' ';
	while (c != EOF) {
		c = fgetc(COPYFROM);
		if ((c == EOF) || (c == '\x0a') || (c == '\x0d') || (i>=10239)) {
			line[i] = 0; copy_html_line(line, COPYTO); i=0;
		} else line[i++] = c;
		
	}
	if (i>0) { line[i] = 0; copy_html_line(line, COPYTO); }

	fclose(COPYFROM);
	fclose(COPYTO);
}

#define MAX_BRANCH_LENGTH 32
#define MAX_KNOTS 1000

typedef struct skein_node {
	char id[20];
	char command[128];
	char annotation[128];
	char branch_start[MAX_BRANCH_LENGTH];
	struct skein_node *parent;
	struct skein_node *child;
	struct skein_node *sibling;
	int relevant;
	int terminal;
} skein_node;

skein_node the_skein[MAX_KNOTS];
int no_knots;

void post_command(FILE *SOL, skein_node *sn) {
	if (sn->annotation[0])
		fprintf(SOL, " ... %s", sn->annotation);
	fprintf(SOL, "\n");
}

void recursively_solve(FILE *SOL, skein_node *sn, char *branch) {
	skein_node *sn2; int branch_counter;

	/* Don't print the command at sn: but run down below. */

	SpareThePoorStack:

	if (sn->child == NULL) return;

	if ((sn->child) && (sn->child->sibling == NULL)) {
		fprintf(SOL, "%s ", sn->child->command); post_command(SOL, sn->child);
		sn = sn->child; goto SpareThePoorStack;
	}

	fprintf(SOL, "Choice:\n");
	for (sn2 = sn->child, branch_counter = 1; sn2;
		sn2 = sn2->sibling) {
		if (strlen(branch) >= MAX_BRANCH_LENGTH-4) {
			error("the skein file branches far too much for a good solution");
			return;
		}
		if (sn2->child == NULL) {
			fprintf(SOL, "  %s -> end", sn2->command);
		} else {
			if (branch[0] == 0) sprintf(sn2->branch_start, "%d", branch_counter++);
			else sprintf(sn2->branch_start, "%s.%d", branch, branch_counter++);
			fprintf(SOL, "  %s -> go to branch (%s)",
				sn2->command, sn2->branch_start);
		}
		post_command(SOL, sn2);
	}
	
	for (sn2 = sn->child; sn2; sn2 = sn2->sibling) {
		if (sn2->child) {
			fprintf(SOL, "\nBranch (%s)\n", sn2->branch_start);
			recursively_solve(SOL, sn2, sn2->branch_start);
		}
	}
}

skein_node *current_skein_node = NULL;

skein_node *find_node_of(char *id) {
	int i;
	for (i=0; i<no_knots; i++)
		if (strcmp(id, the_skein[i].id) == 0)
			return &(the_skein[i]);
	return NULL;
}

void read_skein_line(char *line, int pass) {
	char node_id[32], junk[10240], junk2[10240]; int i;
	/* printf("Pass %d: line |%s|\n", pass, line); */
	if (pass == 1) {
		if (sscanf(line, "%[^<]<item nodeId=\"%[^\"]\"", junk, node_id) == 2) {
			if (no_knots >= MAX_KNOTS) {
				error("the skein file is too large");
				return;
			}
			/* printf("Making %d\n", no_knots); */
			current_skein_node = &(the_skein[no_knots++]);
			strcpy(current_skein_node->id, node_id);
			strcpy(current_skein_node->command, "");
			strcpy(current_skein_node->annotation, "");
			strcpy(current_skein_node->branch_start, "");
			current_skein_node->parent = NULL;
			current_skein_node->child = NULL;
			current_skein_node->sibling = NULL;
			current_skein_node->relevant = FALSE;
			current_skein_node->terminal = TRUE;
		}
		if (sscanf(line, "%[^>]>%[^<]</command%s", junk, node_id, junk2) == 3) {
			if (current_skein_node == NULL) {
				fprintf(stderr, "Command: %s\n", node_id);
				error("the skein file is malformed (A1)");
				return;
			}
			for (i=0; node_id[i]; i++) node_id[i]=toupper(node_id[i]);
			strcpy(current_skein_node->command, node_id);
		}
		if (sscanf(line, "%[^>]>%[^<]</annotation%s", junk, node_id, junk2) == 3) {
			int j;
			if (current_skein_node == NULL) {
				fprintf(stderr, "Annotation: %s\n", node_id);
				error("the skein file is malformed (A2)");
				return;
			}
			i = 0;
			if ((node_id[0] == '*') && (node_id[1] == '*')
				&& (node_id[2] == '*')) {
				current_skein_node->terminal = TRUE;
				i = 3; while (node_id[i] == ' ') i++;
			}
			j=0;
			while (node_id[i]) {
				if (node_id[i] == '&') {
					if ((node_id[i+1] == 'l') && (node_id[i+2] == 't')
						&& (node_id[i+3] == ';')) {
						i+=4;
						current_skein_node->annotation[j++] = '<';
						continue;
					}
					if ((node_id[i+1] == 'a') && (node_id[i+2] == 'm')
						&& (node_id[i+3] == 'p')
						&& (node_id[i+4] == ';')) {
						i+=5;
						current_skein_node->annotation[j++] = '&';
						continue;
					}
					if ((node_id[i+1] == 'a') && (node_id[i+2] == 'p')
						&& (node_id[i+3] == 'o') && (node_id[i+4] == 's')
						&& (node_id[i+5] == ';')) {
						i+=6;
						current_skein_node->annotation[j++] = '\'';
						continue;
					}
				}
				current_skein_node->annotation[j++] = node_id[i++];
			}
			current_skein_node->annotation[j++] = 0;
		}
	} else {
		if (sscanf(line, "%[^<]<item nodeId=\"%[^\"]\"", junk, node_id) == 2)
			current_skein_node = find_node_of(node_id);
		if (sscanf(line, "%[^<]<child nodeId=\"%[^\"]\"", junk, node_id) == 2) {
			skein_node *new_child = find_node_of(node_id);
			if ((new_child == NULL) || (current_skein_node == NULL)) {
				error("the skein file is malformed (B)");
				return;
			}
			new_child->parent = current_skein_node;
			new_child->sibling = NULL;
			if (current_skein_node->child == NULL) {
				current_skein_node->child = new_child;
			} else {
				skein_node *familial = current_skein_node->child;
				while (familial->sibling) familial = familial->sibling;
				familial->sibling = new_child;
			}
		}
	}
}

void look_for_relevance(void) {
	int i;
	for (i=0; i<no_knots; i++)
		if (the_skein[i].terminal) {
			skein_node *sn = &(the_skein[i]);
			while (sn) {
				sn->relevant = TRUE;
				sn = sn->parent;
			}
		}
	for (i=no_knots-1; i>=1; i--) {
		skein_node *sn = &(the_skein[i]);
		if ((sn->relevant == FALSE) && (sn->parent)) {
			if (sn->parent->child == sn) {
				sn->parent->child = sn->sibling;
			} else {
				skein_node *sn2 = sn->parent->child;
				while ((sn2) && (sn2->sibling != sn)) {
					sn2 = sn2->sibling;
				}
				if ((sn2) && (sn2->sibling == sn)) {
					sn2->sibling = sn->sibling;
				}
			}
			sn->parent = NULL;
			sn->sibling = NULL;
		}
	}
}

void walkthrough(char *from, char *to) {
	FILE *SKEIN, *SOL; int i, c, pass; char line[10240];
	no_knots = 0;
	
	for (pass=1; pass<=2; pass++) {
		current_skein_node = NULL;
		SKEIN = fopen(from, "r");
		if (SKEIN == NULL) {
			error("unable to open skein file for solution");
			return;
		}
		i=0; c = ' ';
		while (c != EOF) {
			c = fgetc(SKEIN);
			if ((c == EOF) || (c == '\x0a') || (c == '\x0d') || (i>=10239)) {
				line[i] = 0; read_skein_line(line, pass); i=0;
			} else line[i++] = c;
			
		}
		if (i>0) { line[i] = 0; read_skein_line(line, pass); }		
		fclose(SKEIN);
	}

	look_for_relevance();
	if (the_skein[0].relevant == FALSE) {
		error("no threads in the skein have been marked ***");
		return;
	}

	SOL = fopen(to, "w");
	if (SOL == NULL) {
		error("unable to open destination for solution text file");
		return;
	}

	fprintf(SOL, "Solution to \"");
	copy_variable_to("TITLE", SOL);
	fprintf(SOL, "\" by ");
	copy_variable_to("AUTHOR", SOL);
	fprintf(SOL, "\n\n");

	if (no_knots > 0) recursively_solve(SOL, &(the_skein[0]), "");
	else fprintf(SOL, "(There is no solution since the Skein is empty.)\n\n");

	fclose(SOL);
}

/* ---------- */

void identify(char *symb, int val) {
    printf("Constant %s = %d;\n", symb, val);
}

void interpret(char *command) {
	char keyword[2048];
	char sv[2048], sv2[2048]; int iv;
	
	blurb_line++;
	
	sscanf(command, "%s", keyword);
	
	if (keyword[0] == 0) return; /* Blank line */

	if (command[0] == 0) return; /* Blank line */

    if (keyword[0] == '!') return; /* Comment line */

#ifdef TRACE_CBLORB
	printf("Line %d: command <%s> keyword <%s>\n", blurb_line, command, keyword); */
#endif
	
	if (sscanf(command, "copyright \"%[^\"]\"", sv) == 1) {
		copyright_chunk(sv);
        return;
    }
	if (sscanf(command, "author \"%[^\"]\"", sv) == 1) {
		set_variable_to("AUTHOR", sv);
		author_chunk(sv);
        return;
    }
	if (sscanf(command, "release %d", &iv) == 1) {
		char temp_digits[64];
		sprintf(temp_digits, "%d", iv);
		set_variable_to("RELEASE", temp_digits);
		release_chunk(iv);
        return;
    }
	if (sscanf(command, "auxiliary \"%[^\"]\" \"%[^\"]\"", sv, sv2) == 2) {
		int j;
		if (no_aux >= MAX_AUX) {
			error("Too many auxiliary files");
			return;
		}
		strcpy(theauxes[no_aux].description, sv2);
		j=strlen(sv)-1; while ((j>0) && (sv[j]!='.') && (sv[j]!=SEP_CHAR)) j--;
		if ((j>0) && (sv[j] == '.')) {
			int k;
			strcpy(theauxes[no_aux].filename, sv);
			strcpy(theauxes[no_aux].format, sv+j+1);
			for (k=0; theauxes[no_aux].format[k]; k++)
				theauxes[no_aux].format[k] = tolower(theauxes[no_aux].format[k]);
		} else {
			strcpy(theauxes[no_aux].format, "link");
			sprintf(theauxes[no_aux].filename, "%s%cindex.html", sv, SEP_CHAR);
		}
		no_aux++;
		printf("Auxiliary file: <%s> = <%s>\n", sv, sv2);
        return;
    }
	if (sscanf(command, "web \"%[^\"]\" to \"%[^\"]\"", sv, sv2) == 2) {
		web_copy(sv, sv2);
        return;
    }
	if (sscanf(command, "solution from \"%[^\"]\" to \"%[^\"]\"", sv, sv2) == 2) {
		walkthrough(sv, sv2);
        return;
    }
	if (sscanf(command, "variable [%[A-Z]] = \"%[^\"]\"", sv, sv2) == 2) {
		set_variable_to(sv, sv2);
        return;
    }
	if (sscanf(command, "cover \"%[^\"]\"", sv) == 1) {
		cover_exists = TRUE;
		set_variable_to("BIGCOVER", sv2);
        return;
    }

/*    if ($command =~ /^\s*resolution\s+(\d*)x(\d*)\s*(.*)$/m)
    {   $r_stdx = $1; $r_stdy = $2;
        $r_minx = 0; $r_maxx = 0;
        $r_miny = 0; $r_maxy = 0;

        $resolution_on = 1;

        $rest = $3;
        if ($rest =~ /^\s*min\s+(\d*)x(\d*)\s*$/m)
        {   $r_minx = $1;
            $r_miny = $2;
            return;
        }
        if ($rest =~ /^\s*max\s+(\d*)x(\d*)\s*$/m)
        {   $r_maxx = $1;
            $r_maxy = $2;
            return;
        }
        if ($rest =~ /^\s*min\s+(\d*)x(\d*)\s*max\s+(\d*)x(\d*)\s*$/m)
        {   $r_minx = $1;
            $r_miny = $2;
            $r_maxx = $3;
            $r_maxy = $4;
            return;
        }
        if ($rest =~ /^\s*$/m)
        {   return;
        }
    }
    if ($command =~ /^\s*palette\s+(\d*)\s*bit/)
    {   if (($1 == 16) || ($1 == 32))
        {   palette_simple_chunk($1);
            return;
        }
        error("palette can only be 16 or 32 bit");
        return;
    }
    if ($command =~ /^\s*palette\s*\{(.*)$/m)
    {   $rest = $1;
        begin_chunk("Plte", 0, "");
        while (not($rest =~ /^\s*\}/))
        {   if ($rest =~ /^\s*$/m)
            {   $rest = <BLURB> or fatal("end of blurb file in 'palette'");
                $blurb_line = $blurb_line + 1;
            }
            else
            {   if ($rest =~
            /^\s*([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})\s*(.*)$/m)
                {   $rest = $4;
                    one_byte(hex($1));
                    one_byte(hex($2));
                    one_byte(hex($3));
                }
                else
                {   $rest =~ /^\s*(\S+)\s*(.*)$/m;
                    error("palette entry not six hex digits: $1");
                    $rest = $2;
                }
            }
        }
        end_chunk();
        return;
    } */
    
    if (sscanf(command, "destination \"%[^\"]\"", sv) == 1) {
    	printf("Copy blorb to: [[%s]]\n", sv); return;
    }
    if (sscanf(command, "metadata \"%[^\"]\" include", sv) == 1) {
    	metadata_chunk(sv); return;
    }
    if (sscanf(command, "storyfile \"%[^\"]\" include", sv) == 1) {
    	executable_chunk(sv); return;
    }
    if (sscanf(command, "storyfile \"%[^\"]\"", sv) == 1) {
    	error("can only include story files in this minimal cBlorb"); return;
    }

/*    if ($command =~ /^\s*storyfile\s+"(.*)"/)
    {   open(IDFILE, $1) or fatal("unable to open story file $1");
        binmode(IDFILE);
        begin_chunk("IFhd", 0, "");
        $version = unpack("C", getc(IDFILE));
        printf("! Identifying v$version story file $1\n";

        read IDFILE, $buffer, 1;
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        read IDFILE, $buffer, 14;
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        read IDFILE, $buffer, 4;
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(unpack("C",getc(IDFILE)));
        one_byte(0);
        one_byte(0);
        one_byte(0);
        end_chunk();
        fclose(IDFILE);
        return;
    } */

    if (sscanf(command, "picture %d \"%[^\"]\"", &iv, sv) == 2) {
		picture_chunk(iv, sv);
		return;
	}

/*
        $scalables = $scalables + 1;
        $resolution_on = 1;

        $p_picno[$scalables] = $pnum;
        $p_stdp[$scalables] = 1; $p_stdq[$scalables] = 1;
        $p_minp[$scalables] = -1; $p_maxp[$scalables] = -1;
        $p_minq[$scalables] = -1; $p_maxq[$scalables] = -1;

        if ($rest =~ /^\s*scale\s+(\d*)\/(\d*)\s*$/m)
        {   $p_stdp[$scalables] = $1;
            $p_stdq[$scalables] = $2;
            return;
        }
        if ($rest =~ /^\s*scale\s+max\s*(\d*)\/(\d*)\s*$/m)
        {   $p_maxp[$scalables] = $1;
            $p_maxq[$scalables] = $2;
            return;
        }
        if ($rest =~ /^\s*scale\s+min\s*(\d*)\/(\d*)\s*$/m)
        {   $p_minp[$scalables] = $1;
            $p_minq[$scalables] = $2;
            return;
        }
        if ($rest =~
            /^\s*scale\s+min\s*(\d*)\/(\d*)\s+max\s*(\d*)\/(\d*)\s*$/m)
        {   $p_minp[$scalables] = $1;
            $p_minq[$scalables] = $2;
            $p_maxp[$scalables] = $3;
            $p_maxq[$scalables] = $4;
            return;
        }

        if ($rest =~ /^\s*scale\s*(\d*)\/(\d*)\s*max\s*(\d*)\/(\d*)\s*$/m)
        {   $p_stdp[$scalables] = $1;
            $p_stdq[$scalables] = $2;
            $p_maxp[$scalables] = $3;
            $p_maxq[$scalables] = $4;
            return;
        }
        if ($rest =~ /^\s*scale\s*(\d*)\/(\d*)\s*min\s*(\d*)\/(\d*)\s*$/m)
        {   $p_stdp[$scalables] = $1;
            $p_stdq[$scalables] = $2;
            $p_minp[$scalables] = $3;
            $p_minq[$scalables] = $4;
            return;
        }
        if ($rest =~
  /^\s*scale\s*(\d*)\/(\d*)\s*min\s*(\d*)\/(\d*)\s+max\s*(\d*)\/(\d*)\s*$/m)
        {   $p_stdp[$scalables] = $1;
            $p_stdq[$scalables] = $2;
            $p_minp[$scalables] = $3;
            $p_minq[$scalables] = $4;
            $p_maxp[$scalables] = $5;
            $p_maxq[$scalables] = $6;
            return;
        }
    }

    if ($command =~ /^\s*sound\s+([a-zA-Z_0-9]*)\s*"(.*)"\s*(.*)$/m)
    {   $snumt = $1;
        $fxfile = $2;
        $repeats = $3;

        if ($snumt =~ /^\d+$/m)
        {   $snum = $snumt;
            if ($snum < $next_snum)
            {   error("sound number must be >= $next_snum to avoid clash");
            }
            else
            {   $next_snum = $snum + 1;
            }
        }
        else
        {   $snum = $next_snum;
            $next_snum = $next_snum + 1;
            if ($snumt ne "")
            {   identify("SOUND_$snumt", $snum);
            }
        }

        if ($repeats eq "music")
        {   sound2_chunk($snum, $fxfile);
            return;
        }
        if ($repeats eq "song")
        {   sound3_chunk($snum, $fxfile);
            return;
        }

        sound1_chunk($snum, $fxfile);
        if ($repeats =~ /^repeat\s+forever\s*$/m)
        {   $looped_fx[$repeaters] = $snum;
            $looped_num[$repeaters] = 0;
            $repeaters = $repeaters + 1;
            return;
        }

        if ($repeats =~ /^repeat\s+(\d*)\s*$/m)
        {   $looped_fx[$repeaters] = $snum;
            $looped_num[$repeaters] = $1;
            $repeaters = $repeaters + 1;
            return;
        }

        if ($repeats eq "") { return; }
    }
*/

    if (((strcmp(keyword, "palette"))==0)
    	|| ((strcmp(keyword, "picture"))==0)
        || ((strcmp(keyword, "resolution"))==0)
        || ((strcmp(keyword, "sound"))==0)) {
    	error("command not yet implemented in this minimal cBlorb");
		fprintf(stderr, "Line %d: command <%s> keyword <%s>\n", blurb_line, command, keyword);
        return;
    }

    if (((strcmp(keyword, "copyright"))==0)
    	|| ((strcmp(keyword, "palette"))==0)
    	|| ((strcmp(keyword, "picture"))==0)
        || ((strcmp(keyword, "release"))==0)
        || ((strcmp(keyword, "resolution"))==0)
        || ((strcmp(keyword, "sound"))==0)
        || ((strcmp(keyword, "storyfile"))==0)) {
    	error("incorrect syntax for this command");
        return;
    }

    error("no such blurb command"); fprintf(stderr, "Keyword: %s\n", keyword);
}

/* ---------- */


int main(int argc, char *argv[]) {
	char auth_chunk[2048], line[2048]; char *type; FILE *BLURB, *CHUNKSUB, *IFF;
	int i, x; char c; char *chunk_filename;

	if ((argc <= 1) || (argc > 3)) {
		fprintf(stderr, "usage: cblorb blurbfile [blorbfile]\n");
		exit(1);
	}

	sprintf(temp_dir, "%s", TEMP_PREFIX);
	sprintf(blurb_filename, argv[1]);

	if (argc == 3) {
		output_filename = argv[2];

#ifdef PLATFORM_MACOSX
		/* To avoid bug in OS X Inform.app */
		strcpy(temp_dir, output_filename);
		i = strlen(temp_dir)-1;
		while ((i>=0) && (temp_dir[i] != SEP_CHAR)) i--;
		if (i>=0) temp_dir[i] = 0;
		strcpy(blurb_filename, temp_dir);
		i = strlen(blurb_filename)-1;
		while ((i>=0) && (blurb_filename[i] != SEP_CHAR)) i--;
		if (i>=0) blurb_filename[i+1] = 0;
		sprintf(blurb_filename + strlen(blurb_filename), "Release.blurb");
#endif
#ifdef TRACE_CBLORB
		printf("Temporary area: %s\nBlorb filename: %s\n",
			temp_dir, blurb_filename);
#endif
	}


	filename_space = (char *) (stupidly_big);

	the_present = time(NULL);
	here_and_now = localtime(&the_present);
	sprintf(blorbdate, "%02d%02d%02d at %02d:%02d.%02d",
		here_and_now->tm_year-100, here_and_now->tm_mon + 1,
		here_and_now->tm_mday, here_and_now->tm_hour,
		here_and_now->tm_min, here_and_now->tm_sec);
	
	printf("! %s [executing on %s]\n", VERSION, blorbdate);
	printf("! The blorb spell (safely protect a small object ");
	printf("as though in a strong box).\n");

	sprintf(auth_chunk, "%d", here_and_now->tm_year+1900);
	set_variable_to("YEAR", auth_chunk);

	sprintf(auth_chunk, "%s on %s", VERSION, blorbdate);
	/* author_chunk(auth_chunk); */

	set_variable_to("BLURB", ""); /* Not the same meaning of blurb as below */

	BLURB = fopen(blurb_filename, "r");
	if (BLURB == NULL) fatal("can't open blurb file");

	i=0; c = ' ';
	while (c != EOF) {
		c = fgetc(BLURB);
		if ((c == EOF) || (c == '\x0a') || (c == '\x0d')) {
			line[i] = 0; interpret(line); i=0;
		} else line[i++] = c;
	}
	if (i>0) { line[i] = 0; interpret(line); }

	fclose(BLURB);

	/* if ($resolution_on == 1)
	{   
		begin_chunk("Reso", 0, "");
		four_word($r_stdx);
		four_word($r_stdy);
		four_word($r_minx);
		four_word($r_miny);
		four_word($r_maxx);
		four_word($r_maxy);
	
		for ($x=1; $x<=$scalables; $x=$x+1)
		{   four_word($p_picno[$x]);
			four_word($p_stdp[$x]);
			four_word($p_stdq[$x]);
	
			if ($p_minp[$x] == -1)
			{   $p_minp[$x] = $p_stdp[$x]; $p_minq[$x] = $p_stdq[$x]; }
	
			if ($p_maxp[$x] == -1)
			{   $p_maxp[$x] = $p_stdp[$x]; $p_maxq[$x] = $p_stdq[$x]; }
	
			four_word($p_minp[$x]);
			four_word($p_minq[$x]);
			four_word($p_maxp[$x]);
			four_word($p_maxq[$x]);    
		}
		end_chunk();
	}
	
	if ($repeaters > 0)
	{   begin_chunk("Loop", 0, "");
		for ($x=0; $x<$repeaters; $x = $x + 1)
		{   four_word($looped_fx[$x]);
			four_word($looped_num[$x]);        
		}
		end_chunk();
	} */

	/* Calculate the IFF file size */
	
	past_idx_offset = 12 + 12 + 12*indexed_count;
	iff_size = past_idx_offset + total_size;
	
	/* Now construct the IFF file from the chunks */

	IFF = fopen(output_filename, "wb");
	if (IFF == NULL) fatal_fs("can't open blorb file for output",
		output_filename);

	fprintf(IFF, "FORM");
	four_word(IFF, iff_size - 8);
	fprintf(IFF, "IFRS");
	
	fprintf(IFF, "RIdx");
	four_word(IFF, 4 + indexed_count*12);
	four_word(IFF, indexed_count);
	
	for (x = 0; x < chunk_count; x++)
	{   if (chunk_indexed_array[x] == 1) {
			type = chunk_id_array[x];
			if ((strcmp(type, "Snd1")==0) || (strcmp(type, "Snd2")==0) || (strcmp(type, "Snd3"))==0)
				type = "Snd ";
			fprintf(IFF, "%s", type);
			four_word(IFF, chunk_number_array[x]);
			four_word(IFF, past_idx_offset + chunk_offset_array[x]);
		}
	}
	
	for (x = 0; x <= max_resource_num; x++) {
		picture_numbering[x] = -1;
		sound_numbering[x] = -1;
	}
	
	pcount = 0; scount = 0;
	
#ifdef TRACE_CBLORB
	printf("Chunk table:\n");
	for (x = 0; x < chunk_count; x++) {
		printf("Chunk %02d %s %06x %s <%s>\n",
			x, chunk_id_array[x], chunk_size_array[x],
			(chunk_indexed_array[x] == 1)?"indexed":"unindexed",
			chunk_filename_array[x]);
	}
	printf("End of chunk table\n");	
#endif
	
	for (x = 0; x < chunk_count; x++) {
		type = chunk_id_array[x];
		if (strcmp(type, "Pict")==0)
		{   type = "PNG ";
			picture_numbering[chunk_number_array[x]] = x;
			pcount = pcount + 1;
		}
		if (strcmp(type, "Snd1")==0)
		{   type = "AIFF";
			sound_numbering[chunk_number_array[x]] = x;
			scount = scount + 1;
		}
		if (strcmp(type, "Snd2")==0)
		{   type = "MOD ";
			sound_numbering[chunk_number_array[x]] = x;
			scount = scount + 1;
		}
		if (strcmp(type, "Snd3")==0)
		{   type = "SONG";
			sound_numbering[chunk_number_array[x]] = x;
			scount = scount + 1;
		}
		if (strcmp(type, "Exec")==0)
		{   type = "ZCOD";
		}
	
		if (strcmp(type, "AIFF")!= 0)
		{   fprintf(IFF, "%s", type);
			four_word(IFF, (chunk_size_array[x]) - 8);
		}

		chunk_filename = chunk_filename_array[x];
#ifdef TRACE_CBLORB
		printf("Read from %s\n", chunk_filename);
#endif

		CHUNKSUB = fopen(chunk_filename, "rb");
		if (CHUNKSUB == NULL) fatal_fs("unable to read data",
			chunk_filename);
		for (i=0; i<chunk_size_array[x]-8; i++) {
			int j = fgetc(CHUNKSUB);
			if (j == EOF) fatal_fs("chunk ran out incomplete", chunk_filename);
			fputc(j, IFF);
		}
		fclose(CHUNKSUB);
#ifdef TRACE_CBLORB
		printf("Wrote %d bytes from %s\n", i, chunk_filename);
#endif
		if ((chunk_size_array[x] % 2) == 1) fputc(0, IFF);
	}
	
	fclose(IFF);

	if (error_count > 0) {
		printf("! Completed: %d error(s)\n", error_count);
		exit(1);
	}
	
	printf("! Completed: size %d bytes ", iff_size);
	printf("(%d pictures, %d sounds)\n", pcount, scount);

	return(0);
}
