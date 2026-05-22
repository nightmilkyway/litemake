#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "strtype.h"
#include "error.h"
#include "lex.h"
#include "control.h"
#include "file.h"
#include "node.h"

int litemake_node_strcmp(const char *str1, const char *str2) {
	unsigned u = strlen(str1);
	unsigned v = strlen(str2);
	if(u < v) {return litemake_node_strcmp(str2, str1);}
	for(unsigned s = u, p = v; ; s--, p--) {
		if(str1[s] > str2[p]) {return 1;}
		if(str1[s] < str2[p]) {return -1;}
		if(!p) {break;}
	}
	return 0;
}

int litemake_parser(target_t *sr, unsigned sr_count) {
	unsigned t_dp = 0;
	for(unsigned i = 0; i < sr_count; i++) {
		for(unsigned j = 0; j < sr[i].d_count; j++) {
			for(unsigned k = 0; k < sr_count; k++) {
				for(unsigned l = 0; l < sr[k].n_count; l++) {
					if(!strcmp(&sr[i].dep_matrix[LITEMAKE_MAXPATH*j], sr[k].name[l]) || !litemake_node_strcmp(&sr[i].dep_matrix[LITEMAKE_MAXPATH*j], sr[k].name[l])) {
						sr[i].depend[j] = &sr[k];
						//printf("%s %s\n", sr[i].name[0], sr[k].name[l]);
						t_dp += 1;
						break;
					}
				}
			}
		}
	}
	return 0;
}

int litemake_node_indegree(target_t *sr, unsigned sr_count) {
	for(unsigned x = 0; x < sr_count; x++) {sr[x].i = 0;}

	for(unsigned i = 0; i < sr_count; i++) {
		for(unsigned j = 0; j < sr[i].d_count; j++) {
			if(!sr[i].depend[j]) {continue;}
			sr[i].depend[j]->i += 1;
		}
	}
	return 0;
}

int litemake_node_push(target_t ***stack, target_t *ptr, unsigned *m, unsigned *ix) {
	target_t **tmp_stack;
	if((*ix) >= LITEMAKE_NODESTACK*(*m)) {
		*m += 1;
		tmp_stack = realloc(*stack, sizeof(target_t *)*LITEMAKE_NODESTACK*(*m));
		if(!tmp_stack) {litemake_error(LITEMAKE_ERR_ALLOCATION_FAILED, "memory allocation failed");}
		*stack = tmp_stack;
	}
	(*stack)[*ix] = ptr;
	*ix += 1;
	return 0;
}

int litemake_node_pop(target_t **stack, target_t **ptr, unsigned *m, unsigned *ix) {
	if((*ix) <= 0) {
		litemake_error(LITEMAKE_ERR_QUEUE_UNDERFLOW, "a stack underflow occurred");
	}
	*ix -= 1;
	*ptr = stack[*ix];
	return 0;
}

int litemake_node_kahn(target_t **result, target_t *sr, unsigned sr_count) {
	target_t **stack = calloc(sizeof(target_t *)*LITEMAKE_NODESTACK,1);
	target_t *v;
	unsigned ix = 0, m = 1, rx = 0;

	litemake_node_indegree(sr, sr_count);

	for(unsigned i = 0; i < sr_count; i++) {
		if(sr[i].i == 0) {litemake_node_push(&stack, &sr[i], &m, &ix);}
	}

	if(!ix) {litemake_error(LITEMAKE_ERR_TEMP, "temporary error system");}

	for(;ix != 0;) {
		litemake_node_pop(stack, &v, &m, &ix);
		result[rx++] = v;

		for(unsigned i = 0; i < v->d_count; i++) {
			if(!v->depend[i]) {continue;}
			v->depend[i]->i -= 1;
			if(!v->depend[i]->i) {litemake_node_push(&stack, v->depend[i], &m, &ix);}
		}
	}

	if(rx < sr_count) {litemake_error(LITEMAKE_ERR_TEMP, "temporary error system");}

	free(stack);
}

int litemake_node_off(target_t **ptr_ar, unsigned sr_count, char cwd[LITEMAKE_MAXPATH]) {
	unsigned x = 1; char path[LITEMAKE_MAXPATH]; char buf[LITEMAKE_MAXPATH];
	unsigned l;

	for(unsigned i = sr_count; i > 0; i--) {
		if(is_special(ptr_ar[i-1]->name[0])) {x = 0;}
		for(unsigned j = 0; j < ptr_ar[i-1]->n_count; j++) {
			if((strlen(ptr_ar[i-1]->name[j]) + 1) > LITEMAKE_MAXPATH) {
				l = LITEMAKE_MAXPATH;
			} else {
				l = (strlen(ptr_ar[i-1]->name[j]) + 1);
			}
			term_memcpy(buf, ptr_ar[i-1]->name[j], l);
			litemake_rel(path, cwd, buf);
			for(unsigned k = ptr_ar[i-1]->d_count; k > 0; k--) {
				if(litemake_timestamp(path) < litemake_timestamp(&ptr_ar[i-1]->dep_matrix[(k-1)*LITEMAKE_MAXPATH])) {
					x = 0;
				}
				//printf("%s, %s: %i\n", path, &ptr_ar[i-1]->dep_matrix[(k-1)*LITEMAKE_MAXPATH], litemake_timestamp(path)-litemake_timestamp(&ptr_ar[i-1]->dep_matrix[(k-1)*LITEMAKE_MAXPATH]));
				if(!ptr_ar[i-1]->depend[(k-1)]) {continue;}
				if(ptr_ar[i-1]->depend[(k-1)]->off == 0) {
					x = 0;
				}
			}
		}
		if(x == 0) {
			x = 1;
			ptr_ar[i-1]->off = 0;
		} else {
			x = 1;
			ptr_ar[i-1]->off = 1;
		}
	}
}

int litemake_target_exist(const char *n, char *cwd) {
	char *path = calloc(LITEMAKE_MAXPATH,1); char *buf = calloc(LITEMAKE_MAXPATH,1);
	unsigned l = (strlen(n)+1) > LITEMAKE_MAXPATH ? LITEMAKE_MAXPATH : (strlen(n)+1);
	memcpy(buf, n, l);
	litemake_normalizer(buf);
	litemake_rel(path, cwd, buf);
	if(litemake_isexist(path) == 0) {
		free(path);
		free(buf);
		return 1;
	} else {
		free(path);
		free(buf);
		return 0;
	}
}

target_t *litemake_phony_search(target_t **ptr_ar, unsigned sr_count) {
	for(unsigned i = sr_count; i > 0; i--) {
		if(is_phony(ptr_ar[i-1]->name[0])) {
			return ptr_ar[i-1];
		}
	}
	return NULL;
}

int litemake_node_execute(target_t **ptr_ar, char *cwd, unsigned sr_count) {
	target_t *phony = litemake_phony_search(ptr_ar, sr_count);
	for(unsigned i = sr_count; i > 0; i--) {
			//if(!ptr_ar[i-1]->off) {continue;}
		for(unsigned j = 0; j < ptr_ar[i-1]->c_count; j++) {
			printf("\t%s\n", ptr_ar[i-1]->com_matrix[j]);
			system(ptr_ar[i-1]->com_matrix[j]);
		}
		for(unsigned j = 0; j < ptr_ar[i-1]->n_count; j++) {
			if(phony) {
				for(unsigned k = 0; k < phony->d_count; k++) {
					if(!strcmp(ptr_ar[i-1]->name[j], &phony->dep_matrix[k*LITEMAKE_MAXPATH])) {
						goto fall;
					}
				}
			}
			if(!litemake_target_exist(ptr_ar[i-1]->name[j], cwd)) {return 1;}
			fall:
				continue;
		}
	}
	return 0;
}
