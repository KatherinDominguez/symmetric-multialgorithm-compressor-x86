; compresor_huffman.asm - Stub del Estudiante C
; Por ahora copia los bytes sin comprimir (passthrough)
bits 64
default rel

global comp_ComprimirBufferHuffman

section .text

comp_ComprimirBufferHuffman:
    ; RCX = puntero entrada
    ; RDX = tamaño entrada
    ; R8  = puntero salida
    ; RAX = tamaño del resultado (devolvemos el mismo tamaño)

    push rsi
    push rdi
    push rcx
    push rdx

    mov rsi, rcx            ; fuente = entrada
    mov rdi, r8             ; destino = salida
    mov rcx, rdx            ; contador = tamaño

    rep movsb               ; copiar byte a byte

    pop rdx
    pop rcx
    pop rdi
    pop rsi

    mov rax, rdx            ; devolver tamaño original
    ret