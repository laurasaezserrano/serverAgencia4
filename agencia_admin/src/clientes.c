#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/clientes.h"
#include "../include/sqlite3.h"

#define MAX_INPUT 256

static void limpiar_buffer(void);
static char *leer_cadena(const char *mensaje);
static int   leer_entero(const char *mensaje);
static int   validar_dni(const char *dni);
static int   validar_email(const char *email);
static int   validar_fecha(const char *fecha);
static int   existe_cliente_dni(sqlite3 *db, const char *dni);
static int   obtener_cliente_por_dni(sqlite3 *db, const char *dni, Cliente *c);
static void  mostrar_cliente(const Cliente *c);
static void  pausar(void);

/* =========================================================
   GESTION DE MEMORIA
   ========================================================= */

void cliente_free(Cliente *c) {
    if (c == NULL) return;
    free(c->dni);            c->dni            = NULL;
    free(c->nombre);         c->nombre         = NULL;
    free(c->apellidos);      c->apellidos      = NULL;
    free(c->telefono);       c->telefono       = NULL;
    free(c->email);          c->email          = NULL;
    free(c->fecha_nacimiento); c->fecha_nacimiento = NULL;
}

/* =========================================================
   MENU
   ========================================================= */

void menu_clientes(sqlite3 *db) {
    int opcion;

    do {
        printf("\n=====================================\n");
        printf("         GESTION DE CLIENTES\n");
        printf("=====================================\n");
        printf("1. Alta cliente\n");
        printf("2. Baja cliente\n");
        printf("3. Modificar cliente\n");
        printf("4. Buscar cliente por DNI\n");
        printf("5. Listar clientes\n");
        printf("0. Volver\n");
        printf("=====================================\n");

        opcion = leer_entero("Seleccione una opcion: ");

        switch (opcion) {
            case 1:
                alta_cliente(db);
                pausar();
                break;
            case 2:
                baja_cliente(db);
                pausar();
                break;
            case 3:
                modificar_cliente(db);
                pausar();
                break;
            case 4: {
                char *dni = leer_cadena("Introduzca DNI: ");
                if (dni) {
                    buscar_cliente_por_dni(db, dni);
                    free(dni);
                }
                pausar();
                break;
            }
            case 5:
                listar_clientes(db);
                pausar();
                break;
            case 0:
                printf("Volviendo al menu principal...\n");
                break;
            default:
                printf("Opcion no valida.\n");
                pausar();
        }

    } while (opcion != 0);
}

/* =========================================================
   ALTA
   ========================================================= */

int alta_cliente(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO clientes "
        "(dni, nombre, apellidos, telefono, email, fecha_nacimiento, activo) "
        "VALUES (?, ?, ?, ?, ?, ?, 1);";

    Cliente c;
    memset(&c, 0, sizeof(c));
    int rc = 1;

    printf("\n--- ALTA DE CLIENTE ---\n");

    c.dni = leer_cadena("DNI: ");
    if (!c.dni || !validar_dni(c.dni)) {
        printf("Error: DNI no valido.\n");
        goto cleanup;
    }

    if (existe_cliente_dni(db, c.dni)) {
        printf("Error: ya existe un cliente con ese DNI.\n");
        goto cleanup;
    }

    c.nombre    = leer_cadena("Nombre: ");
    c.apellidos = leer_cadena("Apellidos: ");
    c.telefono  = leer_cadena("Telefono: ");
    c.email     = leer_cadena("Email: ");
    c.fecha_nacimiento = leer_cadena("Fecha nacimiento (YYYY-MM-DD): ");

    if (!c.nombre || strlen(c.nombre) == 0 || !c.apellidos || strlen(c.apellidos) == 0) {
        printf("Error: nombre y apellidos son obligatorios.\n");
        goto cleanup;
    }

    if (!c.email || !validar_email(c.email)) {
        printf("Error: email no valido.\n");
        goto cleanup;
    }

    if (!c.fecha_nacimiento || !validar_fecha(c.fecha_nacimiento)) {
        printf("Error: fecha no valida. Use YYYY-MM-DD.\n");
        goto cleanup;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando INSERT: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    sqlite3_bind_text(stmt, 1, c.dni,              -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c.nombre,            -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c.apellidos,         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c.telefono,          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, c.email,             -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, c.fecha_nacimiento,  -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error insertando cliente: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        goto cleanup;
    }

    sqlite3_finalize(stmt);
    printf("Cliente dado de alta correctamente.\n");
    rc = 0;

cleanup:
    cliente_free(&c);
    return rc;
}

/* =========================================================
   BAJA
   ========================================================= */

int baja_cliente(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    const char *sql_buscar = "SELECT activo FROM clientes WHERE dni = ?;";
    const char *sql_baja   = "DELETE FROM clientes WHERE dni = ?";

    int activo = -1;
    int rc = 1;

    printf("\n--- BAJA DE CLIENTE ---\n");

    char *dni = leer_cadena("Introduzca DNI del cliente a dar de baja: ");
    if (!dni) return 1;

    if (sqlite3_prepare_v2(db, sql_buscar, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando consulta: %s\n", sqlite3_errmsg(db));
        free(dni);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        activo = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    stmt = NULL;

    if (activo == -1) {
        printf("Error: no existe ningun cliente con DNI %s.\n", dni);
        goto cleanup;
    }

    if (activo == 0) {
        printf("Error: el cliente con DNI %s ya estaba dado de baja.\n", dni);
        goto cleanup;
    }

    if (sqlite3_prepare_v2(db, sql_baja, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error al dar de baja al cliente: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        goto cleanup;
    }

    sqlite3_finalize(stmt);
    printf("Cliente con DNI %s dado de baja correctamente.\n", dni);
    rc = 0;

cleanup:
    free(dni);
    return rc;
}

/* =========================================================
   MODIFICAR
   ========================================================= */

int modificar_cliente(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE clientes "
        "SET nombre = ?, apellidos = ?, telefono = ?, email = ?, fecha_nacimiento = ? "
        "WHERE dni = ?;";

    Cliente c;
    memset(&c, 0, sizeof(c));
    int rc = 1;

    printf("\n--- MODIFICAR CLIENTE ---\n");

    c.dni = leer_cadena("Introduzca DNI del cliente a modificar: ");
    if (!c.dni) return 1;

    if (!obtener_cliente_por_dni(db, c.dni, &c)) {
        printf("Error: cliente no encontrado.\n");
        goto cleanup;
    }

    printf("\nDatos actuales:\n");
    mostrar_cliente(&c);

    /* Reemplazar campos con nuevos valores */
    char *nuevo;

    nuevo = leer_cadena("Nuevo nombre: ");
    if (nuevo) { free(c.nombre); c.nombre = nuevo; }

    nuevo = leer_cadena("Nuevos apellidos: ");
    if (nuevo) { free(c.apellidos); c.apellidos = nuevo; }

    nuevo = leer_cadena("Nuevo telefono: ");
    if (nuevo) { free(c.telefono); c.telefono = nuevo; }

    nuevo = leer_cadena("Nuevo email: ");
    if (nuevo) { free(c.email); c.email = nuevo; }

    nuevo = leer_cadena("Nueva fecha nacimiento (YYYY-MM-DD): ");
    if (nuevo) { free(c.fecha_nacimiento); c.fecha_nacimiento = nuevo; }

    if (!c.nombre || strlen(c.nombre) == 0 || !c.apellidos || strlen(c.apellidos) == 0) {
        printf("Error: nombre y apellidos son obligatorios.\n");
        goto cleanup;
    }

    if (!c.email || !validar_email(c.email)) {
        printf("Error: email no valido.\n");
        goto cleanup;
    }

    if (!c.fecha_nacimiento || !validar_fecha(c.fecha_nacimiento)) {
        printf("Error: fecha no valida.\n");
        goto cleanup;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    sqlite3_bind_text(stmt, 1, c.nombre,            -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c.apellidos,         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c.telefono,          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c.email,             -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, c.fecha_nacimiento,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, c.dni,               -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error modificando cliente: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        goto cleanup;
    }

    sqlite3_finalize(stmt);
    printf("Cliente modificado correctamente.\n");
    rc = 0;

cleanup:
    cliente_free(&c);
    return rc;
}

/* =========================================================
   BUSCAR
   ========================================================= */

int buscar_cliente_por_dni(sqlite3 *db, const char *dni) {
    Cliente c;
    memset(&c, 0, sizeof(c));

    printf("\n--- BUSCAR CLIENTE ---\n");

    if (!obtener_cliente_por_dni(db, dni, &c)) {
        printf("No se ha encontrado ningun cliente con DNI %s\n", dni);
        return 1;
    }

    mostrar_cliente(&c);
    cliente_free(&c);
    return 0;
}

/* =========================================================
   LISTAR
   ========================================================= */

void listar_clientes(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, dni, nombre, apellidos, telefono, email, fecha_nacimiento, activo "
        "FROM clientes ORDER BY apellidos, nombre;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- LISTADO DE CLIENTES ---\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Cliente c;
        memset(&c, 0, sizeof(c));

        c.id = sqlite3_column_int(stmt, 0);

        /* strdup reserva exactamente lo necesario para cada campo */
        c.dni              = strdup((const char *)sqlite3_column_text(stmt, 1));
        c.nombre           = strdup((const char *)sqlite3_column_text(stmt, 2));
        c.apellidos        = strdup((const char *)sqlite3_column_text(stmt, 3));
        c.telefono         = strdup((const char *)sqlite3_column_text(stmt, 4));
        c.email            = strdup((const char *)sqlite3_column_text(stmt, 5));
        c.fecha_nacimiento = strdup((const char *)sqlite3_column_text(stmt, 6));
        c.activo           = sqlite3_column_int(stmt, 7);

        mostrar_cliente(&c);
        cliente_free(&c);
    }

    sqlite3_finalize(stmt);
}

/* =========================================================
   FUNCIONES AUXILIARES PRIVADAS
   ========================================================= */

static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/*
 * Lee una linea de stdin y devuelve un puntero malloc'd con el contenido.
 * El llamante debe liberar el resultado con free().
 * Devuelve NULL en caso de error.
 */
static char *leer_cadena(const char *mensaje) {
    char buffer[MAX_INPUT];
    printf("%s", mensaje);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return NULL;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    char *resultado = strdup(buffer);
    return resultado;
}

static int leer_entero(const char *mensaje) {
    char buffer[MAX_INPUT];
    int valor;
    char extra;

    while (1) {
        printf("%s", mensaje);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return 0;
        }

        if (sscanf(buffer, "%d %c", &valor, &extra) == 1) {
            return valor;
        }

        printf("Entrada no valida. Introduzca un numero.\n");
    }
}

static int validar_dni(const char *dni) {
    int i;
    int len = (int)strlen(dni);

    if (len != 9) return 0;
    for (i = 0; i < 8; i++) {
        if (!isdigit((unsigned char)dni[i])) return 0;
    }
    if (!isalpha((unsigned char)dni[8])) return 0;
    return 1;
}

static int validar_email(const char *email) {
    const char *arroba;
    const char *punto;

    if (email == NULL || strlen(email) < 5) return 0;

    arroba = strchr(email, '@');
    if (arroba == NULL) return 0;

    punto = strrchr(email, '.');
    if (punto == NULL || arroba > punto) return 0;
    if (arroba == email || *(arroba + 1) == '\0') return 0;

    return 1;
}

static int validar_fecha(const char *fecha) {
    int anio, mes, dia;

    if (strlen(fecha) != 10) return 0;
    if (fecha[4] != '-' || fecha[7] != '-') return 0;
    if (sscanf(fecha, "%d-%d-%d", &anio, &mes, &dia) != 3) return 0;
    if (anio < 1900 || anio > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;

    return 1;
}

static int existe_cliente_dni(sqlite3 *db, const char *dni) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM clientes WHERE dni = ?;";
    int existe = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando consulta de existencia: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existe = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return existe > 0;
}

/*
 * Rellena la estructura Cliente con datos de la BD.
 * Los campos de texto se reservan dinamicamente con strdup.
 * El llamante debe llamar a cliente_free() cuando termine.
 */
static int obtener_cliente_por_dni(sqlite3 *db, const char *dni, Cliente *c) {
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, dni, nombre, apellidos, telefono, email, fecha_nacimiento, activo "
        "FROM clientes WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando busqueda: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        c->id              = sqlite3_column_int(stmt, 0);
        c->dni             = strdup((const char *)sqlite3_column_text(stmt, 1));
        c->nombre          = strdup((const char *)sqlite3_column_text(stmt, 2));
        c->apellidos       = strdup((const char *)sqlite3_column_text(stmt, 3));
        c->telefono        = strdup((const char *)sqlite3_column_text(stmt, 4));
        c->email           = strdup((const char *)sqlite3_column_text(stmt, 5));
        c->fecha_nacimiento= strdup((const char *)sqlite3_column_text(stmt, 6));
        c->activo          = sqlite3_column_int(stmt, 7);

        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

static void mostrar_cliente(const Cliente *c) {
    printf("\n-------------------------------------\n");
    printf("ID: %d\n", c->id);
    printf("DNI: %s\n",               c->dni            ? c->dni            : "(null)");
    printf("Nombre: %s\n",            c->nombre         ? c->nombre         : "(null)");
    printf("Apellidos: %s\n",         c->apellidos      ? c->apellidos      : "(null)");
    printf("Telefono: %s\n",          c->telefono       ? c->telefono       : "(null)");
    printf("Email: %s\n",             c->email          ? c->email          : "(null)");
    printf("Fecha nacimiento: %s\n",  c->fecha_nacimiento ? c->fecha_nacimiento : "(null)");
    printf("Activo: %s\n", c->activo ? "SI" : "NO");
    printf("-------------------------------------\n");
}

static void pausar(void) {
    printf("\nPulse ENTER para continuar...");
    getchar();
}
