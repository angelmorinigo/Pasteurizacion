#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
// las librerias descargar ultima version para abrir el gestor ctrl+shif+i
#define ONE_WIRE_BUS 7

// ------------------------
// Pines
// -----------------------------ETAPAS-------------
const int RELAY_PIN_4 = 4;   // Calentamiento
const int RELAY_PIN_5 = 5;   // Enfriamiento
const int RELAY_PIN_6 = 6;   // Final

// ------------------------
// Los tiempos son modificables, ideal 30 minutos si se quiere, está expresado en minutos
// ------------------------
unsigned long TIEMPO_CALENTAMIENTO_MIN = 1;
unsigned long TIEMPO_ENFRIAMIENTO_MIN  = 1;
unsigned long TIEMPO_FINAL_MIN         = 1;

float TEMP_MINIMA = 28.0;  // °C — ajustar según proceso
float TEMP_MAXIMA = 29.5;  // °C — ajustar según proceso

bool RELE_ACTIVO_EN_LOW = false; // Si el relé se activa con negativo 0v, colocar  true, si se activa con +5v colocar false

// ------------------------
// Objetos
// ------------------------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RTC_DS3231 rtc;
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // pines del lcd no hace falta modificar 

// ------------------------
// Estados
// ------------------------
enum EstadoProceso {
  CALENTAMIENTO_INICIAL,  // Sube hasta TEMP_MAXIMA por primera vez
  CALENTANDO,             // Mantiene entre min y max, contando 30 o x min
  ENFRIANDO,
  FASE_FINAL,
  TERMINADO
};

EstadoProceso estado = CALENTAMIENTO_INICIAL;

// ------------------------
// Tiempos
// ------------------------
unsigned long inicioFase = 0;

// ------------------------
// LCD alternado
// ------------------------
unsigned long previousMillisPantalla = 0;
const long intervalPantalla = 2000;
bool pantalla = false;

// ------------------------
// Funciones relay
// ------------------------
void activarRelay(int pin) {
  digitalWrite(pin, RELE_ACTIVO_EN_LOW ? LOW : HIGH);
}

void desactivarRelay(int pin) {
  digitalWrite(pin, RELE_ACTIVO_EN_LOW ? HIGH : LOW);
}

void apagarTodosLosReles() {
  desactivarRelay(RELAY_PIN_4);
  desactivarRelay(RELAY_PIN_5);
  desactivarRelay(RELAY_PIN_6);
}

// ------------------------
// Utilidades de tiempo
// ------------------------
unsigned long minutosAMillis(unsigned long minutos) {
  return minutos * 60UL * 1000UL;
}

unsigned long minutosRestantes(unsigned long inicio, unsigned long duracionMin, unsigned long ahora) {
  unsigned long duracionMs = minutosAMillis(duracionMin);
  if (ahora >= inicio + duracionMs) return 0;
  unsigned long restanteMs = (inicio + duracionMs) - ahora;
  return (restanteMs + 59999UL) / 60000UL;
}
// no modificar  los que no estan entre comillas porque  se usan en otro lugar 
const char* nombreEstado(EstadoProceso e) {
  switch (e) {
    case CALENTAMIENTO_INICIAL: return "Calent. inicial";
    case CALENTANDO:            return "Calentando";
    case ENFRIANDO:             return "Enfriando";
    case FASE_FINAL:            return "Fase final";
    case TERMINADO:             return "Terminado";
    default:                    return "Desconocido";
  }
}

// ------------------------
// Setup
// ------------------------
void setup() {
  Serial.begin(9600);
  delay(2000);

  sensors.begin();
  Wire.begin();
  lcd.begin(16, 2);

  pinMode(RELAY_PIN_4, OUTPUT);
  pinMode(RELAY_PIN_5, OUTPUT);
  pinMode(RELAY_PIN_6, OUTPUT);

  apagarTodosLosReles();

  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC ERROR");
    while (1);
  }

  // Descomentar UNA VEZ para ajustar hora, luego volver a comentar:
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Arranca calentamiento inicial: pin 4 encendido
  activarRelay(RELAY_PIN_4);
  estado = CALENTAMIENTO_INICIAL;

  lcd.setCursor(0, 0);
  lcd.print("Sistema OK      ");
  delay(1500);
  lcd.clear();
}

// ------------------------
// Loop
// ------------------------
void loop() {
  unsigned long ahoraMillis = millis();

  // Alternar pantalla cada 2 s
  if (ahoraMillis - previousMillisPantalla >= intervalPantalla) {
    previousMillisPantalla = ahoraMillis;
    pantalla = !pantalla;
  }

  // Leer sensores
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  DateTime now = rtc.now();

  unsigned long restante = 0;  // minutos restantes de la fase activa

  // -----------------------------------------------
  // Máquina de estados
  // -----------------------------------------------
  switch (estado) {

    // --- ETAPA 0: subir temperatura hasta TEMP_MAXIMA ---
    case CALENTAMIENTO_INICIAL:
      activarRelay(RELAY_PIN_4);
      desactivarRelay(RELAY_PIN_5);
      desactivarRelay(RELAY_PIN_6);

      if (temperature >= TEMP_MAXIMA) {
        // Alcanzó el máximo → apagar pin 4 e iniciar conteo de 30 o x min
        desactivarRelay(RELAY_PIN_4);
        estado = CALENTANDO;
        inicioFase = ahoraMillis;
        Serial.println("Temperatura maxima alcanzada. Inicia conteo calentamiento.");
      }
      break;

    // --- ETAPA 1: mantener temperatura y contar tiempo ---
    case CALENTANDO:
      // Control ON/OFF del pin 4 según temperatura
      if (temperature >= TEMP_MAXIMA) {
        desactivarRelay(RELAY_PIN_4);
      } else if (temperature <= TEMP_MINIMA) {
        activarRelay(RELAY_PIN_4);
      }
      // Pin 5 y 6 siempre apagados
      desactivarRelay(RELAY_PIN_5);
      desactivarRelay(RELAY_PIN_6);

      restante = minutosRestantes(inicioFase, TIEMPO_CALENTAMIENTO_MIN, ahoraMillis);

      if (ahoraMillis - inicioFase >= minutosAMillis(TIEMPO_CALENTAMIENTO_MIN)) {
        desactivarRelay(RELAY_PIN_4);
        activarRelay(RELAY_PIN_5);
        estado = ENFRIANDO;
        inicioFase = ahoraMillis;
        Serial.println("Inicia enfriamiento.");
      }
      break;

    // --- ETAPA 2: enfriamiento ---
    case ENFRIANDO:
      desactivarRelay(RELAY_PIN_4);
      activarRelay(RELAY_PIN_5);
      desactivarRelay(RELAY_PIN_6);

      restante = minutosRestantes(inicioFase, TIEMPO_ENFRIAMIENTO_MIN, ahoraMillis);

      if (ahoraMillis - inicioFase >= minutosAMillis(TIEMPO_ENFRIAMIENTO_MIN)) {
        desactivarRelay(RELAY_PIN_5);
        activarRelay(RELAY_PIN_6);
        estado = FASE_FINAL;
        inicioFase = ahoraMillis;
        Serial.println("Inicia fase final.");
      }
      break;

    // --- ETAPA 3: fase final ---
    case FASE_FINAL:
      desactivarRelay(RELAY_PIN_4);
      desactivarRelay(RELAY_PIN_5);
      activarRelay(RELAY_PIN_6);

      restante = minutosRestantes(inicioFase, TIEMPO_FINAL_MIN, ahoraMillis);

      if (ahoraMillis - inicioFase >= minutosAMillis(TIEMPO_FINAL_MIN)) {
        desactivarRelay(RELAY_PIN_6);
        estado = TERMINADO;
        Serial.println("Proceso terminado.");
      }
      break;

    case TERMINADO:
      apagarTodosLosReles();
      break;
  }

  // -----------------------------------------------
  // SERIAL
  // -----------------------------------------------
  Serial.print("Hora: ");
  if (now.hour()   < 10) Serial.print("0"); Serial.print(now.hour());   Serial.print(":");
  if (now.minute() < 10) Serial.print("0"); Serial.print(now.minute()); Serial.print(":");
  if (now.second() < 10) Serial.print("0"); Serial.println(now.second());

  Serial.print("Temp: ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Estado: ");
  Serial.println(nombreEstado(estado));

  if (estado == CALENTANDO || estado == ENFRIANDO || estado == FASE_FINAL) {
    Serial.print("Restan: ");
    Serial.print(restante);
    Serial.println(" min");
  }

   Serial.println("------------------------");

  // -----------------------------------------------
  // LCD
  // -----------------------------------------------
  lcd.clear();  // evita artefactos visuales entre modos

  switch (estado) {

    case CALENTAMIENTO_INICIAL:
      lcd.setCursor(0, 0);
      lcd.print("Calentando...   ");
      lcd.setCursor(0, 1);
      lcd.print("T:");
      lcd.print(temperature, 1);
      lcd.print((char)223);
      lcd.print("C Max:");
      lcd.print((int)TEMP_MAXIMA);
      break;

    case CALENTANDO:
      if (!pantalla) {
        lcd.setCursor(0, 0);
        lcd.print("Etapa 1         ");
        lcd.setCursor(0, 1);
        lcd.print("Restan:");
        lcd.print(restante);
        lcd.print(" min    ");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(temperature, 1);
        lcd.print((char)223);
        lcd.print("C       ");
        lcd.setCursor(0, 1);
        // Indica si el relay está encendido o apagado según temperatura
        bool pin4on = (digitalRead(RELAY_PIN_4) == (RELE_ACTIVO_EN_LOW ? LOW : HIGH));// no modificar
        lcd.print("P4:");
        lcd.print(pin4on ? "ON " : "OFF");
        lcd.print(" Min:");
        lcd.print((int)TEMP_MINIMA);
        lcd.print((char)223);
      }
      break;

    case ENFRIANDO:
      if (!pantalla) {
        lcd.setCursor(0, 0);
        lcd.print("Etapa 2 Enfria  ");
        lcd.setCursor(0, 1);
        lcd.print("Restan:");
        lcd.print(restante);
        lcd.print(" min    ");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(temperature, 1);
        lcd.print((char)223);
        lcd.print("C       ");
        lcd.setCursor(0, 1);
        lcd.print("P5:ON  P4:OFF   ");
      }
      break;

    case FASE_FINAL:
      lcd.setCursor(0, 0);
      lcd.print("Etapa 3 Final   ");
      lcd.setCursor(0, 1);
      lcd.print("Restan:");
      lcd.print(restante);
      lcd.print(" min    ");
      break;

    case TERMINADO:
      lcd.setCursor(0, 0);
      lcd.print("Proceso         ");
      lcd.setCursor(0, 1);
      lcd.print("TERMINADO       ");
      break;
  }

  delay(500);
}