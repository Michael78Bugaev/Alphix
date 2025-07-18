; boot.asm - Загрузчик Multiboot2 для Alphix
; Компилировать: nasm -f elf64 boot.asm -o boot.o

section .multiboot2
align 8
mb2_start:
    dd 0xe85250d6                ; магическое число Multiboot2
    dd 0                         ; архитектура (0 = i386)
    dd mb2_end - mb2_start       ; размер заголовка
    dd 0x100000000 - (0xe85250d6 + 0 + (mb2_end - mb2_start)) ; контрольная сумма

    ; Тег запроса видеорежима
    align 8
    dw 0                         ; тип = видеорежим
    dw 0                         ; флаги
    dd 20                        ; размер
    dd 800                       ; ширина
    dd 600                       ; высота
    dd 4                         ; глубина цвета (4 бита = 16 цветов)

    ; Конечный тег
    align 8
    dw 0                         ; тип = конец
    dw 0                         ; флаги
    dd 8                         ; размер
mb2_end:

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16KB стека
stack_top:
align 16
tss64:
    resb 104

section .text
global _start
extern kernel_main

bits 32
_start:
    ; Установка стека
    mov esp, stack_top

    ; Сохраняем оригинальные значения
    push ebx                     ; сохранить указатель Multiboot2
    push eax                     ; сохранить магическое число

    ; Проверка поддержки long mode
    call check_cpuid
    call check_long_mode

    ; Восстанавливаем оригинальные значения
    pop eax                     ; восстановить магическое число
    pop ebx                     ; восстановить указатель Multiboot2
    mov edi, eax                ; сохранить magic -> EDI
    mov esi, ebx                ; сохранить pointer -> ESI

    ; Настройка страничной адресации
    call setup_page_tables
    call enable_paging

    ; Загрузка временной минимальной GDT (null + kernel code)
    lea     eax, [tmp_gdt_ptr]
    lgdt    [eax]

    ; Переход в long mode
    jmp 0x08:long_mode_start     ; 0x08 = селектор kernel-code

    ; Зависание при ошибке
    hlt

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

setup_page_tables:
    ; PML4[0] -> page_table_l3
    mov eax, page_table_l3
    or  eax, 0b11               ; present + writable
    mov [page_table_l4], eax

    ; Создаём 4 PDP записи по 1 ГиБ для identity-map 0-4 ГиБ
    mov ecx, 0
.map_l3_table:
    mov eax, ecx
    shl eax, 30                 ; ecx * 1GiB (1<<30)
    or  eax, 0b10000011         ; present + writable + PS (1 ГиБ страница)
    mov [page_table_l3 + ecx*8], eax

    inc ecx
    cmp ecx, 4
    jne .map_l3_table

    ret

enable_paging:
    ; Включение PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Загрузка PML4 в CR3
    mov eax, page_table_l4
    mov cr3, eax

    ; Установка long mode бита в EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Включение страничной адресации
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

section .bss
align 4096
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096

section .rodata
gdt64:
    dq 0                            ; 0 – null
.code:      dq 0x00AF9A000000FFFF   ; 8 – kernel code
.user_data: dq 0x00AFF2000000FFFF   ; 16 – user data  (RPL=3)
.user_code: dq 0x00AFFA000000FFFF   ; 24 – user code  (RPL=3)

; ---------------- Минимальная GDT ----------------
align 8
tmp_gdt:
    dq 0                      ; null
    dq 0x00AF9A000000FFFF     ; kernel 64-bit code (DPL0)
tmp_gdt_end:

tmp_gdt_ptr:
    dw tmp_gdt_end - tmp_gdt - 1
    dq tmp_gdt

section .text
bits 64
long_mode_start:
    ; Очистка всех сегментных регистров
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Восстановление параметров Multiboot2 уже в rdi/rsi

    ; Вызов ядра
    call kernel_main

    ; Зависание
    cli
.hang:
    hlt
    jmp .hang 