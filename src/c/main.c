// =========================================================
// Archivo: main.c
// =========================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "ui.h"
#include "archivos.h"
#include "descompresor.h"
#include "estadisticas.h"
#include "huffman.h"
#include "../../include/contratos.h"

int main(void) {
    ui_configurar_consola();

    int opcion = 0;
    char ruta_entrada[480];
    char ruta_salida[496];

    do {
        opcion = ui_mostrar_menu_principal();

        switch (opcion) {

            // ─────────────────────────────────────────────────────────
            // Caso 1: Comprimir archivo
            // ─────────────────────────────────────────────────────────
            case 1: {
                ui_solicitar_ruta("Archivo a comprimir",
                                  ruta_entrada, sizeof(ruta_entrada));

                // --- Lectura del archivo original ---
                ResultadoLectura archivo = file_LeerArchivoCompleto(ruta_entrada);
                if (archivo.error != 0) {
                    ui_mostrar_error("No se pudo leer el archivo.");
                    break;
                }

                // --- Selección de algoritmo de compresión ---
                ui_mostrar_submenu_algoritmo();
                int algoritmo = 0;
                scanf("%d", &algoritmo);
                while (getchar() != '\n');

                // --- Reserva del buffer de salida (margen x2 + tabla Huffman) ---
                uint8_t* buffer_salida = (uint8_t*)malloc(archivo.tamano * 2 + 2048);
                if (!buffer_salida) {
                    ui_mostrar_error("Sin memoria para comprimir.");
                    file_liberar(&archivo);
                    break;
                }

                // --- Inicio de medición de tiempo ---
                uint64_t t_inicio = fpu_obtener_timestamp();

                // --- Barra de progreso visual ---
                printf("\n    Procesando...\n    ");
                for (int p = 0; p <= 100; p += 10) {
                    ui_mostrar_barra_progreso(p);
                    Sleep(50);
                }
                printf("\n");

                // --- Preparación de cabecera y rutas ---
                size_t tam_comprimido = 0;
                CabeceraArchivo cab;
                memset(&cab, 0, sizeof(cab));

                const char* ext = file_obtener_extension(ruta_entrada);
                char ruta_sin_ext[480] = {0};
                snprintf(ruta_sin_ext, sizeof(ruta_sin_ext), "%s", ruta_entrada);
                char* ultimo_punto = strrchr(ruta_sin_ext, '.');
                if (ultimo_punto) *ultimo_punto = '\0';

                if (algoritmo == 2) {
                    // ── Huffman ───────────────────────────────────────
                    memcpy(cab.firma, "HUFF", 4);

                    // Paso 1: Construir diccionario de códigos
                    HuffmanCode* diccionario = huffman_construir_diccionario(
                        archivo.datos, archivo.tamano);
                    if (!diccionario) {
                        ui_mostrar_error("Sin memoria para diccionario Huffman.");
                        free(buffer_salida);
                        file_liberar(&archivo);
                        break;
                    }

                    // Paso 2: Escribir tabla de frecuencias en cabecera extra (2048 B)
                    huffman_escribir_tabla_frecuencias(
                        archivo.datos, archivo.tamano, buffer_salida);

                    // Paso 3: Comprimir bitstream con el módulo ASM
                    size_t tam_bits = comp_ComprimirBufferHuffman(
                        archivo.datos,
                        archivo.tamano,
                        buffer_salida + 2048,
                        (uint8_t*)diccionario
                    );

                    tam_comprimido = 2048 + tam_bits;
                    snprintf(ruta_salida, sizeof(ruta_salida),
                             "%s_comprimido.huff", ruta_sin_ext);
                    free(diccionario);

                } else {
                    // ── RLE ───────────────────────────────────────────
                    memcpy(cab.firma, "RLE_", 4);
                    tam_comprimido = comp_ComprimirBufferRLE(
                        archivo.datos, archivo.tamano, buffer_salida);
                    snprintf(ruta_salida, sizeof(ruta_salida),
                             "%s_comprimido.rle", ruta_sin_ext);
                }

                // --- Fin de medición de tiempo ---
                uint64_t t_fin         = fpu_obtener_timestamp();
                uint64_t microsegundos = t_fin - t_inicio;

                // --- Completar cabecera y escribir archivo comprimido ---
                strncpy(cab.extension, ext, 15);
                cab.extension[15]  = '\0';
                cab.ext_len        = (uint32_t)strlen(ext);
                cab.tam_original   = (uint64_t)archivo.tamano;
                cab.tam_datos      = (uint64_t)tam_comprimido;

                int err = file_EscribirArchivoComprimido(
                              ruta_salida, &cab,
                              buffer_salida, tam_comprimido);

                // --- Mostrar resultado y estadísticas ---
                if (err != 0) {
                    ui_mostrar_error("No se pudo guardar el archivo comprimido.");
                } else {
                    printf("\n    Archivo guardado en:\n    %s\n", ruta_salida);
                    ResultadoEstadisticas est = fpu_CalcularEstadisticas(
                        archivo.tamano, tam_comprimido, microsegundos);
                    fpu_mostrar_estadisticas(&est);
                }

                free(buffer_salida);
                file_liberar(&archivo);
                ui_mostrar_mensaje("Compresion completada.");
                break;
            }

            // ─────────────────────────────────────────────────────────
            // Caso 2: Descomprimir archivo
            // ─────────────────────────────────────────────────────────
            case 2: {
                ui_solicitar_ruta("Archivo a descomprimir (.huff o .rle)",
                                  ruta_entrada, sizeof(ruta_entrada));

                // --- Lectura de cabecera para obtener extensión original ---
                CabeceraArchivo cab;
                uint8_t* datos_comp = NULL;
                size_t   tam_comp   = 0;

                int err = file_LeerArchivoComprimido(
                              ruta_entrada, &cab,
                              &datos_comp, &tam_comp);
                if (err != 0) {
                    ui_mostrar_error("No se pudo leer el archivo comprimido.");
                    break;
                }

                // --- Barra de progreso visual ---
                printf("\n    Procesando...\n    ");
                for (int p = 0; p <= 100; p += 10) {
                    ui_mostrar_barra_progreso(p);
                    Sleep(50);
                }
                printf("\n");

                // --- Descompresión y medición de tiempo ---
                uint64_t t_inicio      = fpu_obtener_timestamp();
                ResultadoDescompresion resultado = des_DescomprimirFlujo(ruta_entrada);
                uint64_t t_fin         = fpu_obtener_timestamp();
                uint64_t microsegundos = t_fin - t_inicio;

                free(datos_comp);

                if (resultado.error != 0) {
                    ui_mostrar_error("No se pudo descomprimir el archivo.");
                    break;
                }

                // --- Reconstrucción de la ruta de salida ---
                char nombre_base[480] = {0};
                snprintf(nombre_base, sizeof(nombre_base), "%s", ruta_entrada);

                char* ultimo_punto = strrchr(nombre_base, '.');
                if (ultimo_punto) *ultimo_punto = '\0';

                char* sufijo = strstr(nombre_base, "_comprimido");
                if (sufijo) *sufijo = '\0';

                snprintf(ruta_salida, sizeof(ruta_salida),
                         "%s_restaurado%s", nombre_base, cab.extension);

                // --- Escritura del archivo restaurado ---
                FILE* f_out = fopen(ruta_salida, "wb");
                if (!f_out) {
                    ui_mostrar_error("No se pudo crear el archivo restaurado.");
                    des_liberar(&resultado);
                    break;
                }

                fwrite(resultado.datos, 1, resultado.tamano, f_out);
                fclose(f_out);

                // --- Mostrar resultado y estadísticas ---
                printf("\n    Archivo restaurado en:\n    %s\n", ruta_salida);

                ResultadoEstadisticas est = fpu_CalcularEstadisticas(
                    cab.tam_original, cab.tam_datos, microsegundos);
                fpu_mostrar_estadisticas(&est);

                des_liberar(&resultado);
                ui_mostrar_mensaje("Descompresion completada.");
                break;
            }

            // ─────────────────────────────────────────────────────────
            // Caso 3: Salir del programa
            // ─────────────────────────────────────────────────────────
            case 3:
                ui_mostrar_mensaje("Saliendo...");
                break;

            // ─────────────────────────────────────────────────────────
            // Opción inválida
            // ─────────────────────────────────────────────────────────
            default:
                ui_mostrar_error("Opcion invalida. Intente de nuevo.");
                break;
        }

    } while (opcion != 3);

    return 0;
}