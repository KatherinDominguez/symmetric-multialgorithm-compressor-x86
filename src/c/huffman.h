#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stddef.h>

// Estructura que coincide con HuffmanCode en compresor.asm
// Cada entrada ocupa exactamente 16 bytes
#pragma pack(push, 1)
typedef struct {
    uint64_t codigo;   // patrón binario del código
    uint64_t bits;     // longitud en bits
} HuffmanCode;
#pragma pack(pop)

// Construye el diccionario de 256 entradas para Huffman
// Devuelve puntero al array, caller libera con free()
HuffmanCode* huffman_construir_diccionario(const uint8_t* datos,
                                           size_t         tamano);

// Serializa la tabla de frecuencias (2048 bytes) al inicio
// del buffer de salida para que el descompresor pueda leerla
void huffman_escribir_tabla_frecuencias(const uint8_t* datos,
                                        size_t         tamano,
                                        uint8_t*       salida);

#endif