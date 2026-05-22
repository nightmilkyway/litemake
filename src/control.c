#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "lex.h"
#include "utils.h"
#include "strtype.h"
#include "control.h"
#include "file.h"

int litemake_varname(const char *str) {
	if(is_digital(str)) {return 1;}
	if(!strcmp(str, "target") || !strcmp(str, "all")) {return 1;}
	return 0;
}

int litemake_special_check(char **str, unsigned count) {
	for(unsigned i = 0; i < count; i++) {
		if(is_special(str[i])) {
			return 1;
		}
	}
	return 0;
}

int litemake_vardigital(const char *str) {
	if(str[0] == '$') {
		if(is_digital(&str[1])) {return 1;}
	}
	return 0;
}

int litemake_dependlist_len(target_t *t) {
	unsigned l = 0;
	for(unsigned i = 0; i < t->d_count; i++) {
		l += strlen(&t->dep_matrix[i*LITEMAKE_MAXPATH]);
	}
	l+=t->d_count-1;
	return l;
}

int litemake_dependlist(char *buf, target_t *t) {
	unsigned l = 0;
	for(unsigned i = 0; i < t->d_count; i++) {
		if(i != 0) {
			buf[l++] = ' ';
		}
		term_memcpy(&buf[l], &t->dep_matrix[i*LITEMAKE_MAXPATH], strlen(&t->dep_matrix[i*LITEMAKE_MAXPATH]));

		l += strlen(&t->dep_matrix[i*LITEMAKE_MAXPATH]);
	}
	return 0;
}

variable_t *litemake_variable_exist(const char *str, variable_t *vars) {
	for(int x = 0; vars[x].name != NULL; x++) {
		if(!strcmp(str, vars[x].name)) {
			return &vars[x];;
		}
	}
	return NULL;
}

variable_t *litemake_variable_1(const char *str, variable_t *vars) {
	for(int x = 0; vars[x].name != NULL; x++) {
		if(!strcmp(str, vars[x].name)) {
			return &vars[x];
		}
	}
	if(litemake_varname(str)) {return NULL;}
	litemake_error(LITEMAKE_ERR_TEMP, "temporary error system");
	return 0;
}

variable_t *litemake_variable(const char *str, variable_t *vars) {
	if(!str) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!vars) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(*str != '$') {return NULL;}

	int i;
	for(i = 0; str[i] == '$'; i++);

	variable_t *v = litemake_variable_1(&str[i], vars), *t;
	for(int x = 1; x < i; x++) {
		t = litemake_variable_1(v->value, vars); if(!t) {return NULL;}
		v = t;
	}
	return v;
}

int litemake_anticopy(target_t *tars, char *token_matrix, unsigned ct, unsigned tars_count) {
	for(unsigned i = 0; i < tars_count; i++) {
		for(unsigned j = 0; j < tars[i].n_count; j++) {
			for(unsigned k = 0; k < ct; k++) {
				if(!strcmp(&token_matrix[k*LITEMAKE_TOKENSIZE], tars[i].name[j])) {litemake_error(LITEMAKE_ERR_TARGET_NAME_IS_ALREADY_USED, "target name is already used");}
			}
		}
	}
	return 0;
}

int litemake_target_control(lexstat_t *st, variable_t *vars, target_t *target, target_t *tars, unsigned tars_count) {
	if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!vars) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!target) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!tars) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!st->token_count) {return 0;}
	if(st->token_count < 3) {return 1;}
	variable_t *v;
	char buf[LITEMAKE_TOKENSIZE];

	unsigned l;

	unsigned ct; for(ct = 0; strcmp(&st->token_matrix[ct*LITEMAKE_TOKENSIZE], ":\0"); ct++);
	unsigned tk; for(tk = ct+1; st->token_int[tk] == LITEMAKE_PARSER_DEPEND; tk++); tk+=1;

	litemake_anticopy(tars, st->token_matrix, ct, tars_count);

	target->name = calloc(ct*sizeof(char *),1);
	target->n_count = ct;

	if(target->n_count != 1 && litemake_special_check(target->name, target->n_count)) {
		litemake_error(LITEMAKE_ERR_INCORRECT_SPECIAL_TARGET, "incorrect declaration of special target");
	}

	for(unsigned x = 0; x < ct; x++) {
		term_memcpy(buf, &st->token_matrix[x*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
		if(st->token_int[x] == LITEMAKE_PARSER_SHARP) {litemake_error(LITEMAKE_ERR_INCORRECT_TARGET, "incorrect use of the commentary");}
		litemake_normalizer(buf);
		v = litemake_variable(buf, vars);
		if(!v) {
			l = strlen(buf)+1;
			target->name[x] = calloc(l,1);
			term_memcpy(target->name[x], buf, l);
		} else {
			l = strlen(v->value)+1;
			target->name[x] = calloc(l,1);
			term_memcpy(target->name[x], v->value, l);
		}
	}

	target->dep_matrix = calloc(LITEMAKE_TOKENSIZE*(tk-ct-1),1);
	target->depend = calloc(sizeof(target_t *)*(tk -ct-1),1);
	target->d_count = tk-ct-1;

	for(unsigned i = ct+1; i < tk; i++) {
		term_memcpy(buf, &st->token_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
		if(st->token_int[i] == LITEMAKE_PARSER_SHARP) {break;}
		litemake_normalizer(buf);
		v = litemake_variable(buf, vars);
		if(!v) {
			l = strlen(buf)+1;
			term_memcpy(&target->dep_matrix[(i-ct-1)*LITEMAKE_TOKENSIZE], buf, l);
		} else {
			l = strlen(v->value)+1;
			term_memcpy(&target->dep_matrix[(i-ct-1)*LITEMAKE_TOKENSIZE], v->value, l);
		}
	}
	return 0;
}

int litemake_var_control(lexstat_t *st, variable_t *vars, variable_t *vptr) {
	if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!vars) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!vptr) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!st->token_count) {return 0;}
	if(st->token_count < 3) {return 1;}

	int ret_val = 1;

	variable_t *v = litemake_variable(&st->token_matrix[0], vars);
	variable_t *v_e = litemake_variable_exist(&st->token_matrix[0], vars);

	if(v_e) {
		vptr = v_e;
		free(vptr->name);
		free(vptr->value);
		ret_val = 0;
	}

	unsigned l, idx = 0;

	if(!v) {
		l = strlen(&st->token_matrix[0])+1;
		vptr->name = calloc(l,1);
		term_memcpy(vptr->name, &st->token_matrix[0], l);
	} else {
		l = strlen(v->value)+1;
		vptr->name = calloc(l,1);
		term_memcpy(vptr->name, v->value, l);
	}

	vptr->value = calloc(LITEMAKE_VARSIZE,1);
	vptr->mt = 1;

	for(unsigned i = 2; i < st->token_count; i++) {
		if(i != 2) {
			if((idx + 1) <= vptr->mt*LITEMAKE_VARSIZE) {
				vptr->mt += 1;
				vptr->value = realloc(vptr->value, LITEMAKE_VARSIZE*vptr->mt);
			}
			vptr->value[idx++] = ' ';
		}
		v = litemake_variable(&st->token_matrix[i*LITEMAKE_TOKENSIZE], vars);
		if(!v) {
			l = strlen(&st->token_matrix[i*LITEMAKE_TOKENSIZE]); //we dont copy '\0' in the string of tokens
			if((idx + l) <= vptr->mt*LITEMAKE_VARSIZE) {
				vptr->mt += 1;
				vptr->value = realloc(vptr->value, LITEMAKE_VARSIZE*vptr->mt);
			}
			term_memcpy(&vptr->value[idx], &st->token_matrix[i*LITEMAKE_TOKENSIZE], l);
			idx+=l;
		} else {
			l = strlen(v->value); //we dont copy '\0' in the string of tokens
			if(idx + l <= vptr->mt*LITEMAKE_VARSIZE) {
				vptr->mt += 1;
				vptr->value = realloc(vptr->value, LITEMAKE_VARSIZE*vptr->mt);
			}
			term_memcpy(&vptr->value[idx], v->value, l);
			idx+=l;
		}
	}

	if((idx + 1) <= vptr->mt*LITEMAKE_VARSIZE) {
		vptr->mt += 1;
		vptr->value = realloc(vptr->value, LITEMAKE_VARSIZE*vptr->mt);
	}
	vptr->value[idx++] = '\0';

	return ret_val;
}

int litemake_cmd_control(lexstat_t *st, variable_t *vars, target_t *target, char *cwd, unsigned target_number) {
	if(!st) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!vars) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!target) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!cwd) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!st->token_count) {return 0;}
	if(st->token_count < 2) {return 1;}
	if(target_number >= target->n_count) {return 1;}

	unsigned l, m, idx = 0, rsv = 0, rsv_i = 0, c = 0, ix = 0;
	variable_t *v; char *tmp_buf, **tmp;
	char path[LITEMAKE_MAXPATH]; char buf[LITEMAKE_TOKENSIZE] = {0};

	target->c_count += 1;
	target->com_matrix = realloc(target->com_matrix,target->c_count*sizeof(char *));
	target->c_mt = realloc(target->c_mt,target->c_count*sizeof(unsigned));
	target->c_mt[target->c_count-1] = 1;
	target->com_matrix[target->c_count-1] = calloc(LITEMAKE_VARSIZE*target->c_mt[target->c_count-1], 1);

	for(unsigned i = 0; i < st->token_count; i++) {
		term_memcpy(buf, &st->token_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
		if(st->token_int[i] == LITEMAKE_PARSER_SHARP) {break;}
		litemake_normalizer(buf);
		litemake_rel(path, cwd, buf);
		switch(litemake_dircheck(path)) {
			case 0:
				c+=litemake_count(path);
				break;
			case 1:
				c+=1;
				break;
			default:
				c+=1;
				break;
		}
		memset(buf, 0, LITEMAKE_TOKENSIZE);
	}
	tmp = calloc(sizeof(char *)*c,1);
	for(unsigned i = 0; i < c; i++) {tmp[i] = calloc(LITEMAKE_MAXPATH,1);}
	for(unsigned i = 0; i < st->token_count; i++) {
		term_memcpy(buf, &st->token_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
		if(st->token_int[i] == LITEMAKE_PARSER_SHARP) {break;}
		litemake_normalizer(buf);
		litemake_rel(path, cwd, buf);
		switch(litemake_dircheck(path)) {
			case 0:
				litemake_dirwrite(&tmp[ix], path);
				ix+=litemake_count(path);
				break;
			case 1:
				term_memcpy(tmp[ix], path, LITEMAKE_MAXPATH);
				ix+=1;
				break;
			default:
				term_memcpy(tmp[ix], &st->token_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_MAXPATH);
				ix+=1;
				break;
		}
	}

	for(unsigned i = 1; i < c; i++) {
		if(i != 1) {
			if((idx + 1) <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
				target->c_mt[target->c_count-1] += 1;
				target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
				memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
			}
			target->com_matrix[target->c_count-1][idx++] = ' ';
		}
		v = litemake_variable(tmp[i], vars);
		if(!v) {
			if(!strcmp(tmp[i], "$all")) {
				l = litemake_dependlist_len(target); //we dont copy '\0' in the string of tokens
				if((idx + l) <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
					target->c_mt[target->c_count-1] += 1;
					target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
					memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
				}
				tmp_buf = calloc(l+1,1);
				litemake_dependlist(tmp_buf, target); tmp_buf[l] = 0;
				term_memcpy(&target->com_matrix[target->c_count-1][idx], tmp_buf, l);
				free(tmp_buf);
				idx+=l;
			} else if(!strcmp(tmp[i], "$target")) {
				l = strlen(target->name[target_number]); //we dont copy '\0' in the string of tokens
				if((idx + l) <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
					target->c_mt[target->c_count-1] += 1;
					target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
					memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
				}
				term_memcpy(&target->com_matrix[target->c_count-1][idx], target->name[target_number], l);
				idx+=l;
			} else if(litemake_vardigital(tmp[i])) {
				m = atoi(&tmp[i][1]); //the first symbol of this string is '$'
				if(m >= target->d_count) {litemake_error(LITEMAKE_ERR_TEMP, "temporary error system");}
				l = strlen(&target->dep_matrix[m*LITEMAKE_MAXPATH]); //we dont copy '\0' in the string of tokens
				if((idx + l) <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
					target->c_mt[target->c_count-1] += 1;
					target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
					memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
				}
				term_memcpy(&target->com_matrix[target->c_count-1][idx], &target->dep_matrix[m*LITEMAKE_MAXPATH], l);
				idx+=l;
			} else {
				l = strlen(tmp[i]); //we dont copy '\0' in the string of tokens
				if((idx + l) <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
					target->c_mt[target->c_count-1] += 1;
					target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
					memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
				}
				term_memcpy(&target->com_matrix[target->c_count-1][idx], tmp[i], l);
				idx+=l;
			}
		} else {
			l = strlen(v->value); //we dont copy '\0' in the string of tokens
			if(idx + l <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
				target->c_mt[target->c_count-1] += 1;
				target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
				memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
			}
			term_memcpy(&target->com_matrix[target->c_count-1][idx], v->value, l);
			idx+=l;
		}
	}

	if((idx + 1) <= target->c_mt[target->c_count-1]*LITEMAKE_VARSIZE) {
		target->c_mt[target->c_count-1] += 1;
		target->com_matrix[target->c_count-1] = realloc(target->com_matrix[target->c_count-1], LITEMAKE_VARSIZE*target->c_mt[target->c_count-1]);
		memset(&target->com_matrix[target->c_count-1][LITEMAKE_VARSIZE*(target->c_mt[target->c_count-1] - 1)], 0, LITEMAKE_VARSIZE);
	}
	target->com_matrix[target->c_count-1][idx++] = '\0';

	free(tmp);

	return 0;
}

int litemake_control_expand(target_t *t, char *cwd) {
	char path[LITEMAKE_MAXPATH]; char buf[LITEMAKE_TOKENSIZE] = {0};
	unsigned c = 0, ix = 0;
	for(unsigned i = 0; i < t->d_count; i++) {
		term_memcpy(buf, &t->dep_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
		litemake_normalizer(buf);
		litemake_rel(path, cwd, buf);
		switch(litemake_dircheck(path)) {
			case 0:
				c+=litemake_count(path);
				break;
			case 1:
				c+=1;
				break;
			default:
				c+=1;
				break;
		}
		memset(buf, 0, LITEMAKE_TOKENSIZE);
	}
	char **tmp = calloc(sizeof(char *)*c,1);
	for(unsigned i = 0; i < c; i++) {tmp[i] = calloc(LITEMAKE_MAXPATH,1);}
	for(unsigned i = 0; i < t->d_count; i++) {
		term_memcpy(buf, &t->dep_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
		litemake_normalizer(buf);
		litemake_rel(path, cwd, buf);
		switch(litemake_dircheck(path)) {
			case 0:
				litemake_dirwrite(&tmp[ix], path);
				ix+=litemake_count(path);
				break;
			case 1:
				term_memcpy(tmp[ix], path, LITEMAKE_MAXPATH);
				ix+=1;
				break;
			default:
				term_memcpy(tmp[ix], &t->dep_matrix[i*LITEMAKE_TOKENSIZE], LITEMAKE_TOKENSIZE);
				ix+=1;
				break;
		}
	}
	free(t->dep_matrix); free(t->depend);
	t->dep_matrix = calloc(LITEMAKE_MAXPATH*c,1); if(!t->dep_matrix) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
	t->depend = calloc(sizeof(target_t *)*c,1); if(!t->depend) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
	t->d_count = c;
	memset(t->dep_matrix, 0, LITEMAKE_MAXPATH*c);
	memset(t->depend, 0, sizeof(target_t *)*c);
	for(unsigned i = 0; i < c; i++) {
		term_memcpy(&t->dep_matrix[i*LITEMAKE_MAXPATH], tmp[i], LITEMAKE_MAXPATH);
	}
	for(unsigned i = 0; i < c; i++) {free(tmp[i]);}
	free(tmp);
	return 0;
}

void control_target_free(target_t *target) {
	for(unsigned i = 0; i < target->n_count; i++) {
		free(target->name[i]);
	}
	free(target->name);
	for(unsigned i = 0; i < target->c_count; i++) {
		free(target->com_matrix[i]);
	}
	free(target->com_matrix);
	free(target->dep_matrix);
	free(target->depend);
	free(target->c_mt);
}

void control_var_free(variable_t *var) {
	free(var->name);
	free(var->value);
}

