# Sistema de Pasteurización Automatizado con Arduino Leonardo

Este proyecto implementa un sistema automatizado de pasteurización utilizando un Arduino Leonardo, sensor de temperatura DS18B20, módulo RTC DS3231 y un display LCD 1602A.

El sistema controla tres etapas principales mediante relés:

 Calentamiento
 Enfriamiento / Pasteurización
 Fase Final
 Funcionamiento General

El proceso se ejecuta de forma completamente automática en tres etapas secuenciales, con tiempos configurables desde el código.

 Etapa 1: Calentamiento
Se activa el pin 4 (resistencia)
Se mantiene pin 5 apagado
Duración configurable (ej: 30 minutos)

 En pantalla:

Calentando
Restan: XX min

 Estado:

El sistema mide temperatura en tiempo real
No afecta la lógica (solo informativo en esta etapa)
❄️ Etapa 2: Enfriamiento / Pasteurización
Se apaga pin 4
Se activa pin 5
Duración configurable (ej: 30 minutos)
No depende de la temperatura

 En pantalla:

Enfriando
Restan: XX min

 Estado:

El sistema sigue mostrando la temperatura
El tiempo continúa aunque la temperatura cambie
 Etapa 3: Fase Final
Se apagan pin 4 y pin 5
Se activa pin 6
Duración configurable (ej: 1 minuto)

 En pantalla:

Fase final
Restan: X min
 Etapa 4: Finalización
Todos los pines se apagan
El proceso termina

 En pantalla:

Proceso
terminado
⏱️ Configuración de Tiempos

Los tiempos se configuran directamente en el código:

unsigned long TIEMPO_CALENTAMIENTO_MIN = 30;
unsigned long TIEMPO_ENFRIAMIENTO_MIN = 30;
unsigned long TIEMPO_FINAL_MIN = 1;

Ejemplo:

unsigned long TIEMPO_CALENTAMIENTO_MIN = 35;
🌡️ Sensor de Temperatura

Se utiliza un sensor DS18B20:

Lectura en tiempo real
Mostrado en LCD y Monitor Serial
No afecta la lógica de tiempos (modo automático por tiempo)
🖥️ Interfaz LCD

Display 1602A sin I2C, mostrando:

Estado del sistema
Tiempo restante por etapa
Temperatura actual
Estado de relés
🔌 Asignación de Pines
Función	Pin Arduino
DS18B20 DATA	7
Relay Calentador	4
Relay Enfriador	5
Relay Final	6
LCD RS	8
LCD E	9
LCD D4–D7	10–13
RTC SDA	SDA
RTC SCL	SCL
 Consideraciones Importantes
Compatible con relés activos en LOW o HIGH:
bool RELE_ACTIVO_EN_LOW = true;
Ajustar hora del RTC solo una vez:
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
El sistema funciona por tiempo, no por temperatura (modo industrial simple)

 Descripción Técnica
Arquitectura basada en máquina de estados
Control de tiempos mediante millis()
No bloqueante
Compatible con múltiples sensores