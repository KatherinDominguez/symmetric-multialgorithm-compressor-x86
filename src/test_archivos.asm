bits 64
default rel

global main

extern fopen
extern fclose
extern fseek
extern ftell
extern printf

section .data
    nombreArchivo db "pruebas/entrada.txt",0
    modoLectura db "rb",0

    msgOk db "Archivo abierto correctamente.",10,0
    msgTamano db "Tamano: %lld bytes",10,0
    msgError db "No se pudo abrir el archivo.",10,0
    msgFin db "Archivo cerrado.",10,0

section .bss
    handle resq 1

section .text

main:
    sub rsp,40

    ; fopen("pruebas/entrada.txt","rb")
    lea rcx,[nombreArchivo]
    lea rdx,[modoLectura]
    call fopen

    test rax,rax
    jz error

    mov [handle],rax

    ; Mensaje
    lea rcx,[msgOk]
    call printf

    ; fseek(handle,0,SEEK_END)
    mov rcx,[handle]
    xor rdx,rdx
    mov r8,2
    call fseek

    ; ftell(handle)
    mov rcx,[handle]
    call ftell

    ; printf("Tamano...")
    mov rdx,rax
    lea rcx,[msgTamano]
    call printf

    ; fclose(handle)
    mov rcx,[handle]
    call fclose

    lea rcx,[msgFin]
    call printf

    xor eax,eax
    add rsp,40
    ret

error:
    lea rcx,[msgError]
    call printf

    xor eax,eax
    add rsp,40
    ret