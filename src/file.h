#ifndef LITEMAKE_FILE_H
#define LITEMAKE_FILE_H

#define LITEMAKE_MAXPATH 1024

int litemake_substr(const char *str1, const char *str2);
unsigned long long litemake_timestamp(const char *fn);
int litemake_dircheck(const char *path);
int litemake_rel(char *fpath, const char *path, const char *target);
int litemake_count(const char *path);
int litemake_dirwrite(char **buf, const char *path);
int litemake_normalizer(char *path);
int litemake_isexist(char str[LITEMAKE_MAXPATH]);

#endif
