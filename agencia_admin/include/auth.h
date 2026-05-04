#ifndef AUTH_H_
#define AUTH_H_

#include "sqlite3.h"

// Definición de estructura
typedef struct {
    char usuario[50];
    int nivel_permiso;
} Sesion;

int login_admin(sqlite3 *db);
void registrar_usuario(sqlite3 *db);
int verificar_credenciales(sqlite3 *db, const char *user, const char *pass);

#endif /* AUTH_H_ */
