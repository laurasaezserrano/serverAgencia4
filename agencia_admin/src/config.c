#include <stdio.h>
#include <string.h>
#include "../include/config.h"

int cargar_config(const char *ruta, Config *cfg) {
    FILE *f = fopen(ruta, "r");
    if (!f) {
        printf("No se pudo abrir config: %s\n", ruta);
        return 1;
    }

    memset(cfg, 0, sizeof(Config));
    char linea[256];

    while (fgets(linea, sizeof(linea), f)) {
        if (sscanf(linea, "ADMIN_USER=%49s", cfg->admin_user) == 1) continue;
        if (sscanf(linea, "ADMIN_PASS=%49s", cfg->admin_pass) == 1) continue;
        if (sscanf(linea, "DB_PATH=%255s", cfg->db_path) == 1) continue;
        if (sscanf(linea, "LOG_PATH=%255s", cfg->log_path) == 1) continue;
    }

    fclose(f);
    return 0;
}
