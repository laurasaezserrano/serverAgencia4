#include <stdio.h>
#include <time.h>
#include <string.h>
#include "../include/log.h"

static FILE *g_log = NULL;

/*
 * log_abrir — abre (o crea) el fichero de log en modo append.
 * Llamar una vez al arrancar el servidor.
 */
void log_abrir(const char *ruta) {
    g_log = fopen(ruta, "a");
    if (!g_log) {
        printf("[AVISO] No se pudo abrir el fichero de log: %s\n", ruta);
    }
}

/*
 * log_escribir — escribe una linea con timestamp en el log.
 * Formato: [YYYY-MM-DD HH:MM:SS] mensaje
 */
void log_escribir(const char *mensaje) {
    time_t ahora = time(NULL);
    struct tm *t  = localtime(&ahora);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

    /* Escribir en el fichero si esta abierto */
    if (g_log) {
        fprintf(g_log, "[%s] %s\n", ts, mensaje);
        fflush(g_log);   /* forzar escritura inmediata en disco */
    }

    /* Mostrar tambien por consola del servidor */
    printf("[%s] %s\n", ts, mensaje);
}

/*
 * log_cerrar — cierra el fichero de log.
 * Llamar antes de salir del servidor.
 */
void log_cerrar(void) {
    if (g_log) {
        fclose(g_log);
        g_log = NULL;
    }
}
