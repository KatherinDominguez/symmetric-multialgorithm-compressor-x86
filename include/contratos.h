// include/contratos.h
#ifndef CONTRATOS_H
#define CONTRATOS_H

#include <stdint.h>
#include <stddef.h>

// --- Módulo A: Memoria ---
extern void* mem_AsignarMemoria(size_t tamano);
extern void  mem_LiberarMemoria(void* ptr, size_t tamano);

// --- Módulo C: Compresión ---
extern size_t comp_ComprimirBufferHuffman(
    const uint8_t* entrada,   // RCX
    size_t         tamano,    // RDX
    uint8_t*       salida,     // R8
    const uint8_t* diccionario   // R9
);
extern size_t comp_ComprimirBufferRLE(
    const uint8_t* entrada,
    size_t         tamano,
    uint8_t*       salida
);

// --- Módulo D: FPU Estadísticas ---
extern void fpu_CalcularEstadisticas_asm(
    uint64_t tamano_original,    // RCX
    uint64_t tamano_comprimido   // RDX
);

#endif