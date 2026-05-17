#include "../include/GestorDatos.h"
#include "../include/protocolo.h"
#include <iostream>
#include <cstdlib>

GestorDatos::GestorDatos(ConexionServidor& conn) : m_conn(conn) {}

GestorDatos::~GestorDatos() {
    invalidarCacheTransportes();
}

// Partir trama por '|' y '#'
std::vector<std::string> GestorDatos::parsearTrama(const std::string& trama) {
    std::vector<std::string> campos;
    std::string campo;
    for (char c : trama) {
        if (c == PROTO_SEP || c == PROTO_FIN) {
            if (!campo.empty()) { campos.push_back(campo); campo.clear(); }
        } else {
            campo += c;
        }
    }
    return campos;
}

// Recibir listado completo LST_BEGIN...LST_END
std::vector<std::string> GestorDatos::recibirListado() {
    std::vector<std::string> filas;
    std::string linea = m_conn.recibir();
    if (linea.find(RESP_LIST_INI) == std::string::npos) return filas;
    while (true) {
        linea = m_conn.recibir();
        if (linea.find(RESP_LIST_FIN) != std::string::npos || linea.empty()) break;
        filas.push_back(linea);
    }
    return filas;
}

// CLIENTES
void GestorDatos::cargarClientes() {
    m_clientes.clear();
    m_conn.enviar(std::string(OP_LIST_CLI) + "|#");
    for (auto& f : recibirListado()) {
        auto campos = parsearTrama(f);
        /* OK|id|dni|nombre|apellidos|tlf|email|fnac -> 8 campos */
        if (campos.size() >= 8)
            m_clientes.push_back(parsearCliente(campos));
    }
    m_clientesCargados = true;
}

const std::vector<Cliente>& GestorDatos::getClientes() {
    if (!m_clientesCargados) cargarClientes();
    return m_clientes;
}

Cliente* GestorDatos::buscarClientePorDni(const std::string& dni) {
    for (auto& c : m_clientes)
        if (c.dni == dni) return &c;

    m_conn.enviar(std::string(OP_GET_CLI) + "|" + dni + "|#");
    auto campos = parsearTrama(m_conn.recibir());
    if (campos.size() >= 8 && campos[0] == RESP_OK) {
        m_clientes.push_back(parsearCliente(campos));
        return &m_clientes.back();
    }
    return nullptr;
}

Cliente GestorDatos::parsearCliente(const std::vector<std::string>& c) {
    /* OK|id|dni|nombre|apellidos|tlf|email|fnac */
    /* desdeTokens espera vector sin el "OK", offset=0 al campo id */
    std::vector<std::string> datos(c.begin() + 1, c.end());
    return Cliente::desdeTokens(datos, 0);
}

// PAQUETES
void GestorDatos::cargarPaquetes() {
    m_paquetes.clear();
    m_conn.enviar(std::string(OP_LIST_PQT) + "|#");
    for (auto& f : recibirListado()) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 8)
            m_paquetes.push_back(parsearPaquete(campos));
    }
    m_paquetesCargados = true;
}

const std::vector<Paquete>& GestorDatos::getPaquetes() {
    if (!m_paquetesCargados) cargarPaquetes();
    return m_paquetes;
}

Paquete* GestorDatos::buscarPaquetePorCodigo(int codigo) {
    for (auto& p : m_paquetes)
        if (p.codigo == codigo) return &p;

    m_conn.enviar(std::string(OP_GET_PQT) + "|" + std::to_string(codigo) + "|#");
    auto campos = parsearTrama(m_conn.recibir());
    if (campos.size() >= 8 && campos[0] == RESP_OK) {
        m_paquetes.push_back(parsearPaquete(campos));
        return &m_paquetes.back();
    }
    return nullptr;
}

Paquete GestorDatos::parsearPaquete(const std::vector<std::string>& c) {
    /* OK|codigo|nombre|precio|origen|destino|totales|disponibles */
    std::vector<std::string> datos(c.begin() + 1, c.end());
    return Paquete::desdeTokens(datos, 0);
}

// ALOJAMIENTOS
void GestorDatos::cargarAlojamientos() {
    m_alojamientos.clear();
    m_conn.enviar(std::string(OP_LIST_ALO) + "|#");
    for (auto& f : recibirListado()) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 6)
            m_alojamientos.push_back(parsearAlojamiento(campos));
    }
    m_alojamientosCargados = true;
}

const std::vector<Alojamiento>& GestorDatos::getAlojamientos() {
    if (!m_alojamientosCargados) cargarAlojamientos();
    return m_alojamientos;
}

Alojamiento GestorDatos::parsearAlojamiento(const std::vector<std::string>& c) {
    /* OK|codigo|nombre|dir|tipo|ciudad */
    std::vector<std::string> datos(c.begin() + 1, c.end());
    return Alojamiento::desdeTokens(datos, 0);
}

// TRANSPORTES (polimorfico)
void GestorDatos::cargarTransportes() {
    invalidarCacheTransportes();
    m_conn.enviar(std::string(OP_LIST_TRP) + "|#");
    for (auto& f : recibirListado()) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 6) {
            Transporte* t = parsearTransporte(campos);
            if (t) m_transportes.push_back(t);
        }
    }
    m_transportesCargados = true;
}

const std::vector<Transporte*>& GestorDatos::getTransportes() {
    if (!m_transportesCargados) cargarTransportes();
    return m_transportes;
}

void GestorDatos::invalidarCacheTransportes() {
    for (auto* t : m_transportes) delete t;
    m_transportes.clear();
    m_transportesCargados = false;
}

Transporte* GestorDatos::parsearTransporte(const std::vector<std::string>& c) {
    /* OK|codigo|tipo|f_salida|f_llegada|id_paquete
     * Dominio::Transporte::crear(tipo, codigo, fSalida, fLlegada, idPaq) */
    std::vector<std::string> datos(c.begin() + 1, c.end());
    return Transporte::desdeTokens(datos, 0);
}

// RESERVAS
std::vector<Reserva> GestorDatos::getReservasPorCliente(const std::string& dni) {
    std::vector<Reserva> reservas;
    m_conn.enviar(std::string(OP_LIST_RES_CLI) + "|" + dni + "|#");
    for (auto& f : recibirListado()) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 5)
            reservas.push_back(parsearReserva(campos));
    }
    return reservas;
}

Reserva GestorDatos::parsearReserva(const std::vector<std::string>& c) {
    /* OK|id|cod_paquete|nombre_paquete|fecha */
    std::vector<std::string> datos(c.begin() + 1, c.end());
    return Reserva::desdeTokens(datos, 0);
}
