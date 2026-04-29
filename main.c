#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "libs/memory.h"
#include "libs/string.h"
#include "libs/math.h"
#include "libs/screen.h"
#include "libs/keyboard.h"
#include "libs/analytics.h"
#include "libs/profile.h"
#include "libs/dictionary.h"

#define LINE_WIDTH 55
#define VISIBLE_LINES 3
#define MAX_LINES 200

// ─────────────────────────────────────────────────────────────
//  Line-break engine
// ─────────────────────────────────────────────────────────────

int line_starts[MAX_LINES];
int num_lines = 0;

void compute_lines(char* target, int target_len) {
    num_lines = 0;
    line_starts[0] = 0;
    int col = 0;
    int last_space = -1;
    for (int i = 0; i < target_len; i++) {
        col++;
        if (target[i] == ' ') last_space = i;
        if (col >= LINE_WIDTH && last_space > line_starts[num_lines]) {
            num_lines++;
            if (num_lines >= MAX_LINES - 1) break;
            line_starts[num_lines] = last_space + 1;
            col = i - last_space;
        }
    }
    num_lines++;
}

int get_line_end(int line, int target_len) {
    if (line + 1 < num_lines) return line_starts[line + 1];
    return target_len;
}

int get_cursor_line(int typed_len) {
    int line = 0;
    for (int i = 1; i < num_lines; i++) {
        if (typed_len >= line_starts[i]) line = i;
    }
    return line;
}

// ─────────────────────────────────────────────────────────────
//  Typing View
// ─────────────────────────────────────────────────────────────

static const char* diff_names[] = {"EASY", "MEDIUM", "HARD", "ADAPTIVE"};
static const char* diff_icons[] = {"🟢", "🟡", "🔴", "🧠"};

void draw_header() {
    clear_screen();
    hide_cursor();
    set_color_cyan(); set_color_bold();
    printf("  ⌨ TYPING TUTOR");
    reset_color();
    set_color_dim(); printf("  —  terminal edition\n\n"); reset_color();
}

void countdown() {
    for (int i = 3; i >= 1; i--) {
        clear_screen();
        printf("\n\n\n\n");
        set_color_cyan(); set_color_bold();
        printf("                    %d\n", i);
        reset_color();
        set_color_dim();
        printf("              get ready...\n");
        reset_color();
        fflush(stdout);
        // Simple busy-wait ~700ms
        struct timeval s, n;
        gettimeofday(&s, NULL);
        while (1) { gettimeofday(&n, NULL); if ((n.tv_sec - s.tv_sec) * 1000000 + (n.tv_usec - s.tv_usec) > 700000) break; }
    }
}

void render_session(char* target, char* typed, int typed_len, int target_len,
                    int correct, int mistakes, int wpm, int accuracy,
                    long elapsed, int time_limit, int streak, int peak_wpm) {
    int cursor_line = get_cursor_line(typed_len);
    int first_visible = cursor_line;

    move_cursor(3, 1);

    // Stats bar
    printf("  ");
    set_color_cyan(); set_color_bold(); printf("WPM %-3d", wpm); reset_color();
    printf("  ");
    set_color_green(); printf("ACC %-3d%%", accuracy); reset_color();
    printf("  ");
    set_color_red(); printf("ERR %-2d", mistakes); reset_color();
    printf("  ");
    set_color_yellow(); printf("🔥%-3d", streak); reset_color();
    printf("  ");
    set_color_dim(); printf("⚡%-3d", peak_wpm); reset_color();
    printf("  ");
    if (time_limit > 0) {
        long left = time_limit - elapsed;
        if (left < 0) left = 0;
        set_color_bold();
        if (left <= 5) set_color_red(); else set_color_yellow();
        printf("⏱ %lds", left);
    } else {
        set_color_dim(); printf("⏱ %lds", elapsed);
    }
    reset_color();
    printf("                    \n");

    // Render text lines
    for (int v = 0; v < VISIBLE_LINES; v++) {
        int line = first_visible + v;
        printf("  ");
        if (line >= num_lines) {
            for (int i = 0; i < LINE_WIDTH; i++) printf(" ");
            printf("\n");
            continue;
        }
        int ls = line_starts[line];
        int le = get_line_end(line, target_len);
        for (int i = ls; i < le; i++) {
            if (i < typed_len) {
                if (typed[i] == target[i]) set_color_green();
                else { set_color_red(); printf("\033[4m"); }
                printf("%c", target[i]);
                reset_color();
            } else if (i == typed_len) {
                printf("\033[7m%c\033[0m", target[i]);
            } else {
                set_color_dim(); printf("%c", target[i]); reset_color();
            }
        }
        int line_len = le - ls;
        for (int i = line_len; i < LINE_WIDTH + 5; i++) printf(" ");
        printf("\n");
    }

    printf("\n  ");
    draw_progress_bar(typed_len, target_len, 45);
    printf("  %d/%d\n", typed_len, target_len);
    set_color_dim(); printf("  ESC quit · Backspace correct"); reset_color();
    fflush(stdout);
}

// ─────────────────────────────────────────────────────────────
//  WPM Sparkline Graph
// ─────────────────────────────────────────────────────────────

void draw_wpm_graph(UserStats* stats) {
    if (stats->wpm_history_len < 2) return;

    set_color_cyan(); set_color_bold();
    printf("  ┌─────────────────────────────────────────┐\n");
    printf("  │           WPM OVER TIME                  │\n");
    printf("  └─────────────────────────────────────────┘\n");
    reset_color();
    printf("\n");

    // Find max WPM for scaling
    int max_wpm = 1;
    for (int i = 0; i < stats->wpm_history_len; i++) {
        if (stats->wpm_history[i] > max_wpm) max_wpm = stats->wpm_history[i];
    }

    int graph_height = 5;
    int graph_width = stats->wpm_history_len;
    if (graph_width > 50) graph_width = 50;

    // Scale factor: sample wpm_history if too many points
    int step = 1;
    if (stats->wpm_history_len > 50) {
        step = my_divide(stats->wpm_history_len, 50);
        if (step < 1) step = 1;
    }

    for (int row = graph_height; row >= 1; row--) {
        int threshold = my_divide(my_multiply(max_wpm, row), graph_height);
        printf("  ");
        set_color_dim();
        if (row == graph_height) printf("%3d│", max_wpm);
        else if (row == 1)       printf("  0│");
        else                     printf("   │");
        reset_color();

        int col = 0;
        for (int i = 0; i < stats->wpm_history_len && col < 50; i += step) {
            int val = stats->wpm_history[i];
            if (val > 0 && val >= threshold) {
                set_color_cyan();
                printf("█");
                reset_color();
            } else {
                printf(" ");
            }
            col++;
        }
        printf("\n");
    }

    // Time axis
    printf("     └");
    int cols_drawn = 0;
    for (int i = 0; i < stats->wpm_history_len && cols_drawn < 50; i += step) {
        printf("─"); cols_drawn++;
    }
    printf("\n");
    printf("      ");
    set_color_dim();
    printf("0s");
    int total_secs = stats->wpm_history_len;
    // pad to right
    for (int i = 2; i < cols_drawn - 3; i++) printf(" ");
    if (cols_drawn > 5) printf("%ds", total_secs);
    reset_color();
    printf("\n\n");
}

// ─────────────────────────────────────────────────────────────
//  Typing Session
// ─────────────────────────────────────────────────────────────

void run_session(UserProfile* profile) {
    reset_memory();

    struct timeval tv;
    gettimeofday(&tv, NULL);
    my_srand(tv.tv_usec);

    char* target = (char*) my_alloc(8192);
    char* typed  = (char*) my_alloc(8192);
    target[0] = '\0';

    // Generate plenty of words — enough for even 120 WPM typists
    int word_count = profile->session_duration * 3;
    if (word_count < 50) word_count = 50;
    if (word_count > 500) word_count = 500; // Cap to prevent buffer overflow

    // Pick word pool based on difficulty
    const char** pool = words;
    int pool_size = WORD_COUNT;
    if (profile->difficulty == 0)      { pool = words_easy;   pool_size = EASY_COUNT; }
    else if (profile->difficulty == 1) { pool = words_medium;  pool_size = MEDIUM_COUNT; }
    else if (profile->difficulty == 2) { pool = words_hard;    pool_size = HARD_COUNT; }
    else if (profile->difficulty == 3 && profile->weak_key_count > 0) {
        // Adaptive: prefer words containing weak keys
        // Build a filtered sub-pool on the stack (indices)
        int adaptive_indices[WORD_COUNT];
        int adaptive_count = 0;
        char wk = profile->weak_keys[my_rand() % profile->weak_key_count];
        for (int w = 0; w < WORD_COUNT; w++) {
            if (word_contains_char(words[w], wk)) {
                adaptive_indices[adaptive_count++] = w;
            }
        }
        // Generate with 70% adaptive, 30% random
        for (int i = 0; i < word_count; i++) {
            unsigned int idx;
            if (adaptive_count > 0 && (my_rand() % 10) < 7)
                idx = adaptive_indices[my_rand() % adaptive_count];
            else
                idx = my_rand() % WORD_COUNT;
            my_strcat(target, words[idx]);
            if (i < word_count - 1) my_strcat(target, " ");
        }
        pool = NULL; // signal we already built target
    }

    if (pool != NULL) {
        for (int i = 0; i < word_count; i++) {
            unsigned int idx = my_rand() % pool_size;
            my_strcat(target, pool[idx]);
            if (i < word_count - 1) my_strcat(target, " ");
        }
    }

    int target_len = my_strlen(target);
    compute_lines(target, target_len);

    // 3-2-1 countdown
    countdown();

    draw_header();
    set_color_dim();
    printf("  %s %s · ⏱  %d sec — start typing!\n\n",
           diff_icons[profile->difficulty], diff_names[profile->difficulty], profile->session_duration);
    reset_color();
    render_session(target, typed, 0, target_len, 0, 0, 0, 100, 0, profile->session_duration, 0, 0);

    enable_raw_mode();

    struct timeval start, now;
    int started = 0;
    int aborted = 0;
    int current_index = 0;
    int correct = 0;
    int mistakes = 0;
    UserStats stats;
    init_stats(&stats);
    struct timeval last_key_time;
    int timed = (profile->session_duration > 0);
    long last_wpm_sample = -1;

    while (current_index < target_len) {
        char c;

        if (started && timed) {
            disable_raw_mode();
            enable_raw_mode_nb();
            c = read_key();

            gettimeofday(&now, NULL);
            long elapsed = now.tv_sec - start.tv_sec;

            // Sample WPM every second
            if (elapsed > last_wpm_sample) {
                int gross = (elapsed > 0) ? my_divide(my_multiply(current_index, 12), (int)elapsed) : 0;
                int penalty = (elapsed > 0) ? my_divide(my_multiply(mistakes, 60), (int)elapsed) : 0;
                int wpm_now = gross - penalty;
                if (wpm_now < 0) wpm_now = 0;
                record_wpm_sample(&stats, wpm_now);
                last_wpm_sample = elapsed;
            }

            if (elapsed >= profile->session_duration) break;

            static long last_drawn_elapsed = -1;
            if (c == 0) {
                if (elapsed != last_drawn_elapsed) {
                    correct = 0; mistakes = 0;
                    for (int i = 0; i < current_index; i++) {
                        if (typed[i] == target[i]) correct++; else mistakes++;
                    }
                    int acc = (current_index > 0) ? my_divide(my_multiply(correct, 100), current_index) : 100;
                    int gross = (elapsed > 0) ? my_divide(my_multiply(current_index, 12), (int)elapsed) : 0;
                    int penalty = (elapsed > 0) ? my_divide(my_multiply(mistakes, 60), (int)elapsed) : 0;
                    int wpm = gross - penalty;
                    if (wpm < 0) wpm = 0;
                    draw_header();
                    render_session(target, typed, current_index, target_len,
                                   correct, mistakes, wpm, acc, elapsed, profile->session_duration,
                                   stats.current_streak, stats.peak_wpm);
                    last_drawn_elapsed = elapsed;
                }
                continue;
            }
            last_drawn_elapsed = elapsed;
        } else {
            c = read_key();
        }

        if (!started) {
            gettimeofday(&start, NULL);
            last_key_time = start;
            started = 1;
        }

        if (c == 27) {
            aborted = 1;
            break;
        }

        if (c == 127 || c == '\b') {
            if (current_index > 0) { current_index--; typed[current_index] = '\0'; }
        } else {
            struct timeval key_time;
            gettimeofday(&key_time, NULL);
            long latency = (key_time.tv_sec - last_key_time.tv_sec) * 1000 +
                           (key_time.tv_usec - last_key_time.tv_usec) / 1000;
            int is_mistake = (c != target[current_index]);
            record_keypress(&stats, c, latency, is_mistake);
            last_key_time = key_time;
            typed[current_index] = c;
            current_index++;
            typed[current_index] = '\0';
        }

        correct = 0; mistakes = 0;
        for (int i = 0; i < current_index; i++) {
            if (typed[i] == target[i]) correct++; else mistakes++;
        }

        int accuracy_now = (current_index > 0) ? my_divide(my_multiply(correct, 100), current_index) : 100;
        gettimeofday(&now, NULL);
        long elapsed = now.tv_sec - start.tv_sec;
        int gross_now = (elapsed > 0) ? my_divide(my_multiply(current_index, 12), (int)elapsed) : 0;
        int penalty_now = (elapsed > 0) ? my_divide(my_multiply(mistakes, 60), (int)elapsed) : 0;
        int wpm_now = gross_now - penalty_now;
        if (wpm_now < 0) wpm_now = 0;

        // Sample WPM for graph (word-count mode too)
        if (elapsed > last_wpm_sample) {
            record_wpm_sample(&stats, wpm_now);
            last_wpm_sample = elapsed;
        }

        draw_header();
        render_session(target, typed, current_index, target_len,
                       correct, mistakes, wpm_now, accuracy_now, elapsed, profile->session_duration,
                       stats.current_streak, stats.peak_wpm);
    }

    disable_raw_mode();
    show_cursor();

    if (aborted) {
        clear_screen_and_save();
        return;
    }

    long seconds = 0;
    if (started) { gettimeofday(&now, NULL); seconds = now.tv_sec - start.tv_sec; }
    if (seconds <= 0) seconds = 1;

    int accuracy_final = (current_index > 0) ? my_divide(my_multiply(correct, 100), current_index) : 0;
    int gross_final = my_divide(my_multiply(current_index, 12), (int)seconds);
    int penalty_final = my_divide(my_multiply(mistakes, 60), (int)seconds);
    int wpm_final = gross_final - penalty_final;
    if (wpm_final < 0) wpm_final = 0;

    // ═══════════════════════════════════════════════════════
    //  ANALYTICS DASHBOARD
    // ═══════════════════════════════════════════════════════
    clear_screen();

    printf("\n");
    set_color_cyan(); set_color_bold();
    printf("  ╔══════════════════════════════════════════════════════╗\n");
    printf("  ║              SESSION ANALYTICS                      ║\n");
    printf("  ╚══════════════════════════════════════════════════════╝\n");
    reset_color();
    printf("\n");

    // ── Big Stats ──
    printf("  ");
    set_color_cyan(); set_color_bold(); printf("⌨ WPM    "); reset_color();
    set_color_bold(); printf("%-4d ", wpm_final); reset_color();
    set_color_cyan();
    int wpm_bar = wpm_final; if (wpm_bar > 50) wpm_bar = 50;
    for (int i = 0; i < wpm_bar; i++) printf("█");
    reset_color(); set_color_dim();
    for (int i = wpm_bar; i < 50; i++) printf("░");
    reset_color(); printf("\n");

    printf("  ");
    set_color_green(); set_color_bold(); printf("✓ ACC    "); reset_color();
    set_color_bold(); printf("%-3d%% ", accuracy_final); reset_color();
    int acc_bar = my_divide(accuracy_final, 2);
    if (accuracy_final >= 95) set_color_green();
    else if (accuracy_final >= 80) set_color_yellow();
    else set_color_red();
    for (int i = 0; i < acc_bar; i++) printf("█");
    reset_color(); set_color_dim();
    for (int i = acc_bar; i < 50; i++) printf("░");
    reset_color(); printf("\n");

    int consistency = get_consistency(&stats);
    printf("  ");
    set_color_yellow(); set_color_bold(); printf("◎ CONS   "); reset_color();
    set_color_bold(); printf("%-3d%% ", consistency); reset_color();
    int con_bar = my_divide(consistency, 2);
    if (consistency >= 80) set_color_green();
    else if (consistency >= 50) set_color_yellow();
    else set_color_red();
    for (int i = 0; i < con_bar; i++) printf("█");
    reset_color(); set_color_dim();
    for (int i = con_bar; i < 50; i++) printf("░");
    reset_color(); printf("\n\n");

    // ── Quick stats row ──
    set_color_dim();
    printf("  ⏱ %ld sec  ·  %d chars  ·  %d errors  ·  ⚡ peak %d wpm  ·  🔥 best streak %d\n",
           seconds, current_index, mistakes, stats.peak_wpm, stats.best_streak);
    reset_color();

    // Speed grade
    printf("  ");
    if (accuracy_final < 60) { set_color_red(); set_color_bold(); printf("GRADE: F  INVALID (Too many errors)"); }
    else if (wpm_final >= 80 && accuracy_final >= 95) { set_color_green(); set_color_bold(); printf("GRADE: S+ LEGENDARY"); }
    else if (wpm_final >= 60 && accuracy_final >= 90) { set_color_green(); set_color_bold(); printf("GRADE: A  EXPERT"); }
    else if (wpm_final >= 40 && accuracy_final >= 85) { set_color_cyan(); set_color_bold(); printf("GRADE: B  PROFICIENT"); }
    else if (wpm_final >= 25 && accuracy_final >= 80) { set_color_yellow(); set_color_bold(); printf("GRADE: C  DEVELOPING"); }
    else { set_color_red(); set_color_bold(); printf("GRADE: D  BEGINNER"); }
    reset_color();
    printf("\n");

    // ── WPM Graph ──
    draw_wpm_graph(&stats);

    // ── Keyboard Heatmap ──
    draw_keyboard_heatmap(&stats);

    // ── Slowest Keys ──
    KeyStat slowest[5];
    get_slowest_keys(&stats, slowest, 5);
    set_color_red(); set_color_bold(); printf("  ⚠ SLOWEST KEYS\n"); reset_color();
    for (int i = 0; i < 5; i++) {
        if (slowest[i].key == ' ' || slowest[i].avg_ms == 0) break;
        printf("    ");
        set_color_red(); printf("'%c' ", slowest[i].key); reset_color();
        set_color_bold(); printf("%3d ms ", slowest[i].avg_ms); reset_color();
        set_color_red();
        int bar = my_divide(slowest[i].avg_ms, 20); if (bar > 25) bar = 25;
        for (int j = 0; j < bar; j++) printf("█");
        reset_color();
        // Show mistake count if any
        int mc = stats.mistakes[(unsigned char)slowest[i].key];
        if (mc > 0) { set_color_dim(); printf(" (%d miss)", mc); reset_color(); }
        printf("\n");
    }
    printf("\n");

    // ── Fastest Keys ──
    KeyStat fastest[5];
    get_fastest_keys(&stats, fastest, 5);
    set_color_green(); set_color_bold(); printf("  ⚡ FASTEST KEYS\n"); reset_color();
    for (int i = 0; i < 5; i++) {
        if (fastest[i].key == ' ' || fastest[i].avg_ms == 0) break;
        printf("    ");
        set_color_green(); printf("'%c' ", fastest[i].key); reset_color();
        set_color_bold(); printf("%3d ms ", fastest[i].avg_ms); reset_color();
        set_color_green();
        int bar = my_divide(fastest[i].avg_ms, 20); if (bar > 25) bar = 25;
        for (int j = 0; j < bar; j++) printf("█");
        reset_color(); printf("\n");
    }
    printf("\n");

    // ── Finger Analysis ──
    char* fkeys[] = {"qaz","wsx","edc","rfvtgb","yhnujm","ik","ol","p"};
    char* fnames[] = {"L.Pinky","L.Ring ","L.Mid  ","L.Index","R.Index","R.Mid  ","R.Ring ","R.Pinky"};

    set_color_cyan(); set_color_bold();
    printf("  ┌─────────────────────────────────────────┐\n");
    printf("  │         FINGER SPEED ANALYSIS            │\n");
    printf("  └─────────────────────────────────────────┘\n");
    reset_color(); printf("\n");

    for (int f = 0; f < 8; f++) {
        int total_lat = 0, total_count = 0;
        for (int k = 0; fkeys[f][k] != '\0'; k++) {
            char ch = fkeys[f][k];
            if (stats.counts[(unsigned char)ch] > 0) {
                total_lat += (int)stats.latencies[(unsigned char)ch];
                total_count += stats.counts[(unsigned char)ch];
            }
        }
        int avg = (total_count > 0) ? my_divide(total_lat, total_count) : 0;
        printf("  "); set_color_dim(); printf("%-7s ", fnames[f]); reset_color();
        if (avg == 0) { set_color_dim(); printf("  ·\n"); reset_color(); }
        else {
            set_latency_color(avg);
            printf("%3d ms  ", avg);
            int bar = my_divide(avg, 15); if (bar > 25) bar = 25;
            for (int j = 0; j < bar; j++) printf("█");
            reset_color(); printf("\n");
        }
    }
    printf("\n");

    // ── Recommendations ──
    set_color_yellow(); set_color_bold(); printf("  💡 RECOMMENDATIONS\n"); reset_color();
    char weak = get_weakest_char(&stats);
    char fast = get_fastest_char(&stats);

    if (weak != ' ' && current_index > 5) {
        if (weak == fast) {
            printf("  → Holding down '%c' won't help you learn to type! Try the actual text!\n", weak);
        } else {
            printf("  → '%c' is your bottleneck (%d ms avg)\n", weak, get_average_latency(&stats, weak));
        }
    }
    if (consistency < 60) printf("  → Inconsistent rhythm — try to keep even tempo\n");
    if (accuracy_final < 90) printf("  → Accuracy first! Slow down to build muscle memory\n");
    else if (wpm_final >= 40 && accuracy_final >= 95) printf("  → Outstanding! You're typing at professional level\n");

    if (fast != ' ' && fast != weak && current_index > 5)
        printf("  → '%c' is your strongest key (%d ms) — nice!\n", fast, get_average_latency(&stats, fast));
    if (stats.best_streak >= 20) printf("  → Incredible %d-char streak! Consistency is building\n", stats.best_streak);
    printf("\n");

    if (current_index > 0) {
        update_profile(profile, wpm_final, accuracy_final, current_index);
        record_session_history(profile, wpm_final, accuracy_final,
                               profile->session_duration, profile->difficulty, current_index);
        // Save weak keys for adaptive mode
        KeyStat slow3[3];
        get_slowest_keys(&stats, slow3, 3);
        char wkeys[3]; int wkc = 0;
        for (int i = 0; i < 3; i++) {
            if (slow3[i].key != ' ' && slow3[i].avg_ms > 0)
                wkeys[wkc++] = slow3[i].key;
        }
        record_weak_keys(profile, wkeys, wkc);
        save_profile(profile);
    }

    set_color_dim(); printf("  ────────────────────────────────────────────\n"); reset_color();
    printf("  [R] Restart  [Any] Menu\n\n");
    fflush(stdout);
}

// ─────────────────────────────────────────────────────────────
//  Main Menu
// ─────────────────────────────────────────────────────────────

int main() {
    atexit(disable_raw_mode);
    UserProfile profile;
    load_profile(&profile);

    while (1) {
        clear_screen_and_save(); show_cursor();
        printf("\n");
        set_color_cyan(); set_color_bold(); printf("  ⌨ TYPING TUTOR\n"); reset_color();
        printf("\n");

        const char* rank = "RECRUIT";
        if (profile.best_wpm >= 80)      rank = "LEGEND";
        else if (profile.best_wpm >= 60) rank = "EXPERT";
        else if (profile.best_wpm >= 40) rank = "PRO";
        else if (profile.best_wpm >= 20) rank = "TYPIST";
        else if (profile.sessions_completed > 0) rank = "ROOKIE";

        set_color_dim();
        printf("  %s · %d sessions · best %d wpm · %d keys\n", rank, profile.sessions_completed, profile.best_wpm, profile.total_keys_typed);
        reset_color();

        // Per-duration personal bests
        if (profile.best_wpm_15 > 0 || profile.best_wpm_30 > 0 || profile.best_wpm_60 > 0) {
            set_color_dim(); printf("  PB:");
            if (profile.best_wpm_15 > 0) printf("  15s=%d", profile.best_wpm_15);
            if (profile.best_wpm_30 > 0) printf("  30s=%d", profile.best_wpm_30);
            if (profile.best_wpm_60 > 0) printf("  60s=%d", profile.best_wpm_60);
            printf(" wpm\n"); reset_color();
        }
        printf("\n");

        // Session history
        if (profile.history_count > 0) {
            set_color_cyan(); set_color_bold();
            printf("  ┌─ RECENT ─────────────────────────────────┐\n");
            reset_color();
            for (int i = 0; i < profile.history_count && i < HISTORY_SIZE; i++) {
                SessionRecord* r = &profile.history[i];
                set_color_dim(); printf("  │ "); reset_color();
                printf("%s ", diff_icons[r->difficulty]);
                set_color_bold(); printf("%3d wpm", r->wpm); reset_color();
                printf("  %3d%%", r->accuracy);
                set_color_dim(); printf("  %ds  %d keys", r->duration, r->keys_typed);
                reset_color();
                // Mini sparkline bar
                printf("  ");
                int bar = r->wpm; if (bar > 30) bar = 30;
                if (r->wpm >= 60) set_color_green();
                else if (r->wpm >= 30) set_color_yellow();
                else set_color_red();
                for (int b = 0; b < bar; b++) printf("▪");
                reset_color();
                printf("\n");
            }
            set_color_cyan(); set_color_bold();
            printf("  └─────────────────────────────────────────┘\n");
            reset_color();
            printf("\n");
        }

        printf("  Pick a duration:\n\n");
        set_color_yellow(); set_color_bold(); printf("  ⏱  TIMED\n"); reset_color();
        printf("  [1] 15 sec    [2] 30 sec    [3] 60 sec\n\n");

        // Difficulty selector
        printf("  Difficulty: ");
        set_color_bold();
        printf("%s %s", diff_icons[profile.difficulty], diff_names[profile.difficulty]);
        reset_color();
        set_color_dim(); printf("  [D] cycle\n"); reset_color();

        // Weak keys hint for adaptive
        if (profile.difficulty == 3 && profile.weak_key_count > 0) {
            set_color_dim(); printf("  Targeting: ");
            set_color_red();
            for (int i = 0; i < profile.weak_key_count; i++) printf("'%c' ", profile.weak_keys[i]);
            reset_color(); printf("\n");
        }
        printf("\n");

        set_color_dim(); printf("  [R] Reset Stats   [X] Exit\n"); reset_color();
        printf("\n");
        set_color_cyan(); printf("  > "); reset_color();
        fflush(stdout);

        char c;
        while (1) {
            enable_raw_mode();
            c = read_key();
            disable_raw_mode();

            if (c == '1' || c == '2' || c == '3' || 
                c == 'd' || c == 'D' || c == 'r' || c == 'R' || 
                c == 'x' || c == 'X' || c == 27) {
                break;
            }
        }

        int go = 0;
        if (c == '1') { profile.session_duration = 15; go = 1; }
        else if (c == '2') { profile.session_duration = 30; go = 1; }
        else if (c == '3') { profile.session_duration = 60; go = 1; }
        else if (c == 'd' || c == 'D') {
            profile.difficulty = (profile.difficulty + 1) % 4;
            save_profile(&profile);
        }
        else if (c == 'r' || c == 'R') {
            profile.sessions_completed = 0; profile.total_keys_typed = 0;
            profile.best_wpm = 0; profile.avg_accuracy = 0;
            profile.best_wpm_15 = 0; profile.best_wpm_30 = 0; profile.best_wpm_60 = 0;
            profile.history_count = 0; profile.weak_key_count = 0;
            save_profile(&profile);
        } else if (c == 'x' || c == 'X' || c == 27) { break; }

        if (go) {
            save_profile(&profile);
            while (1) {
                run_session(&profile);

                // Flush any accidental extra keystrokes from the buffer
                enable_raw_mode_nb();
                while (read_key() != 0);
                disable_raw_mode();

                // Wait for user to acknowledge analytics
                enable_raw_mode();
                char next = read_key();
                disable_raw_mode();
                if (next != 'r' && next != 'R') break;
            }
        }
    }

    clear_screen(); show_cursor();
    printf("\n  See you next time! 🚀\n\n");
    return 0;
}
