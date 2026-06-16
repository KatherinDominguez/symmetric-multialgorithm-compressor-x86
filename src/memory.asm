default rel 
bits 64

global mem_AsignarMemoria
global mem_LiberarMemoria
global mem_CrearNodo
global mem_CrearTablaFrecuencias

extern VirtualAlloc
extern VirtualFree

section .data
    MEM_COMMIT_RESERVE  equ 0x00003000
    PAGE_READWRITE      equ 0x04
    MEM_RELEASE         equ 0x00008000

    %define NODO_FRECUENCIA   0
    %define NODO_CARACTER     8
    %define NODO_HIJO_IZQ     16
    %define NODO_HIJO_DER     24
    %define NODO_TAMANO       32

section .bss
    mem_buffer_simulado resb 10485760
section .text

mem_AsignarMemoria:
    push    rbp
    mov     rbp, rsp
    push    rbx

    sub     rsp, 40

    mov     rbx, rcx

    xor     rcx, rcx
    mov     rdx, rbx
    mov     r8,  MEM_COMMIT_RESERVE
    mov     r9,  PAGE_READWRITE
    call    VirtualAlloc

    add     rsp, 40
    pop     rbx
    pop     rbp
    ret

mem_LiberarMemoria:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 32

    xor     rdx, rdx
    mov     r8,  MEM_RELEASE
    call    VirtualFree

    add     rsp, 32
    pop     rbp
    ret

mem_CrearNodo:
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    r12
    push    r13

    sub     rsp, 32

    mov     r12, rcx
    mov     r13, rdx

    mov     rcx, NODO_TAMANO
    call    mem_AsignarMemoria

    test    rax, rax
    jz      .fin

    mov     rbx, rax

    mov     rcx, rbx
    mov     rdx, NODO_TAMANO
    call    mem_inicializar_bloque

    mov     qword [rbx + NODO_FRECUENCIA], r13
    mov     byte  [rbx + NODO_CARACTER],   r12b
    mov     qword [rbx + NODO_HIJO_IZQ],   0
    mov     qword [rbx + NODO_HIJO_DER],   0

    mov     rax, rbx

.fin:
    add     rsp, 32
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp
    ret

mem_CrearTablaFrecuencias:
    push    rbp
    mov     rbp, rsp
    push    rbx
    sub     rsp, 40

    mov     rcx, 2048
    call    mem_AsignarMemoria

    test    rax, rax
    jz      .fin

    mov     rbx, rax
    mov     rcx, rbx
    mov     rdx, 2048
    call    mem_inicializar_bloque

    mov     rax, rbx

.fin:
    add     rsp, 40
    pop     rbx
    pop     rbp
    ret

mem_inicializar_bloque:
    push    rbp
    mov     rbp, rsp
    push    rdi

    mov     rdi, rcx
    mov     rcx, rdx
    xor     al,  al
    rep stosb

    pop     rdi
    pop     rbp
    ret