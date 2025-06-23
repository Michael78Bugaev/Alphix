#include <cpu/cpu.h>
#include <in/keys.h>

void cpu_init(void) {
    // Ничего не делаем для инициализации CPU
}

cpu_info_t* cpu_get_info(void) {
    // Заглушка: информация о CPU не реализована
    return NULL;
}

void cpu_enable_interrupts(void) {
    // Отладка
    char msg[] = "STI\n";
    for(int i = 0; msg[i]; i++) {
        outb(0xE9, msg[i]);
    }
    __asm__ volatile ("sti");
}

void cpu_disable_interrupts(void) {
    __asm__ __volatile__("cli");
}

void cpu_halt(void) {
    __asm__ __volatile__("hlt");
}

uint64_t cpu_read_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void cpu_write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    __asm__ __volatile__("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

// Портовый ввод-вывод
uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ __volatile__("inb %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ __volatile__("inw %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ __volatile__("inl %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a" (value), "Nd" (port));
}

void outw(uint16_t port, uint16_t value) {
    __asm__ __volatile__("outw %0, %1" : : "a" (value), "Nd" (port));
}

void outl(uint16_t port, uint32_t value) {
    __asm__ __volatile__("outl %0, %1" : : "a" (value), "Nd" (port));
}

// Чтение CR3
uint64_t cpu_read_cr3(void) {
    uint64_t value;
    __asm__ __volatile__("mov %%cr3, %0" : "=r" (value));
    return value;
}

// Запись CR3
void cpu_write_cr3(uint64_t value) {
    __asm__ __volatile__("mov %0, %%cr3" : : "r" (value));
} 

char get_acsii_low(char code)
{
     switch (code)
  {
  case KEY_A:
    return 'a';
  case KEY_B:
    return 'b';
  case KEY_C:
    return 'c';
  case KEY_D:
    return 'd';
  case KEY_E:
    return 'e';
  case KEY_F:
    return 'f';
  case KEY_G:
    return 'g';
  case KEY_H:
    return 'h';
  case KEY_I:
    return 'i';
  case KEY_J:
    return 'j';
  case KEY_K:
    return 'k';
  case KEY_L:
    return 'l';
  case KEY_M:
    return 'm';
  case KEY_N:
    return 'n';
  case KEY_O:
    return 'o';
  case KEY_P:
    return 'p';
  case KEY_Q:
    return 'q';
  case KEY_R:
    return 'r';
  case KEY_S:
    return 's';
  case KEY_T:
    return 't';
  case KEY_U:
    return 'u';
  case KEY_V:
    return 'v';
  case KEY_W:
    return 'w';
  case KEY_X:
    return 'x';
  case KEY_Y:
    return 'y';
  case KEY_Z:
    return 'z';
  case KEY_1:
    return '1';
  case KEY_2:
    return '2';
  case KEY_3:
    return '3';
  case KEY_4:
    return '4';
  case KEY_5:
    return '5';
  case KEY_6:
    return '6';
  case KEY_7:
    return '7';
  case KEY_8:
    return '8';
  case KEY_9:
    return '9';
  case KEY_0:
    return '0';
  case KEY_MINUS:
    return '-';
  case KEY_EQUAL:
    return '=';
  case KEY_SQUARE_OPEN_BRACKET:
    return '[';
  case KEY_SQUARE_CLOSE_BRACKET:
    return ']';
  case KEY_SEMICOLON:
    return ';';
  case KEY_BACKSLASH:
    return '\\';
  case KEY_COMMA:
    return ',';
  case KEY_DOT:
    return '.';
  case KEY_FORESLHASH:
    return '/';
  case KEY_SPACE:
    return ' ';
  case KEY_APOSTROPHE:
    return '\'';
  case KEY_ENTER:
    return '\n';
  default:
    return 0;
  }
}
char get_acsii_high(char code)
{
     switch (code)
  {
  case KEY_A:
    return 'A';
  case KEY_B:
    return 'B';
  case KEY_C:
    return 'C';
  case KEY_D:
    return 'D';
  case KEY_E:
    return 'E';
  case KEY_F:
    return 'F';
  case KEY_G:
    return 'G';
  case KEY_H:
    return 'H';
  case KEY_I:
    return 'I';
  case KEY_J:
    return 'J';
  case KEY_K:
    return 'K';
  case KEY_L:
    return 'L';
  case KEY_M:
    return 'M';
  case KEY_N:
    return 'N';
  case KEY_O:
    return 'O';
  case KEY_P:
    return 'P';
  case KEY_Q:
    return 'Q';
  case KEY_R:
    return 'R';
  case KEY_S:
    return 'S';
  case KEY_T:
    return 'T';
  case KEY_U:
    return 'U';
  case KEY_V:
    return 'V';
  case KEY_W:
    return 'W';
  case KEY_X:
    return 'X';
  case KEY_Y:
    return 'Y';
  case KEY_Z:
    return 'Z';
  case KEY_1:
    return '!';
  case KEY_2:
    return '@';
  case KEY_3:
    return '#';
  case KEY_4:
    return '$';
  case KEY_5:
    return '%';
  case KEY_6:
    return '^';
  case KEY_7:
    return '&';
  case KEY_8:
    return '*';
  case KEY_9:
    return '(';
  case KEY_0:
    return ')';
  case KEY_MINUS:
    return '_';
  case KEY_EQUAL:
    return '+';
  case KEY_SQUARE_OPEN_BRACKET:
    return '{';
  case KEY_SQUARE_CLOSE_BRACKET:
    return '}';
  case KEY_SEMICOLON:
    return ':';
  case KEY_BACKSLASH:
    return '|';
  case KEY_COMMA:
    return '<';
  case KEY_DOT:
    return '>';
  case KEY_FORESLHASH:
    return '?';
  case KEY_SPACE:
    return ' ';
  case KEY_APOSTROPHE:
    return '"';
  default:
    return 0;
  }
}