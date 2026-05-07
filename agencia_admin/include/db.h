#ifndef DB_H_
#define DB_H_

#include "sqlite3.h"

int db_abrir(sqlite3 **db, const char *ruta);
void db_cerrar(sqlite3 *db);
int db_crear_tablas(sqlite3 *db);

#endif /* DB_H_ */
