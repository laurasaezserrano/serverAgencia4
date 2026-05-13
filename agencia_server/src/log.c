#include <stdio.h>
#include <time.h>
#include <string.h>
#include "../include/log.h"

static FILE *g_log = NULL;

void log_abrir(const char *ruta) {
    /* Desactivar buffering de stdout para que todo aparezca inmediatamente */
    setvbuf(stdout, NULL, _IONBF, 0);

    g_log = fopen(ruta, "a");
    if (!g_log) {
        printf("[AVISO] No se pudo abrir el fichero de log: %s\n", ruta);
        fflush(stdout);
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
        fclose(g_log);
        g_log = NULL;
    }
}
