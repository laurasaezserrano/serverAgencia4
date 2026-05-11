#include "../include/Cache.h"

Cache::Cache()
    : clientesCargados(false),
      paquetesCargados(false),
      alojamientosCargados(false),
      transportesCargados(false) {}

Cache::~Cache() {
    for (Transporte *t : transportes) delete t;
    transportes.clear();
}

void Cache::invalidarClientes() {
    clientes.clear();
    clientesPorDni.clear();
    clientesCargados = false;
}

void Cache::invalidarPaquetes() {
    paquetes.clear();
    paquetesPorCod.clear();
    paquetesCargados = false;
}

void Cache::invalidarAlojamientos() {
    alojamientos.clear();
    alojamientosPorCod.clear();
    alojamientosCargados = false;
}

void Cache::invalidarTransportes() {
    for (Transporte *t : transportes) delete t;
    transportes.clear();
    transportesCargados = false;
}

void Cache::invalidarReservas() {
    reservas.clear();
}

void Cache::invalidarTodo() {
    invalidarClientes();
    invalidarPaquetes();
    invalidarAlojamientos();
    invalidarTransportes();
    invalidarReservas();
}

const Cliente *Cache::buscarClientePorDni(const std::string &dni) const {
    auto it = clientesPorDni.find(dni);
    return (it != clientesPorDni.end()) ? &it->second : nullptr;
}

const Paquete *Cache::buscarPaquetePorCod(int cod) const {
    auto it = paquetesPorCod.find(cod);
    return (it != paquetesPorCod.end()) ? &it->second : nullptr;
}

const Alojamiento *Cache::buscarAlojamientoPorCod(const std::string &cod) const {
    auto it = alojamientosPorCod.find(cod);
    return (it != alojamientosPorCod.end()) ? &it->second : nullptr;
}

void Cache::setTransportes(std::vector<Transporte*> &nuevos) {
    for (Transporte *t : transportes) delete t;
    transportes = nuevos;
    transportesCargados = true;
}
