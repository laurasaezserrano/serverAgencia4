#include <stdio.h>
#include <string.h>
#include "../include/paquete.h"

#define ARCHIVO "../agencia_admin/paquetes.dat"

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
    sqlite3_bind_text(stmt,   2, p.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, p.precio);
    sqlite3_bind_text(stmt,   4, p.cod_ciudad_origen, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   5, p.cod_ciudad_destino, -1, SQLITE_STATIC);
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
    FILE *f = fopen(ARCHIVO, "rb+");
    Paquete p;
    int codigo;

    if (f == NULL) return;

    printf("Codigo a eliminar: ");
    scanf("%d", &codigo);

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.cod == codigo && p.activo == 1) {
            p.activo = 0;
            fseek(f, -sizeof(Paquete), SEEK_CUR);
            fwrite(&p, sizeof(Paquete), 1, f);
            printf("Paquete eliminado.\n");
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// CONSULTAR
void consultarPaquete(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    Paquete p;
    int codigo;

    if (f == NULL) return;

    printf("Codigo a consultar: ");
    scanf("%d", &codigo);

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.cod == codigo && p.activo == 1) {
            printf("\n--- PAQUETE ---\n");
            printf("Codigo: %d\n", p.cod);
            printf("Nombre: %s\n", p.nombre);
            printf("Precio: %.2f\n", p.precio);
            printf("Origen: %s\n", p.cod_ciudad_origen);
            printf("Destino: %s\n", p.cod_ciudad_destino);
            printf("Plazas totales: %d\n", p.plazas_totales);
            printf("Plazas disponibles: %d\n", p.plazas_disponibles);
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// LISTADO
void listadoPaquetes(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    Paquete p;

    if (f == NULL) {
        printf("No hay paquetes.\n");
        return;
    }

    printf("\n--- LISTADO DE PAQUETES ---\n");

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.activo == 1) {
            printf("\nCodigo: %d\n", p.cod);
            printf("Nombre: %s\n", p.nombre);
            printf("Precio: %.2f\n", p.precio);
            printf("Origen: %s\n", p.cod_ciudad_origen);
            printf("Destino: %s\n", p.cod_ciudad_destino);
            printf("Plazas totales: %d\n", p.plazas_totales);
            printf("Plazas disponibles: %d\n", p.plazas_disponibles);
        }
    }

    fclose(f);
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
        sqlite3_bind_text(stmt,   2, p.nombre,            -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, (double)p.precio);
        sqlite3_bind_text(stmt,   4, p.cod_ciudad_origen,  -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt,   5, p.cod_ciudad_destino, -1, SQLITE_STATIC);
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
