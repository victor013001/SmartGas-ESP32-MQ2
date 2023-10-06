#include <Arduino.h>
#include <pitches.h>
#include <ESP32Servo.h>

#define PIN_MQ2_ANALOG 36 // GPIO36
#define PIN_BUZZER 5      // GPIO5
#define PIN_LED_RED 23    // GPIO23
#define PIN_LED_GREEN 22  // GPIO22
#define PIN_LED_BLUE 21   // GPIO21
#define SERVO_PIN 26      // GPIO26

#define QUARTER_NOTE 1000
#define GOOD_GAS_VALUE 500
#define WARNING_GAS_VALUE 700
#define TIME_TO_ACTIVATE_ALERTS 5

const float melody[] = {
    NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4,
    NOTE_D4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_G4,
    NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4,
    NOTE_E4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_C4};

const int melodyDurations[] = {
    QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
    QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
    QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE,
    QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE};

Servo servoMotor;
int currentServoPosition = 0;
int timeAfterWarning = 0;

void setupMQ2();
void setupRRGLed();
void setupServo();
int readMQ2Value();
void greenLedOn();
void yellowLedOn();
void redLedOn();
void doBuzzerWarning();
void doServoActivation();
void doServoDeactivation();
void resetTimeAfterWarning();
void increaseTimeAfterWarning();
void getGasValue();

void setup()
{
  setupMQ2();
  setupRRGLed();
  setupServo();
}

void loop()
{
  int gasValue = readMQ2Value();
  if (gasValue < GOOD_GAS_VALUE)
  {
    greenLedOn();
    doServoDeactivation();
    resetTimeAfterWarning();
  }
  else if (gasValue < WARNING_GAS_VALUE)
  {
    yellowLedOn();
  }
  else
  {
    redLedOn();
    increaseTimeAfterWarning();
    if (timeAfterWarning > TIME_TO_ACTIVATE_ALERTS)
    {
      doBuzzerWarning();
      doServoActivation();
    }
  }
  getGasValue();
}

void setupMQ2()
{
  Serial.begin(9600);
  Serial.println("Warming up the MQ2 sensor");
  delay(20000);
}

void setupRRGLed()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
}

void setupServo()
{
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);
}

int readMQ2Value()
{
  return analogRead(PIN_MQ2_ANALOG);
}

void greenLedOn()
{
  digitalWrite(PIN_LED_GREEN, HIGH);
}

void yellowLedOn()
{
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);
}

void redLedOn()
{
  digitalWrite(PIN_LED_RED, HIGH);
}

void doBuzzerWarning()
{
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    tone(PIN_BUZZER, melody[i], melodyDurations[i]);
    delay(melodyDurations[i] * 1.3);
  }
  noTone(PIN_BUZZER);
}

void doServoActivation()
{
  if (currentServoPosition == 0)
  {
    for (int pos = 0; pos <= 180; pos += 1)
    {
      servoMotor.write(pos);
      delay(15);
    }
    currentServoPosition = 180;
  }
}

void doServoDeactivation()
{
  if (currentServoPosition == 180)
  {
    for (int pos = 180; pos >= 0; pos -= 1)
    {
      servoMotor.write(pos);
      delay(15);
    }
    currentServoPosition = 0;
  }
}

void resetTimeAfterWarning()
{
  if (timeAfterWarning > 0)
  {
    timeAfterWarning--;
  }
}

void increaseTimeAfterWarning()
{
  timeAfterWarning++;
}

void getGasValue()
{
  Serial.print("Gas value: ");
  Serial.println(readMQ2Value());
}