# ⌨️ Terminal Typing Tutor

A real-time, interactive terminal-based typing tutor built entirely in **C from scratch**. Every core component — memory management, string manipulation, math operations, screen rendering, and keyboard input — is implemented as a custom library with **zero reliance on standard C libraries** for core logic.

> **Project Track:** Track A — Interactive Terminal Application (Screen Type)  
> **Current Phase:** Phase 1 — Library Integration & Basic Mechanics

---

## 📖 Table of Contents

- [Project Description](#-project-description)
- [Features](#-features)
- [Architecture Overview](#-architecture-overview)
- [Custom Libraries](#-custom-libraries)
  - [memory.c — Virtual RAM Allocator](#1-memoryc--virtual-ram-allocator)
  - [string.c — String Operations](#2-stringc--string-operations)
  - [math.c — Arithmetic & PRNG Engine](#3-mathc--arithmetic--prng-engine)
  - [screen.c — Terminal Rendering](#4-screenc--terminal-rendering)
  - [keyboard.c — Raw Input Handler](#5-keyboardc--raw-input-handler)
- [Library Integration Pipeline](#-library-integration-pipeline)
- [User Controls](#-user-controls)
- [Build & Run Instructions](#-build--run-instructions)
- [Project Structure](#-project-structure)
- [Screenshots / Demo](#-screenshots--demo)
- [Known Issues](#-known-issues)

---

## 🎯 Project Description

This project is a capstone systems-programming exercise in which the entire software stack is hand-built. The application presents the user with a randomly generated sentence composed of 8 words drawn from a 32-word systems-programming dictionary. The user types the sentence in real time while the program provides:

- **Live color-coded feedback** — correctly typed characters turn green; mistakes turn red; upcoming characters appear as dim ghost text.
- **A highlighted cursor** — the current character is rendered with reverse-video to show the exact typing position.
- **A visual progress bar** — a Unicode block-character bar fills up as the user progresses through the sentence.
- **Real-time performance metrics** — Words Per Minute (WPM), accuracy percentage, mistake count, and elapsed time update on every keystroke.
- **A polished results screen** — upon completion (or ESC to quit early), a formatted summary card displays final statistics.

All of the above is powered exclusively by the five custom libraries described below.

---

## ✨ Features

| Feature | Description |
|---|---|
| **ASCII Art Title Screen** | A large, cyan, Unicode block-character banner greets the user on launch. |
| **Random Sentence Generation** | A Xorshift32 PRNG (seeded with microsecond system time) selects 8 random words using fast bitmask indexing (`& 31`). |
| **Live Typing Feedback** | Each character is color-coded in real time: ✅ green = correct, ❌ red = wrong, 🔲 dim = upcoming. |
| **Reverse-Video Cursor** | The next character to type is highlighted with `\033[7m` (ANSI reverse video). |
| **Progress Bar** | A `[████████░░░░] XX%` style progress bar rendered with Unicode block elements. |
| **Real-Time Stats** | WPM, Accuracy, Mistakes, and elapsed Time displayed live beneath the typing area. |
| **Backspace Support** | Users can correct mistakes with the Backspace key. |
| **Clean Session Results** | A bordered summary card with final WPM, Accuracy, Mistakes, Time, and Total Characters. |
| **Raw Terminal Mode** | Non-blocking, character-by-character input with no echo — enabling a fluid, game-like typing experience. |
| **Graceful Cleanup** | `atexit()` restores the terminal to its original state even on early exit. |

---

## 🏗 Architecture Overview

```
┌───────────────────────────────────────────────────────────┐
│                        main.c                             │
│  (Game loop, sentence generation, stats, rendering calls) │
└────────────┬──────────┬──────────┬──────────┬─────────────┘
             │          │          │          │
     ┌───────▼──┐ ┌─────▼────┐ ┌──▼───┐ ┌───▼──────┐
     │keyboard.c│ │ string.c │ │math.c│ │ screen.c │
     │  (input) │ │ (parse)  │ │(calc)│ │ (render) │
     └──────────┘ └──────────┘ └──┬───┘ └──────────┘
                                  │
                            ┌─────▼─────┐
                            │  memory.c  │
                            │  (alloc)   │
                            └───────────┘
```

The application follows a strict **library integration pipeline**:

1. `keyboard.c` captures raw keystrokes from the terminal.
2. `string.c` builds and measures the target sentence, and manages typed-character buffers.
3. `memory.c` allocates all dynamic buffers (target and typed strings) from a 10 KB virtual RAM pool.
4. `math.c` computes WPM, accuracy percentages, and generates random word indices via its PRNG.
5. `screen.c` renders the entire UI — title, typing box, progress bar, stats, and results — using ANSI escape codes.

---

## 📚 Custom Libraries

### 1. `memory.c` — Virtual RAM Allocator

| File | Lines | Description |
|---|---|---|
| `libs/memory.h` | 8 | Public API declarations |
| `libs/memory.c` | 19 | Implementation |

A bump-pointer allocator backed by a **10,000-byte static array** (`MEMORY[10000]`) that acts as a virtual RAM region. No `malloc()` or `free()` from `<stdlib.h>` is used for runtime allocations.

**Functions:**

| Function | Signature | Description |
|---|---|---|
| `my_alloc` | `void* my_alloc(int size)` | Returns a pointer to `size` bytes within the virtual RAM. Advances an internal `offset` cursor. Returns `0` (NULL) if the pool is exhausted. |
| `my_free` | `void my_free(void* ptr)` | Stub — the bump allocator does not support freeing individual blocks (all memory is reclaimed when the process exits). |

**Design Notes:**
- The bump allocator is the simplest possible allocator: O(1) allocation, zero fragmentation overhead, and perfectly suited for a short-lived session-based application like a typing tutor where total memory usage is bounded and predictable.

---

### 2. `string.c` — String Operations

| File | Lines | Description |
|---|---|---|
| `libs/string.h` | 9 | Public API declarations |
| `libs/string.c` | 28 | Implementation |

Custom replacements for `<string.h>` functions — none of `strlen`, `strcpy`, or `strcat` are used from the standard library.

**Functions:**

| Function | Signature | Description |
|---|---|---|
| `my_strlen` | `int my_strlen(char* str)` | Iterates until `'\0'` and returns the character count. Used throughout to measure sentence and typed-buffer lengths. |
| `my_strcpy` | `void my_strcpy(char* dest, const char* src)` | Copies `src` byte-by-byte into `dest`, including the null terminator. |
| `my_strcat` | `void my_strcat(char* dest, const char* src)` | Appends `src` to the end of `dest`. Used in the main loop to build the target sentence word-by-word from the dictionary. |

---

### 3. `math.c` — Arithmetic & PRNG Engine

| File | Lines | Description |
|---|---|---|
| `libs/math.h` | 10 | Public API declarations |
| `libs/math.c` | 35 | Implementation |

Custom arithmetic operations built without the `*` or `/` operators (using repeated addition/subtraction), plus a high-quality pseudo-random number generator.

**Functions:**

| Function | Signature | Description |
|---|---|---|
| `my_multiply` | `int my_multiply(int a, int b)` | Computes `a × b` via repeated addition (`a` added to itself `b` times). Used to calculate `correct × 100` for accuracy and `chars × 12` for WPM. |
| `my_divide` | `int my_divide(int a, int b)` | Computes integer `a ÷ b` via repeated subtraction. Returns `0` on divide-by-zero. Used for accuracy percentage and WPM computation. |
| `my_srand` | `void my_srand(unsigned int seed)` | Seeds the internal PRNG state. Rejects seed `0` (maps it to `1`) to avoid a degenerate all-zero state. Seeded with `gettimeofday()` microseconds in `main()`. |
| `my_rand` | `unsigned int my_rand()` | **Xorshift32** PRNG — a well-known, efficient, full-period generator that uses only three XOR-shift operations. Returns a 32-bit pseudo-random unsigned integer. |

**WPM Formula:**
```
WPM = (total_chars_typed × 12) / elapsed_seconds
```
The factor `12` approximates `60 / 5` (60 seconds per minute, 5 characters per "standard word"), yielding words-per-minute from character count and elapsed time.

---

### 4. `screen.c` — Terminal Rendering

| File | Lines | Description |
|---|---|---|
| `libs/screen.h` | 28 | Public API declarations |
| `libs/screen.c` | 91 | Implementation |

A full terminal rendering library powered by **ANSI escape codes** and **UTF-8 box-drawing characters**. Uses only `<stdio.h>` for `printf` and `fflush` (the allowed hardware-abstraction exception).

**Functions:**

| Function | Signature | Description |
|---|---|---|
| `clear_screen` | `void clear_screen()` | Sends `\033[2J\033[1;1H` — clears the entire terminal and resets the cursor to row 1, column 1. |
| `print_string` | `void print_string(char* str)` | Prints a null-terminated string and flushes stdout. |
| `print_char` | `void print_char(char c)` | Prints a single character and flushes stdout. |
| `print_int` | `void print_int(int n)` | Prints an integer as its decimal representation. |
| `set_color_green` | `void set_color_green()` | Sets foreground to 256-color palette index **82** (bright green). Used for correct characters and accuracy display. |
| `set_color_red` | `void set_color_red()` | Sets foreground to palette index **196** (bright red). Used for mistakes. |
| `set_color_dim` | `void set_color_dim()` | Sets foreground to palette index **240** (gray). Used for ghost text and borders. |
| `set_color_cyan` | `void set_color_cyan()` | Sets foreground to palette index **51** (cyan). Used for the title banner and WPM label. |
| `set_color_yellow` | `void set_color_yellow()` | Sets foreground to palette index **220** (gold). Used for timer and results header. |
| `set_color_bold` | `void set_color_bold()` | Enables bold text (`\033[1m`). Used for stat values. |
| `reset_color` | `void reset_color()` | Resets all ANSI attributes (`\033[0m`). |
| `cursor_left` | `void cursor_left()` | Moves cursor one column left (`\b`). |
| `move_cursor` | `void move_cursor(int row, int col)` | Positions the cursor at absolute `(row, col)` using `\033[row;colH`. Critical for re-rendering the typing area without clearing the screen. |
| `hide_cursor` | `void hide_cursor()` | Hides the blinking terminal cursor (`\033[?25l`). |
| `show_cursor` | `void show_cursor()` | Restores the terminal cursor (`\033[?25h`). |
| `draw_box_top` | `void draw_box_top(int width)` | Renders `╭──…──╮` using UTF-8 box-drawing characters. |
| `draw_box_bottom` | `void draw_box_bottom(int width)` | Renders `╰──…──╯`. |
| `draw_box_row` | `void draw_box_row(int width, char* content)` | Renders `│ content    │` with auto-padding. |
| `draw_progress_bar` | `void draw_progress_bar(int current, int total, int width)` | Renders `[████░░░░] XX%` with filled (`█`) and empty (`░`) Unicode blocks, colored green and gray respectively. |

---

### 5. `keyboard.c` — Raw Input Handler

| File | Lines | Description |
|---|---|---|
| `libs/keyboard.h` | 9 | Public API declarations |
| `libs/keyboard.c` | 31 | Implementation |

Provides raw, non-echoing, character-at-a-time terminal input by manipulating POSIX terminal attributes via `<termios.h>`. Uses `<unistd.h>` for the low-level `read()` syscall and `<stdio.h>` / `<stdlib.h>` as allowed by the hardware-abstraction exception.

**Functions:**

| Function | Signature | Description |
|---|---|---|
| `enable_raw_mode` | `void enable_raw_mode()` | Saves the current terminal settings, then disables `ECHO` (typed characters are not printed) and `ICANON` (input is delivered character-by-character, not line-by-line). |
| `disable_raw_mode` | `void disable_raw_mode()` | Restores the original terminal settings saved during `enable_raw_mode()`. Registered with `atexit()` in `main()` to guarantee cleanup. |
| `read_key` | `char read_key()` | Reads exactly 1 byte from `STDIN_FILENO`. In raw mode this returns immediately when a key is pressed, enabling a responsive game-loop style interaction. |

---

## 🔗 Library Integration Pipeline

The five libraries work together as a unified pipeline during each iteration of the game loop:

```
User presses key
       │
       ▼
 ┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
 │ keyboard │────▶│ string   │────▶│  math    │────▶│ screen   │
 │ read_key │     │ my_strlen│     │ multiply │     │ render   │
 │          │     │ my_strcat│     │ divide   │     │ colors   │
 └──────────┘     └──────────┘     │ my_rand  │     │ progress │
                                   └──────────┘     │ cursor   │
                       ▲                            └──────────┘
                       │
                 ┌──────────┐
                 │ memory   │
                 │ my_alloc │  (buffers allocated at session start)
                 └──────────┘
```

**Concrete data flow for one keypress:**

1. **`keyboard.c`** → `read_key()` returns the raw character `c`.
2. **`main.c`** stores `c` in the `typed[]` buffer (allocated via **`memory.c`** → `my_alloc(256)`).
3. **`string.c`** → `my_strlen()` measures the target sentence length; `my_strcat()` was used at startup to assemble the sentence from dictionary words.
4. **`math.c`** → `my_multiply()` and `my_divide()` compute accuracy (`correct × 100 / total`) and WPM (`chars × 12 / seconds`). At startup, `my_rand()` selected the random word indices.
5. **`screen.c`** → `move_cursor()`, color functions, `draw_progress_bar()` re-render the entire typing view.

---

## 🎮 User Controls

| Key | Action |
|---|---|
| **Any character key** | Types that character and advances the cursor |
| **Backspace** (`⌫`) | Deletes the last typed character (allows correction) |
| **ESC** | Quits the session early and shows results |
| *(Session ends automatically when the full sentence is typed)* | |

---

## 🛠 Build & Run Instructions

### Prerequisites

- **Operating System:** macOS or Linux (any POSIX-compliant system)
- **Compiler:** GCC (or any C99-compatible compiler, e.g., Clang)
- **Terminal:** Any terminal emulator with UTF-8 and 256-color support (iTerm2, Terminal.app, GNOME Terminal, etc.)

### Build

```bash
# Clone or extract the project
cd KNN-Typing_Tutor

# Compile all source files
make
```

This runs:
```bash
gcc -Wall -I. -o tutor main.c libs/memory.c libs/string.c libs/math.c libs/screen.c libs/keyboard.c
```

### Run

```bash
./tutor
```

### Clean Build Artifacts

```bash
make clean
```

---

## 📁 Project Structure

```
KNN-Typing_Tutor/
├── main.c              # Application entry point — game loop, sentence
│                       # generation, rendering orchestration, and results
├── Makefile            # Build configuration (gcc, -Wall, -I.)
├── README.md           # This file
└── libs/
    ├── memory.h        # Virtual RAM allocator — API declarations
    ├── memory.c        # Bump-pointer allocator over a 10 KB static array
    ├── string.h        # String operations — API declarations
    ├── string.c        # strlen, strcpy, strcat (custom implementations)
    ├── math.h          # Arithmetic & PRNG — API declarations
    ├── math.c          # multiply, divide (repeated add/sub), Xorshift32 PRNG
    ├── screen.h        # Terminal rendering — API declarations
    ├── screen.c        # ANSI escape codes, UTF-8 box drawing, progress bar
    ├── keyboard.h      # Raw input handler — API declarations
    └── keyboard.c      # termios raw mode, single-byte read
```

---

## 📸 Screenshots / Demo

> **Note:** Demo screenshots/recording to be included in submission.

**Expected screens during a session:**

1. **Title Screen** — A large cyan ASCII-art "TYPING" banner with the subtitle "TERMINAL TYPING TUTOR".
2. **Typing View** — A bordered sentence with color-coded character feedback, a reverse-video cursor, a progress bar, and live WPM / Accuracy / Mistakes / Time stats.
3. **Results Screen** — A gold-bordered "SESSION COMPLETE!" card showing final WPM, Accuracy, Mistakes, Time Taken, and Total Characters.

---

## ⚠️ Known Issues

| # | Issue | Severity | Notes |
|---|---|---|---|
| 1 | **`my_free()` is a no-op** | Low | The bump allocator does not support individual deallocation. Since the typing tutor runs as a single short-lived session with bounded memory usage (~512 bytes), this has no practical impact. Full deallocation support (free-list, coalescing) is planned for Phase 2. |
| 2 | **`my_multiply` handles only non-negative `b`** | Low | The repeated-addition loop runs `b` times; a negative `b` would result in zero iterations. All current call sites pass non-negative values (character counts × 100, character counts × 12). Signed support is planned for Phase 2. |
| 3 | **`my_divide` handles only non-negative inputs** | Low | The repeated-subtraction loop requires `a >= 0` and `b > 0`. All current call sites satisfy this. Signed support is planned for Phase 2. |
| 4 | **Dictionary is a fixed set of 32 words** | Low | The word pool is static and hard-coded. Expanding the dictionary or loading from a file (stored in virtual RAM) is planned for Phase 2. |
| 5 | **No window-resize handling** | Low | If the terminal is resized mid-session, the layout may become garbled. The user can restart `./tutor` to fix rendering. |
| 6 | **Relies on POSIX `<termios.h>` and `<unistd.h>`** | Info | These are required for raw terminal input and are not available on native Windows (works under WSL). This is consistent with the hardware-abstraction exception allowing `<stdio.h>` and `<stdlib.h>`. |

---

## 📋 Phase 1 Completion Checklist

| Requirement | Status |
|---|---|
| Five custom libraries implemented (`memory`, `string`, `math`, `screen`, `keyboard`) | ✅ |
| Libraries integrated into the main project | ✅ |
| Basic interactive loop running | ✅ |
| User can press a key to see on-screen response | ✅ |
| Static shape / text successfully rendered on screen | ✅ |
| No use of `<string.h>`, `<math.h>`, or default `malloc()`/`free()` for core logic | ✅ |
| Only `<stdio.h>` and `<stdlib.h>` used for hardware abstraction (I/O and process lifecycle) | ✅ |
| No hard-coded computation values — all logic computed dynamically via custom libraries | ✅ |
| No crashes (segmentation faults) during normal usage | ✅ |

---

## 📝 Phase 2 Roadmap (Planned)

- **Dynamic memory deallocation** — Implement a free-list or linked-list allocator in `memory.c` to support `my_free()`.
- **Runtime `alloc()` / `dealloc()` pairing** — Allocate per-session buffers and explicitly free them upon session completion (zero memory leaks).
- **Difficulty levels** — Variable sentence lengths, timed modes, and increasing word complexity.
- **Signed arithmetic support** — Extend `my_multiply` and `my_divide` to handle negative operands.
- **String tokenizer** — Add `my_strtok()` / `my_split()` to `string.c`.
- **Int-to-string conversion** — Add `my_itoa()` to `string.c` to eliminate remaining `printf("%d", ...)` calls.
- **Persistent high scores** — Store best WPM/accuracy in a file.

---

*Built from scratch with zero standard library dependencies for core logic.*
