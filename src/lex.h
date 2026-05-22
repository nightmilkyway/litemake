#ifndef LITEMAKE_LEX_H
#define LITEMAKE_LEX_H

#define LITEMAKE_TOKENCOUNT 16
#define LITEMAKE_TOKENSIZE 256
#define LITEMAKE_TABLESIZE 256
#define LITEMAKE_OPCOUNT 4

typedef enum parser_token {
	LITEMAKE_PARSER_UNKNOWN,
	LITEMAKE_PARSER_TARGET,
	LITEMAKE_PARSER_DEPEND,
	LITEMAKE_PARSER_COM,
	LITEMAKE_PARSER_VARIABLE,
	LITEMAKE_PARSER_ARG,
	LITEMAKE_PARSER_TAB,
	LITEMAKE_PARSER_COLON,
	LITEMAKE_PARSER_EQUAL,
	LITEMAKE_PARSER_DOLLAR,
	LITEMAKE_PARSER_IDENT,
	LITEMAKE_PARSER_SHARP,
	LITEMAKE_PARSER_VALUE,
	LITEMAKE_PARSER_IGNORE,
	LITEMAKE_PARSER_PHONY2
} parser_token;

typedef struct lexstat_t {
        char *token_matrix;
        unsigned token_count;
        parser_token *token_int;
        unsigned ipr;
        unsigned mlp;
} lexstat_t;

typedef struct token_map {
	char *token;
	parser_token token_int;
} token_map;

extern const token_map opbrd[LITEMAKE_OPCOUNT];

void litemake_lexstat_alloc(lexstat_t *st);
void litemake_lexstat_realloc(lexstat_t *st);
void litemake_lexstat_free(lexstat_t *st);
int litemake_bintable_build(const char *ign, const char *del, const char *swap, const char *incl, char *table);
int litemake_token(const char *inpt, const char *table, lexstat_t *st);
int litemake_lexer(lexstat_t *st, ctx_t *ctx);

#endif
