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
}Layout;

typedef struct {
    UIID active, focus, highlight;
    UIID id_counter;

    Layout layout_stack[YAGI_LAYOUT_MAX_COUNT];
    size_t layout_count;
}YagiUi;

UIID yagi_id_next();
static void yagi_expand_layout(Vector2 size);
static Vector2 yagi_next_widget_pos();

void yagi_ui_begin();
void yagi_begin_layout(LayoutType type, Vector2 pos, float padding);
void yagi_begin_sublayout(LayoutType type, float padding);
void yagi_end_layout();
void yagi_ui_end();

void yagi_text(const char* text);
void yagi_empty(Vector2 size);
bool yagi_button(const char* label);

extern YagiUi yagi_ui;

#endif // YAGI_H_

#ifdef YAGI_IMPLEMENTATION
#undef YAGI_IMPLEMENTATION

YagiUi yagi_ui = {0};

static Layout* yagi__top_layout() {
    assert(yagi_ui.layout_count > 0);
    return &yagi_ui.layout_stack[yagi_ui.layout_count - 1];
}

static Vector2 yagi_next_widget_pos() {
    Layout* top = yagi__top_layout();

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

static void yagi_expand_layout(Vector2 widget_size) {
    Layout* top = yagi__top_layout();

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

void yagi_begin_layout(LayoutType type, Vector2 pos, float padding) {
    assert(yagi_ui.layout_count < YAGI_LAYOUT_MAX_COUNT);
    Layout layout = {type, pos, {0, 0}, padding};
    yagi_ui.layout_stack[yagi_ui.layout_count++] = layout;
}

void yagi_begin_sublayout(LayoutType type, float padding) {
    assert(yagi_ui.layout_count > 0);
    assert(yagi_ui.layout_count < YAGI_LAYOUT_MAX_COUNT);

    Layout layout = {type, yagi_next_widget_pos(), {0, 0}, padding};
    yagi_ui.layout_stack[yagi_ui.layout_count++] = layout;
}

void yagi_end_layout() {
    Layout* child = yagi__top_layout();
    yagi_ui.layout_count--;

#ifdef DEBUG
    DrawRectangleLines(child->pos.x, child->pos.y, child->size.x, child->size.y, BLACK);
#endif // DEBUG

    if (yagi_ui.layout_count > 0) {
        yagi_expand_layout(child->size);
    }
}

void yagi_ui_begin() {
    yagi_ui.highlight = 0;
    yagi_ui.id_counter = 0;
}

void yagi_ui_end() {
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) yagi_ui.active = 0;
    else if (yagi_ui.active == 0) yagi_ui.active = UINT64_MAX;
}

void yagi_text(const char* text) {
    Vector2 pos = yagi_next_widget_pos();

    int text_width = MeasureText(text, 20);
    DrawText(text, pos.x, pos.y, 20, BLACK);

    Vector2 size = { text_width, 20 };
    yagi_expand_layout(size);
}

void yagi_empty(Vector2 size) {
    yagi_expand_layout(size);
}

bool yagi_button(const char* label) {
    UIID id = yagi_id_next();
    bool clicked = false;

    Vector2 widget_size = { MeasureText(label, 20), 20 };
    Vector2 pos = yagi_next_widget_pos();
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

    yagi_expand_layout(widget_size);

    return clicked;
}

#endif // YAGI_IMPLEMENTATION
