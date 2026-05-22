#ifndef LITEMAKE_STRTYPE_H
#define LITEMAKE_STRTYPE_H

int is_ident(const char *s);
int is_operand(const char *s);
int is_string(const char *s);
int is_digital(const char *s);
int is_phony(const char *s);
int is_special(const char *s);

#endif
