#ifndef LOG_H_
#define LOG_H_

void log_init(const char *ruta);
void log_write(const char *fmt, ...);
void log_close(void);

#endif
