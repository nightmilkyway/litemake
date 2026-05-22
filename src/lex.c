#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "strtype.h"
#include "lex.h"

const token_map opbrd[LITEMAKE_OPCOUNT] = {
	{"\t\0", LITEMAKE_PARSER_TAB},
    {":\0", LITEMAKE_PARSER_COLON},
    {"=\0", LITEMAKE_PARSER_EQUAL},
    {"#\0", LITEMAKE_PARSER_SHARP},
};

void litemake_lexstat_alloc(lexstat_t *st) {
    if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    st->token_count=0;
    st->ipr=0;
    st->mlp=1;
    st->token_matrix = calloc(LITEMAKE_TOKENCOUNT, LITEMAKE_TOKENSIZE);
    st->token_int = calloc(LITEMAKE_TOKENCOUNT, sizeof(parser_token));
    if(!st->token_matrix) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
    if(!st->token_int) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
}

void litemake_lexstat_realloc(lexstat_t *st) {
    if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}

    char *tmp_matrix; parser_token *tmp_int;

    st->mlp += 1;

    tmp_matrix = realloc(st->token_matrix, LITEMAKE_TOKENCOUNT*LITEMAKE_TOKENSIZE*st->mlp);
    tmp_int = realloc(st->token_int, LITEMAKE_TOKENCOUNT*sizeof(parser_token)*st->mlp);

    if(!tmp_matrix) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
    if(!tmp_int) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}

	st->token_matrix = tmp_matrix;
	st->token_int = tmp_int;
}

void litemake_lexstat_free(lexstat_t *st) {
    if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    free(st->token_matrix);
    free(st->token_int);
}

int litemake_bintable_build(const char *ign, const char *del, const char *swap, const char *incl, char *table) {
    if(!ign) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!del) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!swap) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!incl) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!table) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}

    memset(table, 0, LITEMAKE_TABLESIZE);
    for(int x = 0; ign[x] != 0; x++) {
        table[(unsigned char)ign[x]] = 1;
    }
    for(int x = 0; del[x] != 0; x++) {
        table[(unsigned char)del[x]] = 2;
    }
    for(int x = 0; swap[x] != 0; x++) {
        table[(unsigned char)swap[x]] = 3;
    }
    for(int x = 0; incl[x] != 0; x++) {
        table[(unsigned char)incl[x]] = 4;
    }
    return 0;
}

int litemake_lexstat_clear(lexstat_t *st) {
	memset(st->token_matrix, 0, LITEMAKE_TOKENSIZE*st->token_count);
	memset(st->token_int, 0, sizeof(parser_token)*st->token_count);
    st->token_count = 0;
    return 0;
}

int litemake_token(const char *inpt, const char *table, lexstat_t *st) {
    if(!inpt) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!table) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}

	if(st->token_count) {litemake_lexstat_clear(st);}

	/*
		* IS = string pointer
		* ITX = matrix pointer by X
		* ITY = matrix pointer by Y
		* TG = swap mode
		* IC = include all mode
		* RSV = check if memory for token reserved
		* MLP = blocks count
	*/
    int is, itx=0, ity=0, tg=0, ic=0, rsv=1;

    /*
		* CASE 0: character is letter
		* CASE 1: character is ignorable
		* CASE 2: character is delimiter
		* CASE 3: character is swap character
		* CASE 4: character is including all character
    */

    for(is = 0; inpt[is] != 0; is++) {
        switch(table[(unsigned char)inpt[is]]) {
            case 0:
                if(ity >= LITEMAKE_TOKENSIZE) {ity=0; itx+=1; st->token_matrix[itx*LITEMAKE_TOKENSIZE-1] = 0; rsv=1;} if(itx >= LITEMAKE_TOKENCOUNT*st->mlp) {litemake_lexstat_realloc(st);}
                if(ic) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; break;}
                if(tg) {
                    tg=0;
                    if(!rsv) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = 0; itx+=1; ity=0; rsv=1;}
                }

                st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; rsv=0;

                break;
            case 1:
                if(ity >= LITEMAKE_TOKENSIZE) {ity=0; itx+=1; st->token_matrix[itx*LITEMAKE_TOKENSIZE-1] = 0; rsv=1;} if(itx >= LITEMAKE_TOKENCOUNT*st->mlp) {litemake_lexstat_realloc(st);}
                if(ic) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; break;}
                if(!rsv) {itx+=1; ity=0; rsv=1;}
                break;
            case 2:
                if(ity >= LITEMAKE_TOKENSIZE) {ity=0; itx+=1; st->token_matrix[itx*LITEMAKE_TOKENSIZE-1] = 0; rsv=1;} if((itx+1) >= LITEMAKE_TOKENCOUNT*st->mlp) {litemake_lexstat_realloc(st);}
                if(ic) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; break;}
                if(!rsv) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = 0; itx+=1; ity=0;}
                st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++;
                st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = 0; itx+=1; ity=0; rsv=1;
                break;
            case 3:
                if(ity >= LITEMAKE_TOKENSIZE) {ity=0; itx+=1; st->token_matrix[itx*LITEMAKE_TOKENSIZE-1] = 0; rsv=1;} if(itx >= LITEMAKE_TOKENCOUNT*st->mlp) {litemake_lexstat_realloc(st);}
                if(ic) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; break;}
                if(!tg) {
                    tg=1;
                    if(!rsv) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = 0; itx+=1; ity=0; rsv=1;}
                }

                st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; rsv=0;

                break;
            case 4:
            	if(is > 0 && inpt[is-1] == '\\') {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++; break;}
                if(ity >= LITEMAKE_TOKENSIZE) {ity=0; itx+=1; st->token_matrix[itx*LITEMAKE_TOKENSIZE-1] = 0; rsv=1;} if(itx >= LITEMAKE_TOKENCOUNT*st->mlp) {litemake_lexstat_realloc(st);}
                if(!rsv) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = 0; itx+=1; ity=0;}
                st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = inpt[is]; ity++;
                if(ic) {st->token_matrix[(itx * LITEMAKE_TOKENSIZE) + ity] = 0; itx+=1; ity=0; rsv=1;}
                ic = !ic;
        }
    }

    if(ity) {ity = 0; itx += 1; st->token_matrix[itx*LITEMAKE_TOKENSIZE-1] = 0;}

    st->token_count = itx;

    return 0;
}

int litemake_lexer(lexstat_t *st, ctx_t *ctx) {
    if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
    if(!st->token_count) {return 0;}

    int s = 0;

    for(unsigned x = 0; x < st->token_count; x++) {
        switch(s) {
            case 0:
                if(is_ident(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
                    s = 1;
                    st->token_int[x] = LITEMAKE_PARSER_IDENT;
                    break;
                } else if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "#\0")) {
                    s = 2;
                    st->token_int[x] = LITEMAKE_PARSER_SHARP;
                    break;
                } else if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "\t\0")) {
                    s = 3;
                    st->token_int[x] = LITEMAKE_PARSER_TAB;
                    break;
                } else {
					ctx->err_cd = 0xC0; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
            case 1:
                if(is_ident(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
                    st->token_int[x] = LITEMAKE_PARSER_IDENT;
                    break;
                } else if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], ":\0")) {
                    s = 5;
                    st->token_int[x] = LITEMAKE_PARSER_COLON;
                    for(unsigned i = 0; i < x; i++) {st->token_int[i] = LITEMAKE_PARSER_TARGET;}
                    break;
                } else if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "=\0") && x == 1) {
                    s = 6;
                    st->token_int[x] = LITEMAKE_PARSER_EQUAL;
                    st->token_int[x-1] = LITEMAKE_PARSER_VARIABLE;
                    break;
                } else {
                    ctx->err_cd = 0xC1; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
            case 2:
                st->token_int[x] = LITEMAKE_PARSER_IGNORE;
                break;
            case 3:
                s = 7;
                if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "#\0")) {
                    s = 2;
                    st->token_int[x] = LITEMAKE_PARSER_SHARP;
                    break;
                } else if(!is_operand(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
					st->token_int[x] = LITEMAKE_PARSER_COM;
                } else {
					ctx->err_cd = 0xC2; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
            case 5:
            	s = 8;
				if(!is_operand(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
                    st->token_int[x] = LITEMAKE_PARSER_DEPEND;
                    break;
                } else {
					ctx->err_cd = 0xC5; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
            case 6:
            	s = 9;
                if(!is_operand(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
                    s = 2;
                    st->token_int[x] = LITEMAKE_PARSER_VALUE;
                    break;
                } else {
					ctx->err_cd = 0xC6; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
            case 7:
                if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "#\0")) {
                    s = 2;
                    st->token_int[x] = LITEMAKE_PARSER_SHARP;
                    break;
                } else if(!is_operand(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
					st->token_int[x] = LITEMAKE_PARSER_ARG;
                } else {
					ctx->err_cd = 0xC7; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
			case 8:
                if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "#\0")) {
                    s = 2;
                    st->token_int[x] = LITEMAKE_PARSER_SHARP;
                    break;
                } else if(!is_operand(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
					st->token_int[x] = LITEMAKE_PARSER_DEPEND;
                } else {
					ctx->err_cd = 0xC3; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
			case 9:
				if(!strcmp(&st->token_matrix[x*LITEMAKE_TOKENSIZE], "#\0")) {
                    s = 2;
                    st->token_int[x] = LITEMAKE_PARSER_SHARP;
                    break;
                } else if(!is_operand(&st->token_matrix[x*LITEMAKE_TOKENSIZE])) {
					st->token_int[x] = LITEMAKE_PARSER_VALUE;
                } else {
					ctx->err_cd = 0xC4; ctx->err_msg = litemake_ctxmsg(ctx->err_cd); litemake_ctxerror(ctx);
                }
                break;
        }
    }
    return 0;
}
