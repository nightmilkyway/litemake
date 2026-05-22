#ifndef LITEMAKE_NODE_H
#define LITEMAKE_NODE_H

#define LITEMAKE_NODESTACK 1024

int litemake_parser(target_t *sr, unsigned sr_count);
int litemake_node_indegree(target_t *sr, unsigned sr_count);
int litemake_node_push(target_t ***stack, target_t *ptr, unsigned *m, unsigned *ix);
int litemake_node_pop(target_t **stack, target_t **ptr, unsigned *m, unsigned *ix);
int litemake_node_kahn(target_t **result, target_t *sr, unsigned sr_count);
int litemake_node_off(target_t **ptr_ar, unsigned sr_count, char cwd[LITEMAKE_MAXPATH]);
int litemake_node_execute(target_t **ptr_ar, char *cwd, unsigned sr_count);

#endif
