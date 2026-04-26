#include <LedControl.h>
#include <Adafruit_NeoPixel.h>

// --- CONFIGURAZIONE PIN ---
// MAX7219 (VSPI)
const int PIN_DIN = 23; 
const int PIN_CLK = 18;
const int PIN_LOAD = 5;

// Neopixel
const int PIN_LED = 4;
const int NUM_LEDS = 8;

// Touch e Bottoni
const int TOUCH_START = 14;
const int TOUCH_RESET = 27;
const int PIN_BATT = 34; // Analogico per partitore

// --- ISTANZE ---
LedControl lc = LedControl(PIN_DIN, PIN_CLK, PIN_LOAD, 1); // 1 chip MAX7219
Adafruit_NeoPixel strip(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// --- VARIABILI DI STATO ---
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
bool running = false;

void setup() {
  Serial.begin(115200);
  
  // Inizializzazione MAX7219
  lc.shutdown(0, false);       // Esci dal risparmio energetico
  lc.setIntensity(0, 8);       // Luminosità media (0-15)
  lc.clearDisplay(0);          // Pulisci display
  
  // Inizializzazione Neopixel
  strip.begin();
  strip.show();                // Spegni tutto all'avvio
  
  // Configurazione Input
  pinMode(TOUCH_START, INPUT);
  pinMode(TOUCH_RESET, INPUT);
}

void loop() {
  checkInputs();
  
  if (running) {
    elapsedTime = millis() - startTime;
    updateLedsProgress();
  }
  
  displayTime(elapsedTime);
  delay(10); // Piccolo debouncing software
}

// --- LOGICA INPUT ---
void checkInputs() {
  // Start / Stop
  if (digitalRead(TOUCH_START) == HIGH) {
    delay(200); // Semplice debouncing
    if (!running) {
      startTime = millis() - elapsedTime;
      running = true;
    } else {
      running = false;
    }
    while(digitalRead(TOUCH_START) == HIGH); // Attende rilascio
  }

  // Reset
  if (digitalRead(TOUCH_RESET) == HIGH) {
    running = false;
    elapsedTime = 0;
    strip.clear();
    strip.show();
    while(digitalRead(TOUCH_RESET) == HIGH);
  }
}

// --- GESTIONE DISPLAY ---
void displayTime(unsigned long t) {
  // Calcola minuti, secondi, centesimi
  int minutes = (t / 60000) % 100;
  int seconds = (t / 1000) % 60;
  int centi = (t / 10) % 100;

  // Visualizzazione sui 7 segmenti (mappatura pin DIG0-DIG6)
  // Esempio: MM.SS.CC
  lc.setDigit(0, 6, minutes / 10, false);
  lc.setDigit(0, 5, minutes % 10, true); // Punto decimale
  lc.setDigit(0, 4, seconds / 10, false);
  lc.setDigit(0, 3, seconds % 10, true);
  lc.setDigit(0, 2, centi / 10, false);
  lc.setDigit(0, 1, centi % 10, false);
}

// --- GESTIONE LED ---
void updateLedsProgress() {
  // Esempio: un LED si accende ogni secondo che passa (fino a 8)
  int ledsToLight = (elapsedTime / 1000) % (NUM_LEDS + 1);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < ledsToLight) {
      strip.setPixelColor(i, strip.Color(0, 255, 0)); // Verde
    } else {
      strip.setPixelColor(i, 0); // Spento
    }
  }
  strip.show();
}

// --- LETTURA BATTERIA (OPZIONALE) ---
float readBattery() {
  int raw = analogRead(PIN_BATT);
  // 4095 (12 bit) su 3.3V. Partitore 100k/100k (divide per 2)
  float voltage = (raw / 4095.0) * 3.3 * 2.0;
  return voltage;
}