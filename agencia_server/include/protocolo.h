#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

/*
 * Contrato de comunicacion entre agencia_server y agencia_cliente.
 *
 * FORMATO DE TRAMA:
 *   Peticion  (cliente -> servidor): COD_OP|param1|param2|...|#
 *   Respuesta (servidor -> cliente): OK|dato1|dato2|...|#
 *                                    ERR|mensaje_error|#
 *
 * El caracter '#' marca el fin de cada mensaje.
 * El separador de campos es '|'.
 *
 */

// Configuracion de red
#define PROTO_HOST      "127.0.0.1"   /* IP del servidor (localhost en desarrollo) */
#define PROTO_PORT      8080          /* Puerto TCP del servidor                   */
#define PROTO_BUF       4096          /* Tamano maximo de una trama en bytes       */
#define PROTO_FIN       '#'           /* Caracter centinela de fin de mensaje      */
#define PROTO_SEP       '|'           /* Separador de campos                       */

// Codigos de operacion (cliente -> servidor)
/* Sesion */
#define OP_REGISTRO     "REG"   /* REG|usuario|clave|rol|#  — alta de usuario */
#define OP_LOGIN        "LOG"   /* LOG|usuario|clave|#                         */
#define OP_LOGOUT       "BYE"   /* BYE|#   — cierra la conexion                */

/* Clientes */
#define OP_ALTA_CLI     "ACL"   /* ACL|dni|nombre|apellidos|tlf|email|fnac|#   */
#define OP_BAJA_CLI     "BCL"   /* BCL|dni|#                                   */
#define OP_MOD_CLI      "MCL"   /* MCL|dni|nombre|apellidos|tlf|email|fnac|#   */
#define OP_GET_CLI      "GCL"   /* GCL|dni|#                                   */
#define OP_LIST_CLI     "LCL"   /* LCL|#                                       */

/* Paquetes */
#define OP_LIST_PQT     "LPQ"   /* LPQ|#                                       */
#define OP_GET_PQT      "GPQ"   /* GPQ|codigo|#                                */
#define OP_ALTA_PQT     "APQ"   /* APQ|cod|nombre|precio|origen|destino|plazas|# */
#define OP_BAJA_PQT     "BPQ"   /* BPQ|codigo|#                                */

/* Alojamientos */
#define OP_LIST_ALO     "LAL"   /* LAL|#                                       */
#define OP_GET_ALO      "GAL"   /* GAL|codigo|#                                */
#define OP_ALTA_ALO     "AAL"   /* AAL|cod|nombre|dir|tipo|cod_ciudad|#        */
#define OP_BAJA_ALO     "BAL"   /* BAL|codigo|#                                */

/* Transportes */
#define OP_LIST_TRP     "LTR"   /* LTR|#                                       */
#define OP_ALTA_TRP     "ATR"   /* ATR|cod|tipo|f_salida|f_llegada|id_paquete|# */
#define OP_BAJA_TRP     "BTR"   /* BTR|codigo|#                                */

/* Reservas */
#define OP_ALTA_RES     "ARE"   /* ARE|dni_cliente|cod_paquete|fecha|#         */
#define OP_BAJA_RES     "BRE"   /* BRE|id_reserva|#                            */
#define OP_LIST_RES_CLI "LRC"   /* LRC|dni_cliente|#                           */

/* Informes (calculados en servidor con SQL) */
#define OP_INF_OCUP     "IOC"   /* IOC|#  — paquetes con <10% plazas libres   */
#define OP_INF_RANK     "IRK"   /* IRK|#  — top 5 clientes por ingresos        */
#define OP_INF_DEST     "IDS"   /* IDS|#  — destinos mas reservados (1 mes)   */

/* Codigos de respuesta (servidor -> cliente)*/
#define RESP_OK         "OK"    /* OK|...|#                                    */
#define RESP_ERR        "ERR"   /* ERR|descripcion|#                           */

/* Respuestas especificas de exito */
#define RESP_LOGIN_OK   "OK|AUTH|#"
#define RESP_LOGIN_ERR  "ERR|Credenciales incorrectas|#"

/* Marcador de inicio y fin de listado multi-registro */
#define RESP_LIST_INI   "LST_BEGIN"
#define RESP_LIST_FIN   "LST_END"

#endif /* PROTOCOLO_H_ */
