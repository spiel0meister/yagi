#define CBUILD_IMPLEMENTATION
#include "cbuild.h"

#define cc(cmd) cmd_push_str(cmd, "gcc");

void cflags(Cmd* cmd, bool debug) {
    cmd_push_str(cmd, "-Wall", "-Wextra");
    if (debug) {
        cmd_push_str(cmd, "-ggdb", "-DYAGI_DEBUG");
    } else {
        cmd_push_str(cmd, "-O2");
    }
}

void libs(Cmd* cmd) {
    cmd_push_str(cmd, "-lraylib", "-lm");
}

const char* examples[][2] = {
    { "./fsview.c", "./build/fsview" },
    { "./main.c", "./build/main" },
};
#define EXAMPLES_COUNT (sizeof(examples)/sizeof(examples[0]))

bool build_examples(Cmd* cmd) {
    for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
        if (need_rebuild1(examples[i][1], examples[i][0])) {
            cc(cmd);
            cflags(cmd, true);
            cmd_push_str(cmd, "-o", examples[i][1], examples[i][0]);
            libs(cmd);
            if (!cmd_run_sync_and_reset(cmd)) return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    Cmd cmd = {0};
    build_yourself(&cmd, argc, argv);

    if (!create_dir_if_not_exists("./build")) return 1;

    if (!build_examples(&cmd)) return 1;

    if (need_rebuild1("./build/yagi.o", "yagi.h")) {
        cc(&cmd);
        cflags(&cmd, true);
        cmd_push_str(&cmd, "-c", "-o", "./build/yagi.o", "yagi.c");
        if (!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    return 0;
}
