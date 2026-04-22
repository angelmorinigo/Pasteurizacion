# Control de Pasteurización Automatizado con Arduino Leonardo

![Arduino](https://img.shields.io/badge/Arduino-UNO%2FNano-blue) ![Sensor](https://img.shields.io/badge/Sensor-DS18B20-green) ![RTC](https://img.shields.io/badge/RTC-DS3231-blue) ![LCD](https://img.shields.io/badge/Display-LCD%2016×2-orange)

Firmware para Arduino que automatiza un proceso industrial de 3 fases (calentamiento, enfriamiento y fase final) controlando relés según temperatura y tiempo. Muestra el estado en pantalla LCD y por puerto serial. Los tiempos y umbrales de temperatura son configurables al inicio del archivo.

---

## Hardware requerido

| Componente | Descripción | Pin(es) |
|---|---|---|
| DS18B20 | Sensor de temperatura 1-Wire | D7 |
| DS3231 | Módulo RTC I2C | SDA / SCL |
| LCD 16×2 | Pantalla paralelo 4 bits (sin I2C) | D8 – D13 |
| Relé etapa 1 | Calentamiento | D4 |
| Relé etapa 2 | Enfriamiento | D5 |
| Relé etapa 3 | Fase final | D6 |

---

## Librerías necesarias

Instalar desde el Gestor de Librerías del IDE de Arduino (`Ctrl+Shift+I`), última versión disponible:

- `OneWire`
- `DallasTemperature`
- `RTClib`
- `LiquidCrystal` (incluida en el IDE)

---

## Parámetros configurables

Al inicio del archivo se encuentran todas las variables que se pueden ajustar sin modificar la lógica del programa:

```cpp
// Tiempos de cada etapa (en minutos)
unsigned long TIEMPO_CALENTAMIENTO_MIN = 30;
unsigned long TIEMPO_ENFRIAMIENTO_MIN  = 30;
unsigned long TIEMPO_FINAL_MIN         = 1;

// Umbrales de temperatura
float TEMP_MINIMA = 28.0;  // °C
float TEMP_MAXIMA = 29.5;  // °C

// Polaridad del relé
// true  → se activa con LOW  (0 V)
// false → se activa con HIGH (+5 V)
bool RELE_ACTIVO_EN_LOW = false;
```

---

## Etapas del proceso

### Etapa 0 — Calentamiento inicial
El relé en D4 se activa desde el arranque hasta que la temperatura supera `TEMP_MAXIMA`. No hay límite de tiempo en esta etapa.

### Etapa 1 — Calentando (control termostático)
Una vez alcanzada la temperatura máxima, el sistema mantiene la temperatura entre `TEMP_MINIMA` y `TEMP_MAXIMA` durante `TIEMPO_CALENTAMIENTO_MIN` minutos. El relé D4 se conmuta automáticamente según la temperatura medida.

### Etapa 2 — Enfriamiento
El relé D5 permanece activo durante `TIEMPO_ENFRIAMIENTO_MIN` minutos. Los relés D4 y D6 se mantienen apagados.

### Etapa 3 — Fase final
El relé D6 permanece activo durante `TIEMPO_FINAL_MIN` minutos. Al finalizar, todos los relés se apagan y el sistema queda en estado `TERMINADO`.

---

## Primera puesta en marcha — ajuste del RTC

Para sincronizar el módulo DS3231 con la hora del sistema al momento de compilar, descomentar la siguiente línea en `setup()`, cargar el sketch y luego volver a comentarla antes de la siguiente carga:

```cpp
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

> **Importante:** si esta línea queda activa, el reloj se reiniciará al momento de compilación cada vez que se encienda el Arduino.

---

## Monitor serial

Velocidad: **9600 baud**. Cada 500 ms se imprime un bloque con el estado actual:

```
Hora: 14:32:07
Temp: 28.9 C
Estado: Calentando
Restan: 28 min
------------------------
```

Los campos `Restan` solo aparecen durante las etapas 1, 2 y 3 (no en calentamiento inicial ni en terminado).

---

## LCD — alternancia de pantalla

Durante las etapas activas, la pantalla alterna cada 2 segundos entre dos vistas:

- **Vista A:** tiempo restante de la fase actual.
- **Vista B:** temperatura actual y estado del relé correspondiente.

En estado `TERMINADO` muestra de forma fija:
```
Proceso
TERMINADO
```

---

## Notas

- El nombre de los estados (`Calentando`, `Enfriando`, etc.) se usa también en la salida serial. No modificar las cadenas dentro de la función `nombreEstado()` si se integra con otro sistema que las parsea.
- La detección del estado del relé en el LCD se hace leyendo directamente el pin digital, por lo que refleja el estado real del hardware independientemente de cómo se llame la función de activación.
- Para instalar las librerias se debe pulsar Ctrl+Shift+i para  y buscar las librerias correspondientes.

## 🌐 Contacto

- LinkedIn: [https://www.linkedin.com/in/angel-morinigo/](https://www.linkedin.com/in/angel-morinigo/)
