#include "../include/Dominio.h"
#include <sstream>
#include <iomanip>
#include <iostream>


std::vector<std::string> partirTrama(const std::string &trama, char sep) {
    std::vector<std::string> tokens;
    std::stringstream ss(trama);
    std::string token;
    while (std::getline(ss, token, sep)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Cliente
Cliente Cliente::desdeTokens(std::vector<std::string> &tokens, int offset) {
    /* Formato OK: OK|id|dni|nombre|apellidos|tlf|email|fnac
     * offset apunta al primer campo de datos (id), tras el "OK"  */
    Cliente c;
    if ((int)tokens.size() > offset + 0) c.id              = std::stoi(tokens[offset + 0]);
    if ((int)tokens.size() > offset + 1) c.dni             = tokens[offset + 1];
    if ((int)tokens.size() > offset + 2) c.nombre          = tokens[offset + 2];
    if ((int)tokens.size() > offset + 3) c.apellidos       = tokens[offset + 3];
    if ((int)tokens.size() > offset + 4) c.telefono        = tokens[offset + 4];
    if ((int)tokens.size() > offset + 5) c.email           = tokens[offset + 5];
    if ((int)tokens.size() > offset + 6) c.fechaNacimiento = tokens[offset + 6];
    return c;
}

std::string Cliente::toString() const {
    std::ostringstream os;
    os << "DNI:        " << dni            << "\n"
       << "Nombre:     " << nombre << " " << apellidos << "\n"
       << "Teléfono:   " << telefono       << "\n"
       << "Email:      " << email          << "\n"
       << "Nacimiento: " << fechaNacimiento;
    return os.str();
}

std::string Cliente::serializar() const {
    /* Para ACL y MCL: dni|nombre|apellidos|tlf|email|fnac */
    return dni + "|" + nombre + "|" + apellidos + "|" +
           telefono + "|" + email + "|" + fechaNacimiento;
}

// Paquete


Paquete Paquete::desdeTokens(std::vector<std::string> &tokens, int offset) {
    /* Formato: OK|codigo|nombre|precio|origen|destino|tot|disp */
    Paquete p;
    if ((int)tokens.size() > offset + 0) p.codigo             = std::stoi(tokens[offset + 0]);
    if ((int)tokens.size() > offset + 1) p.nombre             = tokens[offset + 1];
    if ((int)tokens.size() > offset + 2) p.precio             = std::stod(tokens[offset + 2]);
    if ((int)tokens.size() > offset + 3) p.origen             = tokens[offset + 3];
    if ((int)tokens.size() > offset + 4) p.destino            = tokens[offset + 4];
    if ((int)tokens.size() > offset + 5) p.plazasTotales      = std::stoi(tokens[offset + 5]);
    if ((int)tokens.size() > offset + 6) p.plazasDisponibles  = std::stoi(tokens[offset + 6]);
    return p;
}

std::string Paquete::toString() const {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2);
    os << "[" << codigo << "] " << nombre << "\n"
       << "  " << origen << " → " << destino << "\n"
       << "  Precio: " << precio << " €"
       << "  |  Plazas: " << plazasDisponibles << "/" << plazasTotales;
    return os.str();
}

std::string Paquete::serializar() const {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2);
    os << codigo << "|" << nombre << "|" << precio << "|"
       << origen << "|" << destino << "|" << plazasTotales;
    return os.str();
}

// Alojamiento
Alojamiento Alojamiento::desdeTokens(std::vector<std::string> &tokens, int offset) {
    /* Formato: OK|codigo|nombre|dir|tipo|ciudad */
    Alojamiento a;
    if ((int)tokens.size() > offset + 0) a.codigo     = tokens[offset + 0];
    if ((int)tokens.size() > offset + 1) a.nombre     = tokens[offset + 1];
    if ((int)tokens.size() > offset + 2) a.direccion  = tokens[offset + 2];
    if ((int)tokens.size() > offset + 3) a.tipo       = tokens[offset + 3];
    if ((int)tokens.size() > offset + 4) a.codCiudad  = tokens[offset + 4];
    return a;
}

std::string Alojamiento::toString() const {
    std::ostringstream os;
    os << "[" << codigo << "] " << nombre << " (" << tipo << ")\n"
       << "  " << direccion << " — Ciudad: " << codCiudad;
    return os.str();
}

std::string Alojamiento::serializar() const {
    return codigo + "|" + nombre + "|" + direccion + "|" + tipo + "|" + codCiudad;
}

//Transporte — fabrica y metodos base
Transporte* Transporte::crear(const std::string &tipo,
                               const std::string &codigo,
                               const std::string &fSalida,
                               const std::string &fLlegada,
                               int idPaquete) {
    Transporte *t = nullptr;

    /* Polimorfismo: instanciar el subtipo concreto segun 'tipo' */
    if (tipo == "avion")        t = new Avion();
    else if (tipo == "tren")    t = new Tren();
    else if (tipo == "autobus") t = new Autobus();
    else                        t = new TransporteGenerico();

    t->codigo       = codigo;
    t->tipo         = tipo;
    t->fechaSalida  = fSalida;
    t->fechaLlegada = fLlegada;
    t->idPaquete    = idPaquete;
    return t;
}

Transporte* Transporte::desdeTokens(std::vector<std::string> &tokens, int offset) {
    /* Formato: OK|codigo|tipo|f_salida|f_llegada|id_paquete */
    std::string cod, tip, fsal, flleg;
    int idpqt = 0;
    if ((int)tokens.size() > offset + 0) cod   = tokens[offset + 0];
    if ((int)tokens.size() > offset + 1) tip   = tokens[offset + 1];
    if ((int)tokens.size() > offset + 2) fsal  = tokens[offset + 2];
    if ((int)tokens.size() > offset + 3) flleg = tokens[offset + 3];
    if ((int)tokens.size() > offset + 4) idpqt = std::stoi(tokens[offset + 4]);
    return Transporte::crear(tip, cod, fsal, flleg, idpqt);
}

std::string Transporte::toString() const {
    std::ostringstream os;
    os << descripcionMedio() << "  [" << codigo << "]\n"
       << "  Salida: " << fechaSalida
       << "  →  Llegada: " << fechaLlegada
       << "  |  Paquete: " << idPaquete;
    return os.str();
}

std::string Transporte::serializar() const {
    return codigo + "|" + tipo + "|" + fechaSalida + "|" +
           fechaLlegada + "|" + std::to_string(idPaquete);
}

//Reserva
Reserva Reserva::desdeTokens(std::vector<std::string> &tokens, int offset) {
    /* Formato: OK|id|cod_paquete|nombre_paquete|fecha */
    Reserva r;
    if ((int)tokens.size() > offset + 0) r.id            = std::stoi(tokens[offset + 0]);
    if ((int)tokens.size() > offset + 1) r.codPaquete    = std::stoi(tokens[offset + 1]);
    if ((int)tokens.size() > offset + 2) r.nombrePaquete = tokens[offset + 2];
    if ((int)tokens.size() > offset + 3) r.fecha         = tokens[offset + 3];
    return r;
}

std::string Reserva::toString() const {
    std::ostringstream os;
    os << "Reserva #" << id << " — " << nombrePaquete
       << " (paquete " << codPaquete << ")\n"
       << "  Fecha: " << fecha;
    return os.str();
}
