#ifndef PAQUETE_H_
#define PAQUETE_H_
#include "sqlite3.h"

typedef struct {
    int  cod;
    char nombre[50];
    float precio;
    char cod_ciudad_origen[10];
    char cod_ciudad_destino[10];
    int  plazas_totales;
    int  plazas_disponibles;
    int  activo;
} Paquete;

void crearPaquete(sqlite3 *db);
void eliminarPaquete(sqlite3 *db);
void consultarPaquete(sqlite3 *db);
void listadoPaquetes(sqlite3 *db);
void menuPaquetes(sqlite3 *db);

#endif /* PAQUETE_H_ */
