#include "../include/ConexionServidor.h"
#include "../include/protocolo.h"
#include <iostream>
#include <stdexcept>
#include <string>


// Constructor / Destructor

ConexionServidor::ConexionServidor()
    : _socket(INVALID_SOCKET), _conectado(false), _wsaIniciado(false) {}

ConexionServidor::~ConexionServidor() {
    if (_conectado) {
        desconectar();
    }
    if (_wsaIniciado) {
        WSACleanup();
    }
}

// Conectar

bool ConexionServidor::conectar(const std::string &ip, int puerto) {
    /* 1. Inicializar Winsock (solo la primera vez) */
    if (!_wsaIniciado) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            std::cerr << "[ERROR] WSAStartup falló: " << WSAGetLastError() << std::endl;
            return false;
        }
        _wsaIniciado = true;
    }

    /* 2. Crear socket TCP */
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == INVALID_SOCKET) {
        std::cerr << "[ERROR] No se pudo crear el socket." << std::endl;
        return false;
    }

    /* 3. Rellenar dirección del servidor */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((unsigned short)puerto);

    /* inet_addr funciona en todas las versiones de MinGW */
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "[ERROR] IP inválida: " << ip << std::endl;
        closesocket(_socket);
        _socket = INVALID_SOCKET;
        return false;
    }

    /* 4. Conectar */
    if (connect(_socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] No se pudo conectar al servidor "
                  << ip << ":" << puerto
                  << " (¿está arrancado?)" << std::endl;
        closesocket(_socket);
        _socket = INVALID_SOCKET;
        return false;
    }

    _conectado = true;
    _buffer.clear();
    return true;
}

// Enviar
bool ConexionServidor::enviar(const std::string &trama) {
    if (!_conectado) return false;

    int enviados = send(_socket, trama.c_str(), (int)trama.size(), 0);
    if (enviados == SOCKET_ERROR) {
        std::cerr << "[ERROR] send falló: " << WSAGetLastError() << std::endl;
        _conectado = false;
        return false;
    }
    return true;
}

/* Recibir
 * Acumula datos en _buffer hasta encontrar el centinela '#'.
 * Esto maneja correctamente el caso de que TCP fragmente la respuesta
 * en varios paquetes, o que lleguen dos tramas juntas.
 */
std::string ConexionServidor::recibir() {
    if (!_conectado) return "";

    char chunk[PROTO_BUF];

    /* Buscar '#' en lo que ya hay en el buffer antes de leer más */
    while (true) {
        size_t pos = _buffer.find(PROTO_FIN);
        if (pos != std::string::npos) {
            /* Extraer la trama completa (sin '#') */
            std::string trama = _buffer.substr(0, pos);
            /* Dejar en el buffer lo que venga después del '#' */
            _buffer = _buffer.substr(pos + 1);
            return trama;
        }

        /* No hay '#' todavía: leer más datos del socket */
        memset(chunk, 0, sizeof(chunk));
        int recibidos = recv(_socket, chunk, sizeof(chunk) - 1, 0);
        if (recibidos <= 0) {
            /* Servidor cerró la conexión o error */
            _conectado = false;
            return "";
        }
        _buffer.append(chunk, recibidos);
    }
}

// Desconectar
void ConexionServidor::desconectar() {
    if (_conectado) {
        /* Avisar al servidor antes de cerrar */
        send(_socket, "BYE|#", 5, 0);
        closesocket(_socket);
        _socket    = INVALID_SOCKET;
        _conectado = false;
        _buffer.clear();
    }
}

// Conectado
bool ConexionServidor::estaConectado() const {
    return _conectado;
}
