#ifndef YAGI_H_
#define YAGI_H_
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <raylib.h>

typedef uint64_t UIID;
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
    Color bg_color, text_color;
    int font_size, font_spacing;

    Font font;
}YagiStyle;

typedef struct {
    UIID active, focus, highlight;
    UIID id_counter;

    YagiStyle style;

    size_t start_counter;

#define YAGI_LAYOUT_MAX_COUNT 1024
    Layout layout_stack[YAGI_LAYOUT_MAX_COUNT];
    size_t layout_count;
}YagiUi;

typedef struct {
    int* codepoints;
    size_t count;
    size_t capacity;
}InputBuffer;

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

void yagi_ui_set_default_style();
YagiStyle* yagi_ui_get_style();
YagiStyle yagi_ui_get_style_copy();

void yagi_text_with_loc(const char* file, int line, const char* fmt, ...);
void yagi_empty_with_loc(Vector2 size, const char* file, int line);
bool yagi_button_with_loc(const char* label, const char* file, int line);
bool yagi_dropdown_with_loc(int* already_selected, char* labels[], size_t label_count, const char* file, int line);
// TODO: add selection
// TODO: key things (moving by word, etc.)
bool yagi_input_with_loc(int width, InputBuffer* input_buffer, const char* file, int line);
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
#define yagi_input(width, input_buffer) yagi_input_with_loc(width, input_buffer, __FILE__, __LINE__)
#define yagi_slider(width, value_ptr) yagi_slider_with_loc(width, value_ptr, __FILE__, __LINE__)

extern YagiUi yagi_ui;

#endif // YAGI_H_

#ifdef YAGI_IMPLEMENTATION
#undef YAGI_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void yagi__input_buffer_push(InputBuffer* self, int c) {
    if (self->count >= self->capacity) {
        if (self->capacity == 0) self->capacity = 16;
        while (self->count >= self->capacity) self->capacity *= 2;
        self->codepoints = realloc(self->codepoints, sizeof(*self->codepoints) * self->capacity);
    }
    self->codepoints[self->count++] = c;
}

static char yagi_utf8_temp_buf[1024] = {0};
char* yagi_utf8_temp(int* codepoints, int codepoints_count) {
    char* utf8 = LoadUTF8(codepoints, codepoints_count);
    strcpy(yagi_utf8_temp_buf, utf8);
    UnloadUTF8(utf8);
    return yagi_utf8_temp_buf;
}

YagiUi yagi_ui = {0};

YagiStyle* yagi_ui_get_style() {
    return &yagi_ui.style;
}

YagiStyle yagi_ui_get_style_copy() {
    return yagi_ui.style;
}

void yagi_ui_set_default_style() {
    yagi_ui.style = (YagiStyle) {
        .bg_color = WHITE,
        .text_color = BLACK,
        .font_size = 20,
        .font_spacing = 1,
        .font = GetFontDefault()
    };
}

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

    yagi_ui_set_default_style();
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

    Vector2 text_size = MeasureTextEx(yagi_ui.style.font, yagi_text_buffer, yagi_ui.style.font_size, yagi_ui.style.font_spacing);
    DrawTextEx(yagi_ui.style.font, yagi_text_buffer, (Vector2) {pos.x, pos.y}, yagi_ui.style.font_size, yagi_ui.style.font_spacing, yagi_ui.style.text_color);

    yagi_expand_layout_with_loc(text_size, file, line);
}

void yagi_empty_with_loc(Vector2 size, const char* file, int line) {
    yagi_expand_layout_with_loc(size, file, line);
}

bool yagi_button_with_loc(const char* label, const char* file, int line) {
    UIID id = yagi_id_next();
    bool clicked = false;

    Vector2 widget_size = { MeasureText(label, yagi_ui.style.font_size), yagi_ui.style.font_size };
    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, widget_size.x, widget_size.y };
    Rectangle border_rect = { rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4 };

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

    Color bg = yagi_ui.style.bg_color;
    if (yagi_ui.highlight == id) bg = ColorBrightness(bg, -0.5);

    DrawRectangleRec(border_rect, yagi_ui.style.text_color);
    DrawRectangleRec(rect, bg);
    DrawTextEx(yagi_ui.style.font, label, (Vector2) {rect.x + rect.width / 2 - widget_size.x / 2, rect.y + rect.height / 2 - widget_size.y / 2}, yagi_ui.style.font_size, yagi_ui.style.font_spacing, yagi_ui.style.text_color);

    widget_size.x = border_rect.width;
    widget_size.y = border_rect.height;
    yagi_expand_layout_with_loc(widget_size, file, line);

    return clicked;
}

bool yagi_dropdown_with_loc(int* already_selected, char* labels[], size_t label_count, const char* file, int line) {
    int selected = *already_selected;
    bool changed = false;
    UIID id = yagi_id_next();

    yagi_begin_sublayout(LAYOUT_VERT, 0);
    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, 0, yagi_ui.style.font_size };
    for (size_t i = 0; i < label_count; i++) {
        Vector2 text_size = MeasureTextEx(yagi_ui.style.font, labels[i], yagi_ui.style.font_size, yagi_ui.style.font_spacing);
        if (rect.width < text_size.x) rect.width = text_size.x;
    }

    if (selected < 0) {
        Vector2 text_size = MeasureTextEx(yagi_ui.style.font, "Select...", yagi_ui.style.font_size, yagi_ui.style.font_spacing);
        if (rect.width < text_size.x) rect.width = text_size.x;
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

    char* label = selected < 0 ? "Select..." : labels[selected];
    Vector2 text_size = MeasureTextEx(yagi_ui.style.font, label, yagi_ui.style.font_size, yagi_ui.style.font_spacing);
    Color bg = yagi_ui.style.bg_color;
    if (yagi_ui.highlight == id) bg = ColorBrightness(bg, -0.5);

    DrawRectangleRec((Rectangle) { rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4 }, yagi_ui.style.text_color);
    DrawRectangleRec(rect, bg);

    DrawTextEx(yagi_ui.style.font, label, (Vector2) {rect.x + rect.width / 2 - text_size.x / 2, rect.y + rect.height / 2 - 10}, yagi_ui.style.font_size, yagi_ui.style.font_spacing, yagi_ui.style.text_color);

    yagi_expand_layout_with_loc((Vector2) { rect.width, rect.height }, file, line);
    if (yagi_ui.focus == id) {
        for (size_t i = 0; i < label_count; i++) {
            Vector2 text_size = MeasureTextEx(yagi_ui.style.font, labels[i], yagi_ui.style.font_size, yagi_ui.style.font_spacing);
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

            Color bg = yagi_ui.style.bg_color;
            if (yagi_ui.highlight == item_id) bg = ColorBrightness(bg, -0.5);

            DrawRectangleRec((Rectangle) { item_rect.x - 2, item_rect.y - 2, item_rect.width + 4, item_rect.height + 4 }, yagi_ui.style.text_color);
            DrawRectangleRec(item_rect, bg);

            DrawTextEx(yagi_ui.style.font, labels[i], (Vector2) {item_rect.x + item_rect.width / 2 - text_size.x / 2, item_rect.y + item_rect.height / 2 - yagi_ui.style.font_size / 2}, yagi_ui.style.font_size, yagi_ui.style.font_spacing, yagi_ui.style.text_color);
            
            yagi_expand_layout_with_loc((Vector2) { item_rect.width, item_rect.height }, file, line);
        }
    }
    yagi_end_layout_with_loc(file, line);

    if (yagi_ui.focus == id && !collides_main && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        yagi_ui.focus = 0;
    }

    *already_selected = selected;
    return changed;
}


bool yagi_input_with_loc(int width, InputBuffer* input_buffer, const char* file, int line) {
    bool changed = false;
    UIID id = yagi_id_next();

    Vector2 pos = yagi_next_widget_pos_with_loc(file, line);
    Rectangle rect = { pos.x, pos.y, width, yagi_ui.style.font_size };

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
        if (key >= ' ') {
            yagi__input_buffer_push(input_buffer, key);
            changed = true;
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && input_buffer->count > 0) {
            input_buffer->count--;
            changed = true;
        }
    }

    if (is_focused) DrawRectangle(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4, yagi_ui.style.text_color);
    DrawRectangleRec(rect, yagi_ui.style.bg_color);

    size_t codepoint_offset = 0;

    char* utf8 = yagi_utf8_temp(input_buffer->codepoints + codepoint_offset, input_buffer->count - codepoint_offset);
    Vector2 text_size = MeasureTextEx(yagi_ui.style.font, utf8, yagi_ui.style.font_size, yagi_ui.style.font_spacing);
    while (codepoint_offset < input_buffer->count && text_size.x > width) {
        codepoint_offset++;

        utf8 = yagi_utf8_temp(input_buffer->codepoints + codepoint_offset, input_buffer->count - codepoint_offset);
        text_size = MeasureTextEx(yagi_ui.style.font, utf8, yagi_ui.style.font_size, yagi_ui.style.font_spacing);
    }

    Rectangle cursor = { rect.x + text_size.x, rect.y, 2, yagi_ui.style.font_size };
    DrawTextCodepoints(yagi_ui.style.font, input_buffer->codepoints + codepoint_offset, input_buffer->count - codepoint_offset, (Vector2){ rect.x, rect.y }, yagi_ui.style.font_size, yagi_ui.style.font_spacing, yagi_ui.style.text_color);
    if (is_focused) DrawRectangleRec(cursor, yagi_ui.style.text_color);

    if (!collides && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && yagi_ui.focus == id) yagi_ui.focus = 0;
    
    yagi_expand_layout_with_loc((Vector2){width, yagi_ui.style.font_size}, file, line);

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

    DrawRectangle(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4, yagi_ui.style.text_color);
    DrawRectangleRec(rect, yagi_ui.style.bg_color);

    Color circle_color = yagi_ui.style.text_color;
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
