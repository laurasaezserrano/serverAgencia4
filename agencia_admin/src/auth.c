#include <stdio.h>
#include <string.h>
#include "../include/auth.h"
#include "../include/sqlite3.h"
#include "../include/hash.h"

static void limpiar_buffer_auth(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void registrar_usuario(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char usuario[50], clave[50];
    char hash_clave[65];
    const char *sql = "INSERT INTO usuarios (username, password, rol) VALUES (?, ?, 'CLIENTE');";

    printf("\n--- REGISTRO DE NUEVO USUARIO ---\n");
    printf("Elija nombre de usuario: ");
    scanf("%49s", usuario);
    printf("Elija clave: ");
    scanf("%49s", clave);
    limpiar_buffer_auth();

    sha256_hex(clave, hash_clave);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error al preparar registro: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, usuario,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash_clave, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Usuario '%s' registrado con exito como CLIENTE!\n", usuario);
    } else {
        printf("Error: El usuario ya existe o hubo un fallo en la BD.\n");
    }

    sqlite3_finalize(stmt);
}

int verificar_credenciales_rol(sqlite3 *db, const char *user, const char *pass) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT password, rol FROM usuarios WHERE username = ?;";
    int tipo = 0;
    char hash_input[65];

    sha256_hex(pass, hash_input);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *hash_db = (const char *)sqlite3_column_text(stmt, 0);
        const char *rol     = (const char *)sqlite3_column_text(stmt, 1);

        if (hash_db && rol && strcmp(hash_db, hash_input) == 0) {
            if      (strcmp(rol, "ADMIN")   == 0) tipo = 1;
            else if (strcmp(rol, "CLIENTE") == 0) tipo = 2;
        }
    }

    sqlite3_finalize(stmt);
    return tipo;
}

int login_admin(sqlite3 *db) {
    char usuario[50];
    char clave[50];
    int opcion;

    while (1) {
        printf("\n======= ACCESO AGENCIA =======\n");
        printf("1. Iniciar Sesion\n");
        printf("2. Registrarse\n");
        printf("0. Salir del programa\n");
        printf("==============================\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", &opcion) != 1) {
            limpiar_buffer_auth();
            continue;
        }
        limpiar_buffer_auth();

        if (opcion == 0) return 0;

        if (opcion == 2) {
            registrar_usuario(db);
            continue;
        }

        if (opcion == 1) {
            printf("\n--- LOGIN ---\n");
            printf("Usuario: ");
            scanf("%49s", usuario);
            printf("Clave: ");
            scanf("%49s", clave);
            limpiar_buffer_auth();

            int resultado = verificar_credenciales_rol(db, usuario, clave);

            if (resultado > 0) {
                printf("Acceso concedido. Bienvenido %s.\n", usuario);
                return resultado;
            } else {
                printf("Usuario o clave incorrectos. Intente de nuevo.\n");
            }
        } else {
            printf("Opcion no valida.\n");
        }
    }
}
