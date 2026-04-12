#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "libs/memory.h"
#include "libs/string.h"
#include "libs/math.h"
#include "libs/screen.h"
#include "libs/keyboard.h"

// 32 words, 32 = 2^5 so we can use bitmask & 31 instead of modulo
const char* dictionary[] = {
    "the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog",
    "c", "code", "memory", "buffer", "pointer", "variable", "system",
    "terminal", "unix", "linux", "compiler", "debug", "keyboard", "screen",
    "string", "integer", "return", "void", "static", "volatile", "mac", "sys", "process", "alloc"
};

// ASCII art title (hard-coded, printed via write, no external libs)
void draw_title() {
    clear_screen();
    hide_cursor();
    printf("\n");

    set_color_cyan();
    set_color_bold();
    printf("  ████████╗██╗   ██╗██████╗ ██╗███╗  ██╗ ██████╗\n");
    printf("     ██╔══╝╚██╗ ██╔╝██╔══██╗██║████╗ ██║██╔════╝\n");
    printf("     ██║    ╚████╔╝ ██████╔╝██║██╔██╗██║██║  ███╗\n");
    printf("     ██║     ╚██╔╝  ██╔═══╝ ██║██║╚████║██║   ██║\n");
    printf("     ██║      ██║   ██║     ██║██║ ╚███║╚██████╔╝\n");
    printf("     ╚═╝      ╚═╝   ╚═╝     ╚═╝╚═╝  ╚══╝ ╚═════╝\n");
    reset_color();

    set_color_dim();
    printf("\n        T E R M I N A L   T Y P I N G   T U T O R\n");
    reset_color();

    printf("\n");
    set_color_dim();
    printf("  ─────────────────────────────────────────────────────\n");
    reset_color();
    printf("\n");
}

// Re-render the full live view on every keypress
void render_typing_view(char* target, char* typed, int typed_len, int target_len,
                        int correct, int mistakes, int wpm_now, int accuracy_now,
                        long elapsed_sec) {
    // Move to line 10 to overwrite the typing area
    move_cursor(10, 1);

    // === TARGET TEXT BOX ===
    // Box inner width = sentence length + 2 padding (1 each side)
    int box_inner = target_len + 2;

    // Top border ╭───────╮
    printf("\033[38;5;240m  \xe2\x95\xad");
    for (int i = 0; i < box_inner; i++) printf("\xe2\x94\x80");
    printf("\xe2\x95\xae\033[0m\n");

    // Label row:  │  SENTENCE  │
    printf("\033[38;5;240m  \xe2\x94\x82\033[0m ");
    set_color_dim();
    printf("SENTENCE");
    reset_color();
    for (int i = 8; i < box_inner - 1; i++) printf(" ");
    printf("\033[38;5;240m \xe2\x94\x82\033[0m\n");

    // Divider ├───────────┤
    printf("\033[38;5;240m  \xe2\x94\x9c");
    for (int i = 0; i < box_inner; i++) printf("\xe2\x94\x80");
    printf("\xe2\x94\xa4\033[0m\n");

    // Sentence row │ [colored chars] │
    printf("\033[38;5;240m  \xe2\x94\x82\033[0m ");

    int cursor_done = 0;
    for (int i = 0; i < target_len; i++) {
        if (i < typed_len) {
            // Already typed: green correct, red wrong
            if (typed[i] == target[i]) set_color_green();
            else                        set_color_red();
            printf("%c", target[i]);
            reset_color();
        } else if (i == typed_len && !cursor_done) {
            // Cursor position: \033[7m = reverse video (swaps fg/bg)
            // The actual character stays visible — background becomes highlighted
            printf("\033[7m%c\033[0m", target[i]);
            cursor_done = 1;
        } else {
            // Ghost text for upcoming characters
            set_color_dim();
            printf("%c", target[i]);
            reset_color();
        }
    }

    printf(" \033[38;5;240m\xe2\x94\x82\033[0m\n");

    // Bottom border ╰───────╯
    printf("\033[38;5;240m  \xe2\x95\xb0");
    for (int i = 0; i < box_inner; i++) printf("\xe2\x94\x80");
    printf("\xe2\x95\xaf\033[0m\n");

    printf("\n");

    // === PROGRESS BAR ===
    draw_progress_bar(typed_len, target_len, 40);
    printf("  %d/%d chars\n\n", typed_len, target_len);

    // === LIVE STATS ROW ===
    printf("  ");
    set_color_cyan();
    printf("WPM: ");
    set_color_bold();
    printf("%-4d", wpm_now);
    reset_color();

    printf("    ");

    set_color_green();
    printf("Accuracy: ");
    set_color_bold();
    printf("%-3d%%", accuracy_now);
    reset_color();

    printf("    ");

    set_color_red();
    printf("Mistakes: ");
    set_color_bold();
    printf("%-3d", mistakes);
    reset_color();

    printf("    ");

    set_color_yellow();
    printf("Time: ");
    set_color_bold();
    printf("%lds", elapsed_sec);
    reset_color();

    printf("\n\n");
    set_color_dim();
    printf("  [ESC] quit    [Backspace] correct\n");
    reset_color();

    fflush(stdout);
}

void draw_results(int wpm, int accuracy, int mistakes, long seconds, int total_chars) {
    clear_screen();
    show_cursor();
    printf("\n");

    set_color_yellow();
    set_color_bold();
    printf("  ╔══════════════════════════════════════════╗\n");
    printf("  ║           SESSION COMPLETE!              ║\n");
    printf("  ╚══════════════════════════════════════════╝\n");
    reset_color();
    printf("\n");

    printf("  ");
    set_color_cyan();
    printf("⌨  Words Per Minute  : ");
    set_color_bold();
    printf("%d\n", wpm);
    reset_color();

    printf("  ");
    set_color_green();
    printf("✓  Accuracy          : ");
    set_color_bold();
    printf("%d%%\n", accuracy);
    reset_color();

    printf("  ");
    set_color_red();
    printf("✗  Mistakes          : ");
    set_color_bold();
    printf("%d\n", mistakes);
    reset_color();

    printf("  ");
    set_color_yellow();
    printf("⏱  Time Taken        : ");
    set_color_bold();
    printf("%ld seconds\n", seconds);
    reset_color();

    printf("  ");
    set_color_dim();
    printf("Σ  Total Characters  : ");
    reset_color();
    printf("%d\n", total_chars);

    printf("\n");
    set_color_dim();
    printf("  ──────────────────────────────────────────\n");
    printf("  Run ./tutor again to play a new round!\n\n");
    reset_color();
    fflush(stdout);
}

int main() {
    draw_title();

    atexit(disable_raw_mode);

    // Seed PRNG with microsecond time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    my_srand(tv.tv_usec);

    char* target = (char*) my_alloc(256);
    char* typed  = (char*) my_alloc(256);
    target[0] = '\0';

    // Build a random 8-word sentence
    int WORD_COUNT = 8;
    for (int i = 0; i < WORD_COUNT; i++) {
        unsigned int random_index = my_rand() & 31;  // fast bitmask, no division needed
        my_strcat(target, dictionary[random_index]);
        if (i < WORD_COUNT - 1) my_strcat(target, " ");
    }

    int target_len = my_strlen(target);

    // Print static header above the typing box (only once)
    move_cursor(8, 1);
    set_color_dim();
    printf("  Press any key to start the timer...\n");
    reset_color();
    fflush(stdout);

    // Draw initial empty state
    render_typing_view(target, typed, 0, target_len, 0, 0, 0, 100, 0);

    enable_raw_mode();

    struct timeval start, now;
    int started = 0;
    int current_index = 0;
    int correct = 0;
    int mistakes = 0;

    while (current_index < target_len) {
        char c = read_key();

        if (!started) {
            gettimeofday(&start, NULL);
            started = 1;
            // Clear the "press any key" hint
            move_cursor(8, 1);
            printf("                                          ");
            fflush(stdout);
        }

        if (c == 27) break;  // ESC

        if (c == 127 || c == '\b') {
            if (current_index > 0) {
                current_index--;
                typed[current_index] = '\0';
            }
        } else {
            typed[current_index] = c;
            current_index++;
            typed[current_index] = '\0';
        }

        // Recompute stats
        correct = 0;
        mistakes = 0;
        for (int i = 0; i < current_index; i++) {
            if (typed[i] == target[i]) correct++;
            else mistakes++;
        }

        int accuracy_now = (current_index > 0)
            ? my_divide(my_multiply(correct, 100), current_index)
            : 100;

        gettimeofday(&now, NULL);
        long elapsed = now.tv_sec - start.tv_sec;
        int wpm_now = (elapsed > 0) ? my_divide(my_multiply(current_index, 12), elapsed) : 0;

        render_typing_view(target, typed, current_index, target_len,
                           correct, mistakes, wpm_now, accuracy_now, elapsed);
    }

    gettimeofday(&now, NULL);
    disable_raw_mode();
    show_cursor();

    long seconds = now.tv_sec - start.tv_sec;
    int accuracy_final = (current_index > 0)
        ? my_divide(my_multiply(correct, 100), current_index)
        : 0;
    int wpm_final = (seconds > 0)
        ? my_divide(my_multiply(current_index, 12), seconds)
        : 0;

    draw_results(wpm_final, accuracy_final, mistakes, seconds, current_index);

    return 0;
}
