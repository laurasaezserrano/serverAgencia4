/*
 * agencia_cliente.cpp
 * ===================
 * Cliente TCP para la Agencia de Viajes — Fase 2.
 *
 * Requisitos cubiertos:
 *  2.1 ConexionServidor -> encapsula socket TCP.
 *  2.2 Clases dominio -> Cliente, Paquete, Reserva.
 *  2.3 Cache en memoria -> vectors y maps.
 *  2.4 Menu jerarquico reutilizable.
 *  2.5 Informes recibidos del servidor.
 *
 * Plataforma: Windows + Winsock2
 *
 * Compilacion:
 * g++ agencia_cliente.cpp -o agencia_cliente.exe -lws2_32
 */

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

#define PROTO_FIN '#'
#define PROTO_BUF 4096

using namespace std;


class Cliente {
private:
    int id;
    string dni;
    string nombre;
    string apellidos;
    string telefono;
    string email;
    string fechaNacimiento;

public:
    Cliente() : id(0) {}

    Cliente(int _id,
             const string &_dni,
             const string &_nombre,
             const string &_apellidos,
             const string &_telefono,
             const string &_email,
             const string &_fechaNacimiento)
        : id(_id),
          dni(_dni),
          nombre(_nombre),
          apellidos(_apellidos),
          telefono(_telefono),
          email(_email),
          fechaNacimiento(_fechaNacimiento) {}

    string getDni() const {
        return dni;
    }

    string toString() const {
        stringstream ss;

        ss << "ID: " << id << endl;
        ss << "DNI: " << dni << endl;
        ss << "Nombre: " << nombre << endl;
        ss << "Apellidos: " << apellidos << endl;
        ss << "Telefono: " << telefono << endl;
        ss << "Email: " << email << endl;
        ss << "Nacimiento: " << fechaNacimiento << endl;

        return ss.str();
    }
};

class Paquete {
private:
    int codigo;
    string nombre;
    double precio;
    string origen;
    string destino;
    int plazasTotales;
    int plazasDisponibles;

public:
    Paquete()
        : codigo(0), precio(0), plazasTotales(0), plazasDisponibles(0) {}

    Paquete(int _codigo,
             const string &_nombre,
             double _precio,
             const string &_origen,
             const string &_destino,
             int _totales,
             int _disponibles)
        : codigo(_codigo),
          nombre(_nombre),
          precio(_precio),
          origen(_origen),
          destino(_destino),
          plazasTotales(_totales),
          plazasDisponibles(_disponibles) {}

    int getCodigo() const {
        return codigo;
    }

    string toString() const {
        stringstream ss;

        ss << "Codigo: " << codigo << endl;
        ss << "Nombre: " << nombre << endl;
        ss << "Precio: " << fixed << setprecision(2) << precio << endl;
        ss << "Origen: " << origen << endl;
        ss << "Destino: " << destino << endl;
        ss << "Plazas totales: " << plazasTotales << endl;
        ss << "Plazas disponibles: " << plazasDisponibles << endl;

        return ss.str();
    }
};

class Reserva {
private:
    int id;
    int codPaquete;
    string nombrePaquete;
    string fecha;

public:
    Reserva() : id(0), codPaquete(0) {}

    Reserva(int _id,
             int _codPaquete,
             const string &_nombrePaquete,
             const string &_fecha)
        : id(_id),
          codPaquete(_codPaquete),
          nombrePaquete(_nombrePaquete),
          fecha(_fecha) {}

    string toString() const {
        stringstream ss;

        ss << "ID Reserva: " << id << endl;
        ss << "Paquete: " << codPaquete << endl;
        ss << "Nombre paquete: " << nombrePaquete << endl;
        ss << "Fecha: " << fecha << endl;

        return ss.str();
    }
};

/* ============================================================
 * CONEXION SERVIDOR
 * ============================================================ */

class ConexionServidor {
private:
    SOCKET sock;
    bool conectado;

public:
    ConexionServidor() {
    	string bufferInterno;
        sock = INVALID_SOCKET;
        conectado = false;
    }

    bool conectar(const string &ip, int puerto) {
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cout << "Error inicializando Winsock" << endl;
            return false;
        }

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (sock == INVALID_SOCKET) {
            cout << "Error creando socket" << endl;
            return false;
        }

        sockaddr_in servidor;
        servidor.sin_family = AF_INET;
        servidor.sin_port = htons(puerto);
        servidor.sin_addr.s_addr = inet_addr(ip.c_str());

        if (connect(sock, (sockaddr *)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
            cout << "No se pudo conectar con el servidor" << endl;
            closesocket(sock);
            WSACleanup();
            return false;
        }

        conectado = true;
        cout << "Conexion establecida correctamente" << endl;

        return true;
    }

    bool enviar(const string &trama) {
        if (!conectado)
            return false;

        int enviados = send(sock, trama.c_str(), trama.length(), 0);

        return enviados != SOCKET_ERROR;
    }

    string recibir() {

        while (true) {

            size_t pos = bufferInterno.find('#');

            if (pos != string::npos) {

                string trama = bufferInterno.substr(0, pos);

                bufferInterno.erase(0, pos + 1);

                return trama;
            }

            char buffer[PROTO_BUF];

            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

            if (bytes <= 0) {
                return "";
            }

            buffer[bytes] = '\0';

            bufferInterno += buffer;
        }
    }

    void desconectar() {
        if (conectado) {
            enviar("BYE|#");
            closesocket(sock);
            WSACleanup();
            conectado = false;
        }
    }

    ~ConexionServidor() {
        desconectar();
    }
};

/* CLIENTE AGENCIA */

class ClienteAgencia {
private:
    ConexionServidor conexion;

    vector<Cliente> clientes;
    vector<Paquete> paquetes;
    vector<Reserva> reservas;

    map<string, Cliente> mapaClientes;
    map<int, Paquete> mapaPaquetes;

public:
    bool iniciar() {
        string ip;
        int puerto;

        cout << "IP servidor: ";
        cin >> ip;

        cout << "Puerto: ";
        cin >> puerto;

        return conexion.conectar(ip, puerto);
    }

    vector<string> split(const string &texto, char sep) {
        vector<string> tokens;
        string token;
        stringstream ss(texto);

        while (getline(ss, token, sep)) {
            tokens.push_back(token);
        }

        return tokens;
    }

    bool login() {
        string user;
        string pass;

        cout << "Usuario: ";
        cin >> user;

        cout << "Clave: ";
        cin >> pass;

        string trama = "LOG|" + user + "|" + pass + "|#";

        conexion.enviar(trama);

        string respuesta = conexion.recibir();

        cout << endl;
        cout << "Servidor: " << respuesta << endl;

        return respuesta.find("OK|AUTH") != string::npos;
    }

    /* CLIENTES */

    void listarClientes() {
        conexion.enviar("LCL|#");

        clientes.clear();
        mapaClientes.clear();

        while (true) {
            string resp = conexion.recibir();

            if (resp.find("LST_END") != string::npos)
                break;

            if (resp.find("OK|") != string::npos) {

                vector<string> t = split(resp, '|');

                if (t.size() >= 8) {
                    Cliente c(
                        atoi(t[1].c_str()),
                        t[2],
                        t[3],
                        t[4],
                        t[5],
                        t[6],
                        t[7]
                    );

                    clientes.push_back(c);
                    mapaClientes[t[2]] = c;
                }
            }
        }

        cout << endl;
        cout << "===== CLIENTES =====" << endl;

        for (size_t i = 0; i < clientes.size(); i++) {
            cout << clientes[i].toString() << endl;
            cout << "--------------------" << endl;
        }
    }

    void altaCliente() {
        string dni;
        string nombre;
        string apellidos;
        string telefono;
        string email;
        string fecha;

        cin.ignore();

        cout << "DNI: ";
        getline(cin, dni);

        cout << "Nombre: ";
        getline(cin, nombre);

        cout << "Apellidos: ";
        getline(cin, apellidos);

        cout << "Telefono: ";
        getline(cin, telefono);

        cout << "Email: ";
        getline(cin, email);

        cout << "Fecha nacimiento: ";
        getline(cin, fecha);

        string trama = "ACL|" + dni + "|" + nombre + "|" +
                        apellidos + "|" + telefono + "|" +
                        email + "|" + fecha + "|#";

        conexion.enviar(trama);

        cout << conexion.recibir() << endl;
    }

    void bajaCliente() {
        string dni;

        cout << "DNI cliente: ";
        cin >> dni;

        string trama = "BCL|" + dni + "|#";

        conexion.enviar(trama);

        cout << conexion.recibir() << endl;
    }

    void buscarCliente() {
        string dni;

        cout << "DNI cliente: ";
        cin >> dni;

        string trama = "GCL|" + dni + "|#";

        conexion.enviar(trama);

        string resp = conexion.recibir();

        cout << endl;
        cout << resp << endl;
    }

    /* PAQUETES */

    void listarPaquetes() {
        conexion.enviar("LPQ|#");

        paquetes.clear();
        mapaPaquetes.clear();

        while (true) {
            string resp = conexion.recibir();

            if (resp.find("LST_END") != string::npos)
                break;

            if (resp.find("OK|") != string::npos) {

                vector<string> t = split(resp, '|');

                if (t.size() >= 8) {
                    Paquete p(
                        atoi(t[1].c_str()),
                        t[2],
                        atof(t[3].c_str()),
                        t[4],
                        t[5],
                        atoi(t[6].c_str()),
                        atoi(t[7].c_str())
                    );

                    paquetes.push_back(p);
                    mapaPaquetes[p.getCodigo()] = p;
                }
            }
        }

        cout << endl;
        cout << "===== PAQUETES =====" << endl;

        for (size_t i = 0; i < paquetes.size(); i++) {
            cout << paquetes[i].toString() << endl;
            cout << "--------------------" << endl;
        }
    }

    void buscarPaquete() {
        int codigo;

        cout << "Codigo paquete: ";
        cin >> codigo;

        stringstream trama;
        trama << "GPQ|" << codigo << "|#";

        conexion.enviar(trama.str());

        cout << conexion.recibir() << endl;
    }

    void crearPaquete() {
        int codigo;
        string nombre;
        double precio;
        string origen;
        string destino;
        int plazas;

        cin.ignore();

        cout << "Codigo: ";
        cin >> codigo;

        cin.ignore();

        cout << "Nombre: ";
        getline(cin, nombre);

        cout << "Precio: ";
        cin >> precio;

        cin.ignore();

        cout << "Origen: ";
        getline(cin, origen);

        cout << "Destino: ";
        getline(cin, destino);

        cout << "Plazas: ";
        cin >> plazas;

        stringstream trama;

        trama << "APQ|"
              << codigo << "|"
              << nombre << "|"
              << precio << "|"
              << origen << "|"
              << destino << "|"
              << plazas << "|#";

        conexion.enviar(trama.str());

        cout << conexion.recibir() << endl;
    }

    /* RESERVAS */

    void crearReserva() {
        string dni;
        int codigo;
        string fecha;

        cout << "DNI cliente: ";
        cin >> dni;

        cout << "Codigo paquete: ";
        cin >> codigo;

        cout << "Fecha (YYYY-MM-DD): ";
        cin >> fecha;

        stringstream trama;

        trama << "ARE|"
              << dni << "|"
              << codigo << "|"
              << fecha << "|#";

        conexion.enviar(trama.str());

        cout << conexion.recibir() << endl;
    }

    void cancelarReserva() {
        int id;

        cout << "ID reserva: ";
        cin >> id;

        stringstream trama;
        trama << "BRE|" << id << "|#";

        conexion.enviar(trama.str());

        cout << conexion.recibir() << endl;
    }

    void listarReservasCliente() {
        string dni;

        cout << "DNI cliente: ";
        cin >> dni;

        string trama = "LRC|" + dni + "|#";

        conexion.enviar(trama);

        reservas.clear();

        while (true) {
            string resp = conexion.recibir();

            if (resp.find("LST_END") != string::npos)
                break;

            if (resp.find("OK|") != string::npos) {

                vector<string> t = split(resp, '|');

                if (t.size() >= 5) {
                    Reserva r(
                        atoi(t[1].c_str()),
                        atoi(t[2].c_str()),
                        t[3],
                        t[4]
                    );

                    reservas.push_back(r);
                }
            }
        }

        cout << endl;
        cout << "===== RESERVAS =====" << endl;

        for (size_t i = 0; i < reservas.size(); i++) {
            cout << reservas[i].toString() << endl;
            cout << "--------------------" << endl;
        }
    }

    /* INFORMES */

    void informeOcupacion() {
        conexion.enviar("IOC|#");

        cout << endl;
        cout << "===== OCUPACION =====" << endl;

        mostrarListadoGenerico();
    }

    void informeRanking() {
        conexion.enviar("IRK|#");

        cout << endl;
        cout << "===== RANKING CLIENTES =====" << endl;

        mostrarListadoGenerico();
    }

    void informeDestinos() {
        conexion.enviar("IDS|#");

        cout << endl;
        cout << "===== DESTINOS =====" << endl;

        mostrarListadoGenerico();
    }

    void mostrarListadoGenerico() {
        while (true) {
            string resp = conexion.recibir();

            if (resp.find("LST_END") != string::npos)
                break;

            cout << resp << endl;
        }
    }

    /* MENUS */

    void menuClientes() {
        int op;

        do {
            cout << endl;
            cout << "===== MENU CLIENTES =====" << endl;
            cout << "1. Alta cliente" << endl;
            cout << "2. Baja cliente" << endl;
            cout << "3. Buscar cliente" << endl;
            cout << "4. Listar clientes" << endl;
            cout << "0. Volver" << endl;
            cout << "Opcion: ";
            cin >> op;

            switch (op) {
                case 1:
                    altaCliente();
                    break;

                case 2:
                    bajaCliente();
                    break;

                case 3:
                    buscarCliente();
                    break;

                case 4:
                    listarClientes();
                    break;
            }

        } while (op != 0);
    }

    void menuPaquetes() {
        int op;

        do {
            cout << endl;
            cout << "===== MENU PAQUETES =====" << endl;
            cout << "1. Crear paquete" << endl;
            cout << "2. Buscar paquete" << endl;
            cout << "3. Listar paquetes" << endl;
            cout << "0. Volver" << endl;
            cout << "Opcion: ";
            cin >> op;

            switch (op) {
                case 1:
                    crearPaquete();
                    break;

                case 2:
                    buscarPaquete();
                    break;

                case 3:
                    listarPaquetes();
                    break;
            }

        } while (op != 0);
    }

    void menuReservas() {
        int op;

        do {
            cout << endl;
            cout << "===== MENU RESERVAS =====" << endl;
            cout << "1. Crear reserva" << endl;
            cout << "2. Cancelar reserva" << endl;
            cout << "3. Reservas cliente" << endl;
            cout << "0. Volver" << endl;
            cout << "Opcion: ";
            cin >> op;

            switch (op) {
                case 1:
                    crearReserva();
                    break;

                case 2:
                    cancelarReserva();
                    break;

                case 3:
                    listarReservasCliente();
                    break;
            }

        } while (op != 0);
    }

    void menuInformes() {
        int op;

        do {
            cout << endl;
            cout << "===== MENU INFORMES =====" << endl;
            cout << "1. Ocupacion" << endl;
            cout << "2. Ranking clientes" << endl;
            cout << "3. Destinos populares" << endl;
            cout << "0. Volver" << endl;
            cout << "Opcion: ";
            cin >> op;

            switch (op) {
                case 1:
                    informeOcupacion();
                    break;

                case 2:
                    informeRanking();
                    break;

                case 3:
                    informeDestinos();
                    break;
            }

        } while (op != 0);
    }

    void menuPrincipal() {
        int op;

        do {
            cout << endl;
            cout << "================================" << endl;
            cout << "  AGENCIA DE VIAJES - CLIENTE" << endl;
            cout << "================================" << endl;
            cout << "1. Clientes" << endl;
            cout << "2. Paquetes" << endl;
            cout << "3. Reservas" << endl;
            cout << "4. Informes" << endl;
            cout << "0. Salir" << endl;
            cout << "Opcion: ";
            cin >> op;

            switch (op) {
                case 1:
                    menuClientes();
                    break;

                case 2:
                    menuPaquetes();
                    break;

                case 3:
                    menuReservas();
                    break;

                case 4:
                    menuInformes();
                    break;
            }

        } while (op != 0);
    }

    void cerrar() {
        conexion.desconectar();
    }
};


int main() {
    ClienteAgencia app;

    if (!app.iniciar()) {
        cout << "No se pudo iniciar el cliente" << endl;
        return 1;
    }

    if (!app.login()) {
        cout << "Login incorrecto" << endl;
        return 1;
    }

    app.menuPrincipal();

    app.cerrar();

    return 0;
}
