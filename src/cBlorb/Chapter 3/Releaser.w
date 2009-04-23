3/rel: Releaser.

@Purpose: To manage requests to release material other than a Blorb file.

@Interface:

-- Owns struct request (private)

@Definitions:

@ If the previous section, ``Blorb Writer.w'', was the Lord High Executioner,
then this one is the Lord High Everything Else: it keeps track of requests
to write all kinds of interesting things which are {\it not} blorb files,
and then sees that they are carried out. The requests divide as follows:

@d COPY_REQ 0 /* a miscellaneous file */
@d IFICTION_REQ 1 /* the iFiction record of a project */
@d RELEASE_FILE_REQ 2 /* a template file */
@d RELEASE_SOURCE_REQ 3 /* the source text in HTML form */
@d SOLUTION_REQ 4 /* a solution file generated from the skein */
@d SOURCE_REQ 5 /* the source text of a project */
@d WEBSITE_REQ 6 /* a whole website */

@c
int website_requested = FALSE; /* has a |WEBSITE_REQ| been made? */

@ This would use a lot of memory if there were many requests, but there are
not and it does not.

@c
typedef struct request {
	int what_is_requested; /* one of the |*_REQ| values above */
	char details1[MAX_FILENAME_LENGTH];
	char details2[MAX_FILENAME_LENGTH];
	char details3[MAX_FILENAME_LENGTH];
	int private; /* is this request private, i.e., not to contribute to a website? */
	MEMORY_MANAGEMENT
} request;

@-------------------------------------------------------------------------------

@p Receiving requests.
These can have from 0 to 3 textual details attached:

@c
request *request_0(int kind, int privacy) {
	request *req = CREATE(request);
	req->what_is_requested = kind;
	req->details1[0] = 0;
	req->details2[0] = 0;
	req->details3[0] = 0;
	req->private = privacy;
	if (kind == WEBSITE_REQ) website_requested = TRUE;
	return req;
}

request *request_1(int kind, char *text1, int privacy) {
	request *req = request_0(kind, privacy);
	strcpy(req->details1, text1);
	return req;
}

request *request_2(int kind, char *text1, char *text2, int privacy) {
	request *req = request_0(kind, privacy);
	strcpy(req->details1, text1);
	strcpy(req->details2, text2);
	return req;
}

request *request_3(int kind, char *text1, char *text2, char *text3, int privacy) {
	request *req = request_0(kind, privacy);
	strcpy(req->details1, text1);
	strcpy(req->details2, text2);
	strcpy(req->details3, text3);
	return req;
}

@ A convenient abbreviation:

@c
/**/ void request_copy(char *from, char *to) {
	request_2(COPY_REQ, from, to, FALSE);
}

@p Any Last Requests.
Most of the requests are made as the parser reads commands from the blurb
script. At the end of that process, though, the following routine may add
further requests as consequences:

@c
void any_last_requests(void) {
	request_copy_of_auxiliaries();
	char *BIGCOVER = read_placeholder("BIGCOVER");
	if (BIGCOVER) {
		if (cover_is_in_JPEG_format) request_copy(BIGCOVER, "Cover.jpg");
		else request_copy(BIGCOVER, "Cover.png");
	}
	if (website_requested) {
		char *SMALLCOVER = read_placeholder("SMALLCOVER");
		if (SMALLCOVER) {
			if (cover_is_in_JPEG_format) request_copy(SMALLCOVER, "Small Cover.jpg");
			else request_copy(SMALLCOVER, "Small Cover.png");
		}
	}
}

@p Carrying out requests.

@c
/**/ void create_requested_material(void) {
	if (release_folder[0] == 0) return;
	printf("! Release folder: <%s>\n", release_folder);
	if (blorb_file_size > 0) declare_where_blorb_should_be_copied(release_folder);
	any_last_requests();
	request *req;
	LOOP_OVER(req, request) {
		switch (req->what_is_requested) {
			case WEBSITE_REQ: @<Create a website@>; break;
			case SOURCE_REQ: @<Create a plain text source file@>; break;
			case SOLUTION_REQ: @<Create a walkthrough file@>; break;
			case IFICTION_REQ: @<Create an iFiction file@>; break;
			case COPY_REQ: @<Copy a file into the release folder@>; break;
			case RELEASE_FILE_REQ: @<Release a file into the release folder@>; break;
			case RELEASE_SOURCE_REQ: @<Release source text as HTML into the release folder@>; break;
		}
	}
}

@

@<Create a walkthrough file@> =
	char Skein_filename[MAX_FILENAME_LENGTH];
	sprintf(Skein_filename, "%s%cSkein.skein", project_folder, SEP_CHAR);
	char solution_filename[MAX_FILENAME_LENGTH];
	sprintf(solution_filename, "%s%csolution.txt", release_folder, SEP_CHAR);
	walkthrough(Skein_filename, solution_filename);

@

@<Create a plain text source file@> =
	char source_text_filename[MAX_FILENAME_LENGTH];
	sprintf(source_text_filename, "%s%cSource%cstory.ni",
		project_folder, SEP_CHAR, SEP_CHAR);
	char write_to[MAX_FILENAME_LENGTH];
	sprintf(write_to, "%s%csource.txt", release_folder, SEP_CHAR);
	copy_file(source_text_filename, write_to);
		
@

@<Create an iFiction file@> =
	char iFiction_filename[MAX_FILENAME_LENGTH];
	sprintf(iFiction_filename, "%s%cMetadata.iFiction", project_folder, SEP_CHAR);
	char write_to[MAX_FILENAME_LENGTH];
	sprintf(write_to, "%s%ciFiction.xml", release_folder, SEP_CHAR);
	copy_file(iFiction_filename, write_to);

@

@<Copy a file into the release folder@> =
	char write_to[MAX_FILENAME_LENGTH];
	sprintf(write_to, "%s%c%s", release_folder, SEP_CHAR, req->details2);
	copy_file(req->details1, write_to);

@

@<Release a file into the release folder@> =
	release_file_into_website(req->details1, req->details2);

@

@<Release source text as HTML into the release folder@> =
	set_placeholder_to("SOURCEPREFIX", "source", 0);
	set_placeholder_to("SOURCELOCATION", req->details1, 0);
	set_placeholder_to("TEMPLATE", req->details3, 0);
	char *HTML_template = find_file_in_named_template(req->details3, req->details2);
	if (HTML_template == NULL) error_1("can't find HTML template file", req->details2);
	if (trace_mode) printf("! Web page %s from template %s\n", HTML_template, req->details3);
	web_copy_source(HTML_template, release_folder);

@ We copy the CSS file, if we need one; make the home page; and make any
other pages demanded by public released material. After that, it's up to
the template to add more if it wants to.

@<Create a website@> =
	char *t = read_placeholder("TEMPLATE");
	if (use_css_code_styles) {
		char *from = find_file_in_named_template(t, "style.css");
		if (from) {
			char CSS_filename[MAX_FILENAME_LENGTH];
			sprintf(CSS_filename, "%s%cstyle.css", release_folder, SEP_CHAR);
			copy_file(from, CSS_filename);
		}
	}
	release_file_into_website("index.html", t);
	request *req;
	LOOP_OVER(req, request)
		if (req->private == FALSE)
			switch (req->what_is_requested) {
				case WEBSITE_REQ: break;
				case SOLUTION_REQ: break;
				case IFICTION_REQ: break;
				case SOURCE_REQ:
					set_placeholder_to("SOURCEPREFIX", "source", 0);
						char source_text[MAX_FILENAME_LENGTH];
					sprintf(source_text, "%s%cSource%cstory.ni",
						project_folder, SEP_CHAR, SEP_CHAR);
					set_placeholder_to("SOURCELOCATION", source_text, 0);
					release_file_into_website("source.html", t); break;
			}
	@<Add further material as requested by the template@>;

@ Most templates do not request extra files, but they have the option by
including a manifest called ``(extras).txt'':

@<Add further material as requested by the template@> =
	char *from = find_file_in_named_template(t, "(extras).txt");
	if (from) { /* i.e., if the ``(extras).txt'' file exists */
		file_read(from, "can't open (extras) file", FALSE, read_requested_file, 0);
	}

@ If so, then |read_requested_file| is called for each line; we trim white
space and expect the result to be a filename of something within the template.

@c
void read_requested_file(char *filename, text_file_position *tfp) {
	filename = trim_white_space(filename);
	if (filename[0] == 0) return;
	release_file_into_website(filename, read_placeholder("TEMPLATE"));
}

@ There are really three cases when we release something from a website
template. We can copy it verbatim as a binary file, we can expand placeholders
but otherwise copy as a single item, or we can use it to make a mass
generation of source pages.

@c
void release_file_into_website(char *name, char *t) {
	char write_to[MAX_FILENAME_LENGTH];
	sprintf(write_to, "%s%c%s", release_folder, SEP_CHAR, name);

	char *from = find_file_in_named_template(t, name);
	if (from == NULL) {
		error_1("unable to find file in website template", name);
		return;
	}

	if (strcmp(get_filename_extension(name), ".html") == 0)
		@<Release an HTML page from the template into the website@>
	else
		@<Release a binary file from the template into the website@>;
}

@ ``Source.html'' is a special case, as it expands into a whole suite of
pages automagically. Otherwise we work out the filenames and then hand over
to the experts.

@<Release an HTML page from the template into the website@> =
	set_placeholder_to("TEMPLATE", t, 0);
	if (trace_mode) printf("! Web page %s from template %s\n", name, t);
	if (strcmp(name, "source.html") == 0)
		web_copy_source(from, release_folder);
	else
		web_copy(from, write_to);

@

@<Release a binary file from the template into the website@> =
	if (trace_mode) printf("! Binary file %s from template %s\n", name, t);
	copy_file(from, write_to);

@ The home page will need links to any public released resources, and this
is where those are added (to the other links already present, that is).

@c
/**/ void add_links_to_requested_resources(FILE *COPYTO) {
	request *req;
	LOOP_OVER(req, request)
		if (req->private == FALSE)
			switch (req->what_is_requested) {
				case WEBSITE_REQ: break;
				case SOURCE_REQ:
					fprintf(COPYTO, "<li>");
					download_link(COPYTO, "Source Text", NULL, "source.html", "link");
					fprintf(COPYTO, "</li>");
					break;
				case SOLUTION_REQ:
					fprintf(COPYTO, "<li>");
					download_link(COPYTO, "Solution", NULL, "solution.txt", "link");
					fprintf(COPYTO, "</li>");
					break;
				case IFICTION_REQ:
					fprintf(COPYTO, "<li>");
					download_link(COPYTO, "Library Card", NULL, "iFiction.xml", "link");
					fprintf(COPYTO, "</li>");
					break;
			}
}

@p Blorb relocation.
This is a little dodge used to make the process of releasing games in
Inform 7 more seamless: see the manual for an explanation.

@c
void declare_where_blorb_should_be_copied(char *path) {
	char *leaf = read_placeholder("STORYFILE");
	if (leaf == NULL) leaf = "Story";
	printf("Copy blorb to: [[%s%c%s]]\n", path, SEP_CHAR, leaf);
}
