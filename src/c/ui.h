#ifndef UI_H
#define UI_H

void ui_configurar_consola(void);
int  ui_mostrar_menu_principal(void);
void ui_mostrar_submenu_algoritmo(void);
void ui_solicitar_ruta(const char* prompt, char* buffer, int max);
void ui_mostrar_mensaje(const char* msg);
void ui_mostrar_error(const char* msg);
void ui_mostrar_barra_progreso(int porcentaje);
void ui_limpiar_pantalla(void);

#endif