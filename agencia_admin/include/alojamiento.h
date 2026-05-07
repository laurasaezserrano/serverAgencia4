#ifndef ALOJAMIENTO_H_
#define ALOJAMIENTO_H_

#include "sqlite3.h"

/*
 * Estructura Alojamiento con campos de texto como punteros dinamicos.
 * Usar alojamiento_free() para liberar la memoria.
 */
typedef struct {
    char codigo[10];    // ← array fijo, NO char*
    char nombre[50];
    char direccion[100];
    char tipo[20];
    char cod_ciudad[10];
    int  activo;
} Alojamiento;

/* Libera todos los campos dinamicos de un Alojamiento. */
void alojamiento_free(Alojamiento *a);

void guardarAlojamiento(Alojamiento *a);
void altaAlojamiento(sqlite3 *db);
void bajaAlojamiento(sqlite3 *db);
void consultarAlojamiento(sqlite3 *db);
void listadoAlojamientos(sqlite3 *db);
void menuAlojamiento(sqlite3 *db);

#endif /* ALOJAMIENTO_H_ */
