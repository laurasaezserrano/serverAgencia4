#ifndef CLIENTES_H_
#define CLIENTES_H_

#include "sqlite3.h"

/*
 * Estructura Cliente con campos de texto como punteros dinamicos.
 * Usar cliente_free() para liberar la memoria cuando ya no se necesite.
 */
typedef struct {
    int   id;
    char *dni;
    char *nombre;
    char *apellidos;
    char *telefono;
    char *email;
    char *fecha_nacimiento;
    int   activo;
} Cliente;

/* Libera todos los campos dinamicos de un Cliente (no el puntero a la struct). */
void cliente_free(Cliente *c);

void menu_clientes(sqlite3 *db);
int  alta_cliente(sqlite3 *db);
int  baja_cliente(sqlite3 *db);
int  modificar_cliente(sqlite3 *db);
int  buscar_cliente_por_dni(sqlite3 *db, const char *dni);
void listar_clientes(sqlite3 *db);

#endif /* CLIENTES_H_ */
