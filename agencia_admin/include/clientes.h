/*
 * clientes.h
 *
 * Created on: 2 abr 2026
 * Author: zaira.diez
 */

#ifndef CLIENTES_H_
#define CLIENTES_H_

#include "sqlite3.h"

// Definición de la estructura Cliente
typedef struct {
    int id;
    char dni[16];
    char nombre[50];
    char apellidos[100];
    char telefono[20];
    char email[100];
    char fecha_nacimiento[11];
    int activo;
} Cliente;

void menu_clientes(sqlite3 *db);
int alta_cliente(sqlite3 *db);
int baja_cliente(sqlite3 *db);
int modificar_cliente(sqlite3 *db);
int buscar_cliente_por_dni(sqlite3 *db, const char *dni);
void listar_clientes(sqlite3 *db);

#endif /* CLIENTES_H_ */
