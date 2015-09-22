[HTML::] HTML.

@Purpose: Utility functions for writing HTML.

@p Abstraction.
Though the code below does nothing at all interesting, it's written a little
defensively, to increase the chances that the user is producing valid HTML.

@d MAX_TAG_STACK_SIZE 16 /* check tags nested only this far */

@c
typedef struct HTML_file_state {
	int XHTML_flag;
	char *tag_stack[MAX_TAG_STACK_SIZE];
	int tag_sp;
	MEMORY_MANAGEMENT
} HTML_file_state;

void HTML::declare_as_HTML(OUTPUT_STREAM, int XHTML) {
	HTML_file_state *hs = CREATE(HTML_file_state);
	hs->XHTML_flag = XHTML;
	hs->tag_sp = 0;
	Streams::declare_as_HTML(OUT, hs);
}

void HTML::completed(OUTPUT_STREAM) {
	HTML_file_state *hs = CREATE(HTML_file_state);
	if (hs->tag_sp > 0) {
		for (int i=0; i < hs->tag_sp; i++)
			WRITE_TO(STDERR, "%d: %s\n", i, hs->tag_stack[i]);
		internal_error("HTML tags still open");
	}
}

@p Tags.

@c
void HTML::open(OUTPUT_STREAM, char *tag) {
	HTML::open_with_id_and_class(OUT, tag, NULL, NULL);
}
void HTML::open_with_id(OUTPUT_STREAM, char *tag, char *id) {
	HTML::open_with_id_and_class(OUT, tag, NULL, id);
}
void HTML::open_with_class(OUTPUT_STREAM, char *tag, char *class) {
	HTML::open_with_id_and_class(OUT, tag, class, NULL);
}

void HTML::open_with_id_and_class(OUTPUT_STREAM, char *tag, char *class, char *id) {
	HTML_file_state *hs = Streams::get_HTML_file_state(OUT);
	if (hs) {
		if (hs->tag_sp < MAX_TAG_STACK_SIZE) hs->tag_stack[hs->tag_sp] = tag;
		hs->tag_sp++;
	}
	WRITE("<%s", tag);
	if (class) WRITE(" class=\"%s\"", class);
	if (id) WRITE(" id=\"%s\"", id);
	WRITE(">");
}

@ On closure, we check that we're in a tag of the right kind, and throw
an internal error if not.

@c
void HTML::close(OUTPUT_STREAM, char *tag) {
	HTML_file_state *hs = Streams::get_HTML_file_state(OUT);
	if (hs) {
		hs->tag_sp--;
		if (hs->tag_sp < 0) {
			WRITE_TO(STDERR, "tag: %s\n", tag);
			internal_error("closed HTML tag which wasn't open");
		} else if (hs->tag_sp < MAX_TAG_STACK_SIZE) {
			if (CStrings::ne(tag, hs->tag_stack[hs->tag_sp])) {
				WRITE_TO(STDERR, "expected to close tag %s, but actually closed %s\n",
					hs->tag_stack[hs->tag_sp], tag);
				internal_error("closed HTML tag which wasn't open");
			}
		}
	}
	WRITE("</%s>", tag);
}

@p Self-closure.

@c
void HTML::self_closing(OUTPUT_STREAM) {
	HTML_file_state *hs = Streams::get_HTML_file_state(OUT);
	if (hs->XHTML_flag) WRITE(" /");
	WRITE(">");
}

@p Head.

@c
void HTML::begin_head(OUTPUT_STREAM, filename *CSS_file) {
	HTML_file_state *hs = Streams::get_HTML_file_state(OUT);
	if (hs->XHTML_flag) {
		WRITE("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" ");
		WRITE("\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
		WRITE("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	} else {
		WRITE("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" ");
		WRITE("\"http://www.w3.org/TR/html4/loose.dtd\">\n");
		WRITE("<html>\n");
	}
	HTML::open(OUT, "head"); WRITE("\n"); INDENT;
	WRITE("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"");
	HTML::self_closing(OUT);
	WRITE("\n");
	if (CSS_file) {
		WRITE("<link href=\"%/p\" rel=\"stylesheet\" type=\"text/css\"", CSS_file);
		HTML::self_closing(OUT);
		WRITE("\n");
	}
}

void HTML::end_head(OUTPUT_STREAM) {
	OUTDENT; HTML::close(OUT, "head"); WRITE("\n");
}

@p Body.

@c
void HTML::begin_body(OUTPUT_STREAM, char *class) {
	HTML::open_with_class(OUT, "body", class); WRITE("\n"); INDENT;
}

void HTML::end_body(OUTPUT_STREAM) {
	OUTDENT; HTML::close(OUT, "body"); WRITE("\n");
	HTML::close(OUT, "html"); WRITE("\n");
}

@p Divisions.

@c
void HTML::begin_div_with_id(OUTPUT_STREAM, char *id) {
	HTML::open_with_id(OUT, "div", id); WRITE("\n", id); INDENT;
}

void HTML::begin_div_with_class(OUTPUT_STREAM, char *cl) {
	HTML::open_with_class(OUT, "div", cl); WRITE("\n"); INDENT;
}

void HTML::begin_div_with_class_and_id(OUTPUT_STREAM, char *cl, char *id, int hide) {
	WRITE("<div class=\"%s\" id=\"%s\"", cl, id);
	if (hide) { WRITE(" style=\"display: none;\""); }
	WRITE(">\n"); INDENT;
}

void HTML::end_div(OUTPUT_STREAM) {
	OUTDENT; WRITE("</div>\n");
}

@p Images.

@c
void HTML::image(OUTPUT_STREAM, filename *F) {
	WRITE("<img src=\"%/f\"", F);
	HTML::self_closing(OUT);
}

@p Links.

@c
void HTML::anchor(OUTPUT_STREAM, text_stream *id) {
	WRITE("<a id=\"%S\"", id);
	HTML::self_closing(OUT);
}

void HTML::begin_link(OUTPUT_STREAM, text_stream *href) {
	WRITE("<a href=\"%S\">", href);
}

void HTML::end_link(OUTPUT_STREAM) {
	WRITE("</a>");
}

@p Miscellaneous.

@c
void HTML::comment(OUTPUT_STREAM, char *text) {
	WRITE("<!--%s-->\n", text);
}

void HTML::heading(OUTPUT_STREAM, char *tag, char *text) {
	WRITE("<%s>%s</%s>\n", tag, text, tag);
}

void HTML::hr(OUTPUT_STREAM, char *class) {
	WRITE("<hr");
	if (class) WRITE(" class=\"%s\"", class);
	HTML::self_closing(OUT);
	WRITE("\n");
}
