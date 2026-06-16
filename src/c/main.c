#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "ui.h"
#include "archivos.h"
#include "descompresor.h"
#include "estadisticas.h"
#include "../../include/contratos.h"

int main(void) {
    ui_configurar_consola();
    ui_mostrar_modulos_pendientes();

    int opcion = 0;
    char ruta_entrada[480];
    char ruta_salida[496];

    do {
        opcion = ui_mostrar_menu_principal();

        switch (opcion) {

            // ─────────────────────────────────────────────────────────
            case 1: {
                ui_solicitar_ruta("Archivo a comprimir",
                                  ruta_entrada, sizeof(ruta_entrada));

                // Leer archivo original
                ResultadoLectura archivo = file_LeerArchivoCompleto(ruta_entrada);
                if (archivo.error != 0) {
                    ui_mostrar_error("No se pudo leer el archivo.");
                    break;
                }

                // Preguntar algoritmo
                ui_mostrar_submenu_algoritmo();
                int algoritmo = 0;
                scanf("%d", &algoritmo);
                while (getchar() != '\n');

                // Si elige Huffman, avisar que está pendiente y forzar RLE
                if (algoritmo == 2) {
                    ui_mostrar_modulos_pendientes();
                    algoritmo = 1;
                }

                // Reservar buffer de salida
                uint8_t* buffer_salida = (uint8_t*)malloc(archivo.tamano * 2);
                if (!buffer_salida) {
                    ui_mostrar_error("Sin memoria para comprimir.");
                    file_liberar(&archivo);
                    break;
                }

                // Tomar tiempo ANTES
                uint64_t t_inicio = fpu_obtener_timestamp();

                // Barra de progreso visual
                printf("\n    Procesando...\n    ");
                for (int p = 0; p <= 100; p += 10) {
                    ui_mostrar_barra_progreso(p);
                    Sleep(50);
                }
                printf("\n");

                // Comprimir
                size_t tam_comprimido = 0;
                CabeceraArchivo cab;
                memset(&cab, 0, sizeof(cab));

                const char* ext = file_obtener_extension(ruta_entrada);

                // Construir nombre: quitar extensión original y agregar _comprimido.rle
                // Ejemplo: "C:\Downloads\foto.bmp" → "C:\Downloads\foto_comprimido.rle"
                char ruta_sin_ext[480] = {0};
                snprintf(ruta_sin_ext, sizeof(ruta_sin_ext), "%s", ruta_entrada);

                char* ultimo_punto = strrchr(ruta_sin_ext, '.');
                if (ultimo_punto) *ultimo_punto = '\0';

                // TEMPORAL: solo RLE hasta que Estudiante C entregue módulo real
                memcpy(cab.firma, "RLE_", 4);
                tam_comprimido = comp_ComprimirBufferRLE(
                    archivo.datos, archivo.tamano, buffer_salida);
                snprintf(ruta_salida, sizeof(ruta_salida),
                         "%s_comprimido.rle", ruta_sin_ext);

                // TODO: descomentar cuando Estudiante C entregue su módulo
                // if (algoritmo == 1) {
                //     memcpy(cab.firma, "HUFF", 4);
                //     tam_comprimido = comp_ComprimirBufferHuffman(
                //         archivo.datos, archivo.tamano, buffer_salida);
                //     snprintf(ruta_salida, sizeof(ruta_salida),
                //              "%s_comprimido.huff", ruta_sin_ext);
                // } else {
                //     memcpy(cab.firma, "RLE_", 4);
                //     tam_comprimido = comp_ComprimirBufferRLE(
                //         archivo.datos, archivo.tamano, buffer_salida);
                //     snprintf(ruta_salida, sizeof(ruta_salida),
                //              "%s_comprimido.rle", ruta_sin_ext);
                // }

                // Silenciar warning mientras el if/else está comentado
                (void)algoritmo;

                // Tomar tiempo DESPUÉS
                uint64_t t_fin         = fpu_obtener_timestamp();
                uint64_t microsegundos = t_fin - t_inicio;

                // Completar cabecera
                strncpy(cab.extension, ext, 15);
                cab.extension[15]  = '\0';
                cab.ext_len        = (uint32_t)strlen(ext);
                cab.tam_original   = (uint64_t)archivo.tamano;
                cab.tam_datos      = (uint64_t)tam_comprimido;

                // Guardar archivo comprimido
                int err = file_EscribirArchivoComprimido(
                              ruta_salida, &cab,
                              buffer_salida, tam_comprimido);

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
            case 2: {
                ui_solicitar_ruta("Archivo a descomprimir (.huff o .rle)",
                                  ruta_entrada, sizeof(ruta_entrada));

                // Leer cabecera para obtener extensión original y tamaños
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

                // Barra de progreso visual
                printf("\n    Procesando...\n    ");
                for (int p = 0; p <= 100; p += 10) {
                    ui_mostrar_barra_progreso(p);
                    Sleep(50);
                }
                printf("\n");

                // Descomprimir
                uint64_t t_inicio      = fpu_obtener_timestamp();
                ResultadoDescompresion resultado = des_DescomprimirFlujo(ruta_entrada);
                uint64_t t_fin         = fpu_obtener_timestamp();
                uint64_t microsegundos = t_fin - t_inicio;

                free(datos_comp);

                if (resultado.error != 0) {
                    ui_mostrar_error("No se pudo descomprimir el archivo.");
                    break;
                }

                // Construir nombre único basado en el archivo comprimido
                // Ejemplo: "C:\Downloads\foto_comprimido.rle" → "C:\Downloads\foto_restaurado.bmp"
                char nombre_base[480] = {0};
                snprintf(nombre_base, sizeof(nombre_base), "%s", ruta_entrada);

                // Quitar extensión .rle o .huff
                char* ultimo_punto = strrchr(nombre_base, '.');
                if (ultimo_punto) *ultimo_punto = '\0';

                // Quitar sufijo "_comprimido" si existe
                char* sufijo = strstr(nombre_base, "_comprimido");
                if (sufijo) *sufijo = '\0';

                // Construir ruta final con _restaurado + extensión original
                snprintf(ruta_salida, sizeof(ruta_salida),
                         "%s_restaurado%s", nombre_base, cab.extension);

                // Guardar archivo restaurado
                FILE* f_out = fopen(ruta_salida, "wb");
                if (!f_out) {
                    ui_mostrar_error("No se pudo crear el archivo restaurado.");
                    des_liberar(&resultado);
                    break;
                }

                fwrite(resultado.datos, 1, resultado.tamano, f_out);
                fclose(f_out);

                printf("\n    Archivo restaurado en:\n    %s\n", ruta_salida);

                ResultadoEstadisticas est = fpu_CalcularEstadisticas(
                    cab.tam_original, cab.tam_datos, microsegundos);
                fpu_mostrar_estadisticas(&est);

                des_liberar(&resultado);
                ui_mostrar_mensaje("Descompresion completada.");
                break;
            }

            // ─────────────────────────────────────────────────────────
            case 3:
                ui_mostrar_mensaje("Saliendo...");
                break;

            default:
                ui_mostrar_error("Opcion invalida. Intente de nuevo.");
                break;
        }

    } while (opcion != 3);

    return 0;
}