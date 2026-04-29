#ifndef KEYBOARD_H
#define KEYBOARD_H

void enable_raw_mode();
void enable_raw_mode_nb();  // non-blocking for timed sessions
void disable_raw_mode();
char read_key();

#endif
