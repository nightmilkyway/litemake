#include <stdio.h>
#include <stdlib.h>

#include "error.h"

const err_pair_t ctx_err_m[LITEMAKE_ERROR_COUNT] = {
	{"wrong instruction type", 0xC0},
	{"wrong declaration of a target or a variable", 0xC1},
	{"wrong declaration of a command", 0xC2},
	{"dependention must not be an operator", 0xC3},
	{"value of a variable must not be an operator", 0xC4},
	{"a target must have a dependation", 0xC5},
	{"a variable must have a value", 0xC6},
	{"operator cant be an argument of the command", 0xC7},
};

void litemake_error(int e, const char *msg) {
	fprintf(stderr, "litemake error %#.8x, %s\n", e, msg);
	exit(e);
}

char *litemake_ctxmsg(unsigned c) {
	for(unsigned i = 0; i < LITEMAKE_ERROR_COUNT; i++) {
		if(ctx_err_m[i].c == c) {
			return ctx_err_m[i].msg;
		}
	}
	return NULL;
}

void litemake_ctxerror(ctx_t *ctx) {
	fprintf(stderr, "\nlitemake build system occurred an error\n");
	if(ctx->err_path) {fprintf(stderr, "path of the work directory: %s\n", ctx->err_path);}
	fprintf(stderr, "at the string %i\n", ctx->str_i);
	if(ctx->str) {fprintf(stderr, "%s\n", ctx->str);}
	if(ctx->err_msg) {fprintf(stderr, "message: %s\n", ctx->err_msg);}
	fprintf(stderr, "error: 0x%08X\n", ctx->err_cd);
	exit(ctx->err_cd);
}
