# Makefile - Windows (MinGW-w64 + NASM)

CC        = gcc
NASM      = nasm
NASMFLAGS = -f win64
CFLAGS    = -Wall -Wextra -m64 -O2 -I./include

SRC_C   = src/c
SRC_ASM = src/asm
STUBS   = stubs
BUILD   = build

# ─── Objetos C ───────────────────────────────────────────────────────
OBJ_C = $(BUILD)/main.o         \
        $(BUILD)/ui.o           \
        $(BUILD)/archivos.o     \
        $(BUILD)/descompresor.o \
        $(BUILD)/estadisticas.o

# ─── Objetos ASM (stubs hasta que compañeros entreguen) ──────────────
OBJ_ASM = $(BUILD)/stub_memoria.o   \
          $(BUILD)/stub_compresor.o  \
          $(BUILD)/stub_fpu.o

TARGET = compresor.exe

# ─────────────────────────────────────────────────────────────────────
all: $(BUILD) $(TARGET)

$(BUILD):
	if not exist $(BUILD) mkdir $(BUILD)

# ─── Compilar archivos C ─────────────────────────────────────────────
$(BUILD)/main.o: $(SRC_C)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/ui.o: $(SRC_C)/ui.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/archivos.o: $(SRC_C)/archivos.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/descompresor.o: $(SRC_C)/descompresor.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/estadisticas.o: $(SRC_C)/estadisticas.c
	$(CC) $(CFLAGS) -c $< -o $@

# ─── Compilar stubs ASM ──────────────────────────────────────────────
$(BUILD)/stub_memoria.o: $(STUBS)/stub_memoria.asm
	$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD)/stub_compresor.o: $(STUBS)/stub_compresor.asm
	$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD)/stub_fpu.o: $(STUBS)/stub_fpu.asm
	$(NASM) $(NASMFLAGS) $< -o $@

# ─── Enlazar todo ────────────────────────────────────────────────────
$(TARGET): $(OBJ_C) $(OBJ_ASM)
	$(CC) -m64 -o $@ $^ -lkernel32

# ─── Limpiar ─────────────────────────────────────────────────────────
clean:
	if exist $(BUILD) rmdir /s /q $(BUILD)
	if exist $(TARGET) del $(TARGET)