#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

// ─── Nodo del árbol ──────────────────────────────────────────────────
typedef struct NodoH {
    uint64_t     freq;
    uint8_t      caracter;
    int          es_hoja;
    struct NodoH* izq;
    struct NodoH* der;
} NodoH;

// ─── Min-heap ────────────────────────────────────────────────────────
#define MAX_HEAP 256
typedef struct { uint64_t freq; NodoH* nodo; } HeapEntryH;
typedef struct { HeapEntryH e[MAX_HEAP]; int n; } MinHeapH;

static void heap_push(MinHeapH* h, uint64_t f, NodoH* n) {
    int i = h->n++;
    h->e[i].freq = f; h->e[i].nodo = n;
    while (i > 0) {
        int p = (i-1)/2;
        if (h->e[p].freq > h->e[i].freq) {
            HeapEntryH t = h->e[p]; h->e[p] = h->e[i]; h->e[i] = t;
            i = p;
        } else break;
    }
}

static HeapEntryH heap_pop(MinHeapH* h) {
    HeapEntryH r = h->e[0];
    h->e[0] = h->e[--h->n];
    int i = 0;
    while (1) {
        int m = i, l = 2*i+1, ri = 2*i+2;
        if (l < h->n && h->e[l].freq < h->e[m].freq) m = l;
        if (ri < h->n && h->e[ri].freq < h->e[m].freq) m = ri;
        if (m == i) break;
        HeapEntryH t = h->e[i]; h->e[i] = h->e[m]; h->e[m] = t;
        i = m;
    }
    return r;
}

static NodoH* nuevo_nodo(uint8_t c, uint64_t f, NodoH* izq, NodoH* der) {
    NodoH* n = (NodoH*)calloc(1, sizeof(NodoH));
    n->caracter = c; n->freq = f;
    n->es_hoja = (izq == NULL && der == NULL);
    n->izq = izq; n->der = der;
    return n;
}

static void liberar_arbol(NodoH* n) {
    if (!n) return;
    liberar_arbol(n->izq);
    liberar_arbol(n->der);
    free(n);
}

// ─── Recorrido DFS para generar códigos ──────────────────────────────
static void generar_codigos(NodoH* nodo, uint64_t codigo,
                             uint64_t bits, HuffmanCode* tabla) {
    if (!nodo) return;
    if (nodo->es_hoja) {
        tabla[nodo->caracter].codigo = codigo;
        tabla[nodo->caracter].bits   = bits;
        return;
    }
    generar_codigos(nodo->izq, (codigo << 1) | 0, bits + 1, tabla);
    generar_codigos(nodo->der, (codigo << 1) | 1, bits + 1, tabla);
}

// ─── API pública ─────────────────────────────────────────────────────
HuffmanCode* huffman_construir_diccionario(const uint8_t* datos,
                                           size_t         tamano) {
    // Fase 1: contar frecuencias
    uint64_t freq[256] = {0};
    for (size_t i = 0; i < tamano; i++) freq[datos[i]]++;

    // Fase 2: construir min-heap
    MinHeapH heap = {.n = 0};
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            heap_push(&heap, freq[i], nuevo_nodo((uint8_t)i, freq[i], NULL, NULL));

    // Caso especial: un solo símbolo
    if (heap.n == 1) {
        NodoH* u = heap_pop(&heap).nodo;
        heap_push(&heap, u->freq, nuevo_nodo(0, u->freq, u, NULL));
    }

    // Fase 3: construir árbol
    while (heap.n > 1) {
        HeapEntryH a = heap_pop(&heap);
        HeapEntryH b = heap_pop(&heap);
        heap_push(&heap, a.freq + b.freq,
                  nuevo_nodo(0, a.freq + b.freq, a.nodo, b.nodo));
    }

    NodoH* raiz = heap_pop(&heap).nodo;

    // Fase 4: generar diccionario
    HuffmanCode* tabla = (HuffmanCode*)calloc(256, sizeof(HuffmanCode));
    generar_codigos(raiz, 0, 0, tabla);
    liberar_arbol(raiz);

    return tabla;
}

void huffman_escribir_tabla_frecuencias(const uint8_t* datos,
                                        size_t         tamano,
                                        uint8_t*       salida) {
    uint64_t freq[256] = {0};
    for (size_t i = 0; i < tamano; i++) freq[datos[i]]++;
    memcpy(salida, freq, 256 * sizeof(uint64_t));   // 2048 bytes
}