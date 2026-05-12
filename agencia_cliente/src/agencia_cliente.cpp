#include <iostream>
#include <fstream>
#include <string>

#include "../include/ConexionServidor.h"
#include "../include/GestorDatos.h"
#include "../include/Menu.h"

//Leer config.ini
static void cargarConfig(const std::string& ruta,
                         std::string& host, int& puerto) {
    host   = "127.0.0.1";
    puerto = 8080;

    std::ifstream f(ruta);
    if (!f.is_open()) {
        std::cout << "[AVISO] No se encontro " << ruta
                  << " — usando valores por defecto.\n";
        return;
    }

    std::string linea;
    while (std::getline(f, linea)) {
        if (linea.find("SERVER_HOST=") == 0)
            host = linea.substr(12);
        else if (linea.find("SERVER_PORT=") == 0)
            puerto = std::stoi(linea.substr(12));
    }
}

//MAIN
int main() {
    std::string host;
    int puerto;
    cargarConfig("data/config.ini", host, puerto);

    std::cout << "=== Agencia de Viajes — Cliente ===\n\n";
    std::cout << "Conectando a " << host << ":" << puerto << " ...\n";

    ConexionServidor conn;
    if (!conn.conectar(host, puerto)) {
        std::cerr << "[ERROR] No se pudo conectar con el servidor.\n"
                  << "Asegurese de que agencia_server.exe esta en ejecucion.\n";
        return 1;
    }
    std::cout << "Conectado correctamente.\n\n";

    GestorDatos gestor(conn);
    Menu        menu(conn, gestor);
    menu.ejecutar();

    conn.desconectar();
    std::cout << "\nSesion cerrada. Hasta pronto.\n";
    return 0;
}
