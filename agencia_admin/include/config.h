#ifndef CONFIG_H_
#define CONFIG_H_


typedef struct {
    char admin_user[50];
    char admin_pass[50];
    char db_path[256];
    char log_path[256];
} Config;

int cargar_config(const char *ruta, Config *cfg);


#endif /* CONFIG_H_ */
