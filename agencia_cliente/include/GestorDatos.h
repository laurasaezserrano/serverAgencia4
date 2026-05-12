#ifndef GESTORDATOS_H_
#define GESTORDATOS_H_

/*

 * Cache en memoria del cliente.
 *
 * Responsabilidades:
 *   1. Parsear las tramas del servidor y construir objetos del dominio.
 *   2. Almacenar los listados en vectores/maps para no volver a
 *      pedir al servidor datos ya descargados.
 *   3. Invalidar la cache cuando una escritura cambia los datos.
 *
 * Cumple el requisito del enunciado: "minimizar intercambios con el servidor".
 */

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "Modelos.h"
#include "ConexionServidor.h"

class GestorDatos {
public:
    explicit GestorDatos(ConexionServidor& conn);

    /* ── Parseo de tramas ─────────────────────────────────── */
    /* Divide una trama "campo1|campo2|campo3|#" en un vector de strings */
    static std::vector<std::string> parsearTrama(const std::string& trama);

    /* Recibe todas las filas de un listado (LST_BEGIN...LST_END) */
    std::vector<std::string> recibirListado();

    /* ── Clientes ─────────────────────────────────────────── */
    const std::vector<Cliente>& getClientes();   /* usa cache */
    Cliente* buscarClientePorDni(const std::string& dni);
    void invalidarCacheClientes() { m_clientesCargados = false; }

    /* ── Paquetes ─────────────────────────────────────────── */
    const std::vector<Paquete>& getPaquetes();   /* usa cache */
    Paquete* buscarPaquetePorCodigo(int codigo);
    void invalidarCachePaquetes() { m_paquetesCargados = false; }

    /* ── Alojamientos ─────────────────────────────────────── */
    const std::vector<Alojamiento>& getAlojamientos(); /* usa cache */
    void invalidarCacheAlojamientos() { m_alojamientosCargados = false; }

    /* ── Transportes ──────────────────────────────────────── */
    /* Devuelve punteros polimorficos — el caller NO los libera */
    const std::vector<Transporte*>& getTransportes(); /* usa cache */
    void invalidarCacheTransportes();
    ~GestorDatos(); /* libera los Transporte* */

    /* ── Reservas (no se cachean — siempre frescas) ───────── */
    std::vector<Reserva> getReservasPorCliente(const std::string& dni);

private:
    ConexionServidor& m_conn;

    /* Caches */
    std::vector<Cliente>      m_clientes;
    std::vector<Paquete>      m_paquetes;
    std::vector<Alojamiento>  m_alojamientos;
    std::vector<Transporte*>  m_transportes;

    bool m_clientesCargados     = false;
    bool m_paquetesCargados     = false;
    bool m_alojamientosCargados = false;
    bool m_transportesCargados  = false;

    /* Helpers de carga */
    void cargarClientes();
    void cargarPaquetes();
    void cargarAlojamientos();
    void cargarTransportes();

    Cliente     parsearCliente(const std::vector<std::string>& campos);
    Paquete     parsearPaquete(const std::vector<std::string>& campos);
    Alojamiento parsearAlojamiento(const std::vector<std::string>& campos);
    Transporte* parsearTransporte(const std::vector<std::string>& campos);
    Reserva     parsearReserva(const std::vector<std::string>& campos);
};

#endif /* GESTORDATOS_H_ */
