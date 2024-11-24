#ifndef YAGI_H_
#define YAGI_H_
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <raylib.h>

typedef uint64_t UIID;

#define YAGI_LAYOUT_MAX_COUNT 1024

typedef enum { LAYOUT_HORZ, LAYOUT_VERT }LayoutType;

typedef struct {
    LayoutType type;
    Vector2 pos;
    Vector2 size;
    float padding;

    const char* file;
    int line;
}Layout;

typedef struct {
    UIID active, focus, highlight;
    UIID id_counter;

    size_t start_counter;

    Layout layout_stack[YAGI_LAYOUT_MAX_COUNT];
    size_t layout_count;
}YagiUi;

UIID yagi_id_next();
char* yagi_utf8_temp(int* codepoints, int codepoints_count);

static void yagi_expand_layout_with_loc(Vector2 size, const char* file, int line);
static Vector2 yagi_next_widget_pos_with_loc(const char* file, int line);

#define yagi_next_widget_pos() yagi_next_widget_pos_with_loc(__FILE__, __LINE__)
#define yagi_expand_layout(size) yagi_expand_layout_with_loc(size, __FILE__, __LINE__)

void yagi_ui_begin_with_loc(const char* file, int line);
void yagi_begin_layout_with_loc(LayoutType type, Vector2 pos, float padding, const char* file, int line);
void yagi_begin_sublayout_with_loc(LayoutType type, float padding, const char* file, int line);
void yagi_end_layout_with_loc(const char* file, int line);
void yagi_ui_end_with_loc(const char* file, int line);

// TODO: add file and line reporting
void yagi_text_with_loc(const char* file, int line, const char* fmt, ...);
void yagi_empty_with_loc(Vector2 size, const char* file, int line);
bool yagi_button_with_loc(const char* label, const char* file, int line);
bool yagi_dropdown_with_loc(int* already_selected, char* labels[], size_t label_count, const char* file, int line);
// TODO: add selection
// TODO: key things (moving by word, etc.)
bool yagi_input_with_loc(Vector2 size, int* codepoints, size_t* codepoint_count_ptr, size_t codepoint_count_max, const char* file, int line);
bool yagi_slider_with_loc(int width, float* value_ptr, const char* file, int line);

#define yagi_ui_begin() yagi_ui_begin_with_loc(__FILE__, __LINE__)
#define yagi_begin_layout(type, pos, padding) yagi_begin_layout_with_loc(type, pos, padding, __FILE__, __LINE__)
#define yagi_begin_sublayout(type, padding) yagi_begin_sublayout_with_loc(type, padding, __FILE__, __LINE__)
#define yagi_end_layout() yagi_end_layout_with_loc(__FILE__, __LINE__)
#define yagi_ui_end() yagi_ui_end_with_loc(__FILE__, __LINE__)

#define yagi_text(...) yagi_text_with_loc(__FILE__, __LINE__, __VA_ARGS__)
#define yagi_empty(size) yagi_empty_with_loc(size, __FILE__, __LINE__)
#define yagi_button(label) yagi_button_with_loc(label, __FILE__, __LINE__)
#define yagi_dropdown(already_selected, labels, label_count) yagi_dropdown_with_loc(already_selected, labels, label_count, __FILE__, __LINE__)
#define yagi_input(size, codepoints, codepoint_count_ptr, codepoint_count_max) yagi_input_with_loc(size, codepoints, codepoint_count_ptr, codepoint_count_max, __FILE__, __LINE__)
#define yagi_slider(width, value_ptr) yagi_slider_with_loc(width, value_ptr, __FILE__, __LINE__)

extern YagiUi yagi_ui;

#endif // YAGI_H_

#ifdef YAGI_IMPLEMENTATION
#undef YAGI_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char yagi_utf8_temp_buf[1024] = {0};
char* yagi_utf8_temp(int* codepoints, int codepoints_count) {
    char* utf8 = LoadUTF8(codepoints, codepoints_count);
    strcpy(yagi_utf8_temp_buf, utf8);
    UnloadUTF8(utf8);
    return yagi_utf8_temp_buf;
}

YagiUi yagi_ui = {0};

static Layout* yagi__top_layout_with_loc(const char* file, int line) {
    if (yagi_ui.layout_count <= 0) {
        fprintf(stderr, "%s: %d: Layout stack underflow\n", file, line);
        abort();
    }

    return &yagi_ui.layout_stack[yagi_ui.layout_count - 1];
}

static Vector2 yagi_next_widget_pos_with_loc(const char* file, int line) {
    Layout* top = yagi__top_layout_with_loc(file, line);

    Vector2 pos = top->pos;
    switch (top->type) {
        case LAYOUT_HORZ:
            pos.x += top->size.x;
            return pos;
        case LAYOUT_VERT:
            pos.y += top->size.y;
            return pos;
        default:
            assert(0);
    }
}

static void yagi_expand_layout_with_loc(Vector2 widget_size, const char* file, int line) {
    Layout* top = yagi__top_layout_with_loc(file, line);
    
    switch (top->type) {
        case LAYOUT_HORZ:
            top->size.x += widget_size.x + top->padding;
            if (top->size.y < widget_size.y) {
                top->size.y = widget_size.y;
            }
            break;
        case LAYOUT_VERT:
            top->size.y += widget_size.y + top->padding;
            if (top->size.x < widget_size.x) {
                top->size.x = widget_size.x;
            }
            break;
    }
}

UIID yagi_id_next() {
    return ++yagi_ui.id_counter;
}

void yagi_begin_layout_with_loc(LayoutType type, Vector2 pos, float padding, const char* file, int line) {
    if (yagi_ui.layout_count >= YAGI_LAYOUT_MAX_COUNT) {
        fprintf(stderr, "[YAGI] %s: %d: Layout stack overflow\n", file, line);
        abort();
    }

    Layout layout = {type, pos, {0, 0}, padding, file, line};
    yagi_ui.layout_stack[yagi_ui.layout_count++] = layout;
}

void yagi_begin_sublayout_with_loc(LayoutType type, float padding, const char* file, int line) {
    if (yagi_ui.layout_count <= 0) {
        fprintf(stderr, "[YAGI] %s: %d: Layout needed to create sublayout\n", file, line);
        abort();
    }

    yagi_begin_layout_with_loc(type, yagi_next_widget_pos_with_loc(file, line), padding, file, line);
}

void yagi_end_layout_with_loc(const char* file, int line) {
    Layout* child = yagi__top_layout_with_loc(file, line);
    yagi_ui.layout_count--;

#ifdef YAGI_DEBUG
    DrawRectangleLines(child->pos.x, child->pos.y, child->size.x, child->size.y, BLACK);
#endif // YAGI_DEBUG

    if (yagi_ui.layout_count > 0) {
        yagi_expand_layout_with_loc(child->size, file, line);
    }
}

void yagi_ui_begin_with_loc(const char* file, int line) {
    if (yagi_ui.start_counter > 0) {
        fprintf(stderr, "[YAGI] %s:%d: yagi_ui_end was not called\n", file, line);
        abort();
    }
    yagi_ui.start_counter += 1;

    yagi_ui.highlight = 0;
    yagi_ui.id_counter = 0;
}

void yagi_ui_end_with_loc(const char* file, int line) {
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) yagi_ui.active = 0;
    else if (yagi_ui.active == 0) yagi_ui.active = UINT64_MAX;

    if (yagi_ui.layout_count > 0) {
        fprintf(stderr, "[YAGI] Layout stack not empty: \n");
        for (size_t i = 0; i < yagi_ui.layout_count; i++) {
            Layout* layout = &yagi_ui.layout_stack[i];
            fprintf(stderr, "[YAGI]     Layout declared at %s:%d\n", layout->file, layout->line);
        }
        abort();
    }

    if (yagi_ui.start_counter == 0) {
        fprintf(stderr, "[YAGI] %s:%d: yagi_ui_begin was not called\n", file, line);
        abort();
    }
    yagi_ui.start_counter -= 1;
}

void yagi_text_with_loc(const char* file, int line, const char* fmt, ...) {
static char yagi_text_buffer[4096] = {0};
    va_list args;

    va_start(args, fmt);
    vsnprintf(yagi_text_buffer, 4096, fmt, args);
    va_end(args);

    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);

    int text_width = MeasureText(yagi_text_buffer, 20);
    DrawText(yagi_text_buffer, pos.x, pos.y, 20, BLACK);


    Vector2 size = { text_width, 20 };
    yagi_expand_layout_with_loc(size, file, line);
}

void yagi_empty_with_loc(Vector2 size, const char* file, int line) {
    yagi_expand_layout_with_loc(size, file, line);
}

bool yagi_button_with_loc(const char* label, const char* file, int line) {
    UIID id = yagi_id_next();
    bool clicked = false;

    Vector2 widget_size = { MeasureText(label, 20), 20 };
    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, widget_size.x, widget_size.y };

    Vector2 mouse = GetMousePosition();
    bool collides = CheckCollisionPointRec(mouse, rect);
    if (collides) {
        yagi_ui.highlight = id;
        if (yagi_ui.active == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            yagi_ui.active = id;
        }
    }

    if (yagi_ui.active == id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (collides) {
            clicked = true;
        }
        yagi_ui.active = 0;
    }

    Color bg = WHITE;
    if (yagi_ui.highlight == id) bg = ColorBrightness(bg, -0.5);

    DrawRectangleRec((Rectangle) { rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4 }, BLACK);
    DrawRectangleRec(rect, bg);
    DrawText(label, rect.x + rect.width / 2 - widget_size.x / 2, rect.y + rect.height / 2 - widget_size.y / 2, 20, BLACK);

    yagi_expand_layout_with_loc(widget_size, file, line);

    return clicked;
}

bool yagi_dropdown_with_loc(int* already_selected, char* labels[], size_t label_count, const char* file, int line) {
    int selected = *already_selected;
    bool changed = false;
    UIID id = yagi_id_next();

    yagi_begin_sublayout(LAYOUT_VERT, 0);
    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, 0, 20 };
    for (size_t i = 0; i < label_count; i++) {
        int text_width = MeasureText(labels[i], 20);
        if (rect.width < text_width) rect.width = text_width;
    }

    if (*already_selected == -1) {
        int text_width = MeasureText("Select...", 20);
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

    char* label = selected == -1 ? "Select..." : labels[selected];
    int text_width = MeasureText(label, 20);
    Color bg = WHITE;
    if (yagi_ui.highlight == id) bg = ColorBrightness(bg, -0.5);

    DrawRectangleRec((Rectangle) { rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4 }, BLACK);
    DrawRectangleRec(rect, bg);

    DrawText(label, rect.x + rect.width / 2 - (float)text_width / 2, rect.y + rect.height / 2 - 10, 20, BLACK);

    yagi_expand_layout_with_loc((Vector2) { rect.width, rect.height }, file, line);
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
                    changed = true;
                }
                yagi_ui.active = 0;
            }

            Color bg = WHITE;
            if (yagi_ui.highlight == item_id) bg = ColorBrightness(bg, -0.5);

            DrawRectangleRec((Rectangle) { item_rect.x - 2, item_rect.y - 2, item_rect.width + 4, item_rect.height + 4 }, BLACK);
            DrawRectangleRec(item_rect, bg);

            DrawText(labels[i], item_rect.x + item_rect.width / 2 - (float)text_width / 2, item_rect.y + item_rect.height / 2 - 10, 20, BLACK);
            
            yagi_expand_layout_with_loc((Vector2) { item_rect.width, item_rect.height }, file, line);
        }
    }
    yagi_end_layout();

    if (yagi_ui.focus == id && !collides_main && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        yagi_ui.focus = 0;
    }

    *already_selected = selected;
    return changed;
}


bool yagi_input_with_loc(Vector2 size, int* codepoints, size_t* codepoint_count_ptr, size_t codepoint_count_max, const char* file, int line) {
    size_t codepoint_count = *codepoint_count_ptr;
    bool changed = false;
    UIID id = yagi_id_next();

    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, size.x, size.y };

    Vector2 mouse = GetMousePosition();
    bool collides = CheckCollisionPointRec(mouse, rect);
    if (collides) {
        yagi_ui.highlight = id;
        if (yagi_ui.active == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            yagi_ui.active = id;
        }
    }

    if (yagi_ui.active == id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (collides) {
            yagi_ui.focus = id;
        }
        yagi_ui.active = 0;
    }

    bool is_focused = yagi_ui.focus == id;
    if (is_focused) {
        int key = GetCharPressed();
        if (key >= ' ' && codepoint_count < codepoint_count_max) {
            codepoints[codepoint_count++] = key;
            changed = true;
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && codepoint_count > 0) {
            codepoint_count--;
            changed = true;
        }
    }

    if (is_focused) DrawRectangle(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4, BLACK);
    DrawRectangleRec(rect, WHITE);

    size_t codepoint_offset = 0;

    char* utf8 = yagi_utf8_temp(codepoints + codepoint_offset, codepoint_count - codepoint_offset);
    Vector2 text_width = MeasureTextEx(GetFontDefault(), utf8, 20, 1);
    while (codepoint_offset < codepoint_count && text_width.x > size.x) {
        codepoint_offset++;

        utf8 = yagi_utf8_temp(codepoints + codepoint_offset, codepoint_count - codepoint_offset);
        text_width = MeasureTextEx(GetFontDefault(), utf8, 20, 1);
    }

    codepoints[codepoint_count] = 0;
    Rectangle cursor = { rect.x + text_width.x, rect.y, 2, 20 };
    DrawTextCodepoints(GetFontDefault(), codepoints + codepoint_offset, codepoint_count - codepoint_offset, (Vector2){ rect.x, rect.y }, 20, 1, BLACK);
    if (is_focused) DrawRectangleRec(cursor, BLACK);

    if (!collides && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && yagi_ui.focus == id) yagi_ui.focus = 0;
    
    yagi_expand_layout_with_loc(size, file, line);

    *codepoint_count_ptr = codepoint_count;
    return changed;
}

bool yagi_slider_with_loc(int width, float* value_ptr, const char* file, int line) {
    UIID id = yagi_id_next();

    float value = *value_ptr;
    bool changed = false;

    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, width, 4 };
    Vector2 ball_pos = { rect.x + width * value, rect.y + rect.height / 2 };
    float ball_r = rect.height * 2;

    Vector2 mouse = GetMousePosition();
    bool collides_with_ball = CheckCollisionPointCircle(mouse, ball_pos, ball_r);
    if (collides_with_ball) {
        yagi_ui.highlight = id;
        if (yagi_ui.active == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            yagi_ui.active = id;
        }
    }

    if (yagi_ui.active == id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        yagi_ui.active = 0;
    }

    DrawRectangle(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4, BLACK);
    DrawRectangleRec(rect, WHITE);

    Color circle_color = BLACK;
    if (yagi_ui.highlight == id) circle_color = ColorBrightness(circle_color, 0.5);
    DrawCircleV(ball_pos, ball_r, circle_color);

    if (yagi_ui.active == id) {
        Vector2 mouse_delta = GetMouseDelta();
        value = (ball_pos.x - rect.x + mouse_delta.x) / rect.width;
        if (value < 0) value = 0;
        if (value > 1) value = 1;
        changed = true;
    }

    yagi_expand_layout_with_loc((Vector2) { rect.width, rect.height }, file, line);

    *value_ptr = value;
    return changed;
}

#endif // YAGI_IMPLEMENTATION
