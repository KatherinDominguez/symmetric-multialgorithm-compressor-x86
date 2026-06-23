; compresor_rle.asm - Stub del Estudiante C
bits 64
default rel

global comp_ComprimirBufferRLE

section .text

comp_ComprimirBufferRLE:
    ; Mismo passthrough que Huffman 
    push rsi
    push rdi
    push rcx
    push rdx

    mov rsi, rcx
    mov rdi, r8
    mov rcx, rdx

    rep movsb

    pop rdx
    pop rcx
    pop rdi
    pop rsi

    mov rax, rdx
    ret