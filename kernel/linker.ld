/* linker.ld - Скрипт линкера для ядра Alphix */

ENTRY(_start)

SECTIONS
{
    /* Ядро загружается по адресу 1MB */
    . = 1M;
    _kernel_start = .;

    .boot ALIGN(4K) :
    {
        /* Multiboot2 заголовок должен быть в начале */
        *(.multiboot2)
    }

    .text ALIGN(4K) :
    {
        *(.text)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    .data ALIGN(4K) :
    {
        *(.data)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
    _kernel_end = .;

    /* Отбрасываем ненужные секции */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
    }
} 