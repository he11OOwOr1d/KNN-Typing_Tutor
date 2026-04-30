# ⌨️ KNN Typing Tutor

> A dual-platform, advanced typing analytics suite — a raw terminal application built in **Pure C**, and a modern **Next.js + Express** web platform.

---

## 📑 Table of Contents

- [Overview](#-overview)
- [Terminal App — Pure C](#-terminal-application-pure-c)
  - [Features](#-features)
  - [Architecture](#️-architecture)
  - [Quick Start](#-quick-start)
  - [Controls](#-controls)
- [Web Platform — Next.js + Express](#-web-platform-nextjs--express)
  - [Features](#-web-features)
  - [Architecture](#️-web-architecture)
  - [Quick Start](#-web-quick-start)
- [Platform Comparison](#-platform-comparison)
- [Adding Videos & Images to This README](#-adding-videos--images-to-this-readme)

---

## 🧭 Overview

The **KNN Typing Tutor** (Karthik · Nilesh · Nikhil) is designed as a virtual typing coach — not just a speed tracker. It identifies bottlenecks, plots fatigue curves, maps keyboard heatmaps, and lets you race your past self via a Ghost Racer.

| Platform | Stack | Standout |
|---|---|---|
| Terminal App | Pure C, POSIX termios, ANSI | Zero-dependency, microsecond precision |
| Web Platform | Next.js 15, Express, Recharts | Ghost Racer + Fatigue Curve Analysis |

---

## 🖥 Terminal Application (Pure C)

> Zero standard library dependencies for core logic — custom memory, strings, math, and rendering engines.

### ✨ Features

| Feature | Description |
|---|---|
| **⏱️ Timed Sessions** | Press `1` for 15s, `2` for 30s, or `3` for 60s. Live 3-2-1 countdown. |
| **🟢🟡🔴 Difficulty Tiers** | Cycle through EASY → MEDIUM → HARD → ADAPTIVE. |
| **🧠 Adaptive Mode** | Identifies your 3 weakest keys and generates words that target them (70% bias). |
| **🔥 Keyboard Heatmap** | Color-coded QWERTY map after each session: Green = Fast, Yellow = Normal, Red = Slow. |
| **🖐 Finger Speed Breakdown** | Average latency per finger using standard touch-typing zones. |
| **💡 Recommendations Engine** | Automated coaching on pacing, consistency, and bottleneck keys. |
| **💾 Persistent Profile** | Last 5 sessions, per-duration personal bests, and total stats saved to a binary `.bin` file. |
| **🏆 Rank System** | RECRUIT → ROOKIE → TYPIST → PRO → EXPERT → LEGEND, based on best WPM. |


### 🏗️ Architecture

```
main.c ───────────────────────────────────────────────────
  │   Menu loop, session logic, rendering orchestration
  │
  ├── libs/analytics.c   Character latency tracking, weakest/fastest key calc
  ├── libs/profile.c     Binary file persistence (UserProfile struct)
  ├── libs/screen.c      ANSI rendering engine + dynamic heatmap
  ├── libs/keyboard.c    Raw terminal input via termios
  ├── libs/memory.c      Custom 64KB bump allocator (my_alloc / reset_memory)
  ├── libs/string.c      Custom strlen, strcpy, strcat — no string.h
  ├── libs/math.c        Repeat-arithmetic division/multiplication + LCG PRNG
  └── libs/dictionary.h  Categorized pool of ~490 common English words
```

**Key design decisions:**

- **Custom bump allocator** (`libs/memory.c`) — 64KB arena, linearly allocated and reset between sessions, eliminating `malloc`/`free` fragmentation.
- **Raw `termios` input** — canonical mode disabled so keystrokes are read as single bytes the moment they arrive, giving millisecond-accurate timestamps via `gettimeofday`.
- **ANSI-only rendering** — no ncurses; `\033[H\033[J` resets the cursor to the viewport origin each frame, enabling 60+ FPS flicker-free in-place updates.
- **Net WPM formula** — `(correct_chars × 12) / elapsed_seconds`, avoiding floating-point entirely.

### 🚀 Quick Start

**Requirements:** macOS or Linux · GCC or Clang · Terminal with UTF-8 + 256-color support

```bash
# Build
make

# Run
./tutor

# Clean build artifacts
make clean
```

### 🎮 Controls

#### Main Menu

| Key | Action |
|---|---|
| `1` / `2` / `3` | Start session (15s / 30s / 60s) |
| `D` | Cycle difficulty tier |
| `R` | Reset all stats and history |
| `X` / `ESC` | Exit |

#### During Typing

| Key | Action |
|---|---|
| Any key | Type that character |
| `Backspace` | Correct last character |
| `ESC` | Abort session (partial data discarded) |

#### After Session

| Key | Action |
|---|---|
| `R` | Restart same session |
| Any key | Return to main menu |

---

## 🌐 Web Platform (Next.js + Express)

An advanced analytics suite modeled after tools like Monkeytype, with live ghost racing and statistical fatigue detection.

### 🌟 Web Features

| Feature | Description |
|---|---|
| **🏁 Ghost Racer** | Race your personal best frame-by-frame. The ghost replays exact keystroke timestamps at ~60fps. |
| **∞ Endless Mode** | Text loops infinitely with windowed virtualization to prevent UI lag; stops when the timer expires. |
| **📉 Fatigue Curve** | Least-Squares Linear Regression on per-second WPM — a falling red trendline signals cognitive slowdown. |
| **📊 Telemetry Dashboard** | Overlay your last 5 runs on a single timeline; color-coded error heatmap across the QWERTY layout. |
| **🌐 Decoupled Architecture** | Next.js App Router frontend + Express.js backend with file-based JSON persistence. |

**Fatigue Regression Math:**

$$m = \frac{\Sigma(x \cdot y) - n \cdot \bar{x} \cdot \bar{y}}{\Sigma(x^2) - n \cdot \bar{x}^2}, \quad b = \bar{y} - m \cdot \bar{x}$$

A negative slope `m` means your WPM is falling — the red trendline tells you before you feel it.

### 🏗️ Web Architecture

```
frontend/
  app/
    page.tsx              Home — sentence selector + test config
    race/page.tsx         Typing Arena — live Ghost Racer + Endless Mode
    analytics/page.tsx    Telemetry Dashboard — fatigue curve + heatmap
  components/
    TelemetryDashboard.tsx  Recharts overlays, regression rendering

backend/
  src/server.js           Express API — CORS, Ghost endpoints, JSON R/W
  data/
    attempts.json         Persisted session payloads (timestamps, errors)
    sentences.json        Sentence pool
```

### 🚀 Web Quick Start

**Step 1 — Start the Backend (Express)**

```bash
cd backend
npm install
npm start
# Runs on http://localhost:4000
```

**Step 2 — Start the Frontend (Next.js)**

```bash
cd frontend
npm install
npm run dev
# Runs on http://localhost:3000
```

> **Deployment:** Frontend → Vercel · Backend → Render.
> Set `NEXT_PUBLIC_API_URL` (frontend) and `FRONTEND_ORIGIN` (backend) environment variables to resolve CORS in production.

---

## 📊 Platform Comparison

| | Web Platform (Next.js) | Terminal App (Pure C) |
|:---|:---|:---|
| **Visuals** | High-fidelity Recharts graphs, Framer Motion animations | ANSI 256-color, block-character sparklines |
| **Performance** | Virtualized lists, React state | Near-zero latency, custom bump allocator |
| **Storage** | JSON files via Express REST API | Binary `.bin` struct serialization |
| **Standout Feature** | Live Ghost Racer (60fps keystroke replay) | Adaptive weak-key word generation |
| **Analytics** | Linear Regression Fatigue Curve | Finger-by-finger latency breakdown |
| **Platform** | Any browser | macOS / Linux only |

---







