#include <Arduino.h>
#include <pitches.h>

#define PIN_MQ2_ANALOG 36 // GPIO36
#define PIN_BUZZER 5 // GPIO5
#define PIN_LED_RED    23 // GPIO23
#define PIN_LED_GREEN  22 // GPIO22
#define PIN_LED_BLUE   21 // GPIO21

#define QUARTER_NOTE 1000
#define GOOD_GAS_VALUE 500
#define WARNING_GAS_VALUE 700

const float melody[] = {
  NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4,
  NOTE_D4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_G4,
  NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4,
  NOTE_E4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_C4
};

const int melodyDurations[] = {
  QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
  QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
  QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
  QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE
};

void setupMQ2();
void setupRRGLed();
int readMQ2Value();
void greenLedOn();
void yellowLedOn();
void redLedOn();
void doBuzzerWarning();

void setup() {
  setupMQ2();
  setupRRGLed();
}

void loop() {
  int gasValue = readMQ2Value();
  if (gasValue < GOOD_GAS_VALUE) {
    greenLedOn();
  } else if (gasValue < WARNING_GAS_VALUE) {
    yellowLedOn();
  } else {
    redLedOn();
    doBuzzerWarning();
  }
  Serial.print("MQ2 sensor value: ");
  Serial.println(gasValue);
}

void setupMQ2() {
  Serial.begin(9600);
  Serial.println("Warming up the MQ2 sensor");
  delay(20000);
}

void setupRRGLed() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
}

int readMQ2Value() {
  return analogRead(PIN_MQ2_ANALOG);
}

void greenLedOn() {
  digitalWrite(PIN_LED_GREEN, HIGH);
}

void yellowLedOn() {
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);
}

void redLedOn() {
  digitalWrite(PIN_LED_RED, HIGH);
}

void doBuzzerWarning() {
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
      tone(PIN_BUZZER, melody[i], melodyDurations[i]);
      delay(melodyDurations[i] * 1.3);
    }
}