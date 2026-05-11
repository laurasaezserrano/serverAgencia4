#ifndef CACHE_H_
#define CACHE_H_

/*
 * Almacena en memoria los datos ya descargados del servidor.
 * El cliente solo contacta al servidor para escrituras (alta, baja...)
 * y para cargar la primera vez. Las busquedas y filtros son locales.
 */

#include "Dominio.h"
#include <vector>
#include <map>
#include <string>

class Cache {
public:
    /* ── Datos cacheados ──────────────────────────────────────── */
    std::vector<Cliente>     clientes;
    std::vector<Paquete>     paquetes;
    std::vector<Alojamiento> alojamientos;
    std::vector<Transporte*> transportes;   /* punteros por polimorfismo */
    std::vector<Reserva>     reservas;      /* reservas del cliente actual */

    /* Indices por clave para busqueda O(1) */
    std::map<std::string, Cliente>     clientesPorDni;
    std::map<int,         Paquete>     paquetesPorCod;
    std::map<std::string, Alojamiento> alojamientosPorCod;

    /* ── Flags de validez — false = hay que recargar del servidor */
    bool clientesCargados;
    bool paquetesCargados;
    bool alojamientosCargados;
    bool transportesCargados;

    Cache();
    ~Cache();   /* libera los Transporte* del heap */

    /* Invalida un grupo concreto para forzar recarga en la proxima consulta */
    void invalidarClientes();
    void invalidarPaquetes();
    void invalidarAlojamientos();
    void invalidarTransportes();
    void invalidarReservas();
    void invalidarTodo();

    /* Helpers de busqueda local (devuelve nullptr si no encuentra) */
    const Cliente     *buscarClientePorDni(const std::string &dni) const;
    const Paquete     *buscarPaquetePorCod(int cod) const;
    const Alojamiento *buscarAlojamientoPorCod(const std::string &cod) const;

    /* Reemplaza los transportes liberando la memoria anterior */
    void setTransportes(std::vector<Transporte*> &nuevos);
};

#endif /* CACHE_H_ */
