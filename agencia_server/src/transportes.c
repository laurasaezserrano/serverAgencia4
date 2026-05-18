#include <stdio.h>
#include <string.h>
#include "../include/transporte.h"


// GUARDAR TRANSPORTE EN SQLITE
int guardarTransporte(sqlite3 *db, Transporte t) {
    const char *sql =
        "INSERT INTO transportes "
        "(codigo, tipo, fecha_salida, fecha_llegada, id_paquete, activo) "
        "VALUES (?, ?, ?, ?, ?, 1);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando INSERT: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, t.codigo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, t.tipo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, t.fecha_salida, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, t.fecha_llegada, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, t.cod_paquete);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al guardar transporte: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}
// ALTA
void altaTransporte(sqlite3 *db) {
    Transporte t;

    printf("Codigo: ");
    scanf("%s", t.codigo);

    printf("Tipo (avion, tren, autobus...): ");
    scanf("%s", t.tipo);

    printf("Fecha salida (dd/mm/yyyy): ");
    scanf("%s", t.fecha_salida);

    printf("Fecha llegada (dd/mm/yyyy): ");
    scanf("%s", t.fecha_llegada);

    printf("Codigo de paquete: ");
    scanf("%d", &t.cod_paquete);

    t.activo = 1;

    if (guardarTransporte(db, t)) {
        printf("Transporte dado de alta correctamente.\n");
    } else {
        printf("Hubo un problema al guardar el transporte.\n");
    }
}


// BAJA LOGICA
void bajaTransporte(sqlite3 *db) {
    char codigo[10];

    printf("Codigo a dar de baja: ");
    scanf("%s", codigo);

    const char *sql =
        "UPDATE transportes SET activo = 0 WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al dar de baja transporte: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) > 0) {
        printf("Transporte dado de baja correctamente.\n");
    } else {
        printf("No encontrado.\n");
    }
}

// CONSULTAR
void consultarTransporte(sqlite3 *db) {
    char codigo[10];

    printf("Codigo a consultar: ");
    scanf("%s", codigo);

    const char *sql =
        "SELECT codigo, tipo, fecha_salida, fecha_llegada, id_paquete "
        "FROM transportes WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("\n--- Transporte ---\n");
        printf("Codigo: %s\n", sqlite3_column_text(stmt, 0));
        printf("Tipo: %s\n", sqlite3_column_text(stmt, 1));
        printf("Salida: %s\n", sqlite3_column_text(stmt, 2));
        printf("Llegada: %s\n", sqlite3_column_text(stmt, 3));
        printf("Cod paquete: %d\n", sqlite3_column_int(stmt, 4));
    } else {
        printf("No encontrado.\n");
    }

    sqlite3_finalize(stmt);
}

// ASOCIAR TRANSPORTE A PAQUETE
void asociarTransporte(sqlite3 *db) {
    char codigo[10];
    int nuevoCod;

    printf("Codigo transporte: ");
    scanf("%s", codigo);

    printf("Nuevo codigo de paquete: ");
    scanf("%d", &nuevoCod);

    const char *sql =
        "UPDATE transportes SET id_paquete = ? WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, nuevoCod);
    sqlite3_bind_text(stmt, 2, codigo, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al asociar transporte: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) > 0) {
        printf("Asociacion realizada.\n");
    } else {
        printf("No encontrado.\n");
    }
}


// LISTADO
void listadoTransportes(sqlite3 *db) {
    const char *sql =
        "SELECT codigo, tipo, fecha_salida, fecha_llegada, id_paquete "
        "FROM transportes WHERE activo = 1 ORDER BY codigo;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- LISTADO DE TRANSPORTES ---\n");

    int encontrados = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        encontrados = 1;

        printf("\nCodigo: %s\n", sqlite3_column_text(stmt, 0));
        printf("Tipo: %s\n", sqlite3_column_text(stmt, 1));
        printf("Salida: %s\n", sqlite3_column_text(stmt, 2));
        printf("Llegada: %s\n", sqlite3_column_text(stmt, 3));
        printf("Cod paquete: %d\n", sqlite3_column_int(stmt, 4));
    }

    if (!encontrados) {
        printf("No hay transportes registrados.\n");
    }

    sqlite3_finalize(stmt);
}

// MENU ADMIN
void menuTransporte(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU TRANSPORTES ---\n");
        printf("1. Alta transporte\n");
        printf("2. Baja transporte\n");
        printf("3. Consultar transporte\n");
        printf("4. Asociar transporte\n");
        printf("5. Listado transportes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: altaTransporte(db); break;
            case 2: bajaTransporte(db); break;
            case 3: consultarTransporte(db); break;
            case 4: asociarTransporte(db); break;
            case 5: listadoTransportes(db); break;
        }

    } while (opcion != 0);
}

// MENU PARA CLIENTES
void menuTransporte_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU TRANSPORTES ---\n");
        printf("1. Consultar transporte\n");
        printf("2. Listado transportes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: consultarTransporte(db); break;
            case 2: listadoTransportes(db); break;
        }

    } while (opcion != 0);
}
