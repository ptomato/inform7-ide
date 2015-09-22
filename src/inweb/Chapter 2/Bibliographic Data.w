[Bibliographic::] Bibliographic Data.

@Purpose: To manage key-value pairs of bibliographic data, metadata if you like,
associated with a given web.

@p Storing data.
There are never more than a dozen or so key-value pairs, so we make no
effort to store these efficiently either for time or space.

@c
typedef struct bibliographic_datum {
	string key;
	string value;
	int declaration_permitted; /* is the contents page of the web allowed to set this? */
	int declaration_mandatory; /* is it positively required to? */
	int on_or_off; /* boolean: which we handle as the string "On" or "Off" */
	struct bibliographic_datum *alias;
	struct bibliographic_datum *next_bd; /* within the linked list for a web */
	MEMORY_MANAGEMENT
} bibliographic_datum;

@ We keep these in linked lists, and here's a convenient way to scan them:

@d LOOP_OVER_BIBLIOGRAPHIC_DATA(bd, W)
	for (bd = W->first_bd; bd; bd = bd->next_bd)

@ The following check the rules:

@c
int Bibliographic::datum_can_be_declared(web *W, char *key) {
	bibliographic_datum *bd = Bibliographic::look_up_datum(W, key);
	if (bd == NULL) return FALSE;
	return bd->declaration_permitted;
}

int Bibliographic::datum_on_or_off(web *W, char *key) {
	bibliographic_datum *bd = Bibliographic::look_up_datum(W, key);
	if (bd == NULL) return FALSE;
	return bd->on_or_off;
}

@p Initialising a web.
Each web has the following slate of data:

@c
void Bibliographic::initialise_data(web *W) {
	bibliographic_datum *bd;
	bd = Bibliographic::set_datum(W, "Inweb Build", INWEB_BUILD); bd->declaration_permitted = FALSE;

	bd = Bibliographic::set_datum(W, "Author", ""); bd->declaration_mandatory = TRUE;
	bd = Bibliographic::set_datum(W, "Language", ""); bd->declaration_mandatory = TRUE;
	bd = Bibliographic::set_datum(W, "Purpose", ""); bd->declaration_mandatory = TRUE;
	bd = Bibliographic::set_datum(W, "Title", ""); bd->declaration_mandatory = TRUE;

	bd = Bibliographic::set_datum(W, "License", "");
	bd->alias = Bibliographic::set_datum(W, "Licence", ""); /* alias US to UK spelling */

	Bibliographic::set_datum(W, "Short Title", "");
	Bibliographic::set_datum(W, "Capitalized Title", "");
	Bibliographic::set_datum(W, "Build Date", "");
	Bibliographic::set_datum(W, "Build Number", "");
	Bibliographic::set_datum(W, "Index Extras", "");
	Bibliographic::set_datum(W, "Index Template", "");

	bd = Bibliographic::set_datum(W, "Declare Section Usage", "On"); bd->on_or_off = TRUE;
	bd = Bibliographic::set_datum(W, "Namespaces", "Off"); bd->on_or_off = TRUE;
	bd = Bibliographic::set_datum(W, "Strict Usage Rules", "Off"); bd->on_or_off = TRUE;
}

@ Once the declarations for a web have been processed, the following is called
to check that all the mandatory declarations have indeed been made:

@c
void Bibliographic::check_required_data(web *W) {
	bibliographic_datum *bd;
	LOOP_OVER_BIBLIOGRAPHIC_DATA(bd, W)
		if ((bd->declaration_mandatory) &&
			(bd->value[0] == 0))
				Errors::fatal_with_C_string(
					"The Contents.w section does not specify '%s: ...'", bd->key);
}

@p Reading bibliographic data.
Key names are case-sensitive.

@c
char *Bibliographic::get_data(web *W, char *key) {
	bibliographic_datum *bd = Bibliographic::look_up_datum(W, key);
	if (bd) return bd->value;
	return "";
}

int Bibliographic::data_exists(web *W, char *key) {
	bibliographic_datum *bd = Bibliographic::look_up_datum(W, key);
	if ((bd) && (bd->value[0])) return TRUE;
	return FALSE;
}

bibliographic_datum *Bibliographic::look_up_datum(web *W, char *key) {
	bibliographic_datum *bd;
	LOOP_OVER_BIBLIOGRAPHIC_DATA(bd, W)
		if (CStrings::eq(key, bd->key)) {
			if (bd->alias) return bd->alias;
			return bd;
		}
	return NULL;
}

@p Writing bibliographic data.
Note that a key-value pair is created if the key doesn't exist at present,
so this routine never fails.

@c
bibliographic_datum *Bibliographic::set_datum(web *W, char *key, char *val) {
	bibliographic_datum *bd = Bibliographic::look_up_datum(W, key);
	if (bd == NULL) @<Create a new datum, then@>;
	CStrings::copy(bd->value, val);
	if (CStrings::eq(key, "Title")) @<Also set a capitalized form@>;
	return bd;
}

@ We add new data to the front of the web's linked list; though it doesn't
really matter where in the list it goes.

@<Create a new datum, then@> =
	bd = CREATE(bibliographic_datum);
	CStrings::copy(bd->key, key);
	bd->declaration_mandatory = FALSE;
	bd->declaration_permitted = TRUE;
	bd->on_or_off = FALSE;
	bd->alias = NULL;
	bd->next_bd = W->first_bd;
	W->first_bd = bd;

@ A slightly foolish feature, this; if text like "Wuthering Heights" is
written to the "Title" key, then a full-caps "WUTHERING HEIGHTS" is
written to a "Capitalized Title" key. (This enables cover sheets which
want to typeset the title in full caps to do so.)

@<Also set a capitalized form@> =
	string recapped; CStrings::copy(recapped, val);
	for (int i=0; recapped[i]; i++) CStrings::set_char(recapped, i, toupper(recapped[i]));
	Bibliographic::set_datum(W, "Capitalized Title", recapped);
