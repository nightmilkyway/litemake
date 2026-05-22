#ifndef LITEMAKE_CONTROL_H
#define LITEMAKE_CONTROL_H

#define LITEMAKE_VARSIZE 1024

typedef struct variable_t {
	char *name;
	char *value;
	unsigned mt;
} variable_t;

typedef struct target_t {
	char **name;
	char *dep_matrix;
	char **com_matrix;
	struct target_t **depend;
	unsigned n_count;
	unsigned c_count;
	unsigned d_count;
	unsigned *c_mt;
	unsigned i;
	unsigned c;
	unsigned off;
} target_t;

variable_t *litemake_variable_1(const char *str, variable_t *vars);
variable_t *litemake_variable(const char *str, variable_t *vars);
int litemake_target_control(lexstat_t *st, variable_t *vars, target_t *target, target_t *tars, unsigned tars_count);
int litemake_var_control(lexstat_t *st, variable_t *vars, variable_t *vptr);
int litemake_cmd_control(lexstat_t *st, variable_t *vars, target_t *target, char *cwd, unsigned target_number);
int litemake_control_expand(target_t *t, char *cwd);
void control_target_free(target_t *target);
void control_var_free(variable_t *var);

#endif
