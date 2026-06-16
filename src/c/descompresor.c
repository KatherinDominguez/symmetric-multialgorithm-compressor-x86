#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "descompresor.h"
#include "archivos.h"

// ═════════════════════════════════════════════════════════════════════
//  DESCOMPRESIÓN RLE
//  Formato de entrada: secuencia de pares [contador:1 byte][valor:1 byte]
// ═════════════════════════════════════════════════════════════════════
ResultadoDescompresion des_expandir_rle(const uint8_t* datos, 
                                         size_t         tam,
                                         size_t         tam_esperado) {
    ResultadoDescompresion res = {NULL, 0, 0};

    // Usar tam_original de la cabecera como tamaño exacto
    res.datos = (uint8_t*)malloc(tam_esperado);
    if (!res.datos) { res.error = 1; return res; }

    size_t i_entrada = 0;
    size_t i_salida  = 0;

    while (i_entrada + 1 < tam && i_salida < tam_esperado) {
        uint8_t contador = datos[i_entrada];
        uint8_t valor    = datos[i_entrada + 1];
        i_entrada += 2;

        for (uint8_t k = 0; k < contador && i_salida < tam_esperado; k++) {
            res.datos[i_salida++] = valor;
        }
    }

    res.tamano = i_salida;
    return res;
}

// ═════════════════════════════════════════════════════════════════════
//  NODO DEL ÁRBOL DE HUFFMAN (solo para descompresión en C)
//  Cuando el Estudiante A entregue mem_AsignarMemoria, esta estructura
//  puede migrar a usarla; por ahora usa malloc de C.
// ═════════════════════════════════════════════════════════════════════
typedef struct NodoHuff {
    uint8_t          caracter;
    int              es_hoja;
    struct NodoHuff* izq;
    struct NodoHuff* der;
} NodoHuff;

// ─── Crea un nodo hoja ───
static NodoHuff* _crear_hoja(uint8_t c) {
    NodoHuff* n = (NodoHuff*)malloc(sizeof(NodoHuff));
    n->caracter = c;
    n->es_hoja  = 1;
    n->izq = n->der = NULL;
    return n;
}

// ─── Crea un nodo interno ───
static NodoHuff* _crear_interno(NodoHuff* izq, NodoHuff* der) {
    NodoHuff* n = (NodoHuff*)malloc(sizeof(NodoHuff));
    n->caracter = 0;
    n->es_hoja  = 0;
    n->izq = izq;
    n->der = der;
    return n;
}

// ─── Libera el árbol recursivamente ───
static void _liberar_arbol(NodoHuff* nodo) {
    if (!nodo) return;
    _liberar_arbol(nodo->izq);
    _liberar_arbol(nodo->der);
    free(nodo);
}

// ─────────────────────────────────────────────────────────────────────
//  Reconstruye el árbol de Huffman desde la tabla de frecuencias
//  La tabla viene en la cabecera: 256 entradas de 8 bytes (uint64_t)
//  = 2048 bytes fijos después de CabeceraArchivo
// ─────────────────────────────────────────────────────────────────────

// Mini min-heap para construir el árbol
#define MAX_HEAP 256

typedef struct {
    uint64_t  freq;
    NodoHuff* nodo;
} HeapEntry;

typedef struct {
    HeapEntry entries[MAX_HEAP];
    int       size;
} MinHeap;

static void _heap_push(MinHeap* h, uint64_t freq, NodoHuff* nodo) {
    int i = h->size++;
    h->entries[i].freq = freq;
    h->entries[i].nodo = nodo;
    // Subir (bubble up)
    while (i > 0) {
        int padre = (i - 1) / 2;
        if (h->entries[padre].freq > h->entries[i].freq) {
            HeapEntry tmp     = h->entries[padre];
            h->entries[padre] = h->entries[i];
            h->entries[i]     = tmp;
            i = padre;
        } else break;
    }
}

static HeapEntry _heap_pop(MinHeap* h) {
    HeapEntry raiz = h->entries[0];
    h->entries[0]  = h->entries[--h->size];
    // Bajar (bubble down)
    int i = 0;
    while (1) {
        int menor = i;
        int l = 2*i+1, r = 2*i+2;
        if (l < h->size && h->entries[l].freq < h->entries[menor].freq) menor = l;
        if (r < h->size && h->entries[r].freq < h->entries[menor].freq) menor = r;
        if (menor == i) break;
        HeapEntry tmp      = h->entries[i];
        h->entries[i]      = h->entries[menor];
        h->entries[menor]  = tmp;
        i = menor;
    }
    return raiz;
}

static NodoHuff* _reconstruir_arbol(const uint64_t* tabla_freq) {
    MinHeap heap = { .size = 0 };

    for (int i = 0; i < 256; i++) {
        if (tabla_freq[i] > 0)
            _heap_push(&heap, tabla_freq[i], _crear_hoja((uint8_t)i));
    }

    // Caso especial: un solo símbolo
    if (heap.size == 1) {
        NodoHuff* unico = _heap_pop(&heap).nodo;
        return _crear_interno(unico, NULL);
    }

    while (heap.size > 1) {
        HeapEntry a = _heap_pop(&heap);
        HeapEntry b = _heap_pop(&heap);
        _heap_push(&heap, a.freq + b.freq, _crear_interno(a.nodo, b.nodo));
    }

    return _heap_pop(&heap).nodo;
}

// ═════════════════════════════════════════════════════════════════════
//  DESCOMPRESIÓN HUFFMAN
//  datos          → bitstream comprimido (viene después de la cabecera)
//  cabecera_extra → los 2048 bytes de tabla de frecuencias
// ═════════════════════════════════════════════════════════════════════
ResultadoDescompresion des_expandir_huffman(const uint8_t* datos,
                                             size_t         tam,
                                             const uint8_t* cabecera_extra,
                                             size_t         tam_cabecera) {
    ResultadoDescompresion res = {NULL, 0, 0};

    if (tam_cabecera < 256 * sizeof(uint64_t)) {
        fprintf(stderr, "[des_huffman] Tabla de frecuencias incompleta\n");
        res.error = 1;
        return res;
    }

    // Reconstruir árbol desde la tabla de frecuencias
    const uint64_t* tabla = (const uint64_t*)cabecera_extra;
    NodoHuff* raiz = _reconstruir_arbol(tabla);
    if (!raiz) { res.error = 2; return res; }

    // Calcular cuántos bytes originales esperar
    uint64_t total_bytes = 0;
    for (int i = 0; i < 256; i++) total_bytes += tabla[i];

    res.datos = (uint8_t*)malloc(total_bytes + 1);
    if (!res.datos) {
        _liberar_arbol(raiz);
        res.error = 3;
        return res;
    }

    // ── Recorrido bit a bit del bitstream ──────────────────────────
    NodoHuff* actual    = raiz;
    size_t    i_salida  = 0;

    for (size_t byte_idx = 0; byte_idx < tam && i_salida < total_bytes; byte_idx++) {
        uint8_t byte_actual = datos[byte_idx];

        for (int bit = 7; bit >= 0 && i_salida < total_bytes; bit--) {
            // Extraer bit: 1 → derecha, 0 → izquierda
            int ir_derecha = (byte_actual >> bit) & 1;

            actual = ir_derecha ? actual->der : actual->izq;

            if (!actual) {
                fprintf(stderr, "[des_huffman] Puntero nulo en el arbol\n");
                res.error = 4;
                _liberar_arbol(raiz);
                return res;
            }

            // Llegamos a una hoja → carácter recuperado
            if (actual->es_hoja) {
                res.datos[i_salida++] = actual->caracter;
                actual = raiz;   // volver a la raíz para el siguiente código
            }
        }
    }

    res.tamano = i_salida;
    _liberar_arbol(raiz);
    return res;
}

// ═════════════════════════════════════════════════════════════════════
//  PUNTO DE ENTRADA PRINCIPAL DE DESCOMPRESIÓN
//  Lee el archivo, detecta el algoritmo por la firma y delega
// ═════════════════════════════════════════════════════════════════════
ResultadoDescompresion des_DescomprimirFlujo(const char* ruta_entrada) {
    ResultadoDescompresion res = {NULL, 0, 0};

    CabeceraArchivo cabecera;
    uint8_t*        datos_comprimidos = NULL;
    size_t          tam_comprimidos   = 0;

    int err = file_LeerArchivoComprimido(ruta_entrada, &cabecera,
                                       &datos_comprimidos, &tam_comprimidos);
    if (err) {
        res.error = err;
        return res;
    }

    printf("[des] Firma detectada : %.4s\n",  cabecera.firma);
    printf("[des] Extension orig  : %s\n",    cabecera.extension);
    printf("[des] Tam original    : %llu B\n", cabecera.tam_original);
    printf("[des] Tam comprimido  : %llu B\n", cabecera.tam_datos);

    // ── Delegar según algoritmo ────────────────────────────────────
    if (memcmp(cabecera.firma, "RLE_", 4) == 0) {
        res = des_expandir_rle(datos_comprimidos, tam_comprimidos, cabecera.tam_original);

    } else if (memcmp(cabecera.firma, "HUFF", 4) == 0) {
        // Los primeros 2048 bytes del bloque de datos son la tabla de frecuencias
        size_t tam_tabla   = 256 * sizeof(uint64_t);   // 2048 bytes
        uint8_t* bitstream = datos_comprimidos + tam_tabla;
        size_t   tam_bits  = tam_comprimidos   - tam_tabla;

        res = des_expandir_huffman(bitstream, tam_bits,
                                   datos_comprimidos, tam_tabla);
    } else {
        fprintf(stderr, "[des] Algoritmo desconocido\n");
        res.error = 10;
    }

    free(datos_comprimidos);
    return res;
}

// ─────────────────────────────────────────────────────────────────────
void des_liberar(ResultadoDescompresion* r) {
    if (r && r->datos) {
        free(r->datos);
        r->datos  = NULL;
        r->tamano = 0;
    }
}