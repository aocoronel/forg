#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "validade.h"

/*
* Check if path is a file
*/
int isfile(const char *path) {
        struct stat statbuf;
        if (stat(path, &statbuf) != 0) return 0;
        return S_ISREG(statbuf.st_mode);
}

/*
* Check if path is a dir
*/
int isdir(const char *path) {
        struct stat statbuf;
        if (stat(path, &statbuf) != 0) return 0;
        return S_ISDIR(statbuf.st_mode);
}
