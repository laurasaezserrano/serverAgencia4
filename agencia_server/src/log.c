#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "../include/log.h"

static FILE *log_file = NULL;

void log_init(const char *ruta) {
    log_file = fopen(ruta, "a");
    if (!log_file) {
        fprintf(stderr, "[LOG] No se pudo abrir fichero de log: %s\n", ruta);
    }
}

void log_write(const char *fmt, ...) {
    if (!log_file) return;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "[%s] ", timestamp);

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);

    /* También mostrar en consola */
    printf("[%s] ", timestamp);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void log_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
