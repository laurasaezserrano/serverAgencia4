#ifndef PAQUETE_H_
#define PAQUETE_H_

#include "sqlite3.h"

/*
 * Estructura Paquete con campos de texto como punteros dinamicos.
 * Usar paquete_free() para liberar la memoria.
 */
typedef struct {
    int   cod;
    char *nombre;
    float precio;
    char *cod_ciudad_origen;
    char *cod_ciudad_destino;
    int   plazas_totales;
    int   plazas_disponibles;
    int   activo;
} Paquete;

/* Libera todos los campos dinamicos de un Paquete. */
void paquete_free(Paquete *p);

void crearPaquete(sqlite3 *db);
void eliminarPaquete(sqlite3 *db);
void consultarPaquete(sqlite3 *db);
void listadoPaquetes(sqlite3 *db);
void menuPaquetes(sqlite3 *db);

#endif /* PAQUETE_H_ */
