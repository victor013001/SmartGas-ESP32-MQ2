#include <Arduino.h>
#include <pitches.h>
#include <ESP32Servo.h>

#define PIN_MQ2_ANALOG 36 // GPIO36
#define PIN_BUZZER 5      // GPIO5
#define PIN_LED_RED 23    // GPIO23
#define PIN_LED_GREEN 22  // GPIO22
#define PIN_LED_BLUE 21   // GPIO21
#define PIN_SERVO 33      // GPIO33

#define QUARTER_NOTE 1000
#define GOOD_GAS_VALUE 500
#define WARNING_GAS_VALUE 700
#define TIME_TO_ACTIVATE_ALERTS 50

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
void orangeLedOn();
void redLedOn();
void doBuzzerWarning();
void doServoActivation();
void doServoDeactivation();
void resetTimeAfterWarning();
void increaseTimeAfterWarning();
void showGasValue(int gasValue);

void setup()
{
  setupMQ2();
  setupRRGLed();
  setupServo();
  Serial.println("Setup completed");
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
    orangeLedOn();
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
  showGasValue(gasValue);
}

void setupMQ2()
{
  Serial.begin(9600);
  Serial.println("Warming up the MQ2 sensor");
  delay(20000);
}

void setupRRGLed()
{
  Serial.println("Setting up the RGB LED");
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
}

void setupServo()
{
  Serial.println("Setting up the servo motor");
  servoMotor.attach(PIN_SERVO, 500, 2400);
  servoMotor.write(0);
}

int readMQ2Value()
{
  return analogRead(PIN_MQ2_ANALOG);
}

void greenLedOn()
{
  analogWrite(PIN_LED_RED, 0);
  analogWrite(PIN_LED_GREEN, 255);
  analogWrite(PIN_LED_BLUE, 0);
}

void orangeLedOn()
{
  analogWrite(PIN_LED_RED, 255);
  analogWrite(PIN_LED_GREEN, 165);
  analogWrite(PIN_LED_BLUE, 0);
}

void redLedOn()
{
  analogWrite(PIN_LED_RED, 255);
  analogWrite(PIN_LED_GREEN, 0);
  analogWrite(PIN_LED_BLUE, 0);
}

void doBuzzerWarning()
{
  tone(PIN_BUZZER, melody[0], melodyDurations[0]);
  delay(melodyDurations[0] * 1.3);
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
  if (currentServoPosition == 180 && timeAfterWarning == 0)
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

void showGasValue(int gasValue)
{
  Serial.print("Gas value: ");
  Serial.println(gasValue);
}