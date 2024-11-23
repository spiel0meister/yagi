#include <raylib.h>

#define YAGI_IMPLEMENTATION
#include "yagi.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

void yagi_dropdown(int* already_selected, char* labels[], size_t label_count) {
    int selected = *already_selected;
    UIID id = yagi_id_next();

    yagi_begin_sublayout(LAYOUT_VERT, 0);
    Vector2 pos = yagi_next_widget_pos();
    Rectangle rect = { pos.x, pos.y, 0, 20 };
    for (size_t i = 0; i < label_count; i++) {
        int text_width = MeasureText(labels[i], 20);
        if (rect.width < text_width) rect.width = text_width;
    }

    Vector2 mouse = GetMousePosition();
    bool collides_main = CheckCollisionPointRec(mouse, rect);
    if (collides_main) {
        yagi_ui.highlight = id;
        if (yagi_ui.active == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            yagi_ui.active = id;
        }
    }

    if (yagi_ui.active == id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (collides_main) {
            yagi_ui.focus = id;
        }
        yagi_ui.active = 0;
    }

    char* label = labels[selected];
    int text_width = MeasureText(label, 20);
    Color bg = WHITE;
    if (yagi_ui.highlight == id) bg = ColorBrightness(bg, -0.5);

    DrawRectangleRec((Rectangle) { rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4 }, BLACK);
    DrawRectangleRec(rect, bg);

    DrawText(label, rect.x + rect.width / 2 - (float)text_width / 2, rect.y + rect.height / 2 - 10, 20, BLACK);

    yagi_expand_layout((Vector2) { rect.width, rect.height });

    if (yagi_ui.focus == id) {
        for (size_t i = 0; i < label_count; i++) {
            int text_width = MeasureText(labels[i], 20);
            Rectangle item_rect = { rect.x, rect.y + rect.height * (i + 1), rect.width, rect.height };
            UIID item_id = yagi_id_next();

            bool collides = CheckCollisionPointRec(mouse, item_rect);
            if (collides) {
                yagi_ui.highlight = item_id;
                if (yagi_ui.active == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    yagi_ui.active = item_id;
                }
            }

            if (yagi_ui.active == item_id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (collides) {
                    selected = i;
                }
                yagi_ui.active = 0;
            }

            Color bg = WHITE;
            if (yagi_ui.highlight == item_id) bg = ColorBrightness(bg, -0.5);

            DrawRectangleRec((Rectangle) { item_rect.x - 2, item_rect.y - 2, item_rect.width + 4, item_rect.height + 4 }, BLACK);
            DrawRectangleRec(item_rect, bg);

            DrawText(labels[i], item_rect.x + item_rect.width / 2 - (float)text_width / 2, item_rect.y + item_rect.height / 2 - 10, 20, BLACK);
            
            yagi_expand_layout((Vector2) { item_rect.width, item_rect.height });
        }
    }
    yagi_end_layout();

    if (!collides_main && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) yagi_ui.focus = 0;

    *already_selected = selected;
}

char* labels[] = {
    "One",
    "Two",
    "Three"
};
#define LABEL_COUNT (sizeof(labels)/sizeof(labels[0]))

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "yagi");

    int selected = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        yagi_ui_begin();

        yagi_begin_layout(LAYOUT_VERT, (Vector2){10, 10}, 5);
            yagi_text("Hello, World!");
            if (yagi_button("Click me!")) {
                TraceLog(LOG_INFO, "Click me! 1");
            }
            yagi_begin_sublayout(LAYOUT_HORZ, 5);
                yagi_text("Hello, World!");
                yagi_empty((Vector2) { 10, 10 });
                if (yagi_button("Click me!")) {
                    TraceLog(LOG_INFO, "Click me! 2");
                }
                yagi_dropdown(&selected, labels, LABEL_COUNT);
            yagi_end_layout();
            yagi_text("Hello, World!");
        yagi_end_layout();

        yagi_ui_end();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
