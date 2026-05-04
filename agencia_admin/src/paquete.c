/*
 * paquete.c
 *
 *  Created on: 26 mar 2026
 *      Author: zaira.diez
 */

#include <stdio.h>
#include <string.h>
#include "../include/paquete.h"

#define ARCHIVO "paquetes.dat"

// Guardar paquete
int guardarPaquete(Paquete p) {

    // Abrimos en modo "ab" (append binary) para añadir al final
    FILE *f = fopen(ARCHIVO, "ab");

    if (f == NULL) {
    	printf("Error: No se pudo abrir el archivo %s para guardar.\n", ARCHIVO);
        return 0; // Fallo
    }

    // fwrite devuelve el número de elementos escritos correctamente
    size_t escritos = fwrite(&p, sizeof(Paquete), 1, f);

    fclose(f);

    if (escritos == 1) {
        return 1; // Éxito
    } else {
        printf("Error: Fallo al escribir los datos en el disco.\n");
        return 0; // Fallo
    }
}

// CREAR
void crearPaquete(sqlite3 *db) {
    Paquete p;

    printf("Codigo: ");
    scanf("%d", &p.cod);

    printf("Nombre: ");
    scanf(" %[^\n]", p.nombre);

    printf("Precio: ");
    scanf("%f", &p.precio);

    printf("Codigo ciudad origen: ");
    scanf("%s", p.cod_ciudad_origen);

    printf("Codigo ciudad destino: ");
    scanf("%s", p.cod_ciudad_destino);

    printf("Plazas totales: ");
    scanf("%d", &p.plazas_totales);

    p.plazas_disponibles = p.plazas_totales;
    p.activo = 1;

    if (guardarPaquete(p)) {
        printf("Paquete guardado correctamente.\n");
    } else {
        printf("Hubo un problema al guardar el paquete. Inténtelo de nuevo.\n");
    }

    printf("Paquete creado correctamente.\n");
}

// ELIMINAR (baja lógica)
void eliminarPaquete(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb+");
    Paquete p;
    int codigo;

    if (f == NULL) return;

    printf("Codigo a eliminar: ");
    scanf("%d", &codigo);

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.cod == codigo && p.activo == 1) {
            p.activo = 0;
            fseek(f, -sizeof(Paquete), SEEK_CUR);
            fwrite(&p, sizeof(Paquete), 1, f);
            printf("Paquete eliminado.\n");
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// CONSULTAR
void consultarPaquete(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    Paquete p;
    int codigo;

    if (f == NULL) return;

    printf("Codigo a consultar: ");
    scanf("%d", &codigo);

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.cod == codigo && p.activo == 1) {
            printf("\n--- PAQUETE ---\n");
            printf("Codigo: %d\n", p.cod);
            printf("Nombre: %s\n", p.nombre);
            printf("Precio: %.2f\n", p.precio);
            printf("Origen: %s\n", p.cod_ciudad_origen);
            printf("Destino: %s\n", p.cod_ciudad_destino);
            printf("Plazas totales: %d\n", p.plazas_totales);
            printf("Plazas disponibles: %d\n", p.plazas_disponibles);
            fclose(f);
            return;
        }
    }

    printf("No encontrado.\n");
    fclose(f);
}

// LISTADO
void listadoPaquetes(sqlite3 *db) {
    FILE *f = fopen(ARCHIVO, "rb");
    Paquete p;

    if (f == NULL) {
        printf("No hay paquetes.\n");
        return;
    }

    printf("\n--- LISTADO DE PAQUETES ---\n");

    while (fread(&p, sizeof(Paquete), 1, f)) {
        if (p.activo == 1) {
            printf("\nCodigo: %d\n", p.cod);
            printf("Nombre: %s\n", p.nombre);
            printf("Precio: %.2f\n", p.precio);
            printf("Origen: %s\n", p.cod_ciudad_origen);
            printf("Destino: %s\n", p.cod_ciudad_destino);
            printf("Plazas totales: %d\n", p.plazas_totales);
            printf("Plazas disponibles: %d\n", p.plazas_disponibles);
        }
    }

    fclose(f);
}

// MENU ADMIN
void menuPaquetes(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU PAQUETES ---\n");
        printf("1. Crear paquete\n");
        printf("2. Eliminar paquete\n");
        printf("3. Consultar paquete\n");
        printf("4. Listado paquetes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: crearPaquete(db); break;
            case 2: eliminarPaquete(db); break;
            case 3: consultarPaquete(db); break;
            case 4: listadoPaquetes(db); break;
            case 0: printf("Volviendo al menú principal...\n"); break;
            default: printf("Opción no válida.\n");
        }

    } while (opcion != 0);
}

//MENU PARA CLIENTES
void menuPaquetes_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU PAQUETES ---\n");
        printf("1. Consultar paquete\n");
        printf("2. Listado paquetes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: consultarPaquete(db); break;
            case 2: listadoPaquetes(db); break;
            case 0: printf("Volviendo al menú principal...\n"); break;
            default: printf("Opción no válida.\n");
        }

    } while (opcion != 0);
}
