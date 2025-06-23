// Alphix kernel
// Keyboard buffer implementation for PS/2
// cpu/keyboard.c

#include <in/keyboard.h>

#define KB_BUF_SIZE 128
static char kb_buf[KB_BUF_SIZE];
static volatile uint32_t kb_head = 0;
static volatile uint32_t kb_tail = 0;

void keyboard_buffer_push(char c)
{
    uint32_t next = (kb_head + 1) % KB_BUF_SIZE;
    if (next != kb_tail) {
        kb_buf[kb_head] = c;
        kb_head = next;
    }
}

int keyboard_buffer_pop(char *c)
{
    if (kb_head == kb_tail) {
        return 0;
    }
    *c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return 1;
} 