#ifndef LOG_H_
#define LOG_H_

/*
 * log.h — registro de eventos del servidor en fichero de texto.
 *
 * Uso tipico:
 *   log_abrir("data/server.log");
 *   log_escribir("Cliente conectado desde 127.0.0.1");
 *   log_cerrar();
 */

void log_abrir(const char *ruta);
void log_escribir(const char *mensaje);
void log_cerrar(void);

#endif /* LOG_H_ */
