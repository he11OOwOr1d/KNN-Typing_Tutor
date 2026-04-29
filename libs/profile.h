#ifndef PROFILE_H
#define PROFILE_H

#define PROFILE_MAGIC 0x54595047  // "TYPG" — new format with history + difficulty

#define HISTORY_SIZE 5
#define WEAK_KEYS_SIZE 10

typedef struct {
    int wpm;
    int accuracy;
    int duration;       // 15, 30, or 60
    int difficulty;     // 0=easy 1=medium 2=hard 3=adaptive
    int keys_typed;
} SessionRecord;

typedef struct {
    int magic;                // format version check
    int sessions_completed;
    int total_keys_typed;
    int best_wpm;
    int avg_accuracy;
    int session_duration;     // seconds (15, 30, 60)
    int preferred_words;      // internal: how many words to generate
    int difficulty;           // 0=easy 1=medium 2=hard 3=adaptive

    // Per-duration bests
    int best_wpm_15;
    int best_wpm_30;
    int best_wpm_60;

    // Session history ring buffer
    SessionRecord history[HISTORY_SIZE];
    int history_count;        // total sessions ever (capped display at HISTORY_SIZE)

    // Weak keys from last session (for adaptive mode)
    char weak_keys[WEAK_KEYS_SIZE];
    int weak_key_count;
} UserProfile;

void load_profile(UserProfile* profile);
void save_profile(UserProfile* profile);
void update_profile(UserProfile* profile, int wpm, int accuracy, int keys);
void record_session_history(UserProfile* profile, int wpm, int accuracy, int duration, int difficulty, int keys);
void record_weak_keys(UserProfile* profile, char* keys, int count);

#endif
