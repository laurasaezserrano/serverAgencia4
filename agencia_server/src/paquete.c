#include <stdio.h>
#include <string.h>
#include "../include/paquete.h"


// Guardar paquete
int guardarPaquete(sqlite3 *db, Paquete p) {

    const char *sql =
        "INSERT INTO paquetes "
        "(codigo, nombre, precio, origen, destino, plazas_totales, plazas_disponibles, activo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 1);";

    sqlite3_stmt *stmt;

    // Preparar consulta
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error al preparar INSERT: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Asignar valores
    sqlite3_bind_int(stmt,    1, p.cod);
    sqlite3_bind_text(stmt,   2, p.nombre, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, p.precio);
    sqlite3_bind_text(stmt,   4, p.cod_ciudad_origen, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   5, p.cod_ciudad_destino, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,    6, p.plazas_totales);
    sqlite3_bind_int(stmt,    7, p.plazas_disponibles);

    // Ejecutar INSERT
    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("Error al guardar paquete: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);

    return 1;
}



// CREAR
void crearPaquete(sqlite3 *db) {
    Paquete p;

    printf("Codigo: ");
    scanf("%d", &p.cod);

    printf("Nombre: ");
    scanf(" %[^\n]", p.nombre);

    printf("Precio: ");
    scanf("%f", &p.precio);

    printf("Codigo ciudad origen: ");
    scanf("%s", p.cod_ciudad_origen);

    printf("Codigo ciudad destino: ");
    scanf("%s", p.cod_ciudad_destino);

    printf("Plazas totales: ");
    scanf("%d", &p.plazas_totales);

    p.plazas_disponibles = p.plazas_totales;
    p.activo = 1;

    if (guardarPaquete(db, p)) {
        printf("Paquete guardado correctamente.\n");
    } else {
        printf("Hubo un problema al guardar el paquete. Inténtelo de nuevo.\n");
    }

    printf("Paquete creado correctamente.\n");
}

// ELIMINAR (baja lógica)
void eliminarPaquete(sqlite3 *db) {
    int codigo;

    printf("Codigo a eliminar: ");
    scanf("%d", &codigo);

    const char *sql =
        "UPDATE paquetes SET activo = 0 WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, codigo);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al eliminar paquete: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) > 0) {
        printf("Paquete eliminado correctamente.\n");
    } else {
        printf("No se encontro ningun paquete activo con ese codigo.\n");
    }
}

// CONSULTAR
// CONSULTAR PAQUETE EN SQLITE
void consultarPaquete(sqlite3 *db) {
    int codigo;

    printf("Codigo a consultar: ");
    scanf("%d", &codigo);

    const char *sql =
        "SELECT codigo, nombre, precio, origen, destino, plazas_totales, plazas_disponibles "
        "FROM paquetes WHERE codigo = ? AND activo = 1;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, codigo);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("\n--- PAQUETE ---\n");
        printf("Codigo: %d\n", sqlite3_column_int(stmt, 0));
        printf("Nombre: %s\n", sqlite3_column_text(stmt, 1));
        printf("Precio: %.2f\n", sqlite3_column_double(stmt, 2));
        printf("Origen: %s\n", sqlite3_column_text(stmt, 3));
        printf("Destino: %s\n", sqlite3_column_text(stmt, 4));
        printf("Plazas totales: %d\n", sqlite3_column_int(stmt, 5));
        printf("Plazas disponibles: %d\n", sqlite3_column_int(stmt, 6));
    } else {
        printf("No encontrado.\n");
    }

    sqlite3_finalize(stmt);
}

// LISTADO
// LISTADO DE PAQUETES EN SQLITE
void listadoPaquetes(sqlite3 *db) {
    const char *sql =
        "SELECT codigo, nombre, precio, origen, destino, plazas_totales, plazas_disponibles "
        "FROM paquetes WHERE activo = 1 ORDER BY codigo;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- LISTADO DE PAQUETES ---\n");

    int encontrados = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        encontrados = 1;

        printf("\nCodigo: %d\n", sqlite3_column_int(stmt, 0));
        printf("Nombre: %s\n", sqlite3_column_text(stmt, 1));
        printf("Precio: %.2f\n", sqlite3_column_double(stmt, 2));
        printf("Origen: %s\n", sqlite3_column_text(stmt, 3));
        printf("Destino: %s\n", sqlite3_column_text(stmt, 4));
        printf("Plazas totales: %d\n", sqlite3_column_int(stmt, 5));
        printf("Plazas disponibles: %d\n", sqlite3_column_int(stmt, 6));
    }

    if (!encontrados) {
        printf("No hay paquetes registrados.\n");
    }

    sqlite3_finalize(stmt);
}
// MENU ADMIN
void menuPaquetes(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU PAQUETES ---\n");
        printf("1. Crear paquete\n");
        printf("2. Eliminar paquete\n");
        printf("3. Consultar paquete\n");
        printf("4. Listado paquetes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: crearPaquete(db); break;
            case 2: eliminarPaquete(db); break;
            case 3: consultarPaquete(db); break;
            case 4: listadoPaquetes(db); break;
            case 0: printf("Volviendo al menú principal...\n"); break;
            default: printf("Opción no válida.\n");
        }

    } while (opcion != 0);
}

//MENU PARA CLIENTES
void menuPaquetes_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU PAQUETES ---\n");
        printf("1. Consultar paquete\n");
        printf("2. Listado paquetes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: consultarPaquete(db); break;
            case 2: listadoPaquetes(db); break;
            case 0: printf("Volviendo al menú principal...\n"); break;
            default: printf("Opción no válida.\n");
        }

    } while (opcion != 0);
}


void importar_paquetes_dat(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    if (f == NULL) {
        printf("No se encontro paquetes.dat\n");
        return;
    }

    const char *sql =
        "INSERT OR IGNORE INTO paquetes "
        "(codigo, nombre, precio, origen, destino, plazas_totales, plazas_disponibles, activo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 1);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando INSERT: %s\n", sqlite3_errmsg(db));
        fclose(f);
        return;
    }

    Paquete p;
    int importados = 0;

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.activo != 1) continue;  // saltar los dados de baja

        sqlite3_bind_int(stmt,    1, p.cod);
        sqlite3_bind_text(stmt,   2, p.nombre,            -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, (double)p.precio);
        sqlite3_bind_text(stmt,   4, p.cod_ciudad_origen,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,   5, p.cod_ciudad_destino, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt,    6, p.plazas_totales);
        sqlite3_bind_int(stmt,    7, p.plazas_disponibles);

        if (sqlite3_step(stmt) == SQLITE_DONE)
            importados++;
        else
            printf("Error insertando paquete %d: %s\n", p.cod, sqlite3_errmsg(db));

        sqlite3_reset(stmt);  // resetear para la siguiente fila
    }

    sqlite3_finalize(stmt);
    fclose(f);
    printf("%d paquetes importados correctamente.\n", importados);
}
