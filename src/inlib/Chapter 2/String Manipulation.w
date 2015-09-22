[Str::] String Manipulation.

@Purpose: Convenient routines for manipulating strings which are held in
memory as streams.

@p New strings.
Sometimes we want to make a new string in the sense of allocating more
memory to hold it. These objects won't automatically be destroyed, so we
shouldn't call these routines too casually.

The capacity of these strings is unlimited in principle, and the number
here is just the size of the initial memory block, which is fastest to
access.

@c
text_stream *Str::new(void) {
	return Str::new_with_capacity(32);
}

text_stream *Str::new_with_capacity(int c) {
	text_stream *S = CREATE(text_stream);
	if (Streams::open_to_memory(S, c)) return S;
	return NULL;
}

@ Here we open text streams initially equal to the given C strings, and
with the capacity of the initial block large enough to hold the whole
thing plus a little extra, for efficiency's sake.

@c
text_stream *Str::new_from_wide_string(wchar_t *C_string) {
	text_stream *S = CREATE(text_stream);
	if (Streams::open_from_wide_string(S, C_string)) return S;
	return NULL;
}

text_stream *Str::new_from_ISO_string(char *C_string) {
	text_stream *S = CREATE(text_stream);
	if (Streams::open_from_ISO_string(S, C_string)) return S;
	return NULL;
}

text_stream *Str::new_from_UTF8_string(char *C_string) {
	text_stream *S = CREATE(text_stream);
	if (Streams::open_from_UTF8_string(S, C_string)) return S;
	return NULL;
}

text_stream *Str::new_from_locale_string(char *C_string) {
	text_stream *S = CREATE(text_stream);
	if (Streams::open_from_locale_string(S, C_string)) return S;
	return NULL;
}

@ Duplication of an existing string is complicated only by the issue that
we want the duplicate always to be writeable, so that |NULL| can't be
duplicated as |NULL|.

@c
text_stream *Str::duplicate(text_stream *E) {
	if (E == NULL) return Str::new();
	text_stream *S = CREATE(text_stream);
	if (Streams::open_to_memory(S, E->chars_capacity)) {
		Streams::copy(S, E);
		return S;
	}
	return NULL;
}

@ And sometimes we want to use an existing stream object:

@c
text_stream *Str::from_wide_string(text_stream *S, wchar_t *c_string) {
	if (Streams::open_from_wide_string(S, c_string) == FALSE) return NULL;
	return S;
}

text_stream *Str::from_locale_string(text_stream *S, char *c_string) {
	if (Streams::open_from_locale_string(S, c_string) == FALSE) return NULL;
	return S;
}


@ Here's how we do the local stack thing:

@d STRING(S)
	text_stream S##_struct; text_stream *S = Str::from_wide_string(&S##_struct, NULL);

@d STRING_INIT(S, T)
	text_stream S##_struct; text_stream *S = Str::from_wide_string(&S##_struct, T);

@d STRING_FROM_LOCALE(S, T)
	text_stream S##_struct; text_stream *S = Str::from_locale_string(&S##_struct, T);

@d STRING_FINISHED_WITH(S)
	Streams::close(S); S = NULL;

@p Converting to C strings.

@c
void Str::copy_to_ISO_string(char *C_string, text_stream *S, int buffer_size) {
	Streams::write_as_ISO_string(C_string, S, buffer_size);
}

void Str::copy_to_UTF8_string(char *C_string, text_stream *S, int buffer_size) {
	Streams::write_as_UTF8_string(C_string, S, buffer_size);
}

void Str::copy_to_wide_string(wchar_t *C_string, text_stream *S, int buffer_size) {
	Streams::write_as_wide_string(C_string, S, buffer_size);
}

void Str::copy_to_locale_string(char *C_string, text_stream *S, int buffer_size) {
	Streams::write_as_locale_string(C_string, S, buffer_size);
}

@p Converting to integers.

@c
int Str::atoi(text_stream *S, int index) {
	char buffer[32];
	int i = 0;
	for (string_position P = Str::at(S, index); 
		((i < 31) && (P.index < Str::len(S))); P = Str::forward(P))
		buffer[i++] = (char) Str::get(P);
	buffer[i] = 0;
	return atoi(buffer);
}

@p Length.
A puritan would return a |size_t| here, but I am not a puritan.

@c
int Str::len(text_stream *S) {
	return Streams::get_position(S);
}

@p Position markers.
A position marker is a lightweight way to refer to a particular position
in a given string. Position 0 is before the first character; if, for
example, the string contains the word "gazpacho", then position 8 represents
the end of the string, after the "o". Negative positions are not allowed,
but positive ones well past the end of the string are legal. (Doing things
at those positions may well not be, of course.)

@c
typedef struct string_position {
	struct text_stream *S;
	int index;
} string_position;

@ You can then find a position in a given string thus:

@c
string_position Str::start(text_stream *S) {
	string_position P; P.S = S; P.index = 0; return P;
}

string_position Str::at(text_stream *S, int i) {
	if (i < 0) i = 0;
	if (i > Str::len(S)) i = Str::len(S);
	string_position P; P.S = S; P.index = i; return P;
}

string_position Str::end(text_stream *S) {
	string_position P; P.S = S; P.index = Str::len(S); return P;
}

@ And you can step forwards or backwards:

@c
string_position Str::back(string_position P) {
	if (P.index > 0) P.index--; return P;
}

string_position Str::forward(string_position P) {
	P.index++; return P;
}

string_position Str::plus(string_position P, int increment) {
	P.index += increment; return P;
}

int Str::width_between(string_position P1, string_position P2) {
	if (P1.S != P2.S) internal_error("positions are in different strings");
	return P2.index - P1.index;
}

int Str::in_range(string_position P) {
	if (P.index < Str::len(P.S)) return TRUE;
	return FALSE;
}

int Str::index(string_position P) {
	return P.index;
}

@ This leads to the following convenient loop macros:

@d LOOP_THROUGH_TEXT(P, ST)
	for (string_position P = Str::start(ST); P.index < Str::len(P.S); P = Str::forward(P))

@d LOOP_BACKWARDS_THROUGH_TEXT(P, ST)
	for (string_position P = Str::back(Str::end(ST)); P.index >= 0; P = Str::back(P))

@p Character operations.
How to get at individual characters, then, now that we can refer to positions:

@c
wchar_t Str::get(string_position P) {
	if ((P.S == NULL) || (P.index < 0) || (P.index > Str::len(P.S))) return 0;
	return Streams::get_char_at_index(P.S, P.index);
}

wchar_t Str::get_at(text_stream *S, int index) {
	return Str::get(Str::at(S, index));
}

wchar_t Str::get_first_char(text_stream *S) {
	return Str::get(Str::at(S, 0));
}

wchar_t Str::get_last_char(text_stream *S) {
	int L = Str::len(S);
	if (L == 0) return 0;
	return Str::get(Str::at(S, L-1));
}

void Str::put(string_position P, wchar_t C) {
	if (P.index < 0) internal_error("wrote before start of string");
	if (P.S == NULL) internal_error("wrote to null stream");
	int ext = Str::len(P.S);
	if (P.index > ext) internal_error("wrote beyond end of string");
	if (P.index == ext) {
		if (C) PUT_TO(P.S, (int) C);
		return;
	}
	Streams::put_char_at_index(P.S, P.index, C);
}

void Str::put_at(text_stream *S, int index, wchar_t C) {
	Str::put(Str::at(S, index), C);
}

@p Copying.

@c
void Str::clear(text_stream *S) {
	Str::truncate(S, 0);
}

void Str::truncate(text_stream *S, int len) {
	if (len < 0) len = 0;
	if (len < Str::len(S)) Str::put(Str::at(S, len), 0);
}

void Str::concatenate(text_stream *S1, text_stream *S2) {
	Streams::copy(S1, S2);
}

void Str::copy(text_stream *S1, text_stream *S2) {
	Str::clear(S1);
	Streams::copy(S1, S2);
}

void Str::copy_tail(text_stream *S1, text_stream *S2, int from) {
	Str::clear(S1);
	int L = Str::len(S2);
	if (from < L)
		for (string_position P = Str::at(S2, from); P.index < L; P = Str::forward(P))
			PUT_TO(S1, Str::get(P));
}

@ A subtly different operation is to set a string equal to a given C string:

@c
void Str::set_to_wide_string(text_stream *S, wchar_t *text) {
	Str::clear(S); WRITE_TO(S, "%w", text);
}

void Str::set_to_ISO_string(text_stream *S, char *text) {
	Str::clear(S); WRITE_TO(S, "%s", text);
}

@p Comparisons.

@c
int Str::eq(text_stream *S1, text_stream *S2) {
	if ((Str::len(S1) == Str::len(S2)) && (Str::cmp(S1, S2) == 0)) return TRUE;
	return FALSE;
}

int Str::eq_insensitive(text_stream *S1, text_stream *S2) {
	if ((Str::len(S1) == Str::len(S2)) && (Str::cmp_insensitive(S1, S2) == 0)) return TRUE;
	return FALSE;
}

int Str::ne(text_stream *S1, text_stream *S2) {
	if ((Str::len(S1) != Str::len(S2)) || (Str::cmp(S1, S2) != 0)) return TRUE;
	return FALSE;
}

int Str::ne_insensitive(text_stream *S1, text_stream *S2) {
	if ((Str::len(S1) != Str::len(S2)) || (Str::cmp_insensitive(S1, S2) != 0)) return TRUE;
	return FALSE;
}

int Str::cmp(text_stream *S1, text_stream *S2) {
	for (string_position P = Str::start(S1), Q = Str::start(S2);
		(P.index < Str::len(S1)) && (Q.index < Str::len(S2));
		P = Str::forward(P), Q = Str::forward(Q)) {
		int d = (int) Str::get(P) - (int) Str::get(Q);
		if (d != 0) return d;
	}
	return Str::len(S1) - Str::len(S2);
}

int Str::cmp_insensitive(text_stream *S1, text_stream *S2) {
	for (string_position P = Str::start(S1), Q = Str::start(S2);
		(P.index < Str::len(S1)) && (Q.index < Str::len(S2));
		P = Str::forward(P), Q = Str::forward(Q)) {
		int d = tolower((int) Str::get(P)) - tolower((int) Str::get(Q));
		if (d != 0) return d;
	}
	return Str::len(S1) - Str::len(S2);
}

@

@c
void Str::copy_ISO_string(text_stream *S, char *C_string) {
	Str::clear(S);
	Streams::write_ISO_string(S, C_string);
}

void Str::copy_UTF8_string(text_stream *S, char *C_string) {
	Str::clear(S);
	Streams::write_UTF8_string(S, C_string);
}

void Str::copy_wide_string(text_stream *S, wchar_t *C_string) {
	Str::clear(S);
	Streams::write_wide_string(S, C_string);
}

int Str::eq_C_string(text_stream *S1, wchar_t *S2) {
	if (Str::len(S1) == (int) wcslen(S2)) {
		int i=0;
		LOOP_THROUGH_TEXT(P, S1)
			if (Str::get(P) != S2[i++])
				return FALSE;
		return TRUE;	
	}
	return FALSE;
}

@ Remove spaces and tabs from both ends:

@c
void Str::trim_white_space(text_stream *S) {
	int len = Str::len(S), i = 0, j = 0;
	string_position F = Str::start(S);
	LOOP_THROUGH_TEXT(P, S) {
		if (!(Characters::is_space_or_tab(Str::get(P)))) { F = P; break; }
		i++;
	}
	LOOP_BACKWARDS_THROUGH_TEXT(Q, S) {
		if (!(Characters::is_space_or_tab(Str::get(Q)))) break;
		j++;
	}
	if (i+j > Str::len(S)) Str::truncate(S, 0);
	else {
		len = len - j;
		Str::truncate(S, len);
		if (i > 0) {
			string_position P = Str::start(S);
			wchar_t c = 0;
			do {
				c = Str::get(F);
				Str::put(P, c);
				P = Str::forward(P); F = Str::forward(F);
			} while (c != 0);
			len = len - i;
			Str::truncate(S, len);
		}
	}
}

@

@c
void Str::delete_first_character(text_stream *S) {
	LOOP_THROUGH_TEXT(P, S)
		Str::put(P, Str::get(Str::forward(P)));
}

@

@c
void Str::substr(OUTPUT_STREAM, string_position from, string_position to) {
	if (from.S != to.S) internal_error("substr on two different strings");
	for (int i = from.index; i < to.index; i++)
		PUT(Str::get_at(from.S, i));
}

int Str::begins_with_C_string(text_stream *S, wchar_t *prefix) {
	if ((prefix == NULL) || (*prefix == 0)) return TRUE;
	if (S == NULL) return FALSE;
	for (int i = 0; prefix[i]; i++)
		if (Str::get_at(S, i) != prefix[i])
			return FALSE;
	return TRUE;
}
