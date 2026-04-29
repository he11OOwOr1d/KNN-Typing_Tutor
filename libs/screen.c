#include <stdio.h>
#include "screen.h"

void clear_screen() {
    printf("\033[H\033[J");
    fflush(stdout);
}

void clear_screen_and_save() {
    // Explicitly pushes the current view to scrollback history
    // and clears the screen cleanly for the main menu
    printf("\033[2J\033[H");
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

void set_latency_color(int avg) {
    if (avg == 0)      set_color_dim();
    else if (avg < 150) printf("\033[38;5;46m");   // Bright green (fastest)
    else if (avg < 250) printf("\033[38;5;82m");   // Green
    else if (avg < 400) printf("\033[38;5;220m");  // Yellow (normal)
    else if (avg < 600) printf("\033[38;5;208m");  // Orange (slow)
    else                printf("\033[38;5;196m");  // Red (weakness)
    fflush(stdout);
}

void draw_keyboard_heatmap(UserStats* stats) {
    char* rows[] = {
        "qwertyuiop",
        "asdfghjkl",
        "zxcvbnm"
    };
    int offsets[] = {0, 1, 2};

    printf("\n");
    set_color_cyan(); set_color_bold();
    printf("  ┌─────────────────────────────────────────┐\n");
    printf("  │         KEYBOARD HEATMAP                 │\n");
    printf("  └─────────────────────────────────────────┘\n");
    reset_color();
    printf("\n");

    for (int r = 0; r < 3; r++) {
        // Key labels
        printf("  ");
        for (int o = 0; o < offsets[r] * 2; o++) printf(" ");
        for (int i = 0; rows[r][i] != '\0'; i++) {
            char c = rows[r][i];
            int avg = get_average_latency(stats, c);
            if (avg == 0) {
                set_color_dim();
                printf(" %c  ", c - 32);
            } else {
                set_latency_color(avg);
                printf(" %c  ", c - 32);
            }
            reset_color();
        }
        printf("\n\n");
    }

    // Space bar
    int space_avg = get_average_latency(stats, ' ');
    printf("        ");
    if (space_avg == 0) set_color_dim(); else set_latency_color(space_avg);
    printf("      [  SPACE  ]");
    reset_color();
    printf("\n\n");

    // Legend
    set_color_dim(); printf("  ");
    printf("\033[38;5;46m█\033[0m");  set_color_dim(); printf("<150ms  ");
    printf("\033[38;5;82m█\033[0m");  set_color_dim(); printf("<250ms  ");
    printf("\033[38;5;220m█\033[0m"); set_color_dim(); printf("<400ms  ");
    printf("\033[38;5;208m█\033[0m"); set_color_dim(); printf("<600ms  ");
    printf("\033[38;5;196m█\033[0m"); set_color_dim(); printf("600+ms");
    reset_color();
    printf("\n\n");
}
