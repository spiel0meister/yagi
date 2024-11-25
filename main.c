#include <raylib.h>

#define YAGI_IMPLEMENTATION
#include "yagi.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

typedef struct {
    char* name;
    char* surname;
    size_t age;
}Person;

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "yagi");

#ifdef YAGI_DEBUG 
    SetTraceLogLevel(LOG_DEBUG);
#endif // YAGI_DEBUG 

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        yagi_ui_begin();
            yagi_begin_layout(LAYOUT_VERT, ((Vector2){10, 10}), 10);
            yagi_end_layout();
        yagi_ui_end();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
