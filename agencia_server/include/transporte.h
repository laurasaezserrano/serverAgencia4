#ifndef TRANSPORTE_H_
#define TRANSPORTE_H_

#include "sqlite3.h"

typedef struct {
    char codigo[10];
    char tipo[20];
    char fecha_salida[11];
    char fecha_llegada[11];
    int  cod_paquete;
    int  activo;
} Transporte;

void altaTransporte(sqlite3 *db);
void bajaTransporte(void);
void consultarTransporte(void);
void asociarTransporte(void);
void listadoTransportes(void);
void menuTransporte(sqlite3 *db);

#endif
