#ifndef MODELOS_H_
#define MODELOS_H_

/*
 * Modelos.h
 * ---------
 * Clases de dominio del cliente C++.
 * Traduccion de los structs de C del admin a clases con OO:
 *   - Constructores desde trama de protocolo
 *   - Getters/setters
 *   - toString() para mostrar por pantalla
 *
 * JERARQUIA de Transporte (polimorfismo):
 *   Transporte  (base)
 *     Avion
 *     Tren
 *     Autobus
 */

#include <string>
#include <sstream>
#include <iomanip>

/* ═══════════════════════════════════════════════════════════
 * Cliente
 * ═══════════════════════════════════════════════════════════ */
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

    /* Constructor desde campos sueltos */
    Cliente(int id, const std::string& dni, const std::string& nombre,
            const std::string& apellidos, const std::string& telefono,
            const std::string& email, const std::string& fechaNac)
        : id(id), dni(dni), nombre(nombre), apellidos(apellidos),
          telefono(telefono), email(email), fechaNacimiento(fechaNac) {}

    std::string getNombreCompleto() const {
        return nombre + " " + apellidos;
    }

    /* Serializa para enviarlo en trama: ACL|dni|nombre|apellidos|tlf|email|fnac| */
    std::string toTrama() const {
        return dni + "|" + nombre + "|" + apellidos + "|" +
               telefono + "|" + email + "|" + fechaNacimiento + "|";
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "DNI: "      << dni
            << " | Nombre: " << getNombreCompleto()
            << " | Tel: "    << telefono
            << " | Email: "  << email
            << " | F.Nac: "  << fechaNacimiento;
        return oss.str();
    }
};

/* ═══════════════════════════════════════════════════════════
 * Paquete
 * ═══════════════════════════════════════════════════════════ */
class Paquete {
public:
    int         codigo;
    std::string nombre;
    float       precio;
    std::string origen;
    std::string destino;
    int         plazasTotales;
    int         plazasDisponibles;

    Paquete() : codigo(0), precio(0.0f), plazasTotales(0), plazasDisponibles(0) {}

    Paquete(int cod, const std::string& nom, float precio,
            const std::string& origen, const std::string& destino,
            int totales, int disponibles)
        : codigo(cod), nombre(nom), precio(precio),
          origen(origen), destino(destino),
          plazasTotales(totales), plazasDisponibles(disponibles) {}

    bool tieneDisponibilidad() const { return plazasDisponibles > 0; }

    float porcentajeOcupacion() const {
        if (plazasTotales == 0) return 0.0f;
        return 100.0f * (plazasTotales - plazasDisponibles) / plazasTotales;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[" << codigo << "] " << nombre
            << " | " << origen << " -> " << destino
            << " | " << std::fixed << std::setprecision(2) << precio << " EUR"
            << " | Plazas libres: " << plazasDisponibles << "/" << plazasTotales;
        return oss.str();
    }
};

/* ═══════════════════════════════════════════════════════════
 * Alojamiento
 * ═══════════════════════════════════════════════════════════ */
class Alojamiento {
public:
    std::string codigo;
    std::string nombre;
    std::string direccion;
    std::string tipo;
    std::string codCiudad;

    Alojamiento() {}

    Alojamiento(const std::string& cod, const std::string& nom,
                const std::string& dir, const std::string& tipo,
                const std::string& ciudad)
        : codigo(cod), nombre(nom), direccion(dir), tipo(tipo), codCiudad(ciudad) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << "[" << codigo << "] " << nombre
            << " (" << tipo << ")"
            << " | " << direccion
            << " | Ciudad: " << codCiudad;
        return oss.str();
    }
};

/* ═══════════════════════════════════════════════════════════
 * Transporte (clase base) + subclases con polimorfismo
 * Requisito de la rubrica: herencia y polimorfismo
 * ═══════════════════════════════════════════════════════════ */
class Transporte {
public:
    std::string codigo;
    std::string tipo;
    std::string fechaSalida;
    std::string fechaLlegada;
    int         idPaquete;

    Transporte() : idPaquete(0) {}

    Transporte(const std::string& cod, const std::string& tipo,
               const std::string& salida, const std::string& llegada,
               int idPaq)
        : codigo(cod), tipo(tipo), fechaSalida(salida),
          fechaLlegada(llegada), idPaquete(idPaq) {}

    virtual ~Transporte() {}

    /* Metodo virtual — cada subclase muestra su icono/descripcion propia */
    virtual std::string getIcono() const { return "[?]"; }

    virtual std::string toString() const {
        std::ostringstream oss;
        oss << getIcono()
            << " [" << codigo << "] " << tipo
            << " | Salida: "  << fechaSalida
            << " | Llegada: " << fechaLlegada
            << " | Paquete: " << idPaquete;
        return oss.str();
    }

    /* Fabrica estatica: crea el subtipo correcto segun el campo tipo */
    static Transporte* crear(const std::string& cod, const std::string& tipo,
                             const std::string& salida, const std::string& llegada,
                             int idPaq);
};

class Avion : public Transporte {
public:
    Avion(const std::string& cod, const std::string& tipo,
          const std::string& salida, const std::string& llegada, int idPaq)
        : Transporte(cod, tipo, salida, llegada, idPaq) {}

    std::string getIcono() const override { return "[AVN]"; }
};

class Tren : public Transporte {
public:
    Tren(const std::string& cod, const std::string& tipo,
         const std::string& salida, const std::string& llegada, int idPaq)
        : Transporte(cod, tipo, salida, llegada, idPaq) {}

    std::string getIcono() const override { return "[TRN]"; }
};

class Autobus : public Transporte {
public:
    Autobus(const std::string& cod, const std::string& tipo,
            const std::string& salida, const std::string& llegada, int idPaq)
        : Transporte(cod, tipo, salida, llegada, idPaq) {}

    std::string getIcono() const override { return "[BUS]"; }
};

/* Implementacion de la fabrica (inline aqui para no necesitar .cpp separado) */
inline Transporte* Transporte::crear(const std::string& cod, const std::string& tipo,
                                     const std::string& salida, const std::string& llegada,
                                     int idPaq) {
    std::string t = tipo;
    /* Normalizar a minusculas para comparar */
    for (char& c : t) c = (char)tolower((unsigned char)c);

    if (t.find("avion") != std::string::npos || t.find("vuelo") != std::string::npos)
        return new Avion(cod, tipo, salida, llegada, idPaq);
    if (t.find("tren") != std::string::npos)
        return new Tren(cod, tipo, salida, llegada, idPaq);
    /* Por defecto: autobus */
    return new Autobus(cod, tipo, salida, llegada, idPaq);
}

/* ═══════════════════════════════════════════════════════════
 * Reserva
 * ═══════════════════════════════════════════════════════════ */
class Reserva {
public:
    int         id;
    int         codPaquete;
    std::string nombrePaquete;
    std::string dniCliente;
    std::string fecha;

    Reserva() : id(0), codPaquete(0) {}

    Reserva(int id, int codPaq, const std::string& nomPaq,
            const std::string& dni, const std::string& fecha)
        : id(id), codPaquete(codPaq), nombrePaquete(nomPaq),
          dniCliente(dni), fecha(fecha) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << "Reserva #" << id
            << " | Paquete: [" << codPaquete << "] " << nombrePaquete
            << " | Fecha: " << fecha;
        return oss.str();
    }
};

#endif /* MODELOS_H_ */
