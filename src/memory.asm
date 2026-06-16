section .bss
    buffer resb 1024
section .text
    global men_AsignarMemoria
men_AsignarMemoria:
    lea rax, [buffer]
    ret