[Characters::] Characters.

@Purpose: Individual characters.

@p Character classes.
These are abstracted because they're a traditional source of awkwardness
to do with char versus int versus unsigned char types:

@c
char Characters::tolower(char c) {
	return (char) tolower((int) c);
}
char Characters::toupper(char c) {
	return (char) toupper((int) c);
}
int Characters::isalpha(char c) {
	return isalpha((int) c);
}
int Characters::isdigit(char c) {
	return isdigit((int) c);
}
int Characters::is_space_or_tab(int c) {
	if ((c == ' ') || (c == '\t')) return TRUE;
	return FALSE;
}

@ Wider versions:

@c
wchar_t Characters::w_tolower(wchar_t c) {
	return (wchar_t) tolower((int) c);
}
wchar_t Characters::w_toupper(wchar_t c) {
	return (wchar_t) toupper((int) c);
}
int Characters::w_isalpha(wchar_t c) {
	return isalpha((int) c);
}
int Characters::w_isdigit(wchar_t c) {
	return isdigit((int) c);
}

@p Unicode composition.
A routine which converts the Unicode combining accents with letters,
sufficient correctly to handle all characters in the ZSCII set.

@c
int Characters::combine_accent(int accent, int letter) {
	switch(accent) {
		case 0x0300: /* Unicode combining grave */
			switch(letter) {
				case 'a': return 0xE0; case 'e': return 0xE8; case 'i': return 0xEC;
				case 'o': return 0xF2; case 'u': return 0xF9;
				case 'A': return 0xC0; case 'E': return 0xC8; case 'I': return 0xCC;
				case 'O': return 0xD2; case 'U': return 0xD9;
			}
			break;
		case 0x0301: /* Unicode combining acute */
			switch(letter) {
				case 'a': return 0xE1; case 'e': return 0xE9; case 'i': return 0xED;
				case 'o': return 0xF3; case 'u': return 0xFA; case 'y': return 0xFF;
				case 'A': return 0xC1; case 'E': return 0xC9; case 'I': return 0xCD;
				case 'O': return 0xD3; case 'U': return 0xDA;
			}
			break;
		case 0x0302: /* Unicode combining circumflex */
			switch(letter) {
				case 'a': return 0xE2; case 'e': return 0xEA; case 'i': return 0xEE;
				case 'o': return 0xF4; case 'u': return 0xFB;
				case 'A': return 0xC2; case 'E': return 0xCA; case 'I': return 0xCE;
				case 'O': return 0xD4; case 'U': return 0xDB;
			}
			break;
		case 0x0303: /* Unicode combining tilde */
			switch(letter) {
				case 'a': return 0xE3; case 'n': return 0xF1; case 'o': return 0xF5;
				case 'A': return 0xC3; case 'N': return 0xD1; case 'O': return 0xD5;
			}
			break;
		case 0x0308: /* Unicode combining diaeresis */
			switch(letter) {
				case 'a': return 0xE4; case 'e': return 0xEB; case 'u': return 0xFC;
				case 'o': return 0xF6; case 'i': return 0xEF;
				case 'A': return 0xC4; case 'E': return 0xCB; case 'U': return 0xDC;
				case 'O': return 0xD6; case 'I': return 0xCF;
			}
			break;
		case 0x0327: /* Unicode combining cedilla */
			switch(letter) {
				case 'c': return 0xE7; case 'C': return 0xC7;
			}
			break;
	}
	return '?';
}
