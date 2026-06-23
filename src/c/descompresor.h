#ifndef DESCOMPRESOR_H
#define DESCOMPRESOR_H

#include <stdint.h>
#include <stddef.h>
#include "archivos.h"

// Resultado de descompresión
typedef struct {
    uint8_t* datos;       // buffer con bytes restaurados
    size_t   tamano;      // tamaño del archivo restaurado
    int      error;       // 0 = OK
} ResultadoDescompresion;

ResultadoDescompresion des_DescomprimirFlujo(const char* ruta_entrada);
void des_liberar(ResultadoDescompresion* r);

// Internos (expuestos para pruebas unitarias)
ResultadoDescompresion des_expandir_rle  (const uint8_t* datos, size_t tam, size_t tam_esperado);
ResultadoDescompresion des_expandir_huffman(const uint8_t* datos, size_t tam,
                                            const uint8_t* cabecera_extra,
                                            size_t         tam_cabecera);
#endif