; stub_memoria.asm
bits 64
default rel

global mem_AsignarMemoria
global mem_LiberarMemoria

section .bss
    buffer_tmp resb 10485760    ; 10 MB estático de prueba

section .text

mem_AsignarMemoria:
    lea rax, [buffer_tmp]
    ret

mem_LiberarMemoria:
    ; No hace nada en el stub
    ret