#include <stdio.h>

void print_error(const char *message) {
        fprintf(stderr, "\033[38;5;196mERROR:\033[0m %s", message);
}

void print_fatal(const char *message) {
        fprintf(stderr, "\033[38;5;196mFATAL:\033[0m %s", message);
}

void print_warning(const char *message) {
        fprintf(stdout, "\033[38;5;166mWARNING:\033[0m %s", message);
}

void print_verbose(const char *message) {
        fprintf(stdout, "\033[38;5;87m%s\033[0m", message);
}
