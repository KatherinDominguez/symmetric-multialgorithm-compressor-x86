#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "archivos.h"

// ─────────────────────────────────────────────────────────────────────
//  INTERNO: obtener tamaño de archivo sin leerlo completo
// ─────────────────────────────────────────────────────────────────────
static size_t file_obtener_tamano(FILE* f) {
    fseek(f, 0, SEEK_END);
    size_t tam = (size_t)ftell(f);
    rewind(f);
    return tam;
}

// ─────────────────────────────────────────────────────────────────────
//  Lee un archivo completo en un buffer dinámico
//  Devuelve ResultadoLectura; el caller libera con file_liberar()
// ─────────────────────────────────────────────────────────────────────
ResultadoLectura file_LeerArchivoCompleto(const char* ruta) {
    ResultadoLectura res = {NULL, 0, 0};

    FILE* f = fopen(ruta, "rb");
    if (!f) {
        res.error = 1;
        fprintf(stderr, "[archivos] No se pudo abrir: %s\n", ruta);
        return res;
    }

    size_t tam = file_obtener_tamano(f);
    if (tam == 0) {
        res.error = 2;
        fprintf(stderr, "[archivos] El archivo esta vacio: %s\n", ruta);
        fclose(f);
        return res;
    }

    // Límite de 10 MB según especificación del proyecto
    if (tam > 10 * 1024 * 1024) {
        res.error = 3;
        fprintf(stderr, "[archivos] Archivo supera el limite de 10 MB\n");
        fclose(f);
        return res;
    }

    res.datos = (uint8_t*)malloc(tam);
    if (!res.datos) {
        res.error = 4;
        fprintf(stderr, "[archivos] Sin memoria para leer el archivo\n");
        fclose(f);
        return res;
    }

    size_t leidos = fread(res.datos, 1, tam, f);
    fclose(f);

    if (leidos != tam) {
        res.error = 5;
        fprintf(stderr, "[archivos] Lectura incompleta (%zu de %zu bytes)\n",
                leidos, tam);
        free(res.datos);
        res.datos = NULL;
        return res;
    }

    res.tamano = tam;
    return res;
}

// ─────────────────────────────────────────────────────────────────────
//  Escribe el archivo comprimido (.huff o .rle) con su cabecera
// ─────────────────────────────────────────────────────────────────────
int file_EscribirArchivoComprimido(const char*           ruta,
                                  const CabeceraArchivo* cabecera,
                                  const uint8_t*         datos,
                                  size_t                 tam_datos) {
    FILE* f = fopen(ruta, "wb");
    if (!f) {
        fprintf(stderr, "[archivos] No se pudo crear: %s\n", ruta);
        return 1;
    }

    // Escribir cabecera binaria primero
    if (fwrite(cabecera, sizeof(CabeceraArchivo), 1, f) != 1) {
        fprintf(stderr, "[archivos] Error escribiendo cabecera\n");
        fclose(f);
        return 2;
    }

    // Escribir bloque de datos comprimidos
    if (fwrite(datos, 1, tam_datos, f) != tam_datos) {
        fprintf(stderr, "[archivos] Error escribiendo datos comprimidos\n");
        fclose(f);
        return 3;
    }

    fclose(f);
    return 0;   // OK
}

// ─────────────────────────────────────────────────────────────────────
//  Lee un archivo .huff o .rle: extrae cabecera y datos por separado
//  El caller debe liberar *datos_out con free()
// ─────────────────────────────────────────────────────────────────────
int file_LeerArchivoComprimido(const char*      ruta,
                              CabeceraArchivo* cabecera_out,
                              uint8_t**        datos_out,
                              size_t*          tam_out) {
    FILE* f = fopen(ruta, "rb");
    if (!f) {
        fprintf(stderr, "[archivos] No se pudo abrir: %s\n", ruta);
        return 1;
    }

    // Leer y validar cabecera
    if (fread(cabecera_out, sizeof(CabeceraArchivo), 1, f) != 1) {
        fprintf(stderr, "[archivos] Cabecera corrupta o incompleta\n");
        fclose(f);
        return 2;
    }

    // Verificar firma: debe ser "HUFF" o "RLE_"
    if (memcmp(cabecera_out->firma, "HUFF", 4) != 0 &&
        memcmp(cabecera_out->firma, "RLE_", 4) != 0) {
        fprintf(stderr, "[archivos] Firma invalida: no es .huff ni .rle\n");
        fclose(f);
        return 3;
    }

    size_t tam = (size_t)cabecera_out->tam_datos;
    *datos_out = (uint8_t*)malloc(tam);
    if (!*datos_out) {
        fprintf(stderr, "[archivos] Sin memoria para datos comprimidos\n");
        fclose(f);
        return 4;
    }

    size_t leidos = fread(*datos_out, 1, tam, f);
    fclose(f);

    if (leidos != tam) {
        fprintf(stderr, "[archivos] Datos comprimidos incompletos\n");
        free(*datos_out);
        *datos_out = NULL;
        return 5;
    }

    *tam_out = tam;
    return 0;   // OK
}

// ─────────────────────────────────────────────────────────────────────
//  Libera el buffer de una ResultadoLectura
// ─────────────────────────────────────────────────────────────────────
void file_liberar(ResultadoLectura* r) {
    if (r && r->datos) {
        free(r->datos);
        r->datos  = NULL;
        r->tamano = 0;
        r->error  = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Devuelve un puntero a la extensión dentro de la ruta
//  ej: "docs/texto.txt" → ".txt"
// ─────────────────────────────────────────────────────────────────────
const char* file_obtener_extension(const char* ruta) {
    const char* punto = strrchr(ruta, '.');
    return punto ? punto : "";
}