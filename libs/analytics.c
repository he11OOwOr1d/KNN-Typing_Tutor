#include "analytics.h"
#include "math.h"

void init_stats(UserStats* stats) {
    for (int i = 0; i < ANALYTICS_SIZE; i++) {
        stats->latencies[i] = 0;
        stats->counts[i] = 0;
        stats->mistakes[i] = 0;
    }
    for (int i = 0; i < WPM_HISTORY_SIZE; i++) {
        stats->wpm_history[i] = 0;
    }
    stats->wpm_history_len = 0;
    stats->peak_wpm = 0;
    stats->current_streak = 0;
    stats->best_streak = 0;
    stats->total_correct = 0;
}

void record_keypress(UserStats* stats, char c, long latency_ms, int is_mistake) {
    if ((unsigned char)c >= ANALYTICS_SIZE) return;
    stats->latencies[(unsigned char)c] += latency_ms;
    stats->counts[(unsigned char)c]++;
    if (is_mistake) {
        stats->mistakes[(unsigned char)c]++;
        stats->current_streak = 0;
    } else {
        stats->current_streak++;
        stats->total_correct++;
        if (stats->current_streak > stats->best_streak)
            stats->best_streak = stats->current_streak;
    }
}

void record_wpm_sample(UserStats* stats, int wpm) {
    if (stats->wpm_history_len < WPM_HISTORY_SIZE) {
        stats->wpm_history[stats->wpm_history_len] = wpm;
        stats->wpm_history_len++;
    }
    if (wpm > stats->peak_wpm) stats->peak_wpm = wpm;
}

int get_average_latency(UserStats* stats, char c) {
    if ((unsigned char)c >= ANALYTICS_SIZE) return 0;
    if (stats->counts[(unsigned char)c] == 0) return 0;
    return my_divide((int)stats->latencies[(unsigned char)c], stats->counts[(unsigned char)c]);
}

char get_weakest_char(UserStats* stats) {
    int max_latency = 0;
    char weakest = ' ';
    for (int i = 33; i < 126; i++) {
        if (stats->counts[i] == 0) continue;
        int avg = get_average_latency(stats, (char)i);
        if (avg > max_latency) { max_latency = avg; weakest = (char)i; }
    }
    return weakest;
}

char get_fastest_char(UserStats* stats) {
    int min_latency = 999999;
    char fastest = ' ';
    for (int i = 33; i < 126; i++) {
        if (stats->counts[i] == 0) continue;
        int avg = get_average_latency(stats, (char)i);
        if (avg < min_latency && avg > 0) { min_latency = avg; fastest = (char)i; }
    }
    return fastest;
}

int get_total_mistakes(UserStats* stats) {
    int total = 0;
    for (int i = 0; i < ANALYTICS_SIZE; i++) total += stats->mistakes[i];
    return total;
}

void get_slowest_keys(UserStats* stats, KeyStat* out, int n) {
    for (int i = 0; i < n; i++) { out[i].key = ' '; out[i].avg_ms = 0; }
    for (int c = 33; c < 126; c++) {
        if (stats->counts[c] == 0) continue;
        int avg = get_average_latency(stats, (char)c);
        for (int i = 0; i < n; i++) {
            if (avg > out[i].avg_ms) {
                for (int j = n - 1; j > i; j--) out[j] = out[j - 1];
                out[i].key = (char)c; out[i].avg_ms = avg;
                break;
            }
        }
    }
}

void get_fastest_keys(UserStats* stats, KeyStat* out, int n) {
    for (int i = 0; i < n; i++) { out[i].key = ' '; out[i].avg_ms = 999999; }
    for (int c = 33; c < 126; c++) {
        if (stats->counts[c] == 0) continue;
        int avg = get_average_latency(stats, (char)c);
        if (avg <= 0) continue;
        for (int i = 0; i < n; i++) {
            if (avg < out[i].avg_ms) {
                for (int j = n - 1; j > i; j--) out[j] = out[j - 1];
                out[i].key = (char)c; out[i].avg_ms = avg;
                break;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        if (out[i].avg_ms == 999999) { out[i].key = ' '; out[i].avg_ms = 0; }
    }
}

int get_consistency(UserStats* stats) {
    int min_lat = 999999, max_lat = 0, found = 0;
    for (int i = 33; i < 126; i++) {
        if (stats->counts[i] == 0) continue;
        int avg = get_average_latency(stats, (char)i);
        if (avg <= 0) continue;
        found++;
        if (avg < min_lat) min_lat = avg;
        if (avg > max_lat) max_lat = avg;
    }
    if (found < 2 || max_lat == 0) return 100;
    int range = max_lat - min_lat;
    return 100 - my_divide(my_multiply(range, 100), max_lat);
}
