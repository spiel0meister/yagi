#include <raylib.h>

#define YAGI_IMPLEMENTATION
#include "yagi.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "yagi");

#ifdef DEBUG 
    SetTraceLogLevel(LOG_DEBUG);
#endif // DEBUG 

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        yagi_ui_begin();

        yagi_ui_end();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
