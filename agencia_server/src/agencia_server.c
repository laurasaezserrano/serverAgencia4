/*
 * agencia_server.c
 * ================
 * Servidor TCP de la Agencia de Viajes — Fase 2.
 *
 * Responsabilidades:
 *   1. Leer config.ini y abrir agencia.db (misma BD que el admin).
 *   2. Abrir un socket TCP y esperar la conexion del cliente.
 *   3. Bucle: recibir trama -> parsear OP -> ejecutar en BD -> responder.
 *   4. Registrar cada operacion en server.log.
 *   5. Cierre limpio al recibir BYE o SIGINT.
 *
 * Plataforma: Windows (Winsock2).
 * Compilacion (MinGW/Eclipse CDT):
 *   gcc agencia_server.c db.c config.c log.c clientes.c paquete.c
 *       alojamiento.c transportes.c sqlite3.c -o agencia_server.exe
 *       -lws2_32 -I../include
 */

/* ── Winsock debe incluirse antes que cualquier windows.h ──────── */
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/protocolo.h"
#include "../include/config.h"
#include "../include/log.h"
#include "../include/db.h"
#include "../include/hash.h"
#include "../include/clientes.h"
#include "../include/paquete.h"
#include "../include/alojamiento.h"
#include "../include/transporte.h"
#include "../include/sqlite3.h"

/* ws2_32 debe enlazarse manualmente en Eclipse:
 * Project Properties -> C/C++ Build -> Settings
 * -> MinGW C Linker -> Libraries -> Libraries (-l) -> añadir: ws2_32
 */

/* ── Prototipos internos ────────────────────────────────────────── */
static void   procesar_trama(SOCKET cliente, sqlite3 *db, const char *trama);
static void   enviar_respuesta(SOCKET s, const char *resp);

/* Handlers: uno por codigo de operacion */
static void   handle_LOG(SOCKET s, sqlite3 *db, char *params);
static void   handle_ACL(SOCKET s, sqlite3 *db, char *params);
static void   handle_BCL(SOCKET s, sqlite3 *db, char *params);
static void   handle_MCL(SOCKET s, sqlite3 *db, char *params);
static void   handle_GCL(SOCKET s, sqlite3 *db, char *params);
static void   handle_LCL(SOCKET s, sqlite3 *db);
static void   handle_LPQ(SOCKET s, sqlite3 *db);
static void   handle_GPQ(SOCKET s, sqlite3 *db, char *params);
static void   handle_APQ(SOCKET s, sqlite3 *db, char *params);
static void   handle_BPQ(SOCKET s, sqlite3 *db, char *params);
static void   handle_LAL(SOCKET s, sqlite3 *db);
static void   handle_LTR(SOCKET s, sqlite3 *db);
static void   handle_ARE(SOCKET s, sqlite3 *db, char *params);
static void   handle_BRE(SOCKET s, sqlite3 *db, char *params);
static void   handle_LRC(SOCKET s, sqlite3 *db, char *params);
static void   handle_IOC(SOCKET s, sqlite3 *db);
static void   handle_IRK(SOCKET s, sqlite3 *db);
static void   handle_IDS(SOCKET s, sqlite3 *db);

/* ── Utilidades internas ────────────────────────────────────────── */

/*
 * siguiente_token — extrae el siguiente campo separado por '|'.
 * Usa strtok internamente; llamar con el puntero al inicio la primera
 * vez y NULL en las siguientes (igual que strtok estandar).
 */
static char *siguiente_token(char *cadena) {
    return strtok(cadena, "|");
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════ */
int main(void) {
    /* ── 1. Inicializar Winsock ──────────────────────────────── */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Error inicializando Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    /* ── 2. Cargar configuracion ─────────────────────────────── */
    Config cfg;
    if (cargar_config("data/config.ini", &cfg) != 0) {
        printf("Error cargando config.ini\n");
        WSACleanup();
        return 1;
    }

    /* ── 3. Abrir log ────────────────────────────────────────── */
    log_abrir(cfg.log_path[0] ? cfg.log_path : "data/server.log");
    log_escribir("=== Servidor arrancado ===");

    /* ── 4. Abrir base de datos ──────────────────────────────── */
    sqlite3 *db;
    if (db_abrir(&db, cfg.db_path) != 0) {
        log_escribir("ERROR: no se pudo abrir la base de datos");
        log_cerrar();
        WSACleanup();
        return 1;
    }
    db_crear_tablas(db);
    log_escribir("Base de datos abierta correctamente");

    /* ── 5. Crear socket servidor ────────────────────────────── */
    SOCKET srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srv == INVALID_SOCKET) {
        log_escribir("ERROR: no se pudo crear el socket");
        db_cerrar(db);
        log_cerrar();
        WSACleanup();
        return 1;
    }

    /* Reutilizar el puerto inmediatamente despues de cerrar */
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    /* ── 6. Bind ─────────────────────────────────────────────── */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;   /* acepta conexiones de cualquier IP */
    addr.sin_port        = htons((unsigned short)cfg.port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        log_escribir("ERROR: bind fallido (puerto en uso?)");
        closesocket(srv);
        db_cerrar(db);
        log_cerrar();
        WSACleanup();
        return 1;
    }

    /* ── 7. Listen ───────────────────────────────────────────── */
    listen(srv, 1);   /* cola de 1: solo atendemos un cliente a la vez */

    char msg_listen[128];
    sprintf(msg_listen, "Escuchando en puerto %d... (esperando cliente)", cfg.port);
    log_escribir(msg_listen);

    /* ── 8. Accept — bloqueante hasta que conecte el cliente ─── */
    struct sockaddr_in addr_cli;
    int len_cli = sizeof(addr_cli);
    SOCKET cli = accept(srv, (struct sockaddr*)&addr_cli, &len_cli);
    if (cli == INVALID_SOCKET) {
        log_escribir("ERROR: accept fallido");
        closesocket(srv);
        db_cerrar(db);
        log_cerrar();
        WSACleanup();
        return 1;
    }

    /* Obtener IP del cliente para el log */
    /* inet_ntoa es compatible con todas las versiones de MinGW */
    char msg_conn[128];
    sprintf(msg_conn, "Cliente conectado desde %s", inet_ntoa(addr_cli.sin_addr));
    log_escribir(msg_conn);

    /* ── 9. Bucle de atencion ────────────────────────────────── */
    char buf[PROTO_BUF];
    int  activo = 1;

    while (activo) {
        memset(buf, 0, sizeof(buf));
        int recibidos = recv(cli, buf, sizeof(buf) - 1, 0);

        if (recibidos <= 0) {
            /* El cliente cerro la conexion */
            log_escribir("Cliente desconectado");
            break;
        }

        buf[recibidos] = '\0';

        /* Puede llegar mas de una trama junta; procesamos hasta '#' */
        char *ini = buf;
        char *fin;
        while ((fin = strchr(ini, PROTO_FIN)) != NULL) {
            *fin = '\0';   /* terminar la trama aqui */

            /* Ignorar tramas vacias */
            if (strlen(ini) > 0) {
                /* BYE cierra el bucle */
                if (strncmp(ini, OP_LOGOUT, 3) == 0) {
                    log_escribir("Cliente solicito cierre (BYE)");
                    enviar_respuesta(cli, "OK|Hasta luego|#");
                    activo = 0;
                    break;
                }
                procesar_trama(cli, db, ini);
            }
            ini = fin + 1;
        }
    }

    /* ── 10. Cierre limpio ───────────────────────────────────── */
    closesocket(cli);
    closesocket(srv);
    db_cerrar(db);
    log_escribir("=== Servidor cerrado ===");
    log_cerrar();
    WSACleanup();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * DESPACHO DE OPERACIONES
 * Extrae el COD_OP de la trama y llama al handler correspondiente.
 * Formato de entrada: "COD_OP|param1|param2|..."  (sin '#' final)
 * ═══════════════════════════════════════════════════════════════ */
static void procesar_trama(SOCKET cliente, sqlite3 *db, const char *trama) {
    /* Copiar para no modificar el original con strtok */
    char copia[PROTO_BUF];
    strncpy(copia, trama, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    /* Extraer el codigo de operacion (primeros 3 caracteres antes de '|') */
    char *op     = siguiente_token(copia);
    char *params = siguiente_token(NULL);   /* resto de parametros */

    if (!op) {
        enviar_respuesta(cliente, "ERR|Trama vacia|#");
        return;
    }

    /* Log de la operacion recibida */
    char logmsg[256];
    sprintf(logmsg, "OP recibida: %s", trama);
    log_escribir(logmsg);

    /* Despacho */
    if      (strcmp(op, OP_LOGIN)    == 0) handle_LOG(cliente, db, params);
    else if (strcmp(op, OP_ALTA_CLI) == 0) handle_ACL(cliente, db, params);
    else if (strcmp(op, OP_BAJA_CLI) == 0) handle_BCL(cliente, db, params);
    else if (strcmp(op, OP_MOD_CLI)  == 0) handle_MCL(cliente, db, params);
    else if (strcmp(op, OP_GET_CLI)  == 0) handle_GCL(cliente, db, params);
    else if (strcmp(op, OP_LIST_CLI) == 0) handle_LCL(cliente, db);
    else if (strcmp(op, OP_LIST_PQT) == 0) handle_LPQ(cliente, db);
    else if (strcmp(op, OP_GET_PQT)  == 0) handle_GPQ(cliente, db, params);
    else if (strcmp(op, OP_ALTA_PQT) == 0) handle_APQ(cliente, db, params);
    else if (strcmp(op, OP_BAJA_PQT) == 0) handle_BPQ(cliente, db, params);
    else if (strcmp(op, OP_LIST_ALO) == 0) handle_LAL(cliente, db);
    else if (strcmp(op, OP_LIST_TRP) == 0) handle_LTR(cliente, db);
    else if (strcmp(op, OP_ALTA_RES) == 0) handle_ARE(cliente, db, params);
    else if (strcmp(op, OP_BAJA_RES) == 0) handle_BRE(cliente, db, params);
    else if (strcmp(op, OP_LIST_RES_CLI) == 0) handle_LRC(cliente, db, params);
    else if (strcmp(op, OP_INF_OCUP) == 0) handle_IOC(cliente, db);
    else if (strcmp(op, OP_INF_RANK) == 0) handle_IRK(cliente, db);
    else if (strcmp(op, OP_INF_DEST) == 0) handle_IDS(cliente, db);
    else {
        enviar_respuesta(cliente, "ERR|Operacion desconocida|#");
    }
}

/* ── Enviar respuesta al cliente ──────────────────────────────── */
static void enviar_respuesta(SOCKET s, const char *resp) {
    send(s, resp, (int)strlen(resp), 0);
}

/* ═══════════════════════════════════════════════════════════════
 * HANDLERS
 * ═══════════════════════════════════════════════════════════════ */

/* LOG|usuario|clave|  ->  OK|AUTH|rol|#  o  ERR|...|# */
static void handle_LOG(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Parametros insuficientes|#"); return; }

    char *usuario = params;
    char *clave   = siguiente_token(NULL);
    if (!clave) { enviar_respuesta(s, "ERR|Parametros insuficientes|#"); return; }

    /* Bug fix 1: hashear la clave recibida antes de comparar con la BD,
     * igual que hace auth.c del admin con sha256_hex()                  */
    char hash_clave[65];
    sha256_hex(clave, hash_clave);

    sqlite3_stmt *stmt;
    const char *sql = "SELECT rol FROM usuarios WHERE username=? AND password=?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_text(stmt, 1, usuario,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash_clave, -1, SQLITE_STATIC);

    char resp[PROTO_BUF];
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *rol = (const char*)sqlite3_column_text(stmt, 0);
        sprintf(resp, "OK|AUTH|%s|#", rol);
    } else {
        strcpy(resp, "ERR|Credenciales incorrectas|#");
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, resp);
}

/* ACL|dni|nombre|apellidos|tlf|email|fnac|  ->  OK|Cliente dado de alta|# */
static void handle_ACL(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Parametros insuficientes|#"); return; }

    char *dni       = params;
    char *nombre    = siguiente_token(NULL);
    char *apellidos = siguiente_token(NULL);
    char *tlf       = siguiente_token(NULL);
    char *email     = siguiente_token(NULL);
    char *fnac      = siguiente_token(NULL);

    if (!nombre || !apellidos || !tlf || !email || !fnac) {
        enviar_respuesta(s, "ERR|Faltan campos del cliente|#");
        return;
    }

    const char *sql =
        "INSERT INTO clientes (dni,nombre,apellidos,telefono,email,fecha_nacimiento,activo)"
        " VALUES (?,?,?,?,?,?,1);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_text(stmt, 1, dni,       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nombre,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, tlf,       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, email,     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, fnac,      -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        enviar_respuesta(s, "OK|Cliente dado de alta|#");
        log_escribir("Alta de cliente OK");
    } else {
        enviar_respuesta(s, "ERR|DNI ya existe o error BD|#");
    }
    sqlite3_finalize(stmt);
}

/* BCL|dni|  ->  OK|Cliente dado de baja|# */
static void handle_BCL(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|DNI requerido|#"); return; }

    const char *sql = "UPDATE clientes SET activo=0 WHERE dni=? AND activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_text(stmt, 1, params, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    int cambios = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (cambios > 0) {
        enviar_respuesta(s, "OK|Cliente dado de baja|#");
    } else {
        enviar_respuesta(s, "ERR|Cliente no encontrado o ya de baja|#");
    }
}

/* MCL|dni|nombre|apellidos|tlf|email|fnac|  ->  OK|...|# */
static void handle_MCL(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Parametros insuficientes|#"); return; }

    char *dni       = params;
    char *nombre    = siguiente_token(NULL);
    char *apellidos = siguiente_token(NULL);
    char *tlf       = siguiente_token(NULL);
    char *email     = siguiente_token(NULL);
    char *fnac      = siguiente_token(NULL);

    if (!nombre || !apellidos || !tlf || !email || !fnac) {
        enviar_respuesta(s, "ERR|Faltan campos|#");
        return;
    }

    const char *sql =
        "UPDATE clientes SET nombre=?,apellidos=?,telefono=?,email=?,fecha_nacimiento=?"
        " WHERE dni=? AND activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_text(stmt, 1, nombre,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, tlf,       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, email,     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, fnac,      -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, dni,       -1, SQLITE_STATIC);

    sqlite3_step(stmt);
    int cambios = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (cambios > 0) {
        enviar_respuesta(s, "OK|Cliente modificado|#");
    } else {
        enviar_respuesta(s, "ERR|Cliente no encontrado|#");
    }
}

/* GCL|dni|  ->  OK|id|dni|nombre|apellidos|tlf|email|fnac|# */
static void handle_GCL(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|DNI requerido|#"); return; }

    const char *sql =
        "SELECT id,dni,nombre,apellidos,telefono,email,fecha_nacimiento"
        " FROM clientes WHERE dni=? AND activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_text(stmt, 1, params, -1, SQLITE_STATIC);

    char resp[PROTO_BUF];
    if (sqlite3_step(stmt) == SQLITE_ROW) {
    	sprintf(resp, "OK|%d|%s|%s|%s|%s|%s|%s|#",
    	    sqlite3_column_int(stmt, 0),
    	    (const char*)sqlite3_column_text(stmt, 1),
    	    (const char*)sqlite3_column_text(stmt, 2),
    	    (const char*)sqlite3_column_text(stmt, 3),
    	    (const char*)sqlite3_column_text(stmt, 4),
    	    (const char*)sqlite3_column_text(stmt, 5),
    	    (const char*)sqlite3_column_text(stmt, 6));
    } else {
        strcpy(resp, "ERR|Cliente no encontrado|#");
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, resp);
}

/* LCL|  ->  LST_BEGIN|N registros|# + un OK|...|# por cliente + LST_END|# */
static void handle_LCL(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT id,dni,nombre,apellidos,telefono,email,fecha_nacimiento"
        " FROM clientes WHERE activo=1 ORDER BY apellidos,nombre;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");

    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%d|%s|%s|%s|%s|%s|%s|#",
        		sqlite3_column_int(stmt, 0),
        		(const char*)sqlite3_column_text(stmt, 1),
           	    (const char*)sqlite3_column_text(stmt, 2),
           	    (const char*)sqlite3_column_text(stmt, 3),
           	    (const char*)sqlite3_column_text(stmt, 4),
           	    (const char*)sqlite3_column_text(stmt, 5),
           	    (const char*)sqlite3_column_text(stmt, 6));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* LPQ|  ->  LST_BEGIN + filas + LST_END */
static void handle_LPQ(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT codigo,nombre,precio,origen,destino,plazas_totales,plazas_disponibles"
        " FROM paquetes WHERE activo=1 ORDER BY codigo;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");

    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%d|%s|%.2f|%s|%s|%d|%d|#",
            sqlite3_column_int(stmt, 0),
			(const char*)sqlite3_column_text(stmt, 1),
            sqlite3_column_double(stmt, 2),
			(const char*)sqlite3_column_text(stmt, 3),
			(const char*)sqlite3_column_text(stmt, 4),
            sqlite3_column_int(stmt, 5),
            sqlite3_column_int(stmt, 6));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* GPQ|codigo|  ->  OK|codigo|nombre|precio|origen|destino|tot|disp|# */
static void handle_GPQ(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Codigo requerido|#"); return; }

    const char *sql =
        "SELECT codigo,nombre,precio,origen,destino,plazas_totales,plazas_disponibles"
        " FROM paquetes WHERE codigo=? AND activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_int(stmt, 1, atoi(params));

    char resp[PROTO_BUF];
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(resp, "OK|%d|%s|%.2f|%s|%s|%d|%d|#",
            sqlite3_column_int(stmt, 0),
			(const char*)sqlite3_column_text(stmt, 1),
            sqlite3_column_double(stmt, 2),
			(const char*)sqlite3_column_text(stmt, 3),
			(const char*)sqlite3_column_text(stmt, 4),
            sqlite3_column_int(stmt, 5),
            sqlite3_column_int(stmt, 6));
    } else {
        strcpy(resp, "ERR|Paquete no encontrado|#");
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, resp);
}

/* APQ|cod|nombre|precio|origen|destino|plazas|  ->  OK|Paquete creado|# */
static void handle_APQ(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Parametros insuficientes|#"); return; }

    char *cod     = params;
    char *nombre  = siguiente_token(NULL);
    char *precio  = siguiente_token(NULL);
    char *origen  = siguiente_token(NULL);
    char *destino = siguiente_token(NULL);
    char *plazas  = siguiente_token(NULL);

    if (!nombre || !precio || !origen || !destino || !plazas) {
        enviar_respuesta(s, "ERR|Faltan campos del paquete|#");
        return;
    }

    const char *sql =
        "INSERT INTO paquetes (codigo,nombre,precio,origen,destino,plazas_totales,plazas_disponibles,activo)"
        " VALUES (?,?,?,?,?,?,?,1);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    int    plz = atoi(plazas);
    double prc = atof(precio);
    sqlite3_bind_int(stmt,    1, atoi(cod));
    (const char*)sqlite3_bind_text(stmt,   2, nombre,  -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, prc);
    (const char*)sqlite3_bind_text(stmt,   4, origen,  -1, SQLITE_STATIC);
    (const char*)sqlite3_bind_text(stmt,   5, destino, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt,    6, plz);
    sqlite3_bind_int(stmt,    7, plz);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        enviar_respuesta(s, "OK|Paquete creado|#");
    } else {
        enviar_respuesta(s, "ERR|Codigo ya existe o error BD|#");
    }
    sqlite3_finalize(stmt);
}

/* BPQ|codigo|  ->  OK|Paquete eliminado|# */
static void handle_BPQ(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Codigo requerido|#"); return; }

    const char *sql = "UPDATE paquetes SET activo=0 WHERE codigo=?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_int(stmt, 1, atoi(params));
    sqlite3_step(stmt);
    int cambios = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (cambios > 0) enviar_respuesta(s, "OK|Paquete eliminado|#");
    else             enviar_respuesta(s, "ERR|Paquete no encontrado|#");
}

/* LAL|  ->  LST_BEGIN + filas de alojamientos + LST_END */
static void handle_LAL(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT codigo,nombre,direccion,tipo,cod_ciudad FROM alojamientos WHERE activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");
    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%s|%s|%s|%s|%s|#",
        	(const char*)sqlite3_column_text(stmt, 0),
			(const char*)sqlite3_column_text(stmt, 1),
			(const char*)sqlite3_column_text(stmt, 2),
			(const char*)sqlite3_column_text(stmt, 3),
			(const char*)sqlite3_column_text(stmt, 4));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* LTR|  ->  LST_BEGIN + filas de transportes + LST_END */
static void handle_LTR(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT codigo,tipo,fecha_salida,fecha_llegada,id_paquete"
        " FROM transportes WHERE activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");
    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%s|%s|%s|%s|%d|#",
        	(const char*)sqlite3_column_text(stmt, 0),
			(const char*)sqlite3_column_text(stmt, 1),
			(const char*)sqlite3_column_text(stmt, 2),
			(const char*)sqlite3_column_text(stmt, 3),
            sqlite3_column_int(stmt, 4));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* ARE|dni_cliente|cod_paquete|fecha|  ->  OK|Reserva creada|id|# */
static void handle_ARE(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|Parametros insuficientes|#"); return; }

    char *dni      = params;
    char *cod_pqt  = siguiente_token(NULL);
    char *fecha    = siguiente_token(NULL);

    if (!cod_pqt || !fecha) {
        enviar_respuesta(s, "ERR|Faltan campos de reserva|#");
        return;
    }

    /* Comprobar que hay plazas disponibles */
    sqlite3_stmt *stmt;
    const char *sql_check =
        "SELECT plazas_disponibles FROM paquetes WHERE codigo=? AND activo=1;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_int(stmt, 1, atoi(cod_pqt));

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        enviar_respuesta(s, "ERR|Paquete no encontrado|#");
        return;
    }
    int plazas = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (plazas <= 0) {
        enviar_respuesta(s, "ERR|No hay plazas disponibles|#");
        return;
    }

    /* Crear la reserva (tabla ya existe por db_crear_tablas) */
    const char *sql_ins =
        "INSERT INTO reservas (dni_cliente,cod_paquete,fecha,activo) VALUES (?,?,?,1);";
    if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    (const char*)sqlite3_bind_text(stmt, 1, dni,     -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt,  2, atoi(cod_pqt));
    (const char*)sqlite3_bind_text(stmt, 3, fecha,   -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        enviar_respuesta(s, "ERR|No se pudo crear la reserva|#");
        return;
    }
    long long id_reserva = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);


    sprintf(sql_upd,
        "UPDATE paquetes SET plazas_disponibles = plazas_disponibles - 1 WHERE codigo = %d;",
        atoi(cod_pqt));
    sqlite3_exec(db, sql_upd, 0, 0, NULL);

    char resp[PROTO_BUF];
    sprintf(resp, "OK|Reserva creada|%d|#", (int)id_reserva);
    enviar_respuesta(s, resp);
    log_escribir("Reserva creada OK");
}

/* BRE|id_reserva|  ->  OK|Reserva cancelada|# */
static void handle_BRE(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|ID reserva requerido|#"); return; }

    int id = atoi(params);

    /* Recuperar cod_paquete para devolver la plaza */
    sqlite3_stmt *stmt;
    const char *sql_sel = "SELECT cod_paquete FROM reservas WHERE id=? AND activo=1;";
    if (sqlite3_prepare_v2(db, sql_sel, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        enviar_respuesta(s, "ERR|Reserva no encontrada|#");
        return;
    }
    int cod_pqt = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    /* Baja logica de la reserva */
    char sql_baja[128];
    sprintf(sql_baja, "UPDATE reservas SET activo=0 WHERE id=%d;", id);
    sqlite3_exec(db, sql_baja, 0, 0, NULL);

    /* Devolver plaza al paquete */
    char sql_upd[256];
    sprintf(sql_upd,
        "UPDATE paquetes SET plazas_disponibles = plazas_disponibles + 1 WHERE codigo = %d;",
        cod_pqt);
    sqlite3_exec(db, sql_upd, 0, 0, NULL);

    enviar_respuesta(s, "OK|Reserva cancelada|#");
}

/* LRC|dni_cliente|  ->  LST_BEGIN + filas de reservas + LST_END */
static void handle_LRC(SOCKET s, sqlite3 *db, char *params) {
    if (!params) { enviar_respuesta(s, "ERR|DNI requerido|#"); return; }

    const char *sql =
        "SELECT r.id, r.cod_paquete, p.nombre, r.fecha"
        " FROM reservas r JOIN paquetes p ON r.cod_paquete=p.codigo"
        " WHERE r.dni_cliente=? AND r.activo=1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }
    (const char*)sqlite3_bind_text(stmt, 1, params, -1, SQLITE_STATIC);

    enviar_respuesta(s, "LST_BEGIN|#");
    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%d|%d|%s|%s|#",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_int(stmt, 1),
			(const char*)sqlite3_column_text(stmt, 2),
			(const char*)sqlite3_column_text(stmt, 3));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* IOC|  ->  paquetes con <10% de plazas libres */
static void handle_IOC(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT codigo, nombre, plazas_totales, plazas_disponibles"
        " FROM paquetes WHERE activo=1"
        "   AND CAST(plazas_disponibles AS REAL)/plazas_totales < 0.10"
        " ORDER BY plazas_disponibles ASC;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");
    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%d|%s|%d|%d|#",
            sqlite3_column_int(stmt, 0),
			(const char*)sqlite3_column_text(stmt, 1),
            sqlite3_column_int(stmt, 2),
            sqlite3_column_int(stmt, 3));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* IRK|  ->  top 5 clientes por numero de reservas */
static void handle_IRK(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT c.dni, c.nombre || ' ' || c.apellidos, COUNT(r.id) AS total"
        " FROM reservas r JOIN clientes c ON r.dni_cliente=c.dni"
        " WHERE r.activo=1"
        " GROUP BY r.dni_cliente"
        " ORDER BY total DESC LIMIT 5;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");
    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%s|%s|%d|#",
        	(const char*)sqlite3_column_text(stmt, 0),
			(const char*)sqlite3_column_text(stmt, 1),
            sqlite3_column_int(stmt, 2));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}

/* IDS|  ->  destinos mas reservados en el ultimo mes */
static void handle_IDS(SOCKET s, sqlite3 *db) {
    const char *sql =
        "SELECT p.destino, COUNT(r.id) AS total"
        " FROM reservas r JOIN paquetes p ON r.cod_paquete=p.codigo"
        " WHERE r.activo=1"
        "   AND r.fecha >= date('now','-1 month')"
        " GROUP BY p.destino"
        " ORDER BY total DESC LIMIT 10;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        enviar_respuesta(s, "ERR|Error interno BD|#");
        return;
    }

    enviar_respuesta(s, "LST_BEGIN|#");
    char fila[PROTO_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(fila, "OK|%s|%d|#",
            sqlite3_column_text(stmt, 0),
            sqlite3_column_int(stmt, 1));
        enviar_respuesta(s, fila);
    }
    sqlite3_finalize(stmt);
    enviar_respuesta(s, "LST_END|#");
}
