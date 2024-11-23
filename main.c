#include <assert.h>

#include <raylib.h>

#define YAGI_IMPLEMENTATION
#include "yagi.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "yagi");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        yagi_ui_begin();

        yagi_begin_layout(LAYOUT_VERT, (Vector2){10, 10}, 5);
            yagi_text("Hello, World!");
            yagi_button("Click me!");
            yagi_begin_sublayout(LAYOUT_HORZ, 5);
                yagi_text("Hello, World!");
                yagi_button("Click me!");
            yagi_end_layout();
            yagi_text("Hello, World!");
        yagi_end_layout();

        yagi_ui_end();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
