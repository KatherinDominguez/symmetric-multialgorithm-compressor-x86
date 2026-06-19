bits 64
default rel

; Exportación pura para el enlazador de GCC / MinGW
GLOBAL comp_ComprimirBufferHuffman
GLOBAL comp_ComprimirBufferRLE

section .text

; =============================================================================
; comp_ComprimirBufferHuffman
; Convención C (x64): RCX = *origen | RDX = tamano_orig | R8 = *destino
; Salida: RAX = Bytes finales escritos en destino
; =============================================================================
comp_ComprimirBufferHuffman:
    push rbp
    mov rbp, rsp
    ; Preservación estricta de registros de la ABI
    push rbx
    push rsi
    push rdi
    push r12s
    push r13
    push r14

    mov rsi, rcx            ; RSI = Puntero al buffer origen en C
    mov r12, rdx            ; R12 = Tamaño original pasado por C
    mov rdi, r8             ; RDI = Puntero al buffer destino en C
    
    xor rbx, rbx            ; Contador de bytes escritos en destino
    xor r9, r9              ; Acumulador de bits (64 bits)
    xor r10, r10            ; Contador de bits en el acumulador

    cmp r12, 0
    je .salir_huffman

.bucle_bytes:
    cmp r12, 0
    je .finalizar_flujo
    
    movzx rax, byte [rsi]   ; Leer byte del buffer de C
    inc rsi
    dec r12

    ; --- CONEXIÓN CON EL DICCIONARIO ---
    ; Aquí el Estudiante B o E te pasará un puntero a la tabla de códigos generada.
    ; Por ahora, la lógica matemática de empaquetado procesa los bits del registro:
    mov r11, rax            ; R11 = Código binario
    mov rcx, 6              ; RCX = Longitud del código

    ; Algoritmo de acoplamiento por desplazamiento
    dec rcx
    and r11, qword [.mascaras + rcx*8]
    inc rcx

    shl r9, cl              ; Desplazar acumulador a la izquierda
    or r9, r11              ; Acoplar código de longitud variable
    add r10, rcx            ; Sumar bits al contador

.vaciar_bytes:
    cmp r10, 8
    jl .siguiente_byte

    mov rcx, r10
    sub rcx, 8
    mov rax, r9
    shr rax, cl             ; Aislar el byte superior listo
    
    mov [rdi + rbx], al     ; Escribir directamente en la memoria gestionada por C
    inc rbx
    sub r10, 8
    jmp .vaciar_bytes

.siguiente_byte:
    jmp .bucle_bytes

.finalizar_flujo:
    cmp r10, 0
    je .salir_huffman
    mov rcx, 8
    sub rcx, r10
    shl r9, cl              ; Alinear residuo final
    mov [rdi + rbx], r9b
    inc rbx

.salir_huffman:
    mov rax, rbx            ; Retornar tamaño final a C (unsigned long long)
    
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
; Convención C (x64): RCX = *origen | RDX = tamano_orig | R8 = *destino
; Salida: RAX = Bytes finales escritos en destino
; =============================================================================
comp_ComprimirBufferRLE:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12

    mov rsi, rcx            ; Buffer origen desde C
    mov r12, rdx            ; Tamaño original desde C
    mov rdi, r8             ; Buffer destino desde C
    
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
    mov [rdi + rdx], al       ; Byte de datos
    mov [rdi + rdx + 1], r10b ; Byte de conteo
    add rdx, 2
    jmp .bucle_rle_principal

.finalizar_rle:
    mov rax, rdx            ; Retornar tamaño final de RLE a C

.salir_rle:
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

section .rodata
    .mascaras: 
        dq 0x0001, 0x0003, 0x0007, 0x000F
        dq 0x001F, 0x003F, 0x007F, 0x00FF
        dq 0x01FF, 0x03FF, 0x07FF, 0x0FFF
        dq 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
