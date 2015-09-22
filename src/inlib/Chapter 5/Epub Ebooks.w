[Epub::] Epub Ebooks.

@Purpose: To provide for wrapping up woven HTML into ePub ebooks.

@p Ebooks.
Constructing an ePub file (essentially a zipped folder of HTML with some
metadata attached) is simple enough, but the details are finicky. The
HTML pages need to be fully XHTML compliant, which is quite a strict
requirement, and we can't help with that here. But we can at least sort
out the directory structure and the rather complicated indexing and
contents files also required.

See Liza Daly's invaluable tutorial "Build a digital book with EPUB" to
explicate all of this.

While under construction, any single ebook is represented by an instance
of the following structure. Conceptually, we will organise it as a series
of "volumes" (possibly only one), each of which is a series of "chapters"
(possibly only one). The actual content is a series of "pages", which are
essentially individual HTML files, plus some images.

@c
typedef struct ebook {
	struct ebook_datum *metadata_list; /* DCMI-standard bibliographic data */

	struct filename *eventual_epub; /* filename of the final |*.epub| to be made */
	struct pathname *holder; /* directory to put the ingredients into */
	struct pathname *OEBPS_path; /* subdirectory which mysteriously has to be called |OEBPS| */
	struct filename *CSS_file; /* where to find the CSS file to be included */

	struct ebook_volume *ebook_volume_list;
	struct ebook_volume *current_volume; /* the one to which chapters are now being added */

	struct ebook_chapter *ebook_chapter_list;
	struct ebook_chapter *current_chapter; /* the one to which pages are now being added */

	struct ebook_page *ebook_page_list;
	struct ebook_image *ebook_image_list;
	MEMORY_MANAGEMENT
} ebook;

@ DCMI, or "Dublin Core", metadata is a standard set of key-value pairs used to
identify ebooks; we need to maintain a small dictionary, and so small that a
list is entirely sufficient.

@c
typedef struct ebook_datum {
	struct text_stream *key;
	struct text_stream *value;
	struct ebook_datum *next_metadatum;
	MEMORY_MANAGEMENT
} ebook_datum;

@ As noted above, we use the following to stratify the book:

@c
typedef struct ebook_volume {
	struct text_stream *volume_title;
	struct ebook_page *volume_starts; /* on which page the volume starts */
	struct ebook_volume *next_ebook_volume;
	MEMORY_MANAGEMENT
} ebook_volume;

typedef struct ebook_chapter {
	struct text_stream *chapter_title;
	struct ebook_volume *in_volume; /* to which volume this chapter belongs */
	struct ebook_page *chapter_starts; /* on which page the chapter starts */
	struct ebook_chapter *next_ebook_chapter;
	MEMORY_MANAGEMENT
} ebook_chapter;

@ Now for the actual resources which will end up in the EPUB. Here are the
pages:

@c
typedef struct ebook_page {
	struct text_stream *page_title;
	struct text_stream *page_type;
	struct text_stream *page_ID;

	struct filename *relative_URL; /* eventual URL of this page within the ebook */

	struct ebook_volume *in_volume; /* to which volume this page belongs */
	struct ebook_chapter *in_chapter; /* to which chapter this page belongs */

	struct ebook_page *next_ebook_page;
	int nav_entry_written; /* keep track of what we've written to the navigation tree */

	MEMORY_MANAGEMENT
} ebook_page;

typedef struct ebook_image {
	struct text_stream *image_ID;
	struct filename *relative_URL; /* eventual URL of this image within the ebook */
	struct ebook_image *next_ebook_image;
	MEMORY_MANAGEMENT
} ebook_image;

@ The above structures use many linked lists, and to save time, here's some
uniform code for building those. It's not efficient if the lists have sizes
much past 10000, but that's never going to arise.

@d ADD_TO_LL(B, P, type) {
	P->next_##type = NULL;
	if (B->type##_list == NULL) B->type##_list = P;
	else {
		type *Q = B->type##_list;
		while ((Q) && (Q->next_##type)) Q = Q->next_##type;
		Q->next_##type = P;
	}
}

@p Creation.

@c
ebook *Epub::new(text_stream *title) {
	ebook *B = CREATE(ebook);
	B->metadata_list = NULL;
	B->OEBPS_path = NULL;
	B->CSS_file = NULL;
	B->ebook_page_list = NULL;
	B->ebook_image_list = NULL;
	B->ebook_volume_list = NULL; B->current_volume = NULL;
	B->ebook_chapter_list = NULL; B->current_chapter = NULL;
	B->eventual_epub = NULL;
	Epub::attach_metadata(B, L"title", title);
	return B;
}

void Epub::use_CSS(ebook *B, filename *F) {
	B->CSS_file = F;
}

text_stream *Epub::attach_metadata(ebook *B, wchar_t *K, text_stream *V) {
	ebook_datum *L = NULL;
	for (ebook_datum *D = B->metadata_list; D; L = D, D = D->next_metadatum)
		if (Str::eq_C_string(D->key, K)) {
			WRITE_TO(D->value, "%s", V);
			return D->value;
		}
	ebook_datum *D = CREATE(ebook_datum);
	D->key = Str::new_from_wide_string(K);
	D->value = Str::duplicate(V);
	D->next_metadatum = NULL;
	if (L) L->next_metadatum = D; else B->metadata_list = D;
	return D->value;
}

text_stream *Epub::get_metadata(ebook *B, wchar_t *K) {
	for (ebook_datum *D = B->metadata_list; D; D = D->next_metadatum)
		if (Str::eq_C_string(D->key, K))
			return D->value;
	return NULL;
}

text_stream *Epub::ensure_metadata(ebook *B, wchar_t *K) {
	text_stream *S = Epub::get_metadata(B, K);
	if (S == NULL) S = Epub::attach_metadata(B, K, NULL);
	return S;
}

ebook_page *Epub::note_page(ebook *B, filename *F, char *title, char *type) {
	ebook_page *P = CREATE(ebook_page);
	P->relative_URL = F;
	P->nav_entry_written = FALSE;
	P->in_volume = B->current_volume;
	P->in_chapter = B->current_chapter;
	P->page_title = Str::new_from_ISO_string(title);
	P->page_type = Str::new_from_ISO_string(type);
	
	P->page_ID = Str::new();
	WRITE_TO(P->page_ID, "P");
	Filenames::write_unextended_leafname(P->page_ID, F);
	LOOP_THROUGH_TEXT(pos, P->page_ID) {
		wchar_t c = Str::get(pos);
		if ((c == '-') || (c == ' ')) Str::put(pos, '_');
	}

	ADD_TO_LL(B, P, ebook_page);
	return P;
}

void Epub::note_image(ebook *B, filename *F) {
	ebook_image *I = CREATE(ebook_image);
	I->relative_URL = F;
	I->image_ID = Str::new();
	Filenames::write_unextended_leafname(I->image_ID, F);
	ADD_TO_LL(B, I, ebook_image);
}

ebook_volume *Epub::starts_volume(ebook *B, ebook_page *P, char *title) {
	ebook_volume *V = CREATE(ebook_volume);
	V->volume_starts = P;
	P->in_volume = V;
	V->volume_title = Str::new_from_ISO_string(title);
	B->current_volume = V;
	ADD_TO_LL(B, V, ebook_volume);
	return V;
}

ebook_chapter *Epub::starts_chapter(ebook *B, ebook_page *P, char *title) {
	ebook_chapter *C = CREATE(ebook_chapter);
	C->chapter_starts = P;
	C->in_volume = B->current_volume;
	P->in_chapter = C;
	C->chapter_title = Str::new_from_ISO_string(title);
	B->current_chapter = C;
	ADD_TO_LL(B, C, ebook_chapter);
	return C;
}

@p Construction.

@c
pathname *Epub::begin_construction(ebook *B, pathname *P, filename *cover_image) {
	if (Pathnames::create_in_file_system(P) == FALSE) return NULL;
	
	TEMPORARY_TEXT(TEMP)
	WRITE_TO(TEMP, "%S.epub", Epub::get_metadata(B, L"title"));
	B->eventual_epub = Filenames::in_folder_S(P, TEMP);
	DISCARD_TEXT(TEMP)

	pathname *Holder = Pathnames::subfolder(P, "ePub");
	if (Pathnames::create_in_file_system(Holder) == FALSE) return NULL;
	B->holder = Holder;

	@<Write the EPUB mimetype file@>;
	@<Write the EPUB meta-inf directory@>;
	pathname *OEBPS = Pathnames::subfolder(Holder, "OEBPS");
	if (Pathnames::create_in_file_system(OEBPS) == FALSE) return NULL;
	if (cover_image) @<Make the cover image page@>;
	B->OEBPS_path = OEBPS;
	return OEBPS;
}

@

@<Write the EPUB mimetype file@> =
	filename *Mimetype = Filenames::in_folder(Holder, "mimetype");
	text_stream EM_struct; text_stream *OUT = &EM_struct;
	if (STREAM_OPEN_TO_FILE(OUT, Mimetype, ISO_ENC) == FALSE)
		Errors::fatal_with_file("unable to open mimetype file for output: %f",
			Mimetype);
	WRITE("application/epub+zip"); /* EPUB requires there be no newline here */
	STREAM_CLOSE(OUT);

@

@<Write the EPUB meta-inf directory@> =
	pathname *META_INF = Pathnames::subfolder(Holder, "META-INF");
	if (Pathnames::create_in_file_system(META_INF) == FALSE) return NULL;
	filename *container = Filenames::in_folder(META_INF, "container.xml");
	text_stream C_struct; text_stream *OUT = &C_struct;
	if (STREAM_OPEN_TO_FILE(OUT, container, ISO_ENC) == FALSE)
		Errors::fatal_with_file("unable to open container file for output: %f",
			container);
	WRITE("<?xml version=\"1.0\"?>\n");
	WRITE("<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n");
	INDENT;
	WRITE("<rootfiles>\n");
	INDENT;
	WRITE("<rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\" />\n");
	OUTDENT;
	WRITE("</rootfiles>\n");
	OUTDENT;
	WRITE("</container>\n");
	STREAM_CLOSE(OUT);

@ It's a much-lamented fact that EPUB 2, at any rate, has no standard way
to define cover images, and different readers behave slightly differently.
But the following seems to work with iTunes 9.1 and later, and therefore
on Apple devices. (See Keith Fahlgren's post "Best practices in ePub cover
images" at the ThreePress Consulting blog.)

@<Make the cover image page@> =
	filename *cover = Filenames::in_folder(OEBPS, "cover.html");
	text_stream C_struct; text_stream *OUT = &C_struct;
	if (STREAM_OPEN_TO_FILE(OUT, cover, ISO_ENC) == FALSE)
		Errors::fatal_with_file("unable to open cover file for output: %f",
			cover);

	Epub::note_page(B, cover_image, "Cover", "cover");

	HTML::declare_as_HTML(OUT, TRUE);
	HTML::begin_head(OUT, NULL);
	HTML::open(OUT, "title");
	WRITE("Cover");
	HTML::close(OUT, "title");
	WRITE("<style type=\"text/css\"> img { max-width: 100%%; } </style>\n");
	HTML::end_head(OUT);
	HTML::begin_body(OUT, NULL);
	HTML::begin_div_with_id(OUT, "cover-image");
	WRITE("<img src=\"%/f\" alt=\"%S\"/>\n", cover_image, Epub::get_metadata(B, L"title"));
	HTML::end_div(OUT);
	HTML::end_body(OUT);
	HTML::completed(OUT);
	STREAM_CLOSE(OUT);

@

@c
void Epub::end_construction(ebook *B) {
	@<Attach default metadata@>;
	@<Write the EPUB OPF file@>;
	@<Write the EPUB NCX file@>;
	@<Zip the EPUB@>;
}

@

@<Attach default metadata@> =
	text_stream *datestamp = Epub::ensure_metadata(B, L"date");
	if (Str::len(datestamp) == 0) {
		struct tm *the_present = NULL;
		time_t now = time(NULL);
		the_present = localtime(&now);
		WRITE_TO(datestamp, "%04d-%02d-%02d", the_present->tm_year + 1900,
			(the_present->tm_mon)+1, the_present->tm_mday);
	}

	TEMPORARY_TEXT(TEMP)
	WRITE_TO(TEMP, "urn:www.inform7.com:");
	text_stream *identifier = Epub::ensure_metadata(B, L"identifier");
	if (Str::len(identifier) == 0)
		WRITE_TO(TEMP, "%S", Epub::get_metadata(B, L"title"));
	else
		WRITE_TO(TEMP, "%S", identifier);
	Str::copy(identifier, TEMP);
	DISCARD_TEXT(TEMP)

	text_stream *lang = Epub::ensure_metadata(B, L"language");
	if (Str::len(lang) == 0) WRITE_TO(lang, "en-UK");

@

@<Write the EPUB OPF file@> =
	filename *content = Filenames::in_folder(B->OEBPS_path, "content.opf");
	text_stream C_struct; text_stream *OUT = &C_struct;
	if (STREAM_OPEN_TO_FILE(OUT, content, UTF8_ENC) == FALSE)
		Errors::fatal_with_file("unable to open content file for output: %f",
			content);
	
	WRITE("<?xml version='1.0' encoding='utf-8'?>\n");
	WRITE("<package xmlns=\"http://www.idpf.org/2007/opf\"\n");
	WRITE("xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
	WRITE("unique-identifier=\"bookid\" version=\"2.0\">\n"); INDENT;
	@<Write the OPF metadata@>;
	@<Write the OPF manifest@>;
	@<Write the OPF spine@>;
	@<Write the OPF guide@>;
	OUTDENT; WRITE("</package>\n");

	STREAM_CLOSE(OUT);

@ The metadata here conforms to the Dublin Core Metadata Initiative (ebook_datum).
(Other default values are set in the configuration file.)

@<Write the OPF metadata@> =
	WRITE("<metadata>\n"); INDENT;
	for (ebook_datum *D = B->metadata_list; D; D = D->next_metadatum) {
		WRITE("<dc:%S", D->key);
		if (Str::eq_C_string(D->key, L"identifier")) WRITE(" id=\"bookid\"");
		WRITE(">");
		WRITE("%S</dc:%S>\n", D->value, D->key);
	}
	WRITE("<meta name=\"cover\" content=\"cover-image\" />\n");
	OUTDENT; WRITE("</metadata>\n");

@

@<Write the OPF manifest@> =
	WRITE("<manifest>\n"); INDENT;
	WRITE("<item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n");
	@<Manifest the CSS files@>;
	@<Manifest the XHTML files@>;
	@<Manifest the images@>;
	OUTDENT; WRITE("</manifest>\n");

@

@<Manifest the CSS files@> =
	if (B->CSS_file)
		WRITE("<item id=\"css\" href=\"%S\" media-type=\"text/css\"/>\n",
			Filenames::get_leafname(B->CSS_file));

@

@<Manifest the XHTML files@> =
	for (ebook_page *P = B->ebook_page_list; P; P = P->next_ebook_page)
		WRITE("<item id=\"%S\" href=\"%S\" media-type=\"application/xhtml+xml\"/>\n",
			P->page_ID, Filenames::get_leafname(P->relative_URL));

@

@<Manifest the images@> =
	for (ebook_image *I = B->ebook_image_list; I; I = I->next_ebook_image) {
		char *image_type = "";
		switch (Filenames::guess_format(I->relative_URL)) {
			case FORMAT_PERHAPS_PNG: image_type = "png"; break;
			case FORMAT_PERHAPS_JPEG: image_type = "jpeg"; break;
			case FORMAT_PERHAPS_SVG: image_type = "svg"; break;
			case FORMAT_PERHAPS_GIF: image_type = "gif"; break;
			default: Errors::nowhere("image not .gif, .png, .jpg or .svg"); break;
		}
		WRITE("<item id=\"%S\" href=\"%S\" media-type=\"image/%s\"/>\n",
			I->image_ID, Filenames::get_leafname(I->relative_URL), image_type);
	}

@

@<Write the OPF spine@> =
	WRITE("<spine toc=\"ncx\">\n"); INDENT;
	for (ebook_page *P = B->ebook_page_list; P; P = P->next_ebook_page) {
		WRITE("<itemref idref=\"%S\"", P->page_ID);
		if (Str::len(P->page_type) > 0) WRITE(" linear=\"no\"");
		WRITE("/>\n");
	}
	OUTDENT; WRITE("</spine>\n");

@

@<Write the OPF guide@> =
	WRITE("<guide>\n"); INDENT;
	for (ebook_page *P = B->ebook_page_list; P; P = P->next_ebook_page) {
		if (Str::len(P->page_type) > 0) {
			WRITE("<reference href=\"%S\" type=\"%S\" title=\"%S\"/>\n",
				Filenames::get_leafname(P->relative_URL), P->page_type, P->page_title);
		}
	}
	OUTDENT; WRITE("</guide>\n");

@ The NCX duplicates some of what's in the OPF file, for historical reasons;
it's left over from an earlier standard used by book-readers for visually
impaired people.

@<Write the EPUB NCX file@> =
	filename *toc = Filenames::in_folder(B->OEBPS_path, "toc.ncx");
	text_stream C_struct; text_stream *OUT = &C_struct;
	if (STREAM_OPEN_TO_FILE(OUT, toc, UTF8_ENC) == FALSE)
		Errors::fatal_with_file("unable to open ncx file for output: %f",
			toc);

	WRITE("<?xml version='1.0' encoding='utf-8'?>\n");
	WRITE("<!DOCTYPE ncx PUBLIC \"-//NISO//DTD ncx 2005-1//EN\"\n");
	WRITE("	\"http://www.daisy.org/z3986/2005/ncx-2005-1.dtd\">\n");
	WRITE("<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n");

	int depth = 1; /* there are surely at least sections */
	if (B->ebook_chapter_list) depth = 2;
	if (B->ebook_volume_list) depth = 3;

	@<Write the NCX metadata@>;
	@<Write the NCX navigation map@>;
	WRITE("</ncx>\n");

	STREAM_CLOSE(OUT);

@

@<Write the NCX metadata@> =
	WRITE("<head>\n"); INDENT;
	WRITE("<meta name=\"dtb:uid\" content=\"%S\"/>\n", Epub::get_metadata(B, L"identifier"));
	WRITE("<meta name=\"dtb:depth\" content=\"%d\"/>\n", depth);
	WRITE("<meta name=\"dtb:totalPageCount\" content=\"0\"/>\n");
	WRITE("<meta name=\"dtb:maxPageNumber\" content=\"0\"/>\n");
	OUTDENT; WRITE("</head>\n");
	WRITE("<docTitle>\n"); INDENT;
	WRITE("<text>%S</text>\n", Epub::get_metadata(B, L"title"));
	OUTDENT; WRITE("</docTitle>\n");

@

@<Write the NCX navigation map@> =
	WRITE("<navMap>\n"); INDENT;
	int navpoint_count = 1;
	int navmap_depth = 1;
	int phase = 0;
	@<Include the non-section pages in this phase@>;
	ebook_volume *V = NULL;
	for (V = B->ebook_volume_list; V; V = V->next_ebook_volume) {
		@<Begin navPoint@>;
		WRITE("<navLabel><text>%S</text></navLabel>", V->volume_title);
		WRITE("<content src=\"%S\"/>\n", Filenames::get_leafname(V->volume_starts->relative_URL));
		@<Include the chapters and sections in this volume@>;
		@<End navPoint@>;
	}
	@<Include the chapters and sections in this volume@>;
	phase = 1;
	@<Include the non-section pages in this phase@>;
	OUTDENT; WRITE("</navMap>\n");
	if (navmap_depth != 1) internal_error("navMap numbering unbalanced");

@

@<Include the non-section pages in this phase@> =
	for (ebook_page *P = B->ebook_page_list; P; P = P->next_ebook_page) {
		int in_phase = 1;
		if ((Str::eq_C_string(P->page_ID, L"cover")) ||
			(Str::eq_C_string(P->page_ID, L"index")))
			in_phase = 0;
		if ((in_phase == phase) && (P->nav_entry_written == FALSE)) {
			@<Begin navPoint@>;
			WRITE("<navLabel><text>%S</text></navLabel> <content src=\"%S\"/>\n",
				P->page_title, Filenames::get_leafname(P->relative_URL));
			@<End navPoint@>;
		}
	}

@

@<Include the chapters and sections in this volume@> =
	ebook_chapter *C = NULL;
	for (C = B->ebook_chapter_list; C; C = C->next_ebook_chapter)
		if (C->in_volume == V) {
			@<Begin navPoint@>;
			WRITE("<navLabel><text>%S</text></navLabel>", C->chapter_title);
			WRITE("<content src=\"%S\"/>\n", Filenames::get_leafname(C->chapter_starts->relative_URL));
			@<Include the sections in this chapter@>;
			@<End navPoint@>;
		}
	@<Include the sections in this chapter@>;

@

@<Include the sections in this chapter@> =
	for (ebook_page *P = B->ebook_page_list; P; P = P->next_ebook_page) {
		if ((P->in_chapter == C) && (P->nav_entry_written == FALSE)) {
			@<Begin navPoint@>;
			WRITE("<navLabel><text>%S</text></navLabel>", P->page_title);
			WRITE("<content src=\"%S\"/>\n", Filenames::get_leafname(P->relative_URL));
			@<End navPoint@>;
			P->nav_entry_written = TRUE;
		}
	}

@

@<Begin navPoint@> =
	WRITE("<navPoint id=\"navpoint-%d\" playOrder=\"%d\">\n", navpoint_count, navpoint_count);
	navpoint_count++;
	navmap_depth++; INDENT;

@

@<End navPoint@> =
	navmap_depth--;
	if (navmap_depth < 1) internal_error("navMap numbering awry");
	OUTDENT; WRITE("</navPoint>\n");

@

@<Zip the EPUB@> =
	pathname *up = Pathnames::from_string("..");
	filename *ePub_relative =
		Filenames::in_folder_S(up, Filenames::get_leafname(B->eventual_epub));
	@<Issue first zip instruction@>;
	@<Issue second zip instruction@>;

@

@<Issue first zip instruction@> =
	TEMPORARY_TEXT(COMMAND)
	Shell::plain(COMMAND, "cd ");
	Shell::quote_path(COMMAND, B->holder);
	Shell::plain(COMMAND, "; zip -0Xq ");
	Shell::quote_file(COMMAND, ePub_relative);
	Shell::plain(COMMAND, " mimetype");
	Shell::run(COMMAND);
	DISCARD_TEXT(COMMAND)

@

@<Issue second zip instruction@> =
	TEMPORARY_TEXT(COMMAND)
	Shell::plain(COMMAND, "cd ");
	Shell::quote_path(COMMAND, B->holder);
	Shell::plain(COMMAND, "; zip -Xr9Dq ");
	Shell::quote_file(COMMAND, ePub_relative);
	Shell::plain(COMMAND, " *");
	Shell::run(COMMAND);
	DISCARD_TEXT(COMMAND)
