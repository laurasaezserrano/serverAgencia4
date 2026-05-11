#ifndef CONEXIONSERVIDOR_H_
#define CONEXIONSERVIDOR_H_

/*
 * ConexionServidor.h
 * Encapsula toda la logica de sockets Winsock2 del cliente.
 * El resto del programa nunca toca sockets directamente,
 * solo usa enviar() y recibir().
 *
 * Uso:
 *   ConexionServidor cx;
 *   if (cx.conectar("127.0.0.1", 8080)) { ... }
 *   cx.enviar("LPQ|#");
 *   std::string r = cx.recibir();
 *   cx.desconectar();
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

class ConexionServidor {
public:
    ConexionServidor();
    ~ConexionServidor();

    /* Inicializa Winsock y abre conexion TCP al servidor.
     * Devuelve true si la conexion se establecio correctamente. */
    bool conectar(const std::string &ip, int puerto);

    /* Envia una trama al servidor (debe terminar en '#'). */
    bool enviar(const std::string &trama);

    /* Recibe la respuesta completa hasta el centinela '#'.
     * Devuelve la cadena sin el '#' final, o "" si hay error. */
    std::string recibir();

    /* Envia BYE|# y cierra el socket. */
    void desconectar();

    /* true si el socket esta abierto y conectado. */
    bool estaConectado() const;

private:
    SOCKET      _socket;
    bool        _conectado;
    bool        _wsaIniciado;

    /* Buffer interno para acumular datos parciales entre llamadas */
    std::string _buffer;
};

#endif /* CONEXIONSERVIDOR_H_ */
