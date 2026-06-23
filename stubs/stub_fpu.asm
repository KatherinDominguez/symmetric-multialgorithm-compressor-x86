; stub_fpu.asm - Temporal hasta que el Estudiante D entregue su módulo
; fpu_CalcularEstadisticas: por ahora no hace nada,
; estadisticas.c usa su fallback en C automáticamente
bits 64
default rel

global fpu_CalcularEstadisticas_asm

section .text

fpu_CalcularEstadisticas_asm:
    ; RCX = tam_original, RDX = tam_comprimido
    ; El stub no calcula nada, estadisticas.c tiene el fallback
    ret