#include <stdio.h>
#include <time.h>
#include <string.h>
#include <direct.h>     /* _mkdir Windows/MinGW */
#include "../include/log.h"

static FILE *g_log = NULL;

static void crear_dir(const char *ruta) {
    char dir[256];
    strncpy(dir, ruta, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    char *p = dir + strlen(dir) - 1;
    while (p > dir && *p != '/' && *p != '\\') p--;
    if (p > dir) { *p = '\0'; _mkdir(dir); }
}

void log_abrir(const char *ruta) {
    setvbuf(stdout, NULL, _IONBF, 0);  /* consola sin buffer */
    crear_dir(ruta);
    g_log = fopen(ruta, "a");
    if (!g_log)
        printf("[AVISO] No se pudo abrir log: %s\n", ruta);
    else {
        /* escribir cabecera directamente para confirmar que el fichero funciona */
        time_t ahora = time(NULL);
        struct tm *t = localtime(&ahora);
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);
        fprintf(g_log, "[%s] === Servidor arrancado ===\n", ts);
        fflush(g_log);
        printf("[%s] Log abierto en: %s\n", ts, ruta);
    }
}

void log_escribir(const char *mensaje) {
    time_t ahora = time(NULL);
    struct tm *t  = localtime(&ahora);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

    if (g_log) {
        fprintf(g_log, "[%s] %s\n", ts, mensaje);
        fflush(g_log);
    }
    printf("[%s] %s\n", ts, mensaje);
    fflush(stdout);
}

void log_cerrar(void) {
    if (g_log) {
        log_escribir("=== Servidor cerrado ===");
        fclose(g_log);
        g_log = NULL;
    }
}
