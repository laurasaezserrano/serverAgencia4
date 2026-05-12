/*
 * GestorDatos.cpp
 * ---------------
 * Cache en memoria y parseo de tramas del protocolo.
 */

#include "../include/GestorDatos.h"
#include "../include/protocolo.h"
#include <iostream>
#include <cstdlib>   /* atoi, atof */

GestorDatos::GestorDatos(ConexionServidor& conn) : m_conn(conn) {}

GestorDatos::~GestorDatos() {
    invalidarCacheTransportes();
}

/* ── Utilidad: partir trama por '|' ──────────────────────────── */
std::vector<std::string> GestorDatos::parsearTrama(const std::string& trama) {
    std::vector<std::string> campos;
    std::string campo;
    for (char c : trama) {
        if (c == PROTO_SEP || c == PROTO_FIN) {
            if (!campo.empty()) {
                campos.push_back(campo);
                campo.clear();
            }
        } else {
            campo += c;
        }
    }
    return campos;
}

/* ── Recibir listado completo LST_BEGIN...LST_END ─────────────── */
std::vector<std::string> GestorDatos::recibirListado() {
    std::vector<std::string> filas;

    std::string linea = m_conn.recibir(); /* LST_BEGIN|# */
    if (linea.find(RESP_LIST_INI) == std::string::npos) {
        /* No era un listado — puede ser ERR */
        return filas;
    }

    while (true) {
        linea = m_conn.recibir();
        if (linea.find(RESP_LIST_FIN) != std::string::npos) break;
        if (linea.empty()) break;
        filas.push_back(linea);
    }
    return filas;
}

/* ── CLIENTES ─────────────────────────────────────────────────── */
void GestorDatos::cargarClientes() {
    m_clientes.clear();
    m_conn.enviar(std::string(OP_LIST_CLI) + "|#");
    auto filas = recibirListado();
    for (auto& f : filas) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 7)
            m_clientes.push_back(parsearCliente(campos));
    }
    m_clientesCargados = true;
}

const std::vector<Cliente>& GestorDatos::getClientes() {
    if (!m_clientesCargados) cargarClientes();
    return m_clientes;
}

Cliente* GestorDatos::buscarClientePorDni(const std::string& dni) {
    /* Primero buscar en cache */
    for (auto& c : m_clientes)
        if (c.dni == dni) return &c;

    /* Si no estaba, pedir al servidor directamente */
    m_conn.enviar(std::string(OP_GET_CLI) + "|" + dni + "|#");
    std::string resp = m_conn.recibir();
    auto campos = parsearTrama(resp);
    if (campos.size() >= 7 && campos[0] == RESP_OK) {
        m_clientes.push_back(parsearCliente(campos));
        return &m_clientes.back();
    }
    return nullptr;
}

Cliente GestorDatos::parsearCliente(const std::vector<std::string>& c) {
    /* OK|id|dni|nombre|apellidos|tlf|email|fnac */
    int offset = (c[0] == RESP_OK) ? 1 : 0;
    return Cliente(
        atoi(c[offset + 0].c_str()),
        c[offset + 1], c[offset + 2], c[offset + 3],
        c[offset + 4], c[offset + 5], c[offset + 6]
    );
}

/* ── PAQUETES ─────────────────────────────────────────────────── */
void GestorDatos::cargarPaquetes() {
    m_paquetes.clear();
    m_conn.enviar(std::string(OP_LIST_PQT) + "|#");
    auto filas = recibirListado();
    for (auto& f : filas) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 7)
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
    std::string resp = m_conn.recibir();
    auto campos = parsearTrama(resp);
    if (campos.size() >= 7 && campos[0] == RESP_OK) {
        m_paquetes.push_back(parsearPaquete(campos));
        return &m_paquetes.back();
    }
    return nullptr;
}

Paquete GestorDatos::parsearPaquete(const std::vector<std::string>& c) {
    /* OK|codigo|nombre|precio|origen|destino|totales|disponibles */
    int o = (c[0] == RESP_OK) ? 1 : 0;
    return Paquete(
        atoi(c[o+0].c_str()),
        c[o+1],
        (float)atof(c[o+2].c_str()),
        c[o+3], c[o+4],
        atoi(c[o+5].c_str()),
        atoi(c[o+6].c_str())
    );
}

/* ── ALOJAMIENTOS ─────────────────────────────────────────────── */
void GestorDatos::cargarAlojamientos() {
    m_alojamientos.clear();
    m_conn.enviar(std::string(OP_LIST_ALO) + "|#");
    auto filas = recibirListado();
    for (auto& f : filas) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 5)
            m_alojamientos.push_back(parsearAlojamiento(campos));
    }
    m_alojamientosCargados = true;
}

const std::vector<Alojamiento>& GestorDatos::getAlojamientos() {
    if (!m_alojamientosCargados) cargarAlojamientos();
    return m_alojamientos;
}

Alojamiento GestorDatos::parsearAlojamiento(const std::vector<std::string>& c) {
    /* OK|codigo|nombre|direccion|tipo|cod_ciudad */
    int o = (c[0] == RESP_OK) ? 1 : 0;
    return Alojamiento(c[o+0], c[o+1], c[o+2], c[o+3], c[o+4]);
}

/* ── TRANSPORTES (polimorficos) ───────────────────────────────── */
void GestorDatos::cargarTransportes() {
    invalidarCacheTransportes();
    m_conn.enviar(std::string(OP_LIST_TRP) + "|#");
    auto filas = recibirListado();
    for (auto& f : filas) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 5) {
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
    /* OK|codigo|tipo|f_salida|f_llegada|id_paquete */
    int o = (c[0] == RESP_OK) ? 1 : 0;
    return Transporte::crear(
        c[o+0], c[o+1], c[o+2], c[o+3],
        atoi(c[o+4].c_str())
    );
}

/* ── RESERVAS ─────────────────────────────────────────────────── */
std::vector<Reserva> GestorDatos::getReservasPorCliente(const std::string& dni) {
    std::vector<Reserva> reservas;
    m_conn.enviar(std::string(OP_LIST_RES_CLI) + "|" + dni + "|#");
    auto filas = recibirListado();
    for (auto& f : filas) {
        auto campos = parsearTrama(f);
        if (campos.size() >= 4)
            reservas.push_back(parsearReserva(campos));
    }
    return reservas;
}

Reserva GestorDatos::parsearReserva(const std::vector<std::string>& c) {
    /* OK|id|cod_paquete|nombre_paquete|fecha */
    int o = (c[0] == RESP_OK) ? 1 : 0;
    return Reserva(
        atoi(c[o+0].c_str()),
        atoi(c[o+1].c_str()),
        c[o+2], "", c[o+3]
    );
}
