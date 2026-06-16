#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "estadisticas.h"
#include "../../include/contratos.h"

// ─────────────────────────────────────────────────────────────────────
//  INTERNO: mide el tiempo actual en microsegundos usando la API
//  de Windows (QueryPerformanceCounter), sin depender de clock()
// ─────────────────────────────────────────────────────────────────────
static uint64_t fpu_tiempo_microsegundos(void) {
    LARGE_INTEGER frecuencia, contador;
    QueryPerformanceFrequency(&frecuencia);
    QueryPerformanceCounter(&contador);
    // Convertir a microsegundos: (contador / frecuencia) * 1_000_000
    return (uint64_t)((contador.QuadPart * 1000000ULL)
                      / frecuencia.QuadPart);
}

// ─────────────────────────────────────────────────────────────────────
//  Exportamos esta función para que main.c tome el tiempo
//  antes y después de comprimir/descomprimir
// ─────────────────────────────────────────────────────────────────────
uint64_t fpu_obtener_timestamp(void) {
    return fpu_tiempo_microsegundos();
}

// ─────────────────────────────────────────────────────────────────────
//  CÁLCULO PRINCIPAL
//  1. Llama a fpu_CalcularEstadisticas (Módulo D) que usa la FPU
//     para ratio y porcentaje con precisión de 64 bits.
//  2. Calcula velocidad en C (también podría ir a la FPU).
//  3. Formatea los strings para la UI.
// ─────────────────────────────────────────────────────────────────────
ResultadoEstadisticas fpu_CalcularEstadisticas(uint64_t tam_original,
                                            uint64_t tam_comprimido,
                                            uint64_t microsegundos) {
    ResultadoEstadisticas est;
    memset(&est, 0, sizeof(est));

    // ── Llamada al módulo D (FPU en .asm) ─────────────────────────
    // fpu_CalcularEstadisticas escribe los resultados de punto flotante
    // en variables globales de texto que luego leemos.
    // Mientras el Estudiante D no entregue, usamos el fallback de abajo.
    fpu_CalcularEstadisticas_asm(tam_original, tam_comprimido);

    // ── Fallback C (activo hasta que llegue el .asm real) ─────────
    // Cuando el Estudiante D entregue su módulo, este bloque
    // se reemplaza por leer las variables que él escribió.
    if (tam_original > 0) {
        est.ratio_compresion  = (double)tam_comprimido / (double)tam_original;
        est.porcentaje_ahorro = (1.0 - est.ratio_compresion) * 100.0;
    } else {
        est.ratio_compresion  = 0.0;
        est.porcentaje_ahorro = 0.0;
    }

    // ── Velocidad ─────────────────────────────────────────────────
    // tam_original en KB / tiempo en segundos
    if (microsegundos > 0) {
        double segundos       = (double)microsegundos / 1000000.0;
        double kb             = (double)tam_original  / 1024.0;
        est.velocidad_kb_por_seg = kb / segundos;
    }

    // ── Formatear strings para la UI ──────────────────────────────
    snprintf(est.str_ratio,    sizeof(est.str_ratio),
             "%.4f", est.ratio_compresion);

    snprintf(est.str_ahorro,   sizeof(est.str_ahorro),
             "%.2f %%", est.porcentaje_ahorro);

    snprintf(est.str_velocidad, sizeof(est.str_velocidad),
             "%.2f KB/s", est.velocidad_kb_por_seg);

    snprintf(est.str_tam_original, sizeof(est.str_tam_original),
             "%llu B (%.1f KB)",
             (unsigned long long)tam_original,
             (double)tam_original / 1024.0);

    snprintf(est.str_tam_comprimido, sizeof(est.str_tam_comprimido),
             "%llu B (%.1f KB)",
             (unsigned long long)tam_comprimido,
             (double)tam_comprimido / 1024.0);

    return est;
}

// ─────────────────────────────────────────────────────────────────────
//  Imprime la tabla de estadísticas en consola con formato visual
// ─────────────────────────────────────────────────────────────────────
void fpu_mostrar_estadisticas(const ResultadoEstadisticas* est) {

    #define CYAN    "\033[96m"
    #define YELLOW  "\033[93m"
    #define GREEN   "\033[92m"
    #define RED     "\033[91m"
    #define BOLD    "\033[1m"
    #define RESET   "\033[0m"

    printf("\n");
    printf(CYAN "    +------------------------------------------+\n" RESET);
    printf(CYAN "    |" BOLD YELLOW "      ESTADISTICAS DE COMPRESION          " CYAN "|\n" RESET);
    printf(CYAN "    +------------------------------------------+\n" RESET);

    // Tamaño original
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  " GREEN "%-18s" CYAN "|\n" RESET,
           "Tamano original", est->str_tam_original);

    // Tamaño comprimido
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  " GREEN "%-18s" CYAN "|\n" RESET,
           "Tamano comprimido", est->str_tam_comprimido);

    printf(CYAN "    +------------------------------------------+\n" RESET);

    // Ratio — verde si < 1, rojo si >= 1
    const char* color_ratio = (est->ratio_compresion < 1.0) ? GREEN : RED;
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  %s%-18s" CYAN "|\n" RESET,
           "Ratio compresion", color_ratio, est->str_ratio);

    // Ahorro — verde si positivo, rojo si negativo
    const char* color_ahorro = (est->porcentaje_ahorro > 0.0) ? GREEN : RED;
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  %s%-18s" CYAN "|\n" RESET,
           "Ahorro de espacio", color_ahorro, est->str_ahorro);

    // Velocidad
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  " GREEN "%-18s" CYAN "|\n" RESET,
           "Velocidad", est->str_velocidad);

    printf(CYAN "    +------------------------------------------+\n\n" RESET);

    #undef CYAN
    #undef YELLOW
    #undef GREEN
    #undef RED
    #undef BOLD
    #undef RESET
}