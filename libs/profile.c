#include <stdio.h>
#include "profile.h"

static const char* PROFILE_FILE = "profile.bin";

static void reset_defaults(UserProfile* profile) {
    profile->magic = PROFILE_MAGIC;
    profile->sessions_completed = 0;
    profile->total_keys_typed = 0;
    profile->best_wpm = 0;
    profile->avg_accuracy = 0;
    profile->session_duration = 15;
    profile->preferred_words = 50;
    profile->difficulty = 0;

    profile->best_wpm_15 = 0;
    profile->best_wpm_30 = 0;
    profile->best_wpm_60 = 0;

    profile->history_count = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        profile->history[i].wpm = 0;
        profile->history[i].accuracy = 0;
        profile->history[i].duration = 0;
        profile->history[i].difficulty = 0;
        profile->history[i].keys_typed = 0;
    }

    profile->weak_key_count = 0;
    for (int i = 0; i < WEAK_KEYS_SIZE; i++) {
        profile->weak_keys[i] = 0;
    }
}

void load_profile(UserProfile* profile) {
    FILE* f = fopen(PROFILE_FILE, "rb");
    if (!f) {
        reset_defaults(profile);
        return;
    }
    int read = fread(profile, sizeof(UserProfile), 1, f);
    fclose(f);

    // If magic doesn't match, the file is from an old version — reset
    if (read != 1 || profile->magic != PROFILE_MAGIC) {
        reset_defaults(profile);
        save_profile(profile);
    }
}

void save_profile(UserProfile* profile) {
    profile->magic = PROFILE_MAGIC;
    FILE* f = fopen(PROFILE_FILE, "wb");
    if (!f) return;
    fwrite(profile, sizeof(UserProfile), 1, f);
    fclose(f);
}

void update_profile(UserProfile* profile, int wpm, int accuracy, int keys) {
    profile->sessions_completed++;
    profile->total_keys_typed += keys;
    if (wpm > profile->best_wpm) {
        profile->best_wpm = wpm;
    }
    if (profile->sessions_completed == 1) {
        profile->avg_accuracy = accuracy;
    } else {
        profile->avg_accuracy = (profile->avg_accuracy + accuracy) / 2;
    }

    // Track per-duration bests
    if (profile->session_duration == 15 && wpm > profile->best_wpm_15)
        profile->best_wpm_15 = wpm;
    else if (profile->session_duration == 30 && wpm > profile->best_wpm_30)
        profile->best_wpm_30 = wpm;
    else if (profile->session_duration == 60 && wpm > profile->best_wpm_60)
        profile->best_wpm_60 = wpm;
}

void record_session_history(UserProfile* profile, int wpm, int accuracy, int duration, int difficulty, int keys) {
    // Shift history down (newest at index 0)
    for (int i = HISTORY_SIZE - 1; i > 0; i--) {
        profile->history[i] = profile->history[i - 1];
    }
    profile->history[0].wpm = wpm;
    profile->history[0].accuracy = accuracy;
    profile->history[0].duration = duration;
    profile->history[0].difficulty = difficulty;
    profile->history[0].keys_typed = keys;
    if (profile->history_count < HISTORY_SIZE)
        profile->history_count++;
}

void record_weak_keys(UserProfile* profile, char* keys, int count) {
    if (count > WEAK_KEYS_SIZE) count = WEAK_KEYS_SIZE;
    profile->weak_key_count = count;
    for (int i = 0; i < count; i++) {
        profile->weak_keys[i] = keys[i];
    }
}
