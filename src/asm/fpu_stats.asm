; ==========================================================
; MODULO D - ESTADISTICAS FPU
; Calcula:
;   ratio = comprimido / original
;   ahorro = (1 - ratio) * 100
; usando la FPU x87
; ==========================================================

bits 64
default rel

global fpu_CalcularEstadisticas_asm

global g_ratio_compresion
global g_porcentaje_ahorro

section .data

cien dq 100.0
uno  dq 1.0

section .bss

g_ratio_compresion  resq 1
g_porcentaje_ahorro resq 1

tmp_original        resq 1
tmp_comprimido      resq 1

section .text

; ==========================================================
; RCX = tamaño original
; RDX = tamaño comprimido
; ==========================================================

fpu_CalcularEstadisticas_asm:

    ; --------------------------------------
    ; guardar parámetros
    ; --------------------------------------

    mov [tmp_original], rcx
    mov [tmp_comprimido], rdx

    ; --------------------------------------
    ; verificar división por cero
    ; --------------------------------------

    test rcx, rcx
    jnz calcular

    fldz
    fstp qword [g_ratio_compresion]

    fldz
    fstp qword [g_porcentaje_ahorro]

    ret

calcular:

    ; ==================================================
    ; ratio = comprimido / original
    ; ==================================================

    fild qword [tmp_comprimido]

    fild qword [tmp_original]

    fdivp st1, st0

    fst qword [g_ratio_compresion]

    ; ==================================================
    ; ahorro = (1 - ratio) * 100
    ; ==================================================

    fld qword [uno]

    fsub qword [g_ratio_compresion]

    fmul qword [cien]

    fstp qword [g_porcentaje_ahorro]

    ret
