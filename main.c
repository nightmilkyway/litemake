#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unistd.h"

#include "utils.h"
#include "error.h"
#include "strtype.h"
#include "lex.h"
#include "control.h"
#include "file.h"
#include "node.h"

#define LITEMAKE_LINESIZE 8192
#define LITEMAKE_MAXVARIABLES 4096

char *default_file = "litemake";

int main(int argc, char **argv) {
	char *fn, buf[LITEMAKE_LINESIZE];

	if(argc < 2) {fn = default_file;}
	else {fn = argv[1];}

	FILE *ff = fopen(fn, "r");

	if(!ff) {litemake_error(LITEMAKE_ERR_FILE_NOT_READ, "failed to read configuration file");}

	variable_t *variables = calloc(LITEMAKE_MAXVARIABLES*sizeof(variable_t), 1);
	target_t *targets = calloc(LITEMAKE_MAXVARIABLES*sizeof(target_t), 1);
	unsigned v_i = 0, v_t = 0, as = 0, l_count;

	lexstat_t *stat = calloc(LITEMAKE_LINESTAT,sizeof(lexstat_t)); char table[256];
	lexstat_t *tmp_stat;
	unsigned mt = 1;
	char cwd[LITEMAKE_MAXPATH] = {0};
	char cwd_work[LITEMAKE_MAXPATH] = {0};
	ctx_t ctx;

	if(!getcwd(cwd, sizeof(cwd))) {exit(1);}

	memcpy(cwd_work, cwd, LITEMAKE_MAXPATH);

	ctx.err_path = cwd_work;

	printf("litemake v1.3 by NZ (I4)\n\n");
	printf("loading of the configuration file...\n");

	litemake_bintable_build(" \n", "\t:#", "+=", "\"", table);

	for(l_count = 0; fgets(buf, LITEMAKE_LINESIZE, ff) != NULL; l_count++) {
		if(l_count >= LITEMAKE_LINESTAT*mt) {
			mt+=1;
			tmp_stat = realloc(stat, LITEMAKE_LINESTAT*mt*sizeof(lexstat_t));
			if(!tmp_stat) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
			stat = tmp_stat;
		}

		ctx.str = buf;
		ctx.str_i = l_count+1;

		litemake_lexstat_alloc(&stat[l_count]);

		litemake_token(buf, table, &stat[l_count]);

		litemake_lexer(&stat[l_count], &ctx);

		memset(buf, 0, LITEMAKE_LINESIZE);
	}

	fseek(ff, 0, SEEK_SET);

	for(unsigned i = 0; i < l_count; i++) {

		switch(stat[i].token_int[0]) {
			case LITEMAKE_PARSER_SHARP:
				break;
			case LITEMAKE_PARSER_TARGET:
				if(v_t >= LITEMAKE_MAXVARIABLES) {litemake_error(LITEMAKE_ERR_TOO_MANY_TARGETS, "this file have too many rules");}
				litemake_target_control(&stat[i], variables, &targets[v_t], targets, v_t);
				litemake_control_expand(&targets[v_t], cwd);
				v_t += 1;
				break;
			case LITEMAKE_PARSER_VARIABLE:
				if(v_i >= LITEMAKE_MAXVARIABLES) {litemake_error(LITEMAKE_ERR_TOO_MANY_VARIABLES, "this file have too many variables");}
				as = litemake_var_control(&stat[i], variables, &variables[v_i]);
				v_i += as;
				break;
			case LITEMAKE_PARSER_TAB:
				if(!v_t) {litemake_error(LITEMAKE_ERR_COMMAND_WITHOUT_RULE, "the command was defined without any applicable rule");}
				for(unsigned j = 0; j < targets[v_t-1].n_count; j++) {
					litemake_cmd_control(&stat[i], variables, &targets[v_t-1], cwd, j);
				}
				break;
			default:
				break;
		}

		memset(buf, 0, LITEMAKE_LINESIZE);
	}

	printf("the configuration file was readed successfully!\n");
	printf("build started...\n");

	for(unsigned i = 0; i < l_count; i++) {
		litemake_lexstat_free(&stat[i]);
	}

	litemake_parser(targets, v_t);

	target_t **tp = calloc(v_t*sizeof(target_t *), 1);

	litemake_node_kahn(tp, targets, v_t);

	litemake_node_off(tp, v_t, cwd);

	int r = litemake_node_execute(tp, cwd, v_t);

	printf("build finished\n");

	for(unsigned i = 0; i < v_t; i++) {
		control_target_free(&targets[i]);
	}

	for(unsigned i = 0; i < v_i; i++) {
		control_var_free(&variables[i]);
	}

	free(targets);
	free(variables);
	free(tp);

	fclose(ff);

	return 0;
}
