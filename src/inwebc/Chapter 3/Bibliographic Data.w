3/bibl: Bibliographic Data.

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
int bibliographic_datum_can_be_declared(web *W, char *key) {
	bibliographic_datum *bd = look_up_bibliographic_datum(W, key);
	if (bd == NULL) return FALSE;
	return bd->declaration_permitted;
}

int bibliographic_datum_on_or_off(web *W, char *key) {
	bibliographic_datum *bd = look_up_bibliographic_datum(W, key);
	if (bd == NULL) return FALSE;
	return bd->on_or_off;
}

@p Initialising a web.
Each web has the following slate of data:

@c
void initialise_bibliographic_data(web *W) {
	bibliographic_datum *bd;
	bd = set_bibliographic_data(W, "Inweb Build", INWEB_BUILD); bd->declaration_permitted = FALSE;

	bd = set_bibliographic_data(W, "Author", ""); bd->declaration_mandatory = TRUE;
	bd = set_bibliographic_data(W, "Language", ""); bd->declaration_mandatory = TRUE;
	bd = set_bibliographic_data(W, "Purpose", ""); bd->declaration_mandatory = TRUE;
	bd = set_bibliographic_data(W, "Title", ""); bd->declaration_mandatory = TRUE;

	bd = set_bibliographic_data(W, "License", "");
	bd->alias = set_bibliographic_data(W, "Licence", ""); /* alias US to UK spelling */

	set_bibliographic_data(W, "Short Title", "");
	set_bibliographic_data(W, "Capitalized Title", "");
	set_bibliographic_data(W, "Build Date", "");
	set_bibliographic_data(W, "Build Number", "");
	set_bibliographic_data(W, "Index Extras", "");
	set_bibliographic_data(W, "Index Template", "");

	bd = set_bibliographic_data(W, "Declare Section Usage", "On"); bd->on_or_off = TRUE;
	bd = set_bibliographic_data(W, "Namespaces", "Off"); bd->on_or_off = TRUE;
	bd = set_bibliographic_data(W, "Strict Usage Rules", "Off"); bd->on_or_off = TRUE;
}

@ Once the declarations for a web have been processed, the following is called
to check that all the mandatory declarations have indeed been made:

@c
void check_required_bibliographic_data(web *W) {
	bibliographic_datum *bd;
	LOOP_OVER_BIBLIOGRAPHIC_DATA(bd, W)
		if ((bd->declaration_mandatory) &&
			(bd->value[0] == 0))
				fatal_error_with_parameter(
					"The Contents.w section does not specify '%s: ...'", bd->key);
}

@p Reading bibliographic data.
Key names are case-sensitive.

@c
char *get_bibliographic_data(web *W, char *key) {
	bibliographic_datum *bd = look_up_bibliographic_datum(W, key);
	if (bd) return bd->value;
	return "";
}

int bibliographic_data_exists(web *W, char *key) {
	bibliographic_datum *bd = look_up_bibliographic_datum(W, key);
	if ((bd) && (bd->value[0])) return TRUE;
	return FALSE;
}

bibliographic_datum *look_up_bibliographic_datum(web *W, char *key) {
	bibliographic_datum *bd;
	LOOP_OVER_BIBLIOGRAPHIC_DATA(bd, W)
		if (in_string_eq(key, bd->key)) {
			if (bd->alias) return bd->alias;
			return bd;
		}
	return NULL;
}

@p Writing bibliographic data.
Note that a key-value pair is created if the key doesn't exist at present,
so this routine never fails.

@c
bibliographic_datum *set_bibliographic_data(web *W, char *key, char *val) {
	bibliographic_datum *bd = look_up_bibliographic_datum(W, key);
	if (bd == NULL) @<Create a new datum, then@>;
	in_strcpy(bd->value, val);
	if (in_string_eq(key, "Title")) @<Also set a capitalized form@>;
	return bd;
}

@ We add new data to the front of the web's linked list; though it doesn't
really matter where in the list it goes.

@<Create a new datum, then@> =
	bd = CREATE(bibliographic_datum);
	in_strcpy(bd->key, key);
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
	string recapped; in_strcpy(recapped, val);
	for (int i=0; recapped[i]; i++) in_set(recapped, i, toupper(recapped[i]));
	set_bibliographic_data(W, "Capitalized Title", recapped);
