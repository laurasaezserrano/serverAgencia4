#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/clientes.h"
#include "../include/sqlite3.h"

#define MAX_INPUT 256

static void limpiar_buffer(void);
static void leer_cadena(const char *mensaje, char *buffer, int tam);
static int leer_entero(const char *mensaje);
static int validar_dni(const char *dni);
static int validar_email(const char *email);
static int validar_fecha(const char *fecha);
static int existe_cliente_dni(sqlite3 *db, const char *dni);
static int obtener_cliente_por_dni(sqlite3 *db, const char *dni, Cliente *c);
static void mostrar_cliente(const Cliente *c);
static void pausar(void);

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
                char dni[16];
                leer_cadena("Introduzca DNI: ", dni, sizeof(dni));
                buscar_cliente_por_dni(db, dni);
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

int alta_cliente(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO clientes "
        "(dni, nombre, apellidos, telefono, email, fecha_nacimiento, activo) "
        "VALUES (?, ?, ?, ?, ?, ?, 1);";

    Cliente c;

    printf("\n--- ALTA DE CLIENTE ---\n");

    leer_cadena("DNI: ", c.dni, sizeof(c.dni));
    if (!validar_dni(c.dni)) {
        printf("Error: DNI no valido.\n");
        return 1;
    }

    if (existe_cliente_dni(db, c.dni)) {
        printf("Error: ya existe un cliente con ese DNI.\n");
        return 1;
    }

    leer_cadena("Nombre: ", c.nombre, sizeof(c.nombre));
    leer_cadena("Apellidos: ", c.apellidos, sizeof(c.apellidos));
    leer_cadena("Telefono: ", c.telefono, sizeof(c.telefono));
    leer_cadena("Email: ", c.email, sizeof(c.email));
    leer_cadena("Fecha nacimiento (YYYY-MM-DD): ", c.fecha_nacimiento, sizeof(c.fecha_nacimiento));

    if (strlen(c.nombre) == 0 || strlen(c.apellidos) == 0) {
        printf("Error: nombre y apellidos son obligatorios.\n");
        return 1;
    }

    if (!validar_email(c.email)) {
        printf("Error: email no valido.\n");
        return 1;
    }

    if (!validar_fecha(c.fecha_nacimiento)) {
        printf("Error: fecha no valida. Use YYYY-MM-DD.\n");
        return 1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando INSERT: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_text(stmt, 1, c.dni, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c.apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c.telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, c.email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, c.fecha_nacimiento, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error insertando cliente: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    printf("Cliente dado de alta correctamente.\n");
    return 0;
}

int baja_cliente(sqlite3 *db) {
	    sqlite3_stmt *stmt = NULL;
	    const char *sql_buscar =
	        "SELECT activo FROM clientes WHERE dni = ?;";
	    const char *sql_baja =
	        "DELETE FROM clientes WHERE dni = ?";

	    char dni[16];
	    int activo = -1;

	    printf("\n--- BAJA DE CLIENTE ---\n");
	    leer_cadena("Introduzca DNI del cliente a dar de baja: ", dni, sizeof(dni));

	    if (sqlite3_prepare_v2(db, sql_buscar, -1, &stmt, NULL) != SQLITE_OK) {
	        printf("Error preparando consulta: %s\n", sqlite3_errmsg(db));
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
	        return 1;
	    }

	    if (activo == 0) {
	        printf("Error: el cliente con DNI %s ya estaba dado de baja.\n", dni);
	        return 1;
	    }

	    if (sqlite3_prepare_v2(db, sql_baja, -1, &stmt, NULL) != SQLITE_OK) {
	        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
	        return 1;
	    }

	    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

	    if (sqlite3_step(stmt) != SQLITE_DONE) {
	        printf("Error al dar de baja al cliente: %s\n", sqlite3_errmsg(db));
	        sqlite3_finalize(stmt);
	        return 1;
	    }

	    sqlite3_finalize(stmt);
	    printf("Cliente con DNI %s dado de baja correctamente.\n", dni);
	    return 0;

}

int modificar_cliente(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE clientes "
        "SET nombre = ?, apellidos = ?, telefono = ?, email = ?, fecha_nacimiento = ? "
        "WHERE dni = ?;";

    Cliente c;

    printf("\n--- MODIFICAR CLIENTE ---\n");
    leer_cadena("Introduzca DNI del cliente a modificar: ", c.dni, sizeof(c.dni));

    if (!obtener_cliente_por_dni(db, c.dni, &c)) {
        printf("Error: cliente no encontrado.\n");
        return 1;
    }

    printf("\nDatos actuales:\n");
    mostrar_cliente(&c);

    leer_cadena("Nuevo nombre: ", c.nombre, sizeof(c.nombre));
    leer_cadena("Nuevos apellidos: ", c.apellidos, sizeof(c.apellidos));
    leer_cadena("Nuevo telefono: ", c.telefono, sizeof(c.telefono));
    leer_cadena("Nuevo email: ", c.email, sizeof(c.email));
    leer_cadena("Nueva fecha nacimiento (YYYY-MM-DD): ", c.fecha_nacimiento, sizeof(c.fecha_nacimiento));

    if (strlen(c.nombre) == 0 || strlen(c.apellidos) == 0) {
        printf("Error: nombre y apellidos son obligatorios.\n");
        return 1;
    }

    if (!validar_email(c.email)) {
        printf("Error: email no valido.\n");
        return 1;
    }

    if (!validar_fecha(c.fecha_nacimiento)) {
        printf("Error: fecha no valida.\n");
        return 1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando UPDATE: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_text(stmt, 1, c.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c.apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c.telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c.email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, c.fecha_nacimiento, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, c.dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Error modificando cliente: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    printf("Cliente modificado correctamente.\n");
    return 0;
}

int buscar_cliente_por_dni(sqlite3 *db, const char *dni) {
    Cliente c;

    printf("\n--- BUSCAR CLIENTE ---\n");

    if (!obtener_cliente_por_dni(db, dni, &c)) {
        printf("No se ha encontrado ningun cliente con DNI %s\n", dni);
        return 1;
    }

    mostrar_cliente(&c);
    return 0;
}

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

        c.id = sqlite3_column_int(stmt, 0);
        strncpy(c.dni, (const char *)sqlite3_column_text(stmt, 1), sizeof(c.dni) - 1);
        c.dni[sizeof(c.dni) - 1] = '\0';

        strncpy(c.nombre, (const char *)sqlite3_column_text(stmt, 2), sizeof(c.nombre) - 1);
        c.nombre[sizeof(c.nombre) - 1] = '\0';

        strncpy(c.apellidos, (const char *)sqlite3_column_text(stmt, 3), sizeof(c.apellidos) - 1);
        c.apellidos[sizeof(c.apellidos) - 1] = '\0';

        strncpy(c.telefono, (const char *)sqlite3_column_text(stmt, 4), sizeof(c.telefono) - 1);
        c.telefono[sizeof(c.telefono) - 1] = '\0';

        strncpy(c.email, (const char *)sqlite3_column_text(stmt, 5), sizeof(c.email) - 1);
        c.email[sizeof(c.email) - 1] = '\0';

        strncpy(c.fecha_nacimiento, (const char *)sqlite3_column_text(stmt, 6), sizeof(c.fecha_nacimiento) - 1);
        c.fecha_nacimiento[sizeof(c.fecha_nacimiento) - 1] = '\0';

        c.activo = sqlite3_column_int(stmt, 7);

        mostrar_cliente(&c);
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

static void leer_cadena(const char *mensaje, char *buffer, int tam) {
    printf("%s", mensaje);

    if (fgets(buffer, tam, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
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

    if (len != 9) {
        return 0;
    }

    for (i = 0; i < 8; i++) {
        if (!isdigit((unsigned char)dni[i])) {
            return 0;
        }
    }

    if (!isalpha((unsigned char)dni[8])) {
        return 0;
    }

    return 1;
}

static int validar_email(const char *email) {
    const char *arroba = strchr(email, '@');
    const char *punto;

    if (email == NULL || strlen(email) < 5) {
        return 0;
    }

    if (arroba == NULL) {
        return 0;
    }

    punto = strrchr(email, '.');
    if (punto == NULL) {
        return 0;
    }

    if (arroba > punto) {
        return 0;
    }

    if (arroba == email || *(arroba + 1) == '\0') {
        return 0;
    }

    return 1;
}

static int validar_fecha(const char *fecha) {
    int anio, mes, dia;

    if (strlen(fecha) != 10) {
        return 0;
    }

    if (fecha[4] != '-' || fecha[7] != '-') {
        return 0;
    }

    if (sscanf(fecha, "%d-%d-%d", &anio, &mes, &dia) != 3) {
        return 0;
    }

    if (anio < 1900 || anio > 2100) {
        return 0;
    }

    if (mes < 1 || mes > 12) {
        return 0;
    }

    if (dia < 1 || dia > 31) {
        return 0;
    }

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
        c->id = sqlite3_column_int(stmt, 0);

        strncpy(c->dni, (const char *)sqlite3_column_text(stmt, 1), sizeof(c->dni) - 1);
        c->dni[sizeof(c->dni) - 1] = '\0';

        strncpy(c->nombre, (const char *)sqlite3_column_text(stmt, 2), sizeof(c->nombre) - 1);
        c->nombre[sizeof(c->nombre) - 1] = '\0';

        strncpy(c->apellidos, (const char *)sqlite3_column_text(stmt, 3), sizeof(c->apellidos) - 1);
        c->apellidos[sizeof(c->apellidos) - 1] = '\0';

        strncpy(c->telefono, (const char *)sqlite3_column_text(stmt, 4), sizeof(c->telefono) - 1);
        c->telefono[sizeof(c->telefono) - 1] = '\0';

        strncpy(c->email, (const char *)sqlite3_column_text(stmt, 5), sizeof(c->email) - 1);
        c->email[sizeof(c->email) - 1] = '\0';

        strncpy(c->fecha_nacimiento, (const char *)sqlite3_column_text(stmt, 6), sizeof(c->fecha_nacimiento) - 1);
        c->fecha_nacimiento[sizeof(c->fecha_nacimiento) - 1] = '\0';

        c->activo = sqlite3_column_int(stmt, 7);

        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

static void mostrar_cliente(const Cliente *c) {
    printf("\n-------------------------------------\n");
    printf("ID: %d\n", c->id);
    printf("DNI: %s\n", c->dni);
    printf("Nombre: %s\n", c->nombre);
    printf("Apellidos: %s\n", c->apellidos);
    printf("Telefono: %s\n", c->telefono);
    printf("Email: %s\n", c->email);
    printf("Fecha nacimiento: %s\n", c->fecha_nacimiento);
    printf("Activo: %s\n", c->activo ? "SI" : "NO");
    printf("-------------------------------------\n");
}

static void pausar(void) {
    printf("\nPulse ENTER para continuar...");
    getchar();
}
