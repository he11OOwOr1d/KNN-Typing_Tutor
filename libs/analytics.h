#ifndef ANALYTICS_H
#define ANALYTICS_H

#define ANALYTICS_SIZE 128
#define TOP_N 5
#define WPM_HISTORY_SIZE 60  // track WPM every second for up to 60s

typedef struct {
    long latencies[ANALYTICS_SIZE];
    int counts[ANALYTICS_SIZE];
    int mistakes[ANALYTICS_SIZE];

    // WPM over time (sampled every second)
    int wpm_history[WPM_HISTORY_SIZE];
    int wpm_history_len;

    // Session tracking
    int peak_wpm;
    int current_streak;
    int best_streak;
    int total_correct;
} UserStats;

typedef struct {
    char key;
    int avg_ms;
} KeyStat;

void init_stats(UserStats* stats);
void record_keypress(UserStats* stats, char c, long latency_ms, int is_mistake);
void record_wpm_sample(UserStats* stats, int wpm);
int get_average_latency(UserStats* stats, char c);
char get_weakest_char(UserStats* stats);
char get_fastest_char(UserStats* stats);
int get_total_mistakes(UserStats* stats);
void get_slowest_keys(UserStats* stats, KeyStat* out, int n);
void get_fastest_keys(UserStats* stats, KeyStat* out, int n);
int get_consistency(UserStats* stats);

#endif
