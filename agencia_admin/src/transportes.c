/*
 * transportes.c
 *
 * Created on: 26 mar 2026
 * Author: zaira.diez
 *
 * NOTA: Al usar char* dinamicos en la struct Transporte, la serializacion
 * binaria directa ya no es posible. El archivo .dat usa formato texto:
 *   codigo|tipo|fecha_salida|fecha_llegada|cod_paquete|activo\n
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/transporte.h"

#define ARCHIVO    "transportes.dat"
#define MAX_CAMPO  256
#define SEP        "|"

/* =========================================================
   GESTION DE MEMORIA
   ========================================================= */

void transporte_free(Transporte *t) {
    if (t == NULL) return;
    free(t->codigo);        t->codigo        = NULL;
    free(t->tipo);          t->tipo          = NULL;
    free(t->fecha_salida);  t->fecha_salida  = NULL;
    free(t->fecha_llegada); t->fecha_llegada = NULL;
}

/* =========================================================
   HELPERS INTERNOS
   ========================================================= */

static char *leer_campo(const char *mensaje) {
    char buf[MAX_CAMPO];
    printf("%s", mensaje);
    if (fgets(buf, sizeof(buf), stdin) == NULL) return NULL;
    buf[strcspn(buf, "\n")] = '\0';
    return strdup(buf);
}

static void limpiar_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/*
 * Parsea una linea del .dat y rellena la struct con strdup.
 * El llamante debe llamar a transporte_free() cuando termine.
 */
static int parsear_linea(char *linea, Transporte *t) {
    char *tok;
    memset(t, 0, sizeof(*t));

    tok = strtok(linea, SEP); if (!tok) return 0; t->codigo        = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; t->tipo          = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; t->fecha_salida  = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; t->fecha_llegada = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; t->cod_paquete   = atoi(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; t->activo        = atoi(tok);
    return 1;
}

/* Serializa un Transporte como una linea de texto. */
static void guardarTransporte_interno(FILE *f, const Transporte *t) {
    fprintf(f, "%s|%s|%s|%s|%d|%d\n",
            t->codigo        ? t->codigo        : "",
            t->tipo          ? t->tipo          : "",
            t->fecha_salida  ? t->fecha_salida  : "",
            t->fecha_llegada ? t->fecha_llegada : "",
            t->cod_paquete,
            t->activo);
}

/*
 * Reescribe el archivo aplicando la operacion indicada:
 *   - Si codigo_baja != NULL: marca ese registro activo=0.
 *   - Si codigo_asoc != NULL: actualiza cod_paquete al nuevo valor.
 */
static void reescribir_dat(const char *codigo_baja,
                            const char *codigo_asoc, int nuevo_cod_paquete) {
    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) return;

    char **lineas = NULL;
    int    n      = 0;
    char   buf[MAX_CAMPO * 6];

    while (fgets(buf, sizeof(buf), f)) {
        char **tmp = realloc(lineas, (n + 1) * sizeof(char *));
        if (!tmp) break;
        lineas = tmp;
        lineas[n++] = strdup(buf);
    }
    fclose(f);

    int i;
    for (i = 0; i < n; i++) {
        Transporte t;
        char *copia = strdup(lineas[i]);
        if (!parsear_linea(copia, &t)) {
            free(copia);
            continue;
        }

        int modificado = 0;

        if (codigo_baja && t.codigo && strcmp(t.codigo, codigo_baja) == 0 && t.activo == 1) {
            t.activo = 0;
            modificado = 1;
        }

        if (codigo_asoc && t.codigo && strcmp(t.codigo, codigo_asoc) == 0 && t.activo == 1) {
            t.cod_paquete = nuevo_cod_paquete;
            modificado = 1;
        }

        if (modificado) {
            char nuevo[MAX_CAMPO * 6];
            snprintf(nuevo, sizeof(nuevo), "%s|%s|%s|%s|%d|%d\n",
                     t.codigo        ? t.codigo        : "",
                     t.tipo          ? t.tipo          : "",
                     t.fecha_salida  ? t.fecha_salida  : "",
                     t.fecha_llegada ? t.fecha_llegada : "",
                     t.cod_paquete,
                     t.activo);
            free(lineas[i]);
            lineas[i] = strdup(nuevo);
        }

        transporte_free(&t);
        free(copia);
    }

    f = fopen(ARCHIVO, "w");
    if (f) {
        for (i = 0; i < n; i++) {
            if (lineas[i]) fputs(lineas[i], f);
        }
        fclose(f);
    }

    for (i = 0; i < n; i++) free(lineas[i]);
    free(lineas);
}

/* =========================================================
   ALTA
   ========================================================= */

void altaTransporte(sqlite3 *db) {
    (void)db;
    Transporte t;
    memset(&t, 0, sizeof(t));

    printf("\n--- ALTA TRANSPORTE ---\n");

    t.codigo        = leer_campo("Codigo: ");
    t.tipo          = leer_campo("Tipo (avion, tren, autobus...): ");
    t.fecha_salida  = leer_campo("Fecha salida (dd/mm/yyyy): ");
    t.fecha_llegada = leer_campo("Fecha llegada (dd/mm/yyyy): ");

    char *tmp = leer_campo("Codigo de paquete: ");
    t.cod_paquete = tmp ? atoi(tmp) : 0;
    free(tmp);

    t.activo = 1;

    FILE *f = fopen(ARCHIVO, "a");
    if (f == NULL) {
        printf("Error: No se pudo abrir %s para guardar.\n", ARCHIVO);
        transporte_free(&t);
        return;
    }

    guardarTransporte_interno(f, &t);
    fclose(f);

    printf("Transporte dado de alta correctamente.\n");
    transporte_free(&t);
}

/* =========================================================
   BAJA
   ========================================================= */

void bajaTransporte(void) {
    printf("\n--- BAJA TRANSPORTE ---\n");

    char *codigo = leer_campo("Codigo a dar de baja: ");
    if (!codigo) return;

    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay transportes registrados.\n");
        free(codigo);
        return;
    }

    char buf[MAX_CAMPO * 6];
    int encontrado = 0;

    while (fgets(buf, sizeof(buf), f)) {
        Transporte t;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &t) && t.codigo && strcmp(t.codigo, codigo) == 0) {
            encontrado = 1;
            if (t.activo == 0) {
                printf("El transporte ya estaba dado de baja.\n");
                transporte_free(&t);
                free(copia);
                fclose(f);
                free(codigo);
                return;
            }
            transporte_free(&t);
            free(copia);
            break;
        }
        transporte_free(&t);
        free(copia);
    }
    fclose(f);

    if (!encontrado) {
        printf("No encontrado.\n");
        free(codigo);
        return;
    }

    reescribir_dat(codigo, NULL, 0);
    printf("Transporte dado de baja.\n");
    free(codigo);
}

/* =========================================================
   CONSULTAR
   ========================================================= */

void consultarTransporte(void) {
    printf("\n--- CONSULTAR TRANSPORTE ---\n");

    char *codigo = leer_campo("Codigo a consultar: ");
    if (!codigo) return;

    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay transportes registrados.\n");
        free(codigo);
        return;
    }

    char buf[MAX_CAMPO * 6];

    while (fgets(buf, sizeof(buf), f)) {
        Transporte t;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &t) && t.codigo
                && strcmp(t.codigo, codigo) == 0 && t.activo == 1) {
            printf("\n--- Transporte ---\n");
            printf("Codigo:       %s\n", t.codigo);
            printf("Tipo:         %s\n", t.tipo);
            printf("Salida:       %s\n", t.fecha_salida);
            printf("Llegada:      %s\n", t.fecha_llegada);
            printf("Cod paquete:  %d\n", t.cod_paquete);
            transporte_free(&t);
            free(copia);
            fclose(f);
            free(codigo);
            return;
        }
        transporte_free(&t);
        free(copia);
    }

    fclose(f);
    printf("No encontrado.\n");
    free(codigo);
}

/* =========================================================
   ASOCIAR TRANSPORTE A PAQUETE
   ========================================================= */

void asociarTransporte(void) {
    printf("\n--- ASOCIAR TRANSPORTE ---\n");

    char *codigo = leer_campo("Codigo transporte: ");
    if (!codigo) return;

    char *tmp = leer_campo("Nuevo codigo de paquete: ");
    int nuevoCod = tmp ? atoi(tmp) : 0;
    free(tmp);

    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay transportes registrados.\n");
        free(codigo);
        return;
    }

    char buf[MAX_CAMPO * 6];
    int encontrado = 0;

    while (fgets(buf, sizeof(buf), f)) {
        Transporte t;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &t) && t.codigo
                && strcmp(t.codigo, codigo) == 0 && t.activo == 1) {
            encontrado = 1;
            transporte_free(&t);
            free(copia);
            break;
        }
        transporte_free(&t);
        free(copia);
    }
    fclose(f);

    if (!encontrado) {
        printf("No encontrado.\n");
        free(codigo);
        return;
    }

    reescribir_dat(NULL, codigo, nuevoCod);
    printf("Asociacion realizada.\n");
    free(codigo);
}

/* =========================================================
   LISTADO
   ========================================================= */

void listadoTransportes(void) {
    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay transportes.\n");
        return;
    }

    printf("\n--- LISTADO DE TRANSPORTES ---\n");

    char buf[MAX_CAMPO * 6];

    while (fgets(buf, sizeof(buf), f)) {
        Transporte t;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &t) && t.activo == 1) {
            printf("\nCodigo:       %s\n", t.codigo);
            printf("Tipo:         %s\n",   t.tipo);
            printf("Salida:       %s\n",   t.fecha_salida);
            printf("Llegada:      %s\n",   t.fecha_llegada);
            printf("Cod paquete:  %d\n",   t.cod_paquete);
        }
        transporte_free(&t);
        free(copia);
    }

    fclose(f);
}

/* =========================================================
   MENUS
   ========================================================= */

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
        limpiar_stdin();

        switch (opcion) {
            case 1: altaTransporte(db);    break;
            case 2: bajaTransporte();      break;
            case 3: consultarTransporte(); break;
            case 4: asociarTransporte();   break;
            case 5: listadoTransportes();  break;
            case 0: printf("Volviendo...\n"); break;
            default: printf("Opcion no valida.\n");
        }

    } while (opcion != 0);
}

void menuTransporte_cliente(sqlite3 *db) {
    (void)db;
    int opcion;

    do {
        printf("\n--- MENU TRANSPORTES ---\n");
        printf("1. Consultar transporte\n");
        printf("2. Listado transportes\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);
        limpiar_stdin();

        switch (opcion) {
            case 1: consultarTransporte(); break;
            case 2: listadoTransportes();  break;
            case 0: printf("Volviendo...\n"); break;
            default: printf("Opcion no valida.\n");
        }

    } while (opcion != 0);
}
