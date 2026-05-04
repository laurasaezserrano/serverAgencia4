/*
 * transportes.c
 *
 *  Created on: 26 mar 2026
 *      Author: zaira.diez
 */


#include <stdio.h>
#include <string.h>
#include "../include/transporte.h"

#define ARCHIVO "transportes.dat"

void guardarTransporte(Transporte t) {
    FILE *f = fopen(ARCHIVO, "ab");
    if (f != NULL) {
        fwrite(&t, sizeof(Transporte), 1, f);
        fclose(f);
    }
}

void altaTransporte(sqlite3 *db) { // hay que terminar todo lo de db y guardarlo ahi
	sqlite3_stmt *stmt = NULL;
	const char   *sql  =
	"INSERT INTO transportes (codigo, tipo, fecha_salida, fecha_llegada, cod_paquete, activo) "
			"VALUES (?, ?, ?, ?, ?, 1);";

    Transporte t;

    printf("Codigo: ");
    scanf("%s", t.codigo);

    printf("Tipo (avion, tren, autobus...): ");
    scanf("%s", t.tipo);

    printf("Fecha salida (dd/mm/yyyy): ");
    scanf("%s", t.fecha_salida);

    printf("Fecha llegada (dd/mm/yyyy): ");
    scanf("%s", t.fecha_llegada);

    printf("Codigo de paquete: ");
    scanf("%d", &t.cod_paquete);

    t.activo = 1;

    guardarTransporte(t);

    printf("Transporte dado de alta correctamente.\n");

}


void bajaTransporte(void) {
    FILE *f = fopen(ARCHIVO, "rb+");
    Transporte t;
    char codigo[10];

    if (f == NULL) return;

    printf("Codigo a dar de baja: ");
    scanf("%s", codigo);

    while (fread(&t, sizeof(Transporte), 1, f)) {
        if (strcmp(t.codigo, codigo) == 0 && t.activo == 1) {
            t.activo = 0;
            fseek(f, -sizeof(Transporte), SEEK_CUR);
            fwrite(&t, sizeof(Transporte), 1, f);
            printf("Transporte dado de baja.\n");
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// CONSULTAR
void consultarTransporte(void) {
    FILE *f = fopen(ARCHIVO, "rb");
    Transporte t;
    char codigo[10];

    if (f == NULL) return;

    printf("Codigo a consultar: ");
    scanf("%s", codigo);

    while (fread(&t, sizeof(Transporte), 1, f)) {
        if (strcmp(t.codigo, codigo) == 0 && t.activo == 1) {
            printf("\n--- Transporte ---\n");
            printf("Codigo: %s\n", t.codigo);
            printf("Tipo: %s\n", t.tipo);
            printf("Salida: %s\n", t.fecha_salida);
            printf("Llegada: %s\n", t.fecha_llegada);
            printf("Cod paquete: %d\n", t.cod_paquete);
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// ASOCIAR TRANSPORTE A PAQUETE
void asociarTransporte(void) {
    FILE *f = fopen(ARCHIVO, "rb+");
    Transporte t;
    char codigo[10];
    int nuevoCod;

    if (f == NULL) return;

    printf("Codigo transporte: ");
    scanf("%s", codigo);

    printf("Nuevo codigo de paquete: ");
    scanf("%d", &nuevoCod);

    while (fread(&t, sizeof(Transporte), 1, f)) {
        if (strcmp(t.codigo, codigo) == 0 && t.activo == 1) {
            t.cod_paquete = nuevoCod;
            fseek(f, -sizeof(Transporte), SEEK_CUR);
            fwrite(&t, sizeof(Transporte), 1, f);
            printf("Asociacion realizada.\n");
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// LISTADO
void listadoTransportes(void) {
    FILE *f = fopen(ARCHIVO, "rb");
    Transporte t;

    if (f == NULL) {
        printf("No hay transportes.\n");
        return;
    }

    printf("\n--- LISTADO DE TRANSPORTES ---\n");

    while (fread(&t, sizeof(Transporte), 1, f)) {
        if (t.activo == 1) {
            printf("\nCodigo: %s\n", t.codigo);
            printf("Tipo: %s\n", t.tipo);
            printf("Salida: %s\n", t.fecha_salida);
            printf("Llegada: %s\n", t.fecha_llegada);
            printf("Cod paquete: %d\n", t.cod_paquete);
        }
    }

    fclose(f);
}

// MENU ADMIN
void menuTransporte(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU TRANSPORTES ---\n");
        printf("1. Alta transporte\n");
        printf("2. Baja transporte\n");
        printf("3. Consultar transporte\n");
        printf("4. Asociar transporte\n");
        printf("5. Listado transportes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: altaTransporte(db); break;
            case 2: bajaTransporte(); break;
            case 3: consultarTransporte(); break;
            case 4: asociarTransporte(); break;
            case 5: listadoTransportes(); break;
        }

    } while (opcion != 0);
}

// MENU PARA CLIENTES
void menuTransporte_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU TRANSPORTES ---\n");
        printf("1. Consultar transporte\n");
        printf("2. Listado transportes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: consultarTransporte(); break;
            case 2: listadoTransportes(); break;
        }

    } while (opcion != 0);
}
