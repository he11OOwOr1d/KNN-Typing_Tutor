#include <stdio.h>
#include "screen.h"

void clear_screen() {
    printf("\033[2J\033[1;1H");
    fflush(stdout);
}

void print_string(char* str) {
    printf("%s", str);
    fflush(stdout);
}

void print_char(char c) {
    printf("%c", c);
    fflush(stdout);
}

void print_int(int n) {
    printf("%d", n);
    fflush(stdout);
}

void set_color_green()  { printf("\033[38;5;82m");  fflush(stdout); }
void set_color_red()    { printf("\033[38;5;196m"); fflush(stdout); }
void set_color_dim()    { printf("\033[38;5;240m"); fflush(stdout); }
void set_color_cyan()   { printf("\033[38;5;51m");  fflush(stdout); }
void set_color_yellow() { printf("\033[38;5;220m"); fflush(stdout); }
void set_color_bold()   { printf("\033[1m");         fflush(stdout); }
void reset_color()      { printf("\033[0m");         fflush(stdout); }

void cursor_left() {
    printf("\b");
    fflush(stdout);
}

void move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

void hide_cursor() { printf("\033[?25l"); fflush(stdout); }
void show_cursor() { printf("\033[?25h"); fflush(stdout); }

// UTF-8 box drawing characters:
//   top-left: ╭  top-right: ╮
//   bottom-left: ╰  bottom-right: ╯
//   horizontal: ─  vertical: │
void draw_box_top(int width) {
    printf("\033[38;5;240m");
    printf("  \xe2\x95\xad");                  // ╭
    for (int i = 0; i < width; i++) printf("\xe2\x94\x80"); // ─
    printf("\xe2\x95\xae\n");                  // ╮
    fflush(stdout);
}

void draw_box_bottom(int width) {
    printf("  \xe2\x95\xb0");                  // ╰
    for (int i = 0; i < width; i++) printf("\xe2\x94\x80"); // ─
    printf("\xe2\x95\xaf\n");                  // ╯
    printf("\033[0m");
    fflush(stdout);
}

// Print a box row: │ <spaces> │
// Content must be printed separately after calling this
void draw_box_row(int width, char* content) {
    printf("\033[38;5;240m  \xe2\x94\x82\033[0m "); // │
    printf("%s", content);
    // pad remaining spaces
    int len = 0;
    while (content[len]) len++;
    for (int i = len; i < width - 2; i++) printf(" ");
    printf("\033[38;5;240m \xe2\x94\x82\033[0m\n"); // │
    fflush(stdout);
}

// Render a progress bar: [████████░░░░] XX%
void draw_progress_bar(int current, int total, int width) {
    int filled = (total == 0) ? 0 : (current * width) / total;
    printf("  [");
    printf("\033[38;5;82m");
    for (int i = 0; i < filled; i++) printf("\xe2\x96\x88"); // █
    printf("\033[38;5;240m");
    for (int i = filled; i < width; i++) printf("\xe2\x96\x91"); // ░
    printf("\033[0m] ");
    int pct = (total == 0) ? 0 : (current * 100) / total;
    printf("\033[38;5;220m%d%%\033[0m", pct);
    fflush(stdout);
}
