/*
 * alojamiento.c
 *
 * Created on: 26 mar 2026
 * Author: zaira.diez
 *
 * NOTA: Al usar char* dinamicos en la struct Alojamiento, la
 * serializacion binaria directa (fread/fwrite de la struct) ya no
 * es posible. El archivo .dat usa un formato texto delimitado por '|':
 *   codigo|nombre|direccion|tipo|cod_ciudad|activo\n
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/alojamiento.h"

#define ARCHIVO      "alojamientos.dat"
#define MAX_CAMPO    256
#define SEP          "|"

/* =========================================================
   GESTION DE MEMORIA
   ========================================================= */

void alojamiento_free(Alojamiento *a) {
    if (a == NULL) return;
    free(a->codigo);     a->codigo     = NULL;
    free(a->nombre);     a->nombre     = NULL;
    free(a->direccion);  a->direccion  = NULL;
    free(a->tipo);       a->tipo       = NULL;
    free(a->cod_ciudad); a->cod_ciudad = NULL;
}

/* =========================================================
   HELPERS INTERNOS
   ========================================================= */

/*
 * Lee una linea de stdin y devuelve un puntero malloc'd.
 * El llamante debe liberar con free().
 */
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
 * Guarda un Alojamiento en formato texto al final del archivo.
 * Firma actualizada: recibe puntero para evitar copiar la struct.
 */
void guardarAlojamiento(Alojamiento *a) {
    FILE *f = fopen(ARCHIVO, "a");
    if (f == NULL) {
        printf("Error: No se pudo abrir %s para guardar.\n", ARCHIVO);
        return;
    }
    fprintf(f, "%s|%s|%s|%s|%s|%d\n",
            a->codigo     ? a->codigo     : "",
            a->nombre     ? a->nombre     : "",
            a->direccion  ? a->direccion  : "",
            a->tipo       ? a->tipo       : "",
            a->cod_ciudad ? a->cod_ciudad : "",
            a->activo);
    fclose(f);
}

/*
 * Parsea una linea del .dat y rellena la struct con strdup.
 * Devuelve 1 si OK, 0 si la linea es invalida.
 * El llamante debe llamar a alojamiento_free() cuando termine.
 */
static int parsear_linea(char *linea, Alojamiento *a) {
    /* formato: codigo|nombre|direccion|tipo|cod_ciudad|activo */
    char *tok;
    memset(a, 0, sizeof(*a));

    tok = strtok(linea, SEP); if (!tok) return 0; a->codigo     = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; a->nombre     = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; a->direccion  = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; a->tipo       = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; a->cod_ciudad = strdup(tok);
    tok = strtok(NULL,  SEP); if (!tok) return 0; a->activo     = atoi(tok);
    return 1;
}

/*
 * Reescribe el archivo completo a partir de un array de lineas,
 * aplicando la modificacion indicada por 'codigo_baja' (pone activo=0).
 * Si codigo_baja es NULL, simplemente reescribe sin cambios.
 */
static void reescribir_dat(const char *codigo_baja) {
    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) return;

    /* Leer todas las lineas en memoria */
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

    /* Si hay una baja pendiente, marcar la linea correspondiente */
    if (codigo_baja) {
        int i;
        for (i = 0; i < n; i++) {
            Alojamiento a;
            char *copia = strdup(lineas[i]);
            if (parsear_linea(copia, &a)) {
                if (a.codigo && strcmp(a.codigo, codigo_baja) == 0 && a.activo == 1) {
                    /* Sustituir linea con activo=0 */
                    free(lineas[i]);
                    lineas[i] = NULL;
                    char nuevo[MAX_CAMPO * 6];
                    snprintf(nuevo, sizeof(nuevo), "%s|%s|%s|%s|%s|0\n",
                             a.codigo, a.nombre, a.direccion, a.tipo, a.cod_ciudad);
                    lineas[i] = strdup(nuevo);
                }
                alojamiento_free(&a);
            }
            free(copia);
        }
    }

    /* Reescribir */
    f = fopen(ARCHIVO, "w");
    if (f) {
        int i;
        for (i = 0; i < n; i++) {
            if (lineas[i]) fputs(lineas[i], f);
        }
        fclose(f);
    }

    int i;
    for (i = 0; i < n; i++) free(lineas[i]);
    free(lineas);
}

/* =========================================================
   ALTA
   ========================================================= */

void altaAlojamiento(sqlite3 *db) {
    (void)db;
    Alojamiento a;
    memset(&a, 0, sizeof(a));

    printf("\n--- ALTA ALOJAMIENTO ---\n");

    a.codigo     = leer_campo("Codigo: ");
    a.nombre     = leer_campo("Nombre: ");
    a.direccion  = leer_campo("Direccion: ");
    a.tipo       = leer_campo("Tipo (hotel, hostal...): ");
    a.cod_ciudad = leer_campo("Codigo ciudad: ");
    a.activo     = 1;

    guardarAlojamiento(&a);
    printf("Alojamiento dado de alta.\n");

    alojamiento_free(&a);
}

/* =========================================================
   BAJA (logica)
   ========================================================= */

void bajaAlojamiento(sqlite3 *db) {
    (void)db;
    printf("\n--- BAJA ALOJAMIENTO ---\n");

    char *codigo = leer_campo("Codigo a dar de baja: ");
    if (!codigo) return;

    /* Verificar que existe y esta activo */
    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay alojamientos registrados.\n");
        free(codigo);
        return;
    }

    char buf[MAX_CAMPO * 6];
    int encontrado = 0;

    while (fgets(buf, sizeof(buf), f)) {
        Alojamiento a;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &a)) {
            if (a.codigo && strcmp(a.codigo, codigo) == 0) {
                encontrado = 1;
                if (a.activo == 0) {
                    printf("El alojamiento ya estaba dado de baja.\n");
                    alojamiento_free(&a);
                    free(copia);
                    fclose(f);
                    free(codigo);
                    return;
                }
                alojamiento_free(&a);
                free(copia);
                break;
            }
            alojamiento_free(&a);
        }
        free(copia);
    }
    fclose(f);

    if (!encontrado) {
        printf("No encontrado.\n");
        free(codigo);
        return;
    }

    reescribir_dat(codigo);
    printf("Alojamiento dado de baja.\n");
    free(codigo);
}

/* =========================================================
   CONSULTAR
   ========================================================= */

void consultarAlojamiento(sqlite3 *db) {
    (void)db;
    printf("\n--- CONSULTAR ALOJAMIENTO ---\n");

    char *codigo = leer_campo("Codigo a consultar: ");
    if (!codigo) return;

    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay alojamientos registrados.\n");
        free(codigo);
        return;
    }

    char buf[MAX_CAMPO * 6];

    while (fgets(buf, sizeof(buf), f)) {
        Alojamiento a;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &a)) {
            if (a.codigo && strcmp(a.codigo, codigo) == 0 && a.activo == 1) {
                printf("\n--- Alojamiento ---\n");
                printf("Codigo:     %s\n", a.codigo);
                printf("Nombre:     %s\n", a.nombre);
                printf("Direccion:  %s\n", a.direccion);
                printf("Tipo:       %s\n", a.tipo);
                printf("Cod ciudad: %s\n", a.cod_ciudad);
                alojamiento_free(&a);
                free(copia);
                fclose(f);
                free(codigo);
                return;
            }
            alojamiento_free(&a);
        }
        free(copia);
    }

    fclose(f);
    printf("No encontrado.\n");
    free(codigo);
}

/* =========================================================
   LISTADO
   ========================================================= */

void listadoAlojamientos(sqlite3 *db) {
    (void)db;
    FILE *f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        printf("No hay alojamientos.\n");
        return;
    }

    printf("\n--- LISTADO DE ALOJAMIENTOS ---\n");

    char buf[MAX_CAMPO * 6];

    while (fgets(buf, sizeof(buf), f)) {
        Alojamiento a;
        char *copia = strdup(buf);
        if (parsear_linea(copia, &a) && a.activo == 1) {
            printf("\nCodigo:     %s\n", a.codigo);
            printf("Nombre:     %s\n",   a.nombre);
            printf("Direccion:  %s\n",   a.direccion);
            printf("Tipo:       %s\n",   a.tipo);
            printf("Cod ciudad: %s\n",   a.cod_ciudad);
        }
        alojamiento_free(&a);
        free(copia);
    }

    fclose(f);
}

/* =========================================================
   MENUS
   ========================================================= */

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
        limpiar_stdin();

        switch (opcion) {
            case 1: altaAlojamiento(db);    break;
            case 2: bajaAlojamiento(db);    break;
            case 3: consultarAlojamiento(db); break;
            case 4: listadoAlojamientos(db);  break;
            case 0: printf("Volviendo...\n"); break;
            default: printf("Opcion no valida.\n");
        }

    } while (opcion != 0);
}

void menuAlojamiento_cliente(sqlite3 *db) {
    int opcion;

    do {
        printf("\n--- MENU ALOJAMIENTOS ---\n");
        printf("1. Consultar alojamiento\n");
        printf("2. Listado alojamientos\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);
        limpiar_stdin();

        switch (opcion) {
            case 1: consultarAlojamiento(db);  break;
            case 2: listadoAlojamientos(db);   break;
            case 0: printf("Volviendo...\n");  break;
            default: printf("Opcion no valida.\n");
        }

    } while (opcion != 0);
}
