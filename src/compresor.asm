bits 64
default rel

; Exportación contractual estricta para la integración del Estudiante E (main.asm)
GLOBAL comp_ComprimirBufferHuffman
GLOBAL comp_ComprimirBufferRLE

; Estructura acordada para el diccionario (Cada entrada ocupa 16 bytes)
STRUC HuffmanCode
    .codigo: resq 1    ; QWORD (8 bytes): Patrón binario real del carácter
    .bits:   resq 1    ; QWORD (8 bytes): Longitud del código en bits
ENDSTRUC

section .text

; =============================================================================
; comp_ComprimirBufferHuffman
; Contrato x64: RCX = *origen | RDX = tamano_orig | R8 = *destino | R9 = *diccionario
; Salida: RAX = Bytes finales escritos en destino (Bloque comprimido)
; =============================================================================
comp_ComprimirBufferHuffman:
    push rbp
    mov rbp, rsp
    
    ; Preservación estricta de registros de la ABI x64 de Windows
    push rbx
    push rsi
    push rdi
    push r12            ; CORREGIDO: Eliminada la 's' de 'r12s' que causaba error de compilación
    push r13
    push r14

    mov rsi, rcx            ; RSI = Puntero al buffer origen en memoria
    mov r12, rdx            ; R12 = Tamaño original de datos
    mov rdi, r8             ; RDI = Puntero al buffer destino de salida
    mov r14, r9             ; R14 = Puntero al diccionario (Estructura de 256 entradas)
    
    xor rbx, rbx            ; RBX = Contador de bytes netos escritos en destino
    xor r9, r9              ; R9  = Acumulador de bits del flujo (64 bits)
    xor r10, r10            ; R10 = Contador de bits acumulados

    cmp r12, 0
    je .salir_huffman

.bucle_bytes:
    cmp r12, 0
    je .finalizar_flujo
    
    movzx rax, byte [rsi]   ; Leer byte del buffer origen (0 a 255)
    inc rsi
    dec r12

    ; --- CONEXIÓN REAL CON EL DICCIONARIO (Contrato Módulo 2 y 5) ---
    ; Cada entrada mide 16 bytes. Multiplicamos el índice por 16 haciendo un SHL a RAX por 4
    shl rax, 4
    lea r11, [r14 + rax]    ; R11 = Dirección exacta de diccionario[byte]
    
    mov r11, [r11 + HuffmanCode.codigo] ; R11 = Código binario asignado
    mov rcx, [r14 + rax + HuffmanCode.bits] ; RCX = Longitud real en bits

    cmp rcx, 0
    je .bucle_bytes         ; Failsafe: Si el carácter no tiene bits, ignorar

    ; Algoritmo de máscara dinámico para evitar desbordamientos de lectura
    dec rcx
    and rcx, 63             ; Asegura que el índice esté entre 0 y 63
    lea r13, [rel .mascaras]
    and r11, qword [r13 + rcx*8]
    inc rcx

    ; --- ACOPLAMIENTO MATEMÁTICO DE BITS (CORREGIDO) ---
    ; Desplazamos el código nuevo a la posición del residuo actual para evitar invertir el orden
    mov r13, rcx            ; Conservar longitud del código
    mov rcx, r10            ; RCX = Desplazamiento actual en el acumulador
    shl r11, cl             ; Mover código nuevo hacia la izquierda
    or r9, r11              ; Fusionar con los bits previos
    add r10, r13            ; Actualizar contador de bits en el acumulador

.vaciar_bytes:
    cmp r10, 8
    jl .siguiente_byte

    ; Volcar el byte inferior completado directamente a la memoria destino
    mov [rdi + rbx], r9b    
    inc rbx
    shr r9, 8               ; Desplazar el acumulador hacia abajo para limpiar el byte escrito
    sub r10, 8
    jmp .vaciar_bytes

.siguiente_byte:
    jmp .bucle_bytes

.finalizar_flujo:
    ; Alinear y escribir los bits huérfanos sobrantes en el último byte
    cmp r10, 0
    je .salir_huffman
    mov [rdi + rbx], r9b
    inc rbx

.salir_huffman:
    mov rax, rbx            ; Retornar tamaño final del flujo comprimido a main.asm
    
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret


; =============================================================================
; comp_ComprimirBufferRLE
; Contrato x64: RCX = *origen | RDX = tamano_orig | R8 = *destino
; Salida: RAX = Bytes finales escritos en destino (Bloque comprimido)
; =============================================================================
comp_ComprimirBufferRLE:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12

    mov rsi, rcx            ; Buffer origen
    mov r12, rdx            ; Tamaño original
    mov rdi, r8             ; Buffer destino
    
    xor rbx, rbx            ; Índice de lectura
    xor rdx, rdx            ; Índice de escritura / Contador final

    cmp r12, 0
    je .salir_rle

.bucle_rle_principal:
    cmp rbx, r12
    jge .finalizar_rle

    mov al, [rsi + rbx]     ; Extraer carácter base
    mov r10b, 1             ; Contador de repetición (máx 255)
    inc rbx

.bucle_conteo_rle:
    cmp rbx, r12
    jge .escribir_par_rle
    cmp r10b, 255
    je .escribir_par_rle

    mov r11b, [rsi + rbx]
    cmp al, r11b
    jne .escribir_par_rle

    inc r10b
    inc rbx
    jmp .bucle_conteo_rle

.escribir_par_rle:
    mov [rdi + rdx], al       ; Escribir byte de datos original
    mov [rdi + rdx + 1], r10b ; Escribir byte de conteo de repeticiones
    add rdx, 2
    jmp .bucle_rle_principal

.finalizar_rle:
    mov rax, rdx            ; Retornar tamaño final de RLE

.salir_rle:
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

section .rodata
    ; Tabla extendida completa a 64 bits para soportar cualquier longitud de código
    .mascaras: 
        dq 0x0000000000000001, 0x0000000000000003, 0x0000000000000007, 0x000000000000000f
        dq 0x000000000000001f, 0x000000000000003f, 0x000000000000007f, 0x00000000000000ff
        dq 0x00000000000001ff, 0x00000000000003ff, 0x00000000000007ff, 0x0000000000000fff
        dq 0x0000000000001fff, 0x0000000000003fff, 0x0000000000007fff, 0x000000000000ffff
        dq 0x000000000001ffff, 0x000000000003ffff, 0x000000000007ffff, 0x00000000000fffff
        dq 0x00000000001fffff, 0x00000000003fffff, 0x0000000007fffff, 0x0000000000ffffff
        dq 0x0000000001ffffff, 0x0000000003ffffff, 0x0000000007ffffff, 0x000000000fffffff
        dq 0x000000001fffffff, 0x000000003fffffff, 0x000000007fffffff, 0x00000000ffffffff
        dq 0x00000001ffffffff, 0x00000003ffffffff, 0x00000007ffffffff, 0x0000000fffffffff
        dq 0x0000001fffffffff, 0x0000003fffffffff, 0x0000007fffffffff, 0x000000ffffffffff
        dq 0x000001ffffffffff, 0x000003ffffffffff, 0x000007ffffffffff, 0x00000fffffffffff
        dq 0x00001fffffffffff, 0x00003fffffffffff, 0x00007fffffffffff, 0x0000ffffffffffff
        dq 0x0001ffffffffffff, 0x0003ffffffffffff, 0x0007ffffffffffff, 0x000fffffffffffff
        dq 0x001fffffffffffff, 0x003fffffffffffff, 0x007fffffffffffff, 0x00ffffffffffffff
        dq 0x01ffffffffffffff, 0x03ffffffffffffff, 0x07ffffffffffffff, 0x0fffffffffffffff
        dq 0x1fffffffffffffff, 0x3fffffffffffffff, 0x7fffffffffffffff, 0xffffffffffffffff
