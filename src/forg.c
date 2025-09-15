#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "colors.h"
#include <getopt.h>
#include "validade.h"

static struct option long_options[] = { { "dry", no_argument, 0, 'd' },
                                        { "rm", no_argument, 0, 'r' },
                                        { "verbose", no_argument, 0, 'V' },
                                        { "help", no_argument, 0, 'h' },
                                        { 0, 0, 0, 0 } };

#define MAX_PATH 4096
#define MAX_LINE 1024
#define MAX_RULES 1024

enum ForgMode {
        AUTO,
        EXT,
        TAG,
};
enum ForgMode forg_mode = AUTO; // Default is Auto

typedef struct {
        char tag[64]; // Tag name. e.g journal
        char path[256];
} TagMap;

typedef struct {
        char ext[16]; // Extension. e.g .zip
        char path[256];
} ExtMap;

TagMap tag_map[MAX_RULES];
ExtMap ext_map[MAX_RULES];
int tag_count = 0;
int ext_count = 0;
int operations = 0;

// Modes
bool DRY_MODE = false;
bool DEDUPLICATE_MODE = false;

bool VERBOSE = false;

void usage(const char *prog) {
        printf("File Organizer\n");
        printf("Usage: %s [options] <src> <dest>\n", prog);
        printf("Options:\n");
        printf("  -d, --dry       Preview actions\n");
        printf("  -r, --rm        Remove duplicate files\n");
        printf("  -h, --help      Show this message\n");
        printf("  -V, --verbose   Enable verbosity\n");
}

void trim_newline(char *str) {
        size_t len = strlen(str);
        if (len && str[len - 1] == '\n') str[len - 1] = '\0';
}

void load_config(const char *filename) {
        FILE *f = fopen(filename, "r");
        if (!f) {
                perror("Error opening config file");
                exit(1);
        }

        char line[MAX_LINE];
        while (fgets(line, sizeof(line), f)) {
                trim_newline(line);
                if (line[0] == '#' || strlen(line) < 3) continue;

                if (strncmp(line, "ext:", 4) == 0) {
                        char *key = strtok(line + 4, "=");
                        char *val = strtok(NULL, "=");
                        if (key && val) {
                                strncpy(ext_map[ext_count].ext, key,
                                        sizeof(ext_map[0].ext));
                                strncpy(ext_map[ext_count].path, val,
                                        sizeof(ext_map[0].path));
                                ext_count++;
                        }
                } else if (strncmp(line, "tag:", 4) == 0) {
                        char *key = strtok(line + 4, "=");
                        char *val = strtok(NULL, "=");
                        if (key && val) {
                                strncpy(tag_map[tag_count].tag, key,
                                        sizeof(tag_map[0].tag));
                                strncpy(tag_map[tag_count].path, val,
                                        sizeof(tag_map[0].path));
                                tag_count++;
                        }
                }
        }

#ifdef DEBUGGING
        printf("Tag count: %d\nExt count: %d\n", tag_count, ext_count);
#endif /* ifdef DEBUGGING */

        fclose(f);
}

const char *get_ext_path(const char *ext) {
        for (int i = 0; i < ext_count; i++) {
                if (strcmp(ext, ext_map[i].ext) == 0) return ext_map[i].path;
        }
        return NULL;
}

const char *get_tag_path(const char *tag) {
        for (int i = 0; i < tag_count; i++) {
                if (strcmp(tag, tag_map[i].tag) == 0) return tag_map[i].path;
        }
        return NULL;
}

void ensure_directory(const char *path) {
        char path_copy[MAX_PATH];
        size_t len = strlen(path);
        if (len >= sizeof(path_copy)) {
                print_error("failed to prepare directory");
                return;
        }
        strcpy(path_copy, path);

        char *p = path_copy;
        while (*p) {
                if (*p == '/') {
                        *p = '\0';
                        struct stat st;
                        if (stat(path_copy, &st) != 0) {
                                mkdir(path_copy, 0700);
                        }
                        *p = '/';
                }
                p++;
        }
        struct stat st;
        if (stat(path, &st) != 0) {
                mkdir(path, 0700);
        }
}

void move_file(const char *src_file, const char *dest_dir) {
        const char *filename = strrchr(src_file, '/');
        if (filename)
                filename++;
        else
                filename = src_file;

        char dest_path[MAX_PATH];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, filename);

        if (access(dest_path, F_OK) == 0) {
                if (DRY_MODE) {
                        if (DEDUPLICATE_MODE) {
                                printf("Would delete duplicate: %s", dest_dir);
                        } else {
                                print_verbose("File exists");
                        }
                } else {
                        if (DEDUPLICATE_MODE) {
                                if (remove(src_file) == 0) {
                                        print_verbose("Deleted duplicate");
                                } else
                                        perror("Delete failed");
                        } else {
                                if (VERBOSE) print_verbose("File exists");
                        }
                        return;
                }
        }

        if (DRY_MODE) {
                printf("Would move: %s => %s\n", src_file, dest_dir);
        } else {
                if (rename(src_file, dest_path) == 0) {
                        if (VERBOSE) {
                                printf("Moved file: %s => %s\n", src_file,
                                       dest_dir);
                                operations++;
                        }
                } else {
                        print_error("failed to move: ");
                        printf("%s to %s\n", src_file, dest_dir);
                }
        }
}

void walk_and_move(const char *src, const char *dest) {
        DIR *dir = opendir(src);
        if (!dir) {
                perror("Read directory");
                return;
        }
        struct dirent *entry;
        char path[MAX_PATH];
        while ((entry = readdir(dir))) {
                if (strcmp(entry->d_name, ".") == 0 ||
                    strcmp(entry->d_name, "..") == 0)
                        continue;

                snprintf(path, sizeof(path), "%s/%s", src, entry->d_name);

                struct stat st;
                if (stat(path, &st) != 0) continue;

                if (S_ISDIR(st.st_mode)) {
                        walk_and_move(path, dest);
                } else {
                        const char *filename = entry->d_name;
                        char *dot = strrchr(filename, '.');
                        char *dash = strchr(filename, '-');
                        const char *target_subdir = NULL;

                        if (forg_mode == AUTO) {
                                if (dash) {
                                        char tag[64] = { 0 };
                                        strncpy(tag, filename, dash - filename);
                                        target_subdir = get_tag_path(tag);
                                }
                                if (!target_subdir && dot) {
                                        target_subdir = get_ext_path(dot + 1);
                                }
                        } else if (forg_mode == TAG) {
                                if (dash) {
                                        char tag[64] = { 0 };
                                        strncpy(tag, filename, dash - filename);
                                        target_subdir = get_tag_path(tag);
                                }
                        } else if (forg_mode == EXT) {
                                if (dot) target_subdir = get_ext_path(dot + 1);
                        }

                        if (target_subdir) {
                                char final_dir[MAX_PATH];
                                snprintf(final_dir, sizeof(final_dir), "%s/%s",
                                         dest, target_subdir);
                                ensure_directory(final_dir);
                                move_file(path, final_dir);
                        }
                }
        }
        closedir(dir);
}

int main(int argc, char *argv[]) {
        char config_file[MAX_PATH];
        const char *config_path = getenv("HOME");
        const char *dest_dir = NULL;
        const char *src_dir = NULL;
        int opt;

        if (!config_path) {
                print_fatal("Could not get HOME environment variable");
                exit(EXIT_FAILURE);
        }

        snprintf(config_file, sizeof(config_file), "%s/.local/share/forg.conf",
                 config_path);

        while ((opt = getopt_long(argc, argv, ":drVh", long_options, NULL)) !=
               -1) {
                switch (opt) {
                case 'd':
                        DRY_MODE = true;
                        break;
                case 'r':
                        DEDUPLICATE_MODE = true;
                        break;
                case 'V':
                        VERBOSE = true;
                        break;
                case 'h':
                        usage(argv[0]);
                        exit(EXIT_SUCCESS);
                case ':':
                        printf("option needs a value\n");
                        break;
                case '?':
                        printf("unknown option: %c\n", optopt);
                        break;
                }
        }

        if (argc < 3) {
                printf("Usage: forg [options] <src> <dest>\n");
                printf("Use -h option for more details.\n");
                exit(EXIT_FAILURE);
        }

        if (optind < argc) {
                src_dir = argv[optind];
                optind++;
        }
        if (optind < argc) {
                dest_dir = argv[optind];
                optind++;
        }
        if (optind < argc) {
                char *tmp_mode = argv[optind];
                if (strcmp(tmp_mode, "tag") == 0) forg_mode = TAG;
                if (strcmp(tmp_mode, "ext") == 0) forg_mode = EXT;
                optind++;
        }

        if (VERBOSE) {
                char *tmp_mode;
                switch (forg_mode) {
                case AUTO:
                        tmp_mode = "auto";
                        break;
                case TAG:
                        tmp_mode = "tag";
                        break;
                case EXT:
                        tmp_mode = "ext";
                        break;
                }
                printf("Using %s mode\n", tmp_mode);
        }

        if (!isdir(src_dir)) {
                print_fatal("source directory is not a directory!");
                exit(EXIT_FAILURE);
        }
        if (!isdir(dest_dir)) {
                print_fatal("destination directory is not a directory!");
                exit(EXIT_FAILURE);
        }

        if (strcmp(src_dir, dest_dir) == 0) {
                print_warning("source and destination are the same!\n");
                printf("%s", "This may cause issues or slow performance.\n");
                printf("%s", "Continue? (y/n): ");
                char choice = getchar();
                if (choice != 'y' && choice != 'Y') {
                        printf("Aborting.\n");
                        return 1;
                }
        }

        load_config(config_file);

        if (DRY_MODE) {
                printf("Dry run mode enabled.\n");
        };

        walk_and_move(src_dir, dest_dir);

        printf("%d operations finished.", operations);

        exit(EXIT_SUCCESS);
}
