#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "validate.h"

int isfile(const char *path) {
        struct stat statbuf;
        if (stat(path, &statbuf) != 0) return 0;
        return S_ISREG(statbuf.st_mode);
}

int isdir(const char *path) {
        struct stat statbuf;
        if (stat(path, &statbuf) != 0) return 0;
        return S_ISDIR(statbuf.st_mode);
}
