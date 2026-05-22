#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "error.h"
#include "utils.h"
#include "lex.h"

int is_ident(const char *s) {
    if(!s) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    unsigned l = strlen(s);
    for(unsigned i = 0; s[i] != 0; i++) {
		if(s[i] == '#' || s[i] == '\t' || s[i] == ':' || s[i] == '+' || s[i] == '=' || s[i] == '\"') {
			return 0;
		}
	}
    return 1;
}

int is_operand(const char *s) {
    if(!s) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    for(int x = 0; x < ARRAYSIZE(opbrd); x++) {
        if(!strcmp(s, opbrd[x].token)) {
            return 1;
        }
    }
    return 0;
}

int is_string(const char *s) {
    if(!s) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    unsigned l = strlen(s);
	if(s[0] == '\"' && s[l-1] == '\"') {return 1;}
	return 0;
}

int is_digital(const char *s) {
    if(!s) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    unsigned l = strlen(s);
	for(unsigned i = 0; i < l; i++) {
		if(!isdigit(s[i])) {
			return 0;
		}
	}
	return 1;
}

int is_phony(const char *s) {
	if(!s) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!strcmp(s, ".PHONY") || !strcmp(s, ".phony")) {
		return 1;
	}
	return 0;
}

int is_special(const char *s) {
	return is_phony(s);
}
