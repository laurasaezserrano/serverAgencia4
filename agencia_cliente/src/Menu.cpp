/*
 * Menu.cpp — menus jerarquicos del cliente C++
 */
#include "../include/Menu.h"
#include "../include/protocolo.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <conio.h>      /* _getch() para contraseña con asteriscos — Windows */

Menu::Menu(ConexionServidor& conn, GestorDatos& gestor)
    : m_conn(conn), m_gestor(gestor) {}

// Leer contraseña mostrando asteriscos
static std::string leerPassword(const std::string& mensaje) {
	std::cout << mensaje << ": ";
    std::string pass;

    // Limpiar buffer residual
    while (std::cin.peek() == '\n' || std::cin.peek() == '\r') {
    	std::cin.ignore();
    }

    std::getline(std::cin, pass);
    std::cout << '\n';
    return pass;

//	std::cout << mensaje << ": ";
//    std::string pass;
//    char c;
//    while ((c = _getch()) != '\r') {   /* '\r' = Enter en Windows */
//        if (c == '\b') {               /* Backspace */
//            if (!pass.empty()) {
//                pass.pop_back();
//                std::cout << "\b \b";
//            }
//        } else if (c >= 32) {          /* caracter imprimible */
//            pass += c;
//            std::cout << '*';
//        }
//    }
//    std::cout << '\n';
//    return pass;
}

// PUNTO DE ENTRADA
void Menu::ejecutar() {
    if (!menuLogin()) return;
    menuPrincipal();
}

//LOGIN
bool Menu::menuLogin() {
    while (true) {
        limpiarPantalla();
        titulo("AGENCIA DE VIAJES — ACCESO");
        std::cout << " 1. Iniciar sesion\n";
        std::cout << " 2. Registrar nuevo usuario\n";
        std::cout << " 0. Salir\n";
        separador();

        int op = leerEntero("Opcion");
        if (op == 0) return false;

        if (op == 2) {
            menuRegistro();
            continue;
        }
        if (op != 1) { std::cout << "Opcion no valida.\n"; pausar(); continue; }

        // Inicio de sesion
        std::string usuario = leerCadena("Usuario");
        std::string clave   = leerPassword("Clave");

        m_conn.enviar(std::string(OP_LOGIN) + "|" + usuario + "|" + clave + "|#");
        std::string resp = m_conn.recibir();
        auto campos = GestorDatos::parsearTrama(resp);

        if (!campos.empty() && campos[0] == RESP_OK && campos.size() >= 3) {
            m_usuarioActual = usuario;
            m_rolActual     = campos[2];
            std::cout << "\nBienvenido, " << usuario
                      << " [" << m_rolActual << "]\n";
            pausar();
            return true;
        } else {
            std::string err = (campos.size() > 1) ? campos[1] : "Error desconocido";
            std::cout << "\nError: " << err << "\n";
            pausar();
        }
    }
}

//REGISTRO DE USUARIO
void Menu::menuRegistro() {
    limpiarPantalla();
    titulo("REGISTRO DE NUEVO USUARIO");

    std::string usuario = leerCadena("Nombre de usuario");
    std::string clave   = leerPassword("Contrasena");
    std::string clave2  = leerPassword("Confirmar contrasena");

    if (clave != clave2) {
        std::cout << "Las contrasenas no coinciden.\n";
        pausar();
        return;
    }
    if (usuario.empty() || clave.empty()) {
        std::cout << "El usuario y la contrasena no pueden estar vacios.\n";
        pausar();
        return;
    }

    /* Por defecto rol CLIENTE — solo el admin puede crear ADMINs */
    m_conn.enviar(std::string(OP_REGISTRO) + "|" + usuario + "|" + clave + "|CLIENTE|#");
    std::string resp = m_conn.recibir();
    auto campos = GestorDatos::parsearTrama(resp);

    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "\nUsuario registrado correctamente. Ya puede iniciar sesion.\n";
    } else {
        std::string err = (campos.size() > 1) ? campos[1] : "Error desconocido";
        std::cout << "\nError: " << err << "\n";
    }
    pausar();
}

//MENU PRINCIPAL
void Menu::menuPrincipal() {
    while (true) {
        limpiarPantalla();
        titulo("MENU PRINCIPAL — " + m_usuarioActual);
        std::cout << " 1. Mi perfil\n";
        std::cout << " 2. Mis reservas\n";
        std::cout << " 3. Paquetes turisticos\n";
        if (m_rolActual == "ADMIN") {
            std::cout << " 4. Gestion de clientes\n";
            std::cout << " 5. Informes\n";
        }
        std::cout << " 0. Cerrar sesion\n";
        separador();

        int op = leerEntero("Opcion");
        switch (op) {
            case 1: menuPerfil();   break;
            case 2: menuReservas(); break;
            case 3: menuPaquetes(); break;
            case 4: if (m_rolActual == "ADMIN") menuPrincipalClientes(); break;
            case 5: if (m_rolActual == "ADMIN") menuInformes(); break;
            case 0: return;
            default: std::cout << "Opcion no valida.\n"; pausar();
        }
    }
}

// PERFIL
void Menu::menuPerfil() {
    while (true) {
        limpiarPantalla();
        titulo("MI PERFIL");

        if (m_dniActual.empty())
            m_dniActual = leerCadena("Introduzca su DNI para ver su perfil");

        Cliente* c = m_gestor.buscarClientePorDni(m_dniActual);
        if (!c) {
            std::cout << "Cliente no encontrado con DNI: " << m_dniActual << "\n";
            m_dniActual.clear();
            pausar();
            return;
        }

        std::cout << "\n" << c->toString() << "\n";
        separador();
        std::cout << " 1. Editar perfil\n";
        std::cout << " 0. Volver\n";
        separador();

        int op = leerEntero("Opcion");
        if (op == 0) return;
        if (op == 1) { modificarCliente(); m_gestor.invalidarCacheClientes(); }
    }
}

// MIS RESERVAS
void Menu::menuReservas() {
    limpiarPantalla();
    titulo("MIS RESERVAS");

    if (m_dniActual.empty())
        m_dniActual = leerCadena("Introduzca su DNI");

    auto reservas = m_gestor.getReservasPorCliente(m_dniActual);

    if (reservas.empty()) {
        std::cout << "No tiene reservas activas.\n";
        pausar();
        return;
    }

    for (size_t i = 0; i < reservas.size(); i++)
        std::cout << " [" << (i+1) << "] " << reservas[i].toString() << "\n";
    separador();

    int op = leerEntero("Seleccione numero para cancelar, 0 para volver");
    if (op <= 0 || op > (int)reservas.size()) return;

    if (confirmar("Cancelar la reserva #" + std::to_string(reservas[op-1].id))) {
        m_conn.enviar(std::string(OP_BAJA_RES) + "|" +
                      std::to_string(reservas[op-1].id) + "|#");
        auto campos = GestorDatos::parsearTrama(m_conn.recibir());
        if (!campos.empty() && campos[0] == RESP_OK) {
            std::cout << "Reserva cancelada correctamente.\n";
            m_gestor.invalidarCachePaquetes();
        } else {
            std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
        }
        pausar();
    }
}


// PAQUETES TURISTICOS
void Menu::menuPaquetes() {
    while (true) {
        limpiarPantalla();
        titulo("PAQUETES TURISTICOS");
        std::cout << " 1. Ver paquetes disponibles\n";
        std::cout << " 2. Hacer una reserva\n";
        if (m_rolActual == "ADMIN") {
            std::cout << " 3. Nuevo paquete\n";
            std::cout << " 4. Eliminar paquete\n";
        }
        std::cout << " 0. Volver\n";
        separador();

        int op = leerEntero("Opcion");
        switch (op) {
            case 0: return;
            case 1: listarPaquetes(); break;
            case 2: crearReserva();   break;
            case 3: if (m_rolActual == "ADMIN") crearPaquete(); break;
            case 4: if (m_rolActual == "ADMIN") bajaPaquete();  break;
            default: std::cout << "Opcion no valida.\n"; pausar();
        }
    }
}

//GESTION DE CLIENTES (ADMIN)
void Menu::menuPrincipalClientes() {
    while (true) {
        limpiarPantalla();
        titulo("GESTION DE CLIENTES");
        std::cout << " 1. Listar clientes\n";
        std::cout << " 2. Buscar cliente por DNI\n";
        std::cout << " 3. Alta de cliente\n";
        std::cout << " 4. Baja de cliente\n";
        std::cout << " 5. Modificar cliente\n";
        std::cout << " 0. Volver\n";
        separador();

        int op = leerEntero("Opcion");
        switch (op) {
            case 0: return;
            case 1: listarClientes();   break;
            case 2: buscarCliente();    break;
            case 3: altaCliente();      break;
            case 4: bajaCliente();      break;
            case 5: modificarCliente(); break;
            default: std::cout << "Opcion no valida.\n"; pausar();
        }
    }
}

//ACCIONES CLIENTES
void Menu::listarClientes() {
    limpiarPantalla();
    titulo("LISTADO DE CLIENTES");
    m_gestor.invalidarCacheClientes();
    const auto& clientes = m_gestor.getClientes();
    if (clientes.empty()) std::cout << "No hay clientes registrados.\n";
    else for (const auto& c : clientes) std::cout << "  " << c.toString() << "\n";
    pausar();
}

void Menu::buscarCliente() {
    limpiarPantalla();
    titulo("BUSCAR CLIENTE POR DNI");
    Cliente* c = m_gestor.buscarClientePorDni(leerCadena("DNI"));
    if (c) std::cout << "\n" << c->toString() << "\n";
    else   std::cout << "Cliente no encontrado.\n";
    pausar();
}

void Menu::altaCliente() {
    limpiarPantalla();
    titulo("ALTA DE CLIENTE");
    std::string dni       = leerCadena("DNI");
    std::string nombre    = leerCadena("Nombre");
    std::string apellidos = leerCadena("Apellidos");
    std::string tlf       = leerCadena("Telefono");
    std::string email     = leerCadena("Email");
    std::string fnac      = leerCadena("Fecha nacimiento (YYYY-MM-DD)");

    m_conn.enviar(std::string(OP_ALTA_CLI) + "|" +
                  dni + "|" + nombre + "|" + apellidos + "|" +
                  tlf + "|" + email  + "|" + fnac + "|#");
    auto campos = GestorDatos::parsearTrama(m_conn.recibir());
    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "Cliente dado de alta correctamente.\n";
        m_gestor.invalidarCacheClientes();
    } else {
        std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
    }
    pausar();
}

void Menu::bajaCliente() {
    limpiarPantalla();
    titulo("BAJA DE CLIENTE");
    std::string dni = leerCadena("DNI del cliente a dar de baja");
    if (!confirmar("Dar de baja al cliente con DNI " + dni)) return;

    m_conn.enviar(std::string(OP_BAJA_CLI) + "|" + dni + "|#");
    auto campos = GestorDatos::parsearTrama(m_conn.recibir());
    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "Cliente dado de baja correctamente.\n";
        m_gestor.invalidarCacheClientes();
    } else {
        std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
    }
    pausar();
}

void Menu::modificarCliente() {
    limpiarPantalla();
    titulo("MODIFICAR CLIENTE");
    std::string dni = m_dniActual.empty() ? leerCadena("DNI del cliente") : m_dniActual;

    std::string nombre    = leerCadena("Nombre");
    std::string apellidos = leerCadena("Apellidos");
    std::string tlf       = leerCadena("Telefono");
    std::string email     = leerCadena("Email");
    std::string fnac      = leerCadena("Fecha nacimiento (YYYY-MM-DD)");

    m_conn.enviar(std::string(OP_MOD_CLI) + "|" +
                  dni + "|" + nombre + "|" + apellidos + "|" +
                  tlf + "|" + email  + "|" + fnac + "|#");
    auto campos = GestorDatos::parsearTrama(m_conn.recibir());
    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "Cliente modificado correctamente.\n";
        m_gestor.invalidarCacheClientes();
    } else {
        std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
    }
    pausar();
}

// ACCIONES PAQUETES
void Menu::listarPaquetes() {
    limpiarPantalla();
    titulo("PAQUETES DISPONIBLES");
    m_gestor.invalidarCachePaquetes();
    const auto& paquetes = m_gestor.getPaquetes();
    if (paquetes.empty()) std::cout << "No hay paquetes disponibles.\n";
    else for (const auto& p : paquetes) std::cout << "  " << p.toString() << "\n";
    pausar();
}

void Menu::crearPaquete() {
    limpiarPantalla();
    titulo("NUEVO PAQUETE TURISTICO");
    std::string cod     = leerCadena("Codigo (numero)");
    std::string nombre  = leerCadena("Nombre del paquete");
    std::string precio  = leerCadena("Precio (EUR)");
    std::string origen  = leerCadena("Ciudad de origen");
    std::string destino = leerCadena("Ciudad de destino");
    std::string plazas  = leerCadena("Plazas totales");

    m_conn.enviar(std::string(OP_ALTA_PQT) + "|" +
                  cod + "|" + nombre + "|" + precio + "|" +
                  origen + "|" + destino + "|" + plazas + "|#");
    auto campos = GestorDatos::parsearTrama(m_conn.recibir());
    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "Paquete creado correctamente.\n";
        m_gestor.invalidarCachePaquetes();
    } else {
        std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
    }
    pausar();
}

void Menu::bajaPaquete() {
    limpiarPantalla();
    titulo("ELIMINAR PAQUETE");
    std::string cod = leerCadena("Codigo del paquete a eliminar");
    if (!confirmar("Eliminar paquete " + cod)) return;

    m_conn.enviar(std::string(OP_BAJA_PQT) + "|" + cod + "|#");
    auto campos = GestorDatos::parsearTrama(m_conn.recibir());
    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "Paquete eliminado.\n";
        m_gestor.invalidarCachePaquetes();
    } else {
        std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
    }
    pausar();
}

// RESERVAS
void Menu::crearReserva() {
    limpiarPantalla();
    titulo("NUEVA RESERVA");

    const auto& paquetes = m_gestor.getPaquetes();
    if (paquetes.empty()) { std::cout << "No hay paquetes disponibles.\n"; pausar(); return; }
    for (const auto& p : paquetes) std::cout << "  " << p.toString() << "\n";
    separador();

    std::string dni    = m_dniActual.empty() ? leerCadena("Su DNI") : m_dniActual;
    std::string codPqt = leerCadena("Codigo del paquete");
    std::string fecha  = leerCadena("Fecha de viaje (YYYY-MM-DD)");

    Paquete* p = m_gestor.buscarPaquetePorCodigo(atoi(codPqt.c_str()));
    if (p && !p->tieneDisponibilidad()) {
        std::cout << "No hay plazas disponibles en ese paquete.\n";
        pausar(); return;
    }

    if (!confirmar("Confirmar reserva")) return;

    m_conn.enviar(std::string(OP_ALTA_RES) + "|" + dni + "|" + codPqt + "|" + fecha + "|#");
    auto campos = GestorDatos::parsearTrama(m_conn.recibir());
    if (!campos.empty() && campos[0] == RESP_OK) {
        std::cout << "Reserva confirmada. ID: " << (campos.size() > 2 ? campos[2] : "?") << "\n";
        m_gestor.invalidarCachePaquetes();
    } else {
        std::cout << "Error: " << (campos.size() > 1 ? campos[1] : "desconocido") << "\n";
    }
    pausar();
}

// INFORMES (ADMIN)
void Menu::menuInformes() {
    while (true) {
        limpiarPantalla();
        titulo("INFORMES DE NEGOCIO");
        std::cout << " 1. Ocupacion critica (< 10% plazas libres)\n";
        std::cout << " 2. Ranking de clientes (top 5)\n";
        std::cout << " 3. Destinos populares (ultimo mes)\n";
        std::cout << " 0. Volver\n";
        separador();

        int op = leerEntero("Opcion");
        switch (op) {
            case 0: return;
            case 1: informeOcupacion(); break;
            case 2: informeRanking();   break;
            case 3: informeDestinos();  break;
            default: std::cout << "Opcion no valida.\n"; pausar();
        }
    }
}

void Menu::informeOcupacion() {
    limpiarPantalla();
    titulo("INFORME — OCUPACION CRITICA");
    m_conn.enviar(std::string(OP_INF_OCUP) + "|#");
    auto filas = m_gestor.recibirListado();
    if (filas.empty()) { std::cout << "No hay paquetes en situacion critica.\n"; }
    else for (auto& f : filas) {
        auto c = GestorDatos::parsearTrama(f);
        if (c.size() >= 4) {
            int o = (c[0] == RESP_OK) ? 1 : 0;
            std::cout << "  [" << c[o] << "] " << c[o+1]
                      << " — Totales: " << c[o+2] << " Libres: " << c[o+3] << "\n";
        }
    }
    pausar();
}

void Menu::informeRanking() {
    limpiarPantalla();
    titulo("INFORME — TOP 5 CLIENTES");
    m_conn.enviar(std::string(OP_INF_RANK) + "|#");
    auto filas = m_gestor.recibirListado();
    int pos = 1;
    for (auto& f : filas) {
        auto c = GestorDatos::parsearTrama(f);
        if (c.size() >= 3) {
            int o = (c[0] == RESP_OK) ? 1 : 0;
            std::cout << " " << pos++ << ". " << c[o+1]
                      << " (" << c[o] << ") — " << c[o+2] << " reserva(s)\n";
        }
    }
    if (filas.empty()) std::cout << "Sin datos.\n";
    pausar();
}

void Menu::informeDestinos() {
    limpiarPantalla();
    titulo("INFORME — DESTINOS POPULARES (ULTIMO MES)");
    m_conn.enviar(std::string(OP_INF_DEST) + "|#");
    auto filas = m_gestor.recibirListado();
    int pos = 1;
    for (auto& f : filas) {
        auto c = GestorDatos::parsearTrama(f);
        if (c.size() >= 2) {
            int o = (c[0] == RESP_OK) ? 1 : 0;
            std::cout << " " << pos++ << ". " << c[o] << " — " << c[o+1] << " reserva(s)\n";
        }
    }
    if (filas.empty()) std::cout << "Sin datos en el ultimo mes.\n";
    pausar();
}

// HELPERS UI
int Menu::leerEntero(const std::string& mensaje) {
	int val;
	while (true) {
        if (!mensaje.empty()) std::cout << mensaje << ": ";
        if (std::cin >> val) {
            // Limpiar TODO lo que quede en la línea, no solo el \n
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Valor no valido, intente de nuevo: ";
    }
}

std::string Menu::leerCadena(const std::string& mensaje) {
	std::string val;
	std::cout << mensaje << ": ";
    // Limpiar \n residuales antes del getline
    while (std::cin.peek() == '\n' || std::cin.peek() == '\r') {
        std::cin.ignore();
    }
    std::getline(std::cin, val);
    return val;
}

void Menu::pausar() {
    std::cout << "\nPresione Enter para continuar...";
    if (std::cin.rdbuf()->in_avail() == 0) {
        std::cin.get();
    } else {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void Menu::limpiarPantalla() { system("cls"); }

void Menu::separador(char c, int n) { std::cout << std::string(n, c) << "\n"; }

void Menu::titulo(const std::string& texto) {
    separador('=');
    std::cout << "  " << texto << "\n";
    separador('=');
}

bool Menu::confirmar(const std::string& pregunta) {
    std::cout << pregunta << " (s/n): ";
    std::string resp;
    std::getline(std::cin, resp);
    return (!resp.empty() && (resp[0] == 's' || resp[0] == 'S'));
}
