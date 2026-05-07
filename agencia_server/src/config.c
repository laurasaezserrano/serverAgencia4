#include <stdio.h>
#include <string.h>
#include "../include/config.h"

int cargar_config(const char *ruta, Config *cfg) {
    FILE *f = fopen(ruta, "r");
    if (!f) {
        printf("No se pudo abrir config: %s\n", ruta);
        /* Defaults */
        cfg->server_port = 8080;
        strncpy(cfg->db_path, "agencia.db", 255);
        strncpy(cfg->log_path, "data/server.log", 255);
        return 0;
    }

    memset(cfg, 0, sizeof(Config));
    cfg->server_port = 8080; /* default port */
    char linea[256];

    while (fgets(linea, sizeof(linea), f)) {
        sscanf(linea, "ADMIN_USER=%49s", cfg->admin_user);
        sscanf(linea, "ADMIN_PASS=%49s", cfg->admin_pass);
        sscanf(linea, "DB_PATH=%255s", cfg->db_path);
        sscanf(linea, "LOG_PATH=%255s", cfg->log_path);
        sscanf(linea, "SERVER_PORT=%d", &cfg->server_port);
    }

    fclose(f);
    return 0;
}
