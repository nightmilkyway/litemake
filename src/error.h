#ifndef LITEMAKE_ERROR_H
#define LITEMAKE_ERROR_H

#define LITEMAKE_ERROR_COUNT 256

typedef enum litemake_err {
	LITEMAKE_ERR_SUCCESSFUL_EXIT = 0,
	LITEMAKE_ERR_FAILED_EXIT,
	LITEMAKE_ERR_ALLOCATION_FAILED,
	LITEMAKE_ERR_FILE_NOT_READ,
	LITEMAKE_ERR_NULL_POINTER,
	LITEMAKE_ERR_TEMP,
	LITEMAKE_ERR_QUEUE_UNDERFLOW,
	LITEMAKE_ERR_FAILED_TO_GET_DATA,
	LITEMAKE_ERR_FAILED_TO_OPEN_DIR,
	LITEMAKE_ERR_NOT_A_DIRECTORY,
	LITEMAKE_ERR_TARGET_NAME_IS_ALREADY_USED,
	LITEMAKE_ERR_COMMAND_WITHOUT_RULE,
	LITEMAKE_ERR_TOO_MANY_TARGETS,
	LITEMAKE_ERR_TOO_MANY_VARIABLES,
	LITEMAKE_ERR_INCORRECT_SPECIAL_TARGET,
	LITEMAKE_ERR_INCORRECT_TARGET,
	LITEMAKE_ERR_TARGET_IS_A_DIRECTORY
} litemake_err;

typedef struct err_pair_t {
	char *msg;
	unsigned c;
} err_pair_t;

extern const err_pair_t ctx_err_m[LITEMAKE_ERROR_COUNT];

typedef struct ctx_t {
	char *str;
	char *err_msg;
	char *err_path;
	unsigned err_cd;
	unsigned str_i;
} ctx_t;

void litemake_error(int e, const char *msg);
char *litemake_ctxmsg(unsigned c);
void litemake_ctxerror(ctx_t *ctx);

#endif
