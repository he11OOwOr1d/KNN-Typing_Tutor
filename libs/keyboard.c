#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"

static struct termios orig_termios;
static int has_orig = 0;

void disable_raw_mode() {
    if (has_orig) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
}

// Blocking raw mode — waits for a keypress. Used for menus.
void enable_raw_mode() {
    if (!has_orig) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        has_orig = 1;
    }
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;   // wait for 1 byte
    raw.c_cc[VTIME] = 0;  // no timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Non-blocking raw mode — returns after 100ms even with no input. Used for timed sessions.
void enable_raw_mode_nb() {
    if (!has_orig) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        has_orig = 1;
    }
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;   // don't require any bytes
    raw.c_cc[VTIME] = 1;  // 100ms timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char read_key() {
    char c = '\0';
    int n = read(STDIN_FILENO, &c, 1);
    if (n < 0) return 27;  // error → ESC
    return c;  // 0 if timeout (non-blocking), actual char if blocking
}
