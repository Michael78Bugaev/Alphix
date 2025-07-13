# Makefile для Alphix OS

# Компиляторы и инструменты
CC = x86_64-elf-gcc
AS = nasm
LD = x86_64-elf-ld
GRUB_MKRESCUE = grub-mkrescue

# Флаги компиляции
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
         -fno-stack-protector -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
         -mcmodel=kernel -fno-pic -no-pie -I include -nostdlib

ASFLAGS = -f elf64
LDFLAGS = -n -T kernel/linker.ld --allow-multiple-definition

# Директории
KERNEL_DIR = kernel
ISO_DIR = iso
BUILD_DIR = build
BOOT_DIR = $(ISO_DIR)/boot

# Автоматический поиск файлов
# Находим все .c файлы во всех поддиректориях
C_SOURCES = $(shell find . -path ./$(BUILD_DIR) -prune -o -path ./$(ISO_DIR) -prune -o -name "*.c" -print)
# Находим все .asm файлы во всех поддиректориях
ASM_SOURCES = $(shell find . -path ./$(BUILD_DIR) -prune -o -path ./$(ISO_DIR) -prune -o -name "*.asm" -print)

# Преобразуем пути исходников в объектные файлы
C_OBJECTS = $(patsubst ./%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst ./%.asm, $(BUILD_DIR)/%.o, $(ASM_SOURCES))

# Все объектные файлы
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Конечные цели
KERNEL = $(BUILD_DIR)/alphix.elf
ISO = alphix.iso

# Основная цель
all: $(ISO)

# Создание ISO образа
$(ISO): $(KERNEL)
	@echo "[ISO] Creating ISO image..."
	@mkdir -p $(BOOT_DIR)
	@cp $(KERNEL) $(BOOT_DIR)/
	@$(GRUB_MKRESCUE) -o $@ $(ISO_DIR) 2>/dev/null
	@echo "[ISO] Image is ready: $@"

# Линковка ядра
$(KERNEL): $(OBJECTS)
	@echo "[LD] Linking kernel..."
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $^
	@echo "Image is ready: $@"

# Компиляция ассемблерных файлов
$(BUILD_DIR)/%.o: %.asm
	@echo "[AS] $<"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

# Компиляция C файлов
$(BUILD_DIR)/%.o: %.c
	@echo "[CC] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@


# Запуск в QEMU
run: $(ISO)
	@echo "[QEMU] Running Alphix..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 128M -debugcon stdio -global isa-debugcon.iobase=0xe9

# Запуск с отладкой
debug: $(ISO)
	@echo "[QEMU] Running Alphix in debug mode..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 128M -vga std -s -S &
	@echo "[GDB] For connection use: gdb $(KERNEL) -ex 'target remote :1234'"

# Очистка
clean:
	@echo "[CLEAN] Cleaning..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(ISO)
	@rm -f $(BOOT_DIR)/alphix.elf

# Полная пересборка
rebuild: clean all

# Показать найденные файлы (для отладки)
show-sources:
	@echo "C sources:"
	@echo $(C_SOURCES) | tr ' ' '\n' | sort
	@echo ""
	@echo "ASM sources:"
	@echo $(ASM_SOURCES) | tr ' ' '\n' | sort
	@echo ""
	@echo "Object files:"
	@echo $(OBJECTS) | tr ' ' '\n' | sort

.PHONY: all run debug clean rebuild show-sources
