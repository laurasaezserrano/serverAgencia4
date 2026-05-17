#ifndef MENU_H_
#define MENU_H_

/*
 *   Login
 *   └── Menu Principal
 *         ├── 1. Perfil
 *         ├── 2. Mis Reservas
 *         ├── 3. Paquetes Turisticos
 *         └── 4. Informes  (solo ADMIN)
 */

#include "ConexionServidor.h"
#include "GestorDatos.h"
#include <string>

class Menu {
public:
    Menu(ConexionServidor& conn, GestorDatos& gestor);

    /* Punto de entrada: muestra login y luego el menu principal */
    void ejecutar();

private:
    ConexionServidor& m_conn;
    GestorDatos&      m_gestor;
    std::string       m_usuarioActual;
    std::string       m_rolActual;      /* "ADMIN" o "CLIENTE" */
    std::string       m_dniActual;      /* DNI del cliente logueado */

    // Login
    bool menuLogin();
    void menuRegistro();

    // Menu principal
    void menuPrincipal();

    // Submenus
    void menuPerfil();
    void menuReservas();
    void menuPaquetes();
    void menuPrincipalClientes(); /* gestion de clientes, solo ADMIN */
    void menuInformes();          /* solo ADMIN */

    //Acciones de clientes
    void altaCliente();
    void bajaCliente();
    void modificarCliente();
    void buscarCliente();
    void listarClientes();

    // Acciones de paquetes
    void listarPaquetes();
    void crearPaquete();
    void bajaPaquete();

    // Acciones de reservas
    void crearReserva();
    void cancelarReserva();
    void verMisReservas();

    // Informes
    void informeOcupacion();
    void informeRanking();
    void informeDestinos();

    //Helpers UI
    int    leerEntero(const std::string& mensaje);
    float  leerFloat(const std::string& mensaje);
    std::string leerCadena(const std::string& mensaje);
    void   pausar();
    void   limpiarPantalla();
    void   separador(char c = '-', int n = 50);
    void   titulo(const std::string& texto);
    bool   confirmar(const std::string& pregunta);
};

#endif /* MENU_H_ */
