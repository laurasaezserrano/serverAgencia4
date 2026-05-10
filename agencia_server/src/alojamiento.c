/*
 * alojamiento.c
 *
 *  Created on: 26 mar 2026
 *      Author: zaira.diez
 */


#include <stdio.h>
#include <string.h>
#include "../include/alojamiento.h"

#define ARCHIVO "alojamientos.dat"

// Guardar alojamiento
void guardarAlojamiento(Alojamiento a) {
    FILE *f = fopen(ARCHIVO, "ab");
    if (f != NULL) {
        fwrite(&a, sizeof(Alojamiento), 1, f);
        fclose(f);
    }
}

// ALTA
void altaAlojamiento(sqlite3 *db) {
    Alojamiento a;

    printf("Codigo: ");
    scanf("%s", a.codigo);

    printf("Nombre: ");
    scanf(" %[^\n]", a.nombre);

    printf("Direccion: ");
    scanf(" %[^\n]", a.direccion);

    printf("Tipo (hotel, hostal...): ");
    scanf("%s", a.tipo);

    printf("Codigo ciudad: ");
    scanf("%s", a.cod_ciudad);

    a.activo = 1;

    guardarAlojamiento(a);

    printf("Alojamiento dado de alta.\n");
}

// BAJA (lógica)
void bajaAlojamiento(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb+");
    Alojamiento a;
    char codigo[10];

    if (f == NULL) return;

    printf("Codigo a dar de baja: ");
    scanf("%s", codigo);

    while (fread(&a, sizeof(Alojamiento), 1, f)) {
        if (strcmp(a.codigo, codigo) == 0 && a.activo == 1) {
            a.activo = 0;
            fseek(f, -sizeof(Alojamiento), SEEK_CUR);
            fwrite(&a, sizeof(Alojamiento), 1, f);
            printf("Alojamiento dado de baja.\n");
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// CONSULTAR
void consultarAlojamiento(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    Alojamiento a;
    char codigo[10];

    if (f == NULL) return;

    printf("Codigo a consultar: ");
    scanf("%s", codigo);

    while (fread(&a, sizeof(Alojamiento), 1, f)) {
        if (strcmp(a.codigo, codigo) == 0 && a.activo == 1) {
            printf("\n--- Alojamiento ---\n");
            printf("Codigo: %s\n", a.codigo);
            printf("Nombre: %s\n", a.nombre);
            printf("Direccion: %s\n", a.direccion);
            printf("Tipo: %s\n", a.tipo);
            printf("Cod ciudad: %s\n", a.cod_ciudad);
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// LISTADO
void listadoAlojamientos(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    Alojamiento a;

    if (f == NULL) {
        printf("No hay alojamientos.\n");
        return;
    }

    printf("\n--- LISTADO DE ALOJAMIENTOS ---\n");

    while (fread(&a, sizeof(Alojamiento), 1, f)) {
        if (a.activo == 1) {
            printf("\nCodigo: %s\n", a.codigo);
            printf("Nombre: %s\n", a.nombre);
            printf("Direccion: %s\n", a.direccion);
            printf("Tipo: %s\n", a.tipo);
            printf("Cod ciudad: %s\n", a.cod_ciudad);
        }
    }

    fclose(f);
}

// MENU ADMIN
void menuAlojamiento(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU ALOJAMIENTOS ---\n");
        printf("1. Alta alojamiento\n");
        printf("2. Baja alojamiento\n");
        printf("3. Consultar alojamiento\n");
        printf("4. Listado alojamientos\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: altaAlojamiento(db); break;
            case 2: bajaAlojamiento(db); break;
            case 3: consultarAlojamiento(db); break;
            case 4: listadoAlojamientos(db); break;
        }

    } while (opcion != 0);
}

// MENU PARA CLIENTES
void menuAlojamiento_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU ALOJAMIENTOS ---\n");
        printf("1. Consultar alojamiento\n");
        printf("2. Listado alojamientos\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: consultarAlojamiento(db); break;
            case 2: listadoAlojamientos(db); break;
        }

    } while (opcion != 0);
}
