#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "ui.h"

// Activa UTF-8 y colores ANSI en la consola de Windows
void ui_configurar_consola(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // Habilitar colores ANSI (Windows 10+)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void ui_limpiar_pantalla(void) {
    system("cls");
}

// ─────────────────────────────────────────
//  Colores ANSI
// ─────────────────────────────────────────
#define COLOR_RESET   "\033[0m"
#define COLOR_CYAN    "\033[96m"
#define COLOR_YELLOW  "\033[93m"
#define COLOR_GREEN   "\033[92m"
#define COLOR_RED     "\033[91m"
#define COLOR_BOLD    "\033[1m"

int ui_mostrar_menu_principal(void) {
    ui_limpiar_pantalla();

    printf(COLOR_CYAN
           "\n"
           "    +================================+\n"
           "    |                                |\n"
           "    |    COMPRESOR  x86-64  UMSS     |\n"
           "    |                                |\n"
           "    +================================+\n"
           COLOR_RESET);

    printf(COLOR_GREEN
           "    |                                |\n"
           "    |   [1]  Comprimir archivo       |\n"
           "    |   [2]  Descomprimir archivo    |\n"
           COLOR_RESET);

    printf(COLOR_RED
           "    |   [3]  Salir                   |\n"
           COLOR_RESET);

    printf(COLOR_CYAN
           "    +================================+\n"
           COLOR_RESET);

    printf("\n    Seleccione una opcion: ");

    int opcion = 0;
    scanf("%d", &opcion);
    while (getchar() != '\n');

    return opcion;
}

void ui_mostrar_submenu_algoritmo(void) {
    printf(COLOR_CYAN
           "\n"
           "    +================================+\n"
           "    |    Algoritmo de compresion     |\n"
           "    +================================+\n"
           COLOR_RESET);

    printf(COLOR_GREEN
           "    |   [1]  RLE      (.rle)         |\n"
           COLOR_RESET);

    printf(COLOR_YELLOW
           "    |   [2]  Huffman  (.huff) [prox] |\n"
           COLOR_RESET);

    printf(COLOR_CYAN
           "    +================================+\n"
           COLOR_RESET);

    printf("\n    Seleccione: ");
}

void ui_solicitar_ruta(const char* prompt, char* buffer, int max) {
    printf(COLOR_YELLOW "\n  %s\n  Ruta: " COLOR_RESET, prompt);
    fgets(buffer, max, stdin);
    // Quitar el salto de línea
    buffer[strcspn(buffer, "\n")] = '\0';
}

void ui_mostrar_mensaje(const char* msg) {
    printf(COLOR_GREEN
           "\n    +--------------------------------+\n"
           "      %s\n"
           "    +--------------------------------+\n"
           COLOR_RESET, msg);
    printf("\n    Presione Enter para continuar...");
    getchar();
}

void ui_mostrar_error(const char* msg) {
    printf(COLOR_RED
           "\n    +--------------------------------+\n"
           "      [ERROR] %s\n"
           "    +--------------------------------+\n"
           COLOR_RESET, msg);
    printf("\n    Presione Enter para continuar...");
    getchar();
}

void ui_mostrar_barra_progreso(int porcentaje) {
    int lleno = porcentaje / 5;   // 20 bloques = 100%
    int vacio = 20 - lleno;

    printf(COLOR_CYAN "\r    Progreso: [" COLOR_GREEN);
    for (int i = 0; i < lleno; i++) printf("=");
    if (lleno < 20) printf(">");
    printf(COLOR_RESET COLOR_CYAN);
    for (int i = 0; i < vacio - 1 && vacio > 0; i++) printf(".");
    printf("] " COLOR_YELLOW "%3d%%" COLOR_RESET, porcentaje);
    fflush(stdout);
}
