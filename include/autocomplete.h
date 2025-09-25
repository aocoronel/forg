#include "bashgen.h"
#include "zshgen.h"
#include "stddef.h"

struct ProgramArguments args[] = {
        { "SHELL", "echo -e 'bash\\nzsh'" },
};

struct ProgramCommands commands[] = {
        {"autocomplete", "SHELL", "Generate autocompletion for bash or zsh"},
};

struct ProgramFlag flags[] = {
        { "-d", "--dry", NULL, "Preview actions" },
        { "-r", "--remove", NULL, "Remove duplicate files" },
        { "-h", "--help", NULL, "Displays this message and exits" },
        { "-V", "--verbose", NULL, "Enable verbosity" },
};

ProgramInfo program_info = {
        .flagc = sizeof(flags) / sizeof(flags[0]),
        .cmdc = sizeof(commands) / sizeof(commands[0]),
        .name = "forg",
        .desc = "A simple file organizer",
        .usage = "[OPTIONS] [SRC] [DEST] <MODE>",
        .commands = commands,
        .flags = flags,
};

// struct ProgramEnv envs[] = {
// };

CompletionInfo completion_info = {
        .info = &program_info,
        .argc = sizeof(args) / sizeof(args[0]),
        // .envc = sizeof(envs) / sizeof(envs[0]),
        .args = args,
        // .envs = envs,
};
