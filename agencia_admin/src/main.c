#include <stdio.h>
#include <string.h> // Necesario para strcmp
#include "../include/config.h"
#include "../include/db.h"
#include "../include/auth.h"
#include "../include/menu.h"

int main() {
    setbuf(stdout, NULL);

    printf("Inicio del programa...\n");

    Config cfg;
    sqlite3 *db;

    // 1. Cargar configuración
    if (cargar_config("data/config.ini", &cfg) != 0) {
        printf("Error cargando configuración\n");
        return 1;
    }

    printf("Config cargada\n");

    // 2. Abrir Base de Datos
    if (db_abrir(&db, cfg.db_path) != 0) {
        printf("Error abriendo base de datos\n");
        return 1;
    }

    printf("BD abierta\n");

    db_crear_tablas(db);

    printf("Entrando en el sistema de acceso...\n");

    /*
       - Si devuelve 1: Es Administrador.
       - Si devuelve 2: Es Cliente.
       - Si devuelve 0: Salir.
    */
    int tipo_usuario = login_admin(db);

    if (tipo_usuario == 0) {
        printf("Saliendo del programa...\n");
        db_cerrar(db);
        return 0;
    }

    // 4. Redirección según el tipo de usuario
    if (tipo_usuario == 1) {
        printf("\nAcceso concedido como ADMINISTRADOR\n");
        mostrar_menu(db);
    }
    else if (tipo_usuario == 2) {
        printf("\nAcceso concedido como CLIENTE\n");
        mostrar_menu_cliente(db);
    }

    // 5. Cierre
    db_cerrar(db);
    printf("Fin del programa\n");

    return 0;
}
