#ifndef DOMINIO_H_
#define DOMINIO_H_

/*
 * Dominio.h
 * ---------
 * Clases de dominio del cliente C++.
 * No acceden a la BD directamente — representan datos en memoria
 * recibidos del servidor via protocolo.
 *
 * Jerarquia de Transporte (OO exigida por la rubrica):
 *   Transporte  (clase base abstracta)
 *     ├── Avion
 *     ├── Tren
 *     └── Autobus
 */

#include <string>
#include <vector>

//Cliente

class Cliente {
public:
    int         id;
    std::string dni;
    std::string nombre;
    std::string apellidos;
    std::string telefono;
    std::string email;
    std::string fechaNacimiento;

    Cliente() : id(0) {}

    /* Construye un Cliente deserializando los campos de una trama OK.
     * Formato: OK|id|dni|nombre|apellidos|tlf|email|fnac
     * (el "OK|" ya fue extraído por el llamador)              */
    static Cliente desdeTokens(std::vector<std::string> &tokens, int offset = 0);

    std::string toString() const;
    /* Serializa para enviar en trama ACL/MCL */
    std::string serializar() const;
};

// Paquete

class Paquete {
public:
    int         codigo;
    std::string nombre;
    double      precio;
    std::string origen;
    std::string destino;
    int         plazasTotales;
    int         plazasDisponibles;

    Paquete() : codigo(0), precio(0.0), plazasTotales(0), plazasDisponibles(0) {}

    bool tieneDisponibilidad() const { return plazasDisponibles > 0; }

    static Paquete desdeTokens(std::vector<std::string> &tokens, int offset = 0);

    std::string toString() const;
    std::string serializar() const;
};

//Alojamiento
class Alojamiento {
public:
    std::string codigo;
    std::string nombre;
    std::string direccion;
    std::string tipo;
    std::string codCiudad;

    Alojamiento() {}

    static Alojamiento desdeTokens(std::vector<std::string> &tokens, int offset = 0);

    std::string toString() const;
    std::string serializar() const;
};

//Transporte — clase base abstracta
class Transporte {
public:
    std::string codigo;
    std::string tipo;
    std::string fechaSalida;
    std::string fechaLlegada;
    int         idPaquete;

    Transporte() : idPaquete(0) {}
    virtual ~Transporte() {}

    /* Metodo virtual puro: cada subclase describe su medio */
    virtual std::string descripcionMedio() const = 0;

    /* Fabrica: crea el subtipo correcto segun el campo 'tipo' */
    static Transporte* crear(const std::string &tipo,
                             const std::string &codigo,
                             const std::string &fSalida,
                             const std::string &fLlegada,
                             int idPaquete);

    static Transporte* desdeTokens(std::vector<std::string> &tokens, int offset = 0);

    std::string toString() const;
    std::string serializar() const;
};

//Subclases concretas

class Avion : public Transporte {
public:
    Avion() { tipo = "avion"; }
    std::string descripcionMedio() const override {
        return "✈  Avión";
    }
};

class Tren : public Transporte {
public:
    Tren() { tipo = "tren"; }
    std::string descripcionMedio() const override {
        return "🚂  Tren";
    }
};

class Autobus : public Transporte {
public:
    Autobus() { tipo = "autobus"; }
    std::string descripcionMedio() const override {
        return "🚌  Autobús";
    }
};

/* Tipo generico para transportes con tipo desconocido */
class TransporteGenerico : public Transporte {
public:
    TransporteGenerico() {}
    std::string descripcionMedio() const override {
        return "🚗  " + tipo;
    }
};

//Reserva
class Reserva {
public:
    int         id;
    int         codPaquete;
    std::string nombrePaquete;
    std::string fecha;
    std::string dniCliente;

    Reserva() : id(0), codPaquete(0) {}

    static Reserva desdeTokens(std::vector<std::string> &tokens, int offset = 0);

    std::string toString() const;
};


std::vector<std::string> partirTrama(const std::string &trama, char sep = '|');

#endif /* DOMINIO_H_ */
