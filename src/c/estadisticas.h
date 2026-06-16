#ifndef ESTADISTICAS_H
#define ESTADISTICAS_H

#include <stdint.h>

// ─── Resultado que devuelve el módulo D (FPU) ──────────────────────
// fpu_CalcularEstadisticas escribe aquí sus resultados formateados
// para que main.c / ui.c los imprima directamente
typedef struct {
    double   ratio_compresion;          // ej: 0.4123
    double   porcentaje_ahorro;         // ej: 58.77
    double   velocidad_kb_por_seg;      // ej: 1240.5

    // Versiones ya formateadas como texto (listas para imprimir)
    char     str_ratio[32];             // "0.4123"
    char     str_ahorro[32];            // "58.77 %"
    char     str_velocidad[32];         // "1240.50 KB/s"
    char     str_tam_original[32];      // "204800 bytes  (200.0 KB)"
    char     str_tam_comprimido[32];    // "84377 bytes   ( 82.4 KB)"
} ResultadoEstadisticas;

// ─── Función principal ─────────────────────────────────────────────
// Llama internamente a fpu_CalcularEstadisticas (módulo D en .asm)
// tam_original    → bytes antes de comprimir
// tam_comprimido  → bytes después de comprimir
// microsegundos   → tiempo que tardó el proceso (medido en main.c)
ResultadoEstadisticas fpu_CalcularEstadisticas(uint64_t tam_original,
                                            uint64_t tam_comprimido,
                                            uint64_t microsegundos);

// Imprime el bloque de estadísticas en consola con formato de tabla
void fpu_mostrar_estadisticas(const ResultadoEstadisticas* est);
// Al final de estadisticas.h, antes del #endif
uint64_t fpu_obtener_timestamp(void);
#endif