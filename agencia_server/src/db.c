#include <stdio.h>
#include "../include/sqlite3.h"
#include "../include/db.h"

int db_abrir(sqlite3 **db, const char *ruta) {
    if (sqlite3_open(ruta, db) != SQLITE_OK) {
        printf("Error al abrir la base de datos: %s\n", sqlite3_errmsg(*db));
        return 1;
    }
    return 0;
}

void db_cerrar(sqlite3 *db) {
    if (db != NULL) {
        sqlite3_close(db);
    }
}

int db_crear_tablas(sqlite3 *db) {
    char *err = NULL;

    const char *sql_usuarios =
        "CREATE TABLE IF NOT EXISTS usuarios ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "password TEXT NOT NULL, "
        "rol TEXT NOT NULL" // 'ADMIN' o 'CLIENTE'
        ");";

    const char *sql_clientes =
        "CREATE TABLE IF NOT EXISTS clientes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "dni TEXT NOT NULL UNIQUE, "
        "nombre TEXT NOT NULL, "
        "apellidos TEXT NOT NULL, "
        "telefono TEXT, "
        "email TEXT, "
        "fecha_nacimiento TEXT, "
        "activo INTEGER NOT NULL DEFAULT 1"
        ");";

    const char *sql_alojamientos =
        "CREATE TABLE IF NOT EXISTS alojamientos ("
        "codigo TEXT PRIMARY KEY, "
        "nombre TEXT NOT NULL, "
        "direccion TEXT, "
        "tipo TEXT, "
        "cod_ciudad TEXT, "
        "activo INTEGER DEFAULT 1"
        ");";

    const char *sql_paquetes =
        "CREATE TABLE IF NOT EXISTS paquetes ("
        "codigo INTEGER PRIMARY KEY, "
        "nombre TEXT NOT NULL, "
        "precio REAL, "
        "destino TEXT, "
        "origen TEXT, "
        "plazas_totales INTEGER, "
        "plazas_disponibles INTEGER, "
        "activo INTEGER DEFAULT 1"
        ");";

    const char *sql_transportes =
        "CREATE TABLE IF NOT EXISTS transportes ("
        "codigo TEXT PRIMARY KEY, "
        "tipo TEXT, "
        "fecha_salida TEXT, "
        "fecha_llegada TEXT, "
        "id_paquete INTEGER, "
        "activo INTEGER DEFAULT 1, "
        "FOREIGN KEY(id_paquete) REFERENCES paquetes(codigo)"
        ");";

    // INSERTAR ADMIN (Si no existe)
    const char *sql_insert_admin =
        "INSERT OR IGNORE INTO usuarios (username, password, rol) "
        "VALUES ('admin', '1234', 'ADMIN');";

    // EJECUCIÓN DE TABLAS
    if (sqlite3_exec(db, sql_usuarios, 0, 0, &err) != SQLITE_OK) {
        printf("Error SQL al crear usuarios: %s\n", err);
        sqlite3_free(err); return 1;
    }

    if (sqlite3_exec(db, sql_clientes, 0, 0, &err) != SQLITE_OK) {
        printf("Error SQL al crear clientes: %s\n", err);
        sqlite3_free(err); return 1;
    }

    if (sqlite3_exec(db, sql_alojamientos, 0, 0, &err) != SQLITE_OK) {
        printf("Error SQL al crear alojamientos: %s\n", err);
        sqlite3_free(err); return 1;
    }

    if (sqlite3_exec(db, sql_paquetes, 0, 0, &err) != SQLITE_OK) {
        printf("Error SQL al crear paquetes: %s\n", err);
        sqlite3_free(err); return 1;
    }

    if (sqlite3_exec(db, sql_transportes, 0, 0, &err) != SQLITE_OK) {
        printf("Error SQL al crear transportes: %s\n", err);
        sqlite3_free(err); return 1;
    }

    // EJECUCIÓN INSERT ADMIN
    if (sqlite3_exec(db, sql_insert_admin, 0, 0, &err) != SQLITE_OK) {
        printf("Error SQL al insertar admin: %s\n", err);
        sqlite3_free(err); return 1;
    }

    return 0;
}
