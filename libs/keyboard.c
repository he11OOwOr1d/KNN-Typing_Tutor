#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"

static struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    
    struct termios raw = orig_termios;
    // Disable canonical mode (don't wait for Enter)
    // Disable echo (don't print keys we type)
    raw.c_lflag &= ~(ECHO | ICANON);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char read_key() {
    char c = '\0';
    // Read 1 byte from standard input.
    // In raw mode, this will return immediately when a key is pressed.
    read(STDIN_FILENO, &c, 1);
    return c;
}
