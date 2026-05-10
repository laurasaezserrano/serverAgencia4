#ifndef ALOJAMIENTO_H_
#define ALOJAMIENTO_H_
#include "sqlite3.h"

typedef struct {
    char codigo[10];
    char nombre[50];
    char direccion[100];
    char tipo[20];
    char cod_ciudad[10];
    int  activo;
} Alojamiento;

void guardarAlojamiento(Alojamiento a);
void altaAlojamiento(sqlite3 *db);
void bajaAlojamiento(sqlite3 *db);
void consultarAlojamiento(sqlite3 *db);
void listadoAlojamientos(sqlite3 *db);
void menuAlojamiento(sqlite3 *db);

#endif
