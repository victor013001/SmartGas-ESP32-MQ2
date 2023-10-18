#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

#include "config.h"

Servo servoMotor;
int currentServoPosition = 0;
int timeAfterWarning = 0;
WiFiClient espClient;
PubSubClient client(espClient);

void setupSerial();
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
void connectWiFi();
void reconnectMQTTClient();
void createMQTTClient();
void clientCallback(char *topic, byte *payload, unsigned int length);

void setup()
{
  setupSerial();
  setupMQ2();
  setupRRGLed();
  setupServo();
  delay(1000);
  connectWiFi();
  createMQTTClient();
  Serial.println("Setup completed");
}

void loop()
{
  // Is this for persistent connection?
  reconnectMQTTClient();
  client.loop();
  delay(1000);

  int gasValue = readMQ2Value();
  if (gasValue < GOOD_GAS_VALUE)
  {
    Serial.println("Good gas value");
    greenLedOn();
    doServoDeactivation();
    resetTimeAfterWarning();
  }
  else if (gasValue < WARNING_GAS_VALUE)
  {
    Serial.println("Warning gas value");
    orangeLedOn();
  }
  else
  {
    Serial.println("Dangerous gas value");
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

void setupSerial()
{
  Serial.begin(9600);
  while (!Serial)
    ;
}

void setupMQ2()
{
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
  delay(1000);
}

void connectWiFi()
{
  Serial.print("Connecting to ");
  Serial.print(SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    WiFi.begin(SSID, PASSWORD);
    delay(500);
  }
  Serial.println();
  Serial.print(ID.c_str());
  Serial.println(" connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTTClient()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(CLIENT_NAME.c_str()))
    {
      Serial.print("connected to Broker: ");
      Serial.println(BROKER.c_str());
      // Topic(s) subscription
      client.subscribe(TOPIC.c_str());
    }
    else
    {
      Serial.print("Retying in 5 seconds - failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void createMQTTClient()
{
  client.setServer(BROKER.c_str(), 1883);
  client.setCallback(clientCallback);
  reconnectMQTTClient();
}

void clientCallback(char *topic, byte *payload, unsigned int length)
{
  String response;

  for (int i = 0; i < length; i++)
  {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(TOPIC.c_str());
  Serial.print("] ");
  Serial.println(response);
  // Handle message arrived
}