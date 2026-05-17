-- importar_paquetes.sql
-- Ejecutar con: sqlite3 agencia.db < importar_paquetes.sql

INSERT OR IGNORE INTO paquetes (codigo, nombre, precio, origen, destino, plazas_totales, plazas_disponibles, activo) VALUES
(1,  'Escapada a Paris',          899.99,  'Madrid',    'Paris',      30, 30, 1),
(2,  'Roma Clasica',             1050.00,  'Barcelona', 'Roma',       25, 25, 1),
(3,  'Cancun Todo Incluido',     1799.50,  'Madrid',    'Cancun',     40, 40, 1),
(4,  'Tokio Express',            2350.00,  'Madrid',    'Tokio',      20, 20, 1),
(5,  'Nueva York City Break',    1599.00,  'Barcelona', 'Nueva York', 35, 35, 1),
(6,  'Lisboa en Fin de Semana',   299.00,  'Madrid',    'Lisboa',     50, 50, 1),
(7,  'Safari en Kenia',          3200.00,  'Madrid',    'Nairobi',    15, 15, 1),
(8,  'Islas Griegas',            1250.00,  'Barcelona', 'Atenas',     30, 30, 1),
(9,  'Amsterdam y Tulipanes',     750.00,  'Madrid',    'Amsterdam',  28, 28, 1),
(10, 'Ruta por Marruecos',        680.00,  'Sevilla',   'Marrakech',  22, 22, 1);

-- Verificar que se insertaron correctamente
SELECT codigo, nombre, precio, origen, destino, plazas_disponibles
FROM paquetes
WHERE activo = 1
ORDER BY codigo;