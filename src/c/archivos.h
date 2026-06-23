#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include <stdint.h>
#include <stddef.h>

// ─── Resultado de lectura ───────────────────────────────────────────
typedef struct {
    uint8_t* datos;       // puntero al buffer con el contenido
    size_t   tamano;      // cantidad de bytes leídos
    int      error;       // 0 = OK, otro = código de error
} ResultadoLectura;

// ─── Cabecera del archivo comprimido ───────────────────────────────
// Layout binario en disco:
//  [4 bytes]  firma        → "HUFF" o "RLE_"
//  [4 bytes]  ext_len      → longitud de la extensión original
//  [N bytes]  extension    → ej: ".txt\0"
//  [8 bytes]  tam_original → tamaño del archivo sin comprimir
//  [8 bytes]  tam_datos    → tamaño del bloque comprimido
//  [resto]    datos        → flujo comprimido
#pragma pack(push, 1)
typedef struct {
    char     firma[4];        // "HUFF" o "RLE_"
    uint32_t ext_len;
    char     extension[16];   // extensión original, ej: ".txt"
    uint64_t tam_original;
    uint64_t tam_datos;
} CabeceraArchivo;
#pragma pack(pop)

// ─── Funciones públicas ─────────────────────────────────────────────
ResultadoLectura file_LeerArchivoCompleto(const char* ruta);
int  file_EscribirArchivoComprimido(const char* ruta,
                                   const CabeceraArchivo* cabecera,
                                   const uint8_t* datos,
                                   size_t tam_datos);
int  file_LeerArchivoComprimido(const char* ruta,
                               CabeceraArchivo* cabecera_out,
                               uint8_t** datos_out,
                               size_t*   tam_out);
void file_liberar(ResultadoLectura* r);
const char* file_obtener_extension(const char* ruta);

#endif