#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "vendor/printfc.h"
#include <getopt.h>
#include "validate.h"

#define MAX_PATH 4096
#define MAX_LINE 1024
#define MAX_RULES 1024

// Extra Modes
bool dry_mode = false;
bool deduplicate_mode = false;
bool verbose = false;
bool debug_mode = false;

int tag_count = 0;
int ext_count = 0;
int operations = 0;

enum ForgMode {
        AUTO,
        EXT,
        TAG,
};
enum ForgMode forg_mode = AUTO; // Default is Auto

static struct option long_options[] = { { "dry", no_argument, 0, 'd' },
                                        { "rm", no_argument, 0, 'r' },
                                        { "verbose", no_argument, 0, 'V' },
                                        { "help", no_argument, 0, 'h' },
                                        { 0, 0, 0, 0 } };

typedef struct {
        char path[256]; // Destination. e.g docs/
        char tag[64]; // Tag name. e.g journal << journal-botany.pdf
} TagMap;

typedef struct {
        char path[256]; // Destination. e.g archives/
        char ext[16]; // Extension. e.g .zip
} ExtMap;

ExtMap ext_map[MAX_RULES];
TagMap tag_map[MAX_RULES];

const char *get_ext_path(const char *ext);
const char *get_tag_path(const char *tag);
int ensure_directory(const char *path);
int load_config(const char *filename);
void move_file(const char *src_file, const char *dst_dir);
void trim_newline(char *str);
void usage(const char *prog);
void walk_and_move(const char *src, const char *dest);

int main(int argc, char *argv[]) {
        int opt = 0;
        const char *home_env = getenv("HOME");
        const char *dst_dir = NULL;
        const char *src_dir = NULL;
        char config_file[MAX_PATH];

        if (!home_env) {
                printfc(FATAL, "Could not get HOME environment variable");
                return EXIT_FAILURE;
        }

        snprintf(config_file, sizeof(config_file), "%s/.local/share/forg.conf",
                 home_env);

        while ((opt = getopt_long(argc, argv, ":drVh", long_options, NULL)) !=
               -1) {
                switch (opt) {
                case 'd':
                        dry_mode = true;
                        break;
                case 'r':
                        deduplicate_mode = true;
                        break;
                case 'V':
                        verbose = true;
                        break;
                case 'h':
                        usage(argv[0]);
                        return EXIT_SUCCESS;
                case ':':
                        printf("option '%c' needs a value\n", opt);
                        break;
                case '?':
                        printf("unknown option: %c\n", optopt);
                        break;
                }
        }

        if (argc < 3) {
                printf("Usage: forg [options] <src> <dest> <mode>\n");
                printf("Use -h option for more details.\n");
                return EXIT_FAILURE;
        }

        if (optind < argc) {
                src_dir = argv[optind];
                optind++;
        }
        if (optind < argc) {
                dst_dir = argv[optind];
                optind++;
        }
        if (optind < argc) {
                char *tmp_mode = argv[optind];
                if (strcmp(tmp_mode, "tag") == 0) forg_mode = TAG;
                if (strcmp(tmp_mode, "ext") == 0) forg_mode = EXT;
                optind++;
        }

        if (verbose) {
                char *tmp_mode = NULL;
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
                printfc(FATAL, "source directory is not a directory!");
                return EXIT_FAILURE;
        }
        if (!isdir(dst_dir)) {
                printfc(FATAL, "destination directory is not a directory!");
                return EXIT_FAILURE;
        }

        if (strcmp(src_dir, dst_dir) == 0) {
                printfc(WARN, "source and destination are the same!\n");
                printf("%s", "This may cause issues or slow performance.\n");
                printf("%s", "Continue? (y/N): ");
                char choice = getchar();
                if (choice != 'y' && choice != 'Y') {
                        printf("Aborting.\n");
                        return EXIT_FAILURE;
                }
        }

        if (load_config(config_file) != 0) {
                return EXIT_FAILURE;
        }

        if (dry_mode) {
                printf("Dry run mode enabled.\n");
        };

        walk_and_move(src_dir, dst_dir);

        printf("%d operations finished.", operations);

        return EXIT_SUCCESS;
}

void usage(const char *prog) {
        printf("File Organizer\n");
        printf("Usage: %s [options] <src> <dest> <mode>\n", prog);
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

int load_config(const char *filename) {
        FILE *fp = fopen(filename, "r");
        if (!fp) {
                perror("Loading config");
                return 1;
        }

        char line[MAX_LINE];
        while (fgets(line, sizeof(line), fp)) {
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

        if (debug_mode)
                printfc(DEBUG, "Loaded %d tags and %d extensions\n", tag_count,
                        ext_count);

        fclose(fp);
        return 0;
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

int ensure_directory(const char *path) {
        char path_copy[MAX_PATH];
        size_t len = strlen(path);
        if (len >= sizeof(path_copy)) {
                return 1;
        }
        strcpy(path_copy, path);

        char *p = path_copy;
        while (*p) {
                if (*p == '/') {
                        *p = '\0';
                        struct stat st;
                        if (stat(path_copy, &st) != 0) {
                                if (mkdir(path_copy, 0700) != 0) return 2;
                        }
                        *p = '/';
                }
                p++;
        }
        struct stat st;
        if (stat(path, &st) != 0) {
                if (mkdir(path, 0700) != 0) return 2;
        }
        return 0;
}

void move_file(const char *src_file, const char *dst_dir) {
        const char *filename = strrchr(src_file, '/');
        if (filename)
                filename++;
        else
                filename = src_file;

        char dest_path[MAX_PATH];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dst_dir, filename);

        if (access(dest_path, F_OK) == 0) {
                if (dry_mode) {
                        if (deduplicate_mode) {
                                printf("Would delete duplicate: %s\n",
                                       dest_path);
                        } else {
                                printfc(INFO, "File exists: %s\n", dest_path);
                        }
                } else {
                        if (deduplicate_mode) {
                                if (remove(src_file) == 0) {
                                        if (verbose) {
                                                printfc(WARN,
                                                        "Deleted duplicate: %s\n",
                                                        dest_path);
                                        }
                                } else
                                        perror("Delete");
                        } else {
                                if (verbose && isfile(dest_path)) {
                                        printfc(INFO, "File exists: %s\n",
                                                dst_dir);
                                }
                        }
                        return;
                }
        }

        if (dry_mode) {
                printf("Would move: %s => %s\n", src_file, dst_dir);
        } else {
                if (rename(src_file, dest_path) == 0) {
                        if (verbose) {
                                printf("Moved file: %s => %s\n", src_file,
                                       dst_dir);
                                operations++;
                        }
                } else {
                        printfc(ERROR, "failed to move: %s to %s\n", src_file,
                                dst_dir);
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

                                int err = ensure_directory(final_dir);
                                switch (err) {
                                case 0:
                                        break;
                                case 1:
                                        printfc(ERROR,
                                                "failed to prepare directory. Directory length is too large.\n");
                                        printfc(DEBUG, "Directory: %s\n",
                                                final_dir);
                                        continue;
                                case 2:
                                        printfc(ERROR,
                                                "failed to make directory.");
                                        continue;
                                }
                                move_file(path, final_dir);
                        }
                }
        }
        closedir(dir);
}
