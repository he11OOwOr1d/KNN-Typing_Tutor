#ifndef SCREEN_H
#define SCREEN_H
#include "analytics.h"

void clear_screen();
void clear_screen_and_save();
void enter_alt_screen();
void exit_alt_screen();
void print_string(char* str);
void print_char(char c);
void print_int(int n);

void set_color_green();
void set_color_red();
void set_color_dim();
void set_color_cyan();
void set_color_yellow();
void set_color_bold();
void reset_color();

void cursor_left();
void move_cursor(int row, int col);
void hide_cursor();
void show_cursor();

void draw_box_top(int width);
void draw_box_bottom(int width);
void draw_box_row(int width, char* content);
void draw_neon_box(int width, int height);
void draw_progress_bar(int current, int total, int width);
void set_latency_color(int avg);
void draw_keyboard_heatmap(UserStats* stats);

#endif
