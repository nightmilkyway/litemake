#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "error.h"
#include "file.h"

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

int litemake_isexist(char *str) {
	//char buf[LITEMAKE_MAXPATH]; memcpy(buf, str, LITEMAKE_MAXPATH);
	//return access(buf, F_OK);
	return 0;
}

int litemake_strcmp0(const char *str1, const char *str2) {
	unsigned x;
	for(x = 0; str1[x] != 0 && str2[x] != 0; x++) {
		if(str1[x] > str2[x]) {return 1;}
		if(str1[x] < str2[x]) {return -1;}
	}
	if(str1[x] == 0 && str2[x] != 0) {return -1;}
	return 0;
}

// 1 if str2 subpath of str1, 0 for else
int litemake_substr_1(const char *str1, const char *str2) {
	unsigned j = 0;
	#ifdef _WIN32

	for(; str2[j] == '\\'; j++) {if(str2[j] == 0) {return 0;}}

	if(!litemake_strcmp0(str1, &str2[j])) {return 1;}

	return 0;

	#else

	for(; str2[j] == '/'; j++) {if(str2[j] == 0) {return 0;}}

	if(!litemake_strcmp0(str1, &str2[j])) {return 1;}

	return 0;

	#endif
}

// 1 if str2 subpath of str1, 0 for else
int litemake_substr(const char *str1, const char *str2) {
	unsigned i = 0, s = 0;
	#ifdef _WIN32

	for(;str1[i] != 0;) {
		s = 1;
		for(; str1[i] == '\\' && str1[i] != 0; i++);
		if(!litemake_substr_1(&str1[i], str2)) {s = 0;}
		for(; str1[i] != '\\' && str1[i] != 0; i++) {}
		if(s) {return 1;}
	}

	return 0;

	#else

	for(;str1[i] != 0;) {
		s = 1;
		for(; str1[i] == '/' && str1[i] != 0; i++);
		if(!litemake_substr_1(&str1[i], str2)) {s = 0;}
		for(; str1[i] != '/' && str1[i] != 0; i++) {}
		if(s) {return 1;}
	}

	return 0;

	#endif
}

unsigned long long litemake_timestamp(const char *fn) {
	struct stat st;
	if(!stat(fn, &st)) {
		return st.st_mtime;
	} else {
		return 0;
	}
	return 0;
}

int litemake_dircheck(const char *path) {
	struct stat st;

	if(!path[0]) {return 2;}

	if(stat(path, &st)) {litemake_error(LITEMAKE_ERR_FAILED_TO_GET_DATA, "failed to get file data");}

	if (S_ISDIR(st.st_mode)) {
		return 0;
	} else if (S_ISREG(st.st_mode)) {
		return 1;
	} else {
		return 2;
	}
}

int litemake_rel(char *fpath, const char *path, const char *target) {
    struct dirent *entry;
    struct stat st;
    DIR *dir = opendir(path);

    if(litemake_dircheck(path)) {litemake_error(LITEMAKE_ERR_NOT_A_DIRECTORY, "not a directory");}
    if (!dir) {litemake_error(LITEMAKE_ERR_FAILED_TO_OPEN_DIR, "failed to open directory");};

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[LITEMAKE_MAXPATH]; char *fmt;

        if(path[strlen(path)-1] == '\\') {fmt = "%s%s";}
        else {fmt = "%s\\%s";}

        snprintf(full_path, LITEMAKE_MAXPATH, fmt, path, entry->d_name);

        if(stat(full_path, &st)) {continue;}

        if(litemake_substr(full_path, target)) {
			term_memcpy(fpath, full_path, LITEMAKE_MAXPATH);

			closedir(dir); return 1;
        }

        if (S_ISDIR(st.st_mode)) {
            if(litemake_rel(fpath, full_path, target)) {
				closedir(dir); return 1;
            }
        }
    }

    memset(fpath, 0, LITEMAKE_MAXPATH);

    closedir(dir);

    return 0;
}

int litemake_count(const char *path) {
    struct dirent *entry;
    struct stat st;
    if(litemake_dircheck(path)) {litemake_error(LITEMAKE_ERR_NOT_A_DIRECTORY, "not a directory");}
    DIR *dir = opendir(path);
    unsigned c = 0;

    if (!dir) {litemake_error(LITEMAKE_ERR_FAILED_TO_OPEN_DIR, "failed to open directory");};

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[LITEMAKE_MAXPATH]; char *fmt;

        if(path[strlen(path)-1] == '\\') {fmt = "%s%s";}
        else {fmt = "%s\\%s";}

        snprintf(full_path, LITEMAKE_MAXPATH, fmt, path, entry->d_name);

        if(stat(full_path, &st)) {continue;}

        if (S_ISREG(st.st_mode)) {
            c++;
        }
    }

    closedir(dir);

    return c;
}

int litemake_dirwrite(char **buf, const char *path) {
    struct dirent *entry;
    struct stat st;
    if(litemake_dircheck(path)) {litemake_error(LITEMAKE_ERR_NOT_A_DIRECTORY, "not a directory");}
    DIR *dir = opendir(path);
    unsigned c = 0;

    if (!dir) {litemake_error(LITEMAKE_ERR_FAILED_TO_OPEN_DIR, "failed to open directory");};

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[LITEMAKE_MAXPATH]; char *fmt;

        if(path[strlen(path)-1] == '\\') {fmt = "%s%s";}
        else {fmt = "%s\\%s";}

        snprintf(full_path, LITEMAKE_MAXPATH, fmt, path, entry->d_name);

        if(stat(full_path, &st)) {continue;}

        if (S_ISREG(st.st_mode)) {
			term_memcpy(buf[c], full_path, LITEMAKE_MAXPATH);
            c++;
        }
    }

    closedir(dir);

    return c;
}

int litemake_normalizer(char *path) {
	unsigned l = strlen(path);

	#ifdef _WIN32

	for(unsigned i = 0; i < l; i++) {
		if(path[i] == '/') {
			path[i] = '\\';
		}
	}

	#else

	#endif

	return 0;
}
