; stubs/stub_compresor.asm
; RLE real simple para pruebas de integración
; Huffman es passthrough hasta que llegue el módulo C real
bits 64
default rel

global comp_ComprimirBufferHuffman
global comp_ComprimirBufferRLE

section .text

; ── Huffman stub: passthrough (copia sin comprimir) ──────────────────
; El formato real lo hará el Estudiante C
; Por ahora NO se usa en pruebas (main.c fuerza RLE)
comp_ComprimirBufferHuffman:
    push    rsi
    push    rdi
    push    rbx
    mov     rsi, rcx        ; fuente
    mov     rdi, r8         ; destino
    mov     rbx, rdx        ; tamaño
    mov     rcx, rdx
    rep     movsb
    mov     rax, rbx
    pop     rbx
    pop     rdi
    pop     rsi
    ret

; ── RLE real: comprime pares [contador][valor] ────────────────────────
; RCX = entrada, RDX = tamaño entrada, R8 = salida
; RAX = tamaño salida
comp_ComprimirBufferRLE:
    push    rsi
    push    rdi
    push    rbx
    push    r12
    push    r13

    mov     rsi, rcx        ; puntero entrada
    mov     r12, rdx        ; tamaño entrada
    mov     rdi, r8         ; puntero salida
    xor     rbx, rbx        ; índice entrada = 0
    xor     r13, r13        ; índice salida  = 0

    test    r12, r12
    jz      .fin_rle

.siguiente_byte:
    cmp     rbx, r12
    jge     .fin_rle

    movzx   rax, byte [rsi + rbx]   ; byte actual
    inc     rbx
    mov     rcx, 1                   ; contador = 1

.contar:
    cmp     rbx, r12
    jge     .escribir
    cmp     rcx, 255
    je      .escribir
    movzx   rdx, byte [rsi + rbx]
    cmp     dl, al
    jne     .escribir
    inc     rcx
    inc     rbx
    jmp     .contar

.escribir:
    mov     byte [rdi + r13], cl    ; escribir contador
    inc     r13
    mov     byte [rdi + r13], al    ; escribir valor
    inc     r13
    jmp     .siguiente_byte

.fin_rle:
    mov     rax, r13                ; retornar tamaño comprimido

    pop     r13
    pop     r12
    pop     rbx
    pop     rdi
    pop     rsi
    ret