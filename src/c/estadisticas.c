#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "estadisticas.h"
#include "../../include/contratos.h"

// Variables globales exportadas por fpu_stats.asm (Módulo D)
extern double g_ratio_compresion;
extern double g_porcentaje_ahorro;

static uint64_t fpu_tiempo_microsegundos(void) {
    LARGE_INTEGER frecuencia, contador;
    QueryPerformanceFrequency(&frecuencia);
    QueryPerformanceCounter(&contador);
    return (uint64_t)((contador.QuadPart * 1000000ULL)
                      / frecuencia.QuadPart);
}

uint64_t fpu_obtener_timestamp(void) {
    return fpu_tiempo_microsegundos();
}

ResultadoEstadisticas fpu_CalcularEstadisticas(uint64_t tam_original,
                                               uint64_t tam_comprimido,
                                               uint64_t microsegundos) {
    ResultadoEstadisticas est;
    memset(&est, 0, sizeof(est));

    // ── Llamada al Módulo D (FPU real en .asm) ────────────────────
    // RCX = tam_original, RDX = tam_comprimido
    // Escribe resultados en g_ratio_compresion y g_porcentaje_ahorro
    fpu_CalcularEstadisticas_asm(tam_original, tam_comprimido);

    // ── Leer resultados calculados por la FPU ─────────────────────
    est.ratio_compresion  = g_ratio_compresion;
    est.porcentaje_ahorro = g_porcentaje_ahorro;

    // ── Velocidad (C, no necesita FPU) ────────────────────────────
    if (microsegundos > 0) {
        double segundos          = (double)microsegundos / 1000000.0;
        double kb                = (double)tam_original  / 1024.0;
        est.velocidad_kb_por_seg = kb / segundos;
    }

    // ── Formatear strings ─────────────────────────────────────────
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

    printf(CYAN "    |  " RESET "%-22s" CYAN ":  " GREEN "%-18s" CYAN "|\n" RESET,
           "Tamano original", est->str_tam_original);
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  " GREEN "%-18s" CYAN "|\n" RESET,
           "Tamano comprimido", est->str_tam_comprimido);

    printf(CYAN "    +------------------------------------------+\n" RESET);

    const char* color_ratio  = (est->ratio_compresion < 1.0)  ? GREEN : RED;
    const char* color_ahorro = (est->porcentaje_ahorro > 0.0) ? GREEN : RED;

    printf(CYAN "    |  " RESET "%-22s" CYAN ":  %s%-18s" CYAN "|\n" RESET,
           "Ratio compresion", color_ratio, est->str_ratio);
    printf(CYAN "    |  " RESET "%-22s" CYAN ":  %s%-18s" CYAN "|\n" RESET,
           "Ahorro de espacio", color_ahorro, est->str_ahorro);
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