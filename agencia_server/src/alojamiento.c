#include <stdio.h>
#include <string.h>
#include "../include/alojamiento.h"
#include "../include/sqlite3.h"

// GUARDAR ALOJAMIENTO EN SQLITE
int guardarAlojamiento(sqlite3 *db, Alojamiento a) {
    const char *sql =
        "INSERT INTO alojamientos "
        "(codigo, nombre, direccion, tipo, cod_ciudad, activo) "
        "VALUES (?, ?, ?, ?, ?, 1);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando INSERT: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, a.codigo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, a.nombre, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, a.direccion, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, a.tipo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, a.cod_ciudad, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al guardar alojamiento: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

// ALTA
void altaAlojamiento(sqlite3 *db) {
    Alojamiento a;

    printf("Codigo: ");
    scanf("%s", a.codigo);

    printf("Nombre: ");
    scanf(" %[^\n]", a.nombre);

    printf("Direccion: ");
    scanf(" %[^\n]", a.direccion);

    printf("Tipo (hotel, hostal...): ");
    scanf("%s", a.tipo);

    printf("Codigo ciudad: ");
    scanf("%s", a.cod_ciudad);

    a.activo = 1;

    if (guardarAlojamiento(db, a)) {
        printf("Alojamiento dado de alta correctamente.\n");
    } else {
        printf("Hubo un problema al guardar el alojamiento.\n");
    }
}

// BAJA LOGICA
void bajaAlojamiento(sqlite3 *db) {
    char codigo[10];

    printf("Codigo a dar de baja: ");
    scanf("%s", codigo);

    const char *sql =
        "UPDATE alojamientos SET activo = 0 WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al dar de baja alojamiento: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) > 0) {
        printf("Alojamiento dado de baja correctamente.\n");
    } else {
        printf("No encontrado.\n");
    }
}

// CONSULTAR
void consultarAlojamiento(sqlite3 *db) {
    char codigo[10];

    printf("Codigo a consultar: ");
    scanf("%s", codigo);

    const char *sql =
        "SELECT codigo, nombre, direccion, tipo, cod_ciudad "
        "FROM alojamientos WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("\n--- Alojamiento ---\n");
        printf("Codigo: %s\n", sqlite3_column_text(stmt, 0));
        printf("Nombre: %s\n", sqlite3_column_text(stmt, 1));
        printf("Direccion: %s\n", sqlite3_column_text(stmt, 2));
        printf("Tipo: %s\n", sqlite3_column_text(stmt, 3));
        printf("Cod ciudad: %s\n", sqlite3_column_text(stmt, 4));
    } else {
        printf("No encontrado.\n");
    }

    sqlite3_finalize(stmt);
}

// LISTADO
void listadoAlojamientos(sqlite3 *db) {
    const char *sql =
        "SELECT codigo, nombre, direccion, tipo, cod_ciudad "
        "FROM alojamientos WHERE activo = 1 ORDER BY codigo;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- LISTADO DE ALOJAMIENTOS ---\n");

    int encontrados = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        encontrados = 1;

        printf("\nCodigo: %s\n", sqlite3_column_text(stmt, 0));
        printf("Nombre: %s\n", sqlite3_column_text(stmt, 1));
        printf("Direccion: %s\n", sqlite3_column_text(stmt, 2));
        printf("Tipo: %s\n", sqlite3_column_text(stmt, 3));
        printf("Cod ciudad: %s\n", sqlite3_column_text(stmt, 4));
    }

    if (!encontrados) {
        printf("No hay alojamientos registrados.\n");
    }

    sqlite3_finalize(stmt);
}

// MENU ADMIN
void menuAlojamiento(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU ALOJAMIENTOS ---\n");
        printf("1. Alta alojamiento\n");
        printf("2. Baja alojamiento\n");
        printf("3. Consultar alojamiento\n");
        printf("4. Listado alojamientos\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: altaAlojamiento(db); break;
            case 2: bajaAlojamiento(db); break;
            case 3: consultarAlojamiento(db); break;
            case 4: listadoAlojamientos(db); break;
            case 0: printf("Volviendo al menú principal...\n"); break;
            default: printf("Opcion no valida.\n");
        }

    } while (opcion != 0);
}

// MENU PARA CLIENTES
void menuAlojamiento_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU ALOJAMIENTOS ---\n");
        printf("1. Consultar alojamiento\n");
        printf("2. Listado alojamientos\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: consultarAlojamiento(db); break;
            case 2: listadoAlojamientos(db); break;
            case 0: printf("Volviendo al menú principal...\n"); break;
            default: printf("Opcion no valida.\n");
        }

    } while (opcion != 0);
}
