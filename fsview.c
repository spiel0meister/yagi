#define YAGI_IMPLEMENTATION
#include "yagi.h"

#define CBUILD_IMPLEMENTATION
#include "cbuild.h"

bool read_dir_files(Files* files, const char* path) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", path);
        return false;
    }

    char path_buffer[4096] = {0};
    realpath(path, path_buffer);

    struct dirent* ent = NULL;
    for (ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
        int n = strlen(path_buffer);
        sprintf(path_buffer, "%s/%s", path_buffer, ent->d_name);
        if (ent->d_name[0] == '.') continue;
        if (ent->d_type == 4) {
            if (!read_dir_files(files, path_buffer)) return false;
        } else {
            File file = {0};
            strcpy(file.value, path_buffer);
            files_append(files, file);
        }
        path_buffer[n] = 0;
    }

    closedir(dir);
    return true;
}

int main(void) {
    InitWindow(800, 450, "fsview - yagi example");

    Files files = {0};

    if (!read_dir_files(&files, ".")) return 1;

    Vector2 ui_start = (Vector2){10, 10};
    while (!WindowShouldClose()) {
        Vector2 wheel_delta = GetMouseWheelMoveV();
        ui_start.y += wheel_delta.y * 50;

        BeginDrawing();
        ClearBackground(WHITE);

        yagi_ui_begin();
        yagi_begin_layout(LAYOUT_VERT, ui_start, 10);

        for (size_t i = 0; i < files.count; ++i) {
            yagi_begin_sublayout(LAYOUT_HORZ, 10);
            yagi_text("%s", files.items[i].value);
            yagi_end_layout();
        }

        yagi_end_layout();
        yagi_ui_end();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
