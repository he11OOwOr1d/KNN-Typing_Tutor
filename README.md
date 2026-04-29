# ⌨️ KNN Typing Tutor

A real-time terminal typing tutor built from scratch in **Pure C** with KNN-powered analytics, persistent profiles, and a live keyboard heatmap.

> **Zero standard library dependencies** for core logic — custom memory, strings, math, and rendering.

---

## 🚀 Quick Start

```bash
make && ./tutor
```

---

## ✨ Features

| Feature | Description |
|---|---|
| **⏱️ Timed Sessions** | Press `1` for 15s, `2` for 30s, or `3` for 60s. Live 3-2-1 countdown on screen. |
| **🟢🟡🔴🧠 Difficulty Tiers** | Cycle difficulties: EASY, MEDIUM, HARD, and ADAPTIVE. |
| **🧠 Adaptive Mode** | Automatically targets and generates words using your 3 weakest keys from the last session. |
| **🔥 Keyboard Heatmap** | After each session, see a clean, color-coded map of your keyboard: Green=Fast, Yellow=Normal, Red=Slow. |
| **💡 Analytics & Feedback** | Finger speed breakdown and actionable feedback on pacing and bottlenecks. |
| **💾 Persistent Profile & History** | Your 5 most recent sessions, per-duration personal bests, and total stats are saved automatically. |
| **🏆 Rank System** | Progress from RECRUIT → ROOKIE → TYPIST → PRO → EXPERT → LEGEND based on your best WPM. |

---

## 🎮 Controls

### Main Menu
| Key | Action |
|---|---|
| **1 / 2 / 3** | Start session (15s / 30s / 60s) |
| **D** | Cycle difficulty tier |
| **R** | Reset all stats and history |
| **X / ESC** | Exit |

### During Typing
| Key | Action |
|---|---|
| **Any key** | Type that character |
| **Backspace** | Correct last character |
| **ESC** | Quit session early |

### After Session
| Key | Action |
|---|---|
| **R** | Restart same session |
| **Any Key** | Return to menu |

---

## 🏗 Architecture

```
main.c ──────────────────────────────────────────────────
  │   Menu loop, session logic, rendering orchestration
  │
  ├── libs/analytics.c   Character latency tracking
  ├── libs/profile.c     Binary file persistence
  ├── libs/screen.c      ANSI rendering engine + heatmap
  ├── libs/keyboard.c    Raw terminal input (termios)
  ├── libs/memory.c      64KB bump allocator
  ├── libs/string.c      Custom strlen/strcpy/strcat
  ├── libs/math.c        Repeat-arithmetic + PRNG
  └── libs/dictionary.h  Categorized 490 common English words
```

---

## 🛠 Build & Run

```bash
# Build
make

# Run
./tutor

# Clean
make clean
```

**Requirements:** macOS or Linux, GCC/Clang, terminal with UTF-8 + 256-color support.
