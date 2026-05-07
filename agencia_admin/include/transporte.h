#ifndef TRANSPORTE_H_
#define TRANSPORTE_H_

#include "sqlite3.h"

/*
 * Estructura Transporte con campos de texto como punteros dinamicos.
 * Usar transporte_free() para liberar la memoria.
 */
typedef struct {
    char *codigo;
    char *tipo;
    char *fecha_salida;
    char *fecha_llegada;
    int   cod_paquete;
    int   activo;
} Transporte;

/* Libera todos los campos dinamicos de un Transporte. */
void transporte_free(Transporte *t);

void altaTransporte(sqlite3 *db);
void bajaTransporte(void);
void consultarTransporte(void);
void asociarTransporte(void);
void listadoTransportes(void);
void menuTransporte(sqlite3 *db);

#endif /* TRANSPORTE_H_ */
