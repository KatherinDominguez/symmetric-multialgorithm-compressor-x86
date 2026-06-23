; =========================================================
; Archivo: memoria.asm
; =========================================================

default rel 
bits 64

global mem_AsignarMemoria
global mem_LiberarMemoria
global mem_CrearNodo
global mem_CrearTablaFrecuencias

extern VirtualAlloc
extern VirtualFree

; ---------------------------------------------------------
; Constantes de la API de Windows para manejo de memoria
; ---------------------------------------------------------
section .data
    MEM_COMMIT_RESERVE  equ 0x00003000
    PAGE_READWRITE      equ 0x04
    MEM_RELEASE         equ 0x00008000

; ---------------------------------------------------------
; Offsets del layout de NodoHuffman en memoria (32 bytes)
; ---------------------------------------------------------
    %define NODO_FRECUENCIA   0
    %define NODO_CARACTER     8
    %define NODO_HIJO_IZQ     16
    %define NODO_HIJO_DER     24
    %define NODO_TAMANO       32

; ---------------------------------------------------------
; Buffer estático de respaldo (10 MB reservados en BSS)
; ---------------------------------------------------------
section .bss
    mem_buffer_simulado resb 10485760

section .text

; =========================================================
; mem_AsignarMemoria
; Contrato x64: RCX = tamano (bytes a reservar)
; Salida: RAX = puntero al bloque asignado (0 si falla)
; Delega en VirtualAlloc de la WinAPI
; =========================================================
mem_AsignarMemoria:
    push    rbp
    mov     rbp, rsp
    push    rbx

    sub     rsp, 40             ; Sombra de llamada (ABI x64 Windows)

    mov     rbx, rcx            ; RBX = tamaño solicitado

    xor     rcx, rcx            ; RCX = NULL (el SO elige la dirección)
    mov     rdx, rbx            ; RDX = tamaño
    mov     r8,  MEM_COMMIT_RESERVE
    mov     r9,  PAGE_READWRITE
    call    VirtualAlloc        ; RAX = dirección base o NULL

    add     rsp, 40
    pop     rbx
    pop     rbp
    ret

; =========================================================
; mem_LiberarMemoria
; Contrato x64: RCX = ptr | RDX = tamano (ignorado por VirtualFree)
; Delega en VirtualFree con MEM_RELEASE
; =========================================================
mem_LiberarMemoria:
    push    rbp
    mov     rbp, rsp

    sub     rsp, 32             ; Sombra de llamada

    xor     rdx, rdx            ; dwSize = 0 (requerido por MEM_RELEASE)
    mov     r8,  MEM_RELEASE
    call    VirtualFree

    add     rsp, 32
    pop     rbp
    ret

; =========================================================
; mem_CrearNodo
; Contrato x64: RCX = caracter (uint8) | RDX = frecuencia (uint64)
; Salida: RAX = puntero al NodoHuffman inicializado (0 si falla)
; =========================================================
mem_CrearNodo:
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    r12
    push    r13

    sub     rsp, 32

    mov     r12, rcx            ; R12 = caracter
    mov     r13, rdx            ; R13 = frecuencia

    ; Paso 1: Asignar bloque de NODO_TAMANO bytes
    mov     rcx, NODO_TAMANO
    call    mem_AsignarMemoria

    test    rax, rax
    jz      .fin                ; Fallo de asignación → retornar NULL

    mov     rbx, rax            ; RBX = puntero al nodo recién asignado

    ; Paso 2: Limpiar el bloque a cero
    mov     rcx, rbx
    mov     rdx, NODO_TAMANO
    call    mem_inicializar_bloque

    ; Paso 3: Escribir campos del nodo
    mov     qword [rbx + NODO_FRECUENCIA], r13
    mov     byte  [rbx + NODO_CARACTER],   r12b
    mov     qword [rbx + NODO_HIJO_IZQ],   0
    mov     qword [rbx + NODO_HIJO_DER],   0

    mov     rax, rbx            ; Retornar puntero al nodo

.fin:
    add     rsp, 32
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp
    ret

; =========================================================
; mem_CrearTablaFrecuencias
; Sin parámetros
; Salida: RAX = puntero a bloque de 2048 bytes en cero
;         (256 entradas × 8 bytes cada una)
; =========================================================
mem_CrearTablaFrecuencias:
    push    rbp
    mov     rbp, rsp
    push    rbx

    sub     rsp, 40

    ; Paso 1: Asignar 2048 bytes (256 × sizeof(uint64_t))
    mov     rcx, 2048
    call    mem_AsignarMemoria

    test    rax, rax
    jz      .fin                ; Fallo → retornar NULL

    mov     rbx, rax

    ; Paso 2: Inicializar toda la tabla a cero
    mov     rcx, rbx
    mov     rdx, 2048
    call    mem_inicializar_bloque

    mov     rax, rbx            ; Retornar puntero a la tabla

.fin:
    add     rsp, 40
    pop     rbx
    pop     rbp
    ret

; =========================================================
; mem_inicializar_bloque  [función interna]
; Contrato x64: RCX = *bloque | RDX = tamano
; Rellena el bloque con ceros usando REP STOSB
; =========================================================
mem_inicializar_bloque:
    push    rbp
    mov     rbp, rsp
    push    rdi

    mov     rdi, rcx            ; RDI = destino
    mov     rcx, rdx            ; RCX = contador de bytes
    xor     al,  al             ; AL  = valor de relleno (0)
    rep stosb                   ; Limpiar bloque completo

    pop     rdi
    pop     rbp
    ret