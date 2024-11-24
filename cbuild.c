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

int main(int argc, char* argv[]) {
    Cmd cmd = {0};
    build_yourself(&cmd, argc, argv);

    if (need_rebuild1("main", "main.c")) {
        cc(&cmd);
        cflags(&cmd, true);
        cmd_push_str(&cmd, "-o", "main", "main.c");
        libs(&cmd);
        if (!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    return 0;
}
