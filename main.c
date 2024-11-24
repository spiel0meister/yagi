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

    Person people[] = {
        { "Joe", "Mama", 69 },
        { "Jake", "Paul", 29 },
        { "Mike", "Tyson", 999 },
    };
#define PEOPLE_COUNT (sizeof(people) / sizeof(people[0]))

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        yagi_ui_begin();
            yagi_begin_layout(LAYOUT_VERT, ((Vector2){10, 10}), 10);
            for (size_t i = 0; i < PEOPLE_COUNT; i++) {
                yagi_begin_sublayout(LAYOUT_HORZ, 10);
                yagi_text("%s %s (%d)", people[i].name, people[i].surname, people[i].age);
                yagi_end_layout();
            }
            yagi_end_layout();
        yagi_ui_end();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
