#ifndef ALOJAMIENTO_H_
#define ALOJAMIENTO_H_

#include "sqlite3.h"

/*
 * Estructura Alojamiento con campos de texto como punteros dinamicos.
 * Usar alojamiento_free() para liberar la memoria.
 */
typedef struct {
    char *codigo;
    char *nombre;
    char *direccion;
    char *tipo;
    char *cod_ciudad;
    int   activo;
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
