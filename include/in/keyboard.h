// Alphix kernel
// Keyboard buffer interface
// include/cpu/keyboard.h

#ifndef _ALPHIX_KEYBOARD_H
#define _ALPHIX_KEYBOARD_H

#include <stdint.h>

// Push a received character into keyboard buffer
void keyboard_buffer_push(char c);

// Pop a character from keyboard buffer; returns 1 if char available, 0 otherwise
int keyboard_buffer_pop(char *c);

#endif // _ALPHIX_KEYBOARD_H 