#include "config.h"

void setupSerial();
void setupRGBLed();
void blueLedOn();
void connectWiFi();
void createMQTTClient();
void clientBuzzerCallback(char *topic, byte *payload, unsigned int length);
void publishBuzzerStatus(const char *status);
void stopBuzzerWarning();
void reconnectMQTTClient();
void setupMQ2();
void setupBuzzer();
int readMQ2Value();
void persistMQTTConnection();
void showGasValue(int gasValue);
void publishGasValue(int gasValue);
bool isMQ2ValueGood(int gasValue);
void goodMQ2ValueActions();
void greenLedOn();
void reduceTimeAfterWarning();
void doBuzzerDeactivation();
void publishServoStatus(const char *status);
bool isMQ2ValueWarning(int gasValue);
void warningMQ2ValueActions();
void orangeLedOn();
void emergencyMQ2ValueActions();
void redLedOn();
void increaseTimeAfterWarning();
void doBuzzerActivation();
void doBuzzerWarning();
void activateBuzzerIfNeeded();

void setup()
{
  setupSerial();
  setupRGBLed();
  blueLedOn();
  connectWiFi();
  createMQTTClient();
  setupMQ2();
  setupBuzzer();
  Serial.println("SmartGas >> Setup completed");
}

void loop()
{
  persistMQTTConnection();
  int gasValue = readMQ2Value();
  showGasValue(gasValue);
  if (isMQ2ValueGood(gasValue))
  {
    goodMQ2ValueActions();
  }
  else if (isMQ2ValueWarning(gasValue))
  {
    warningMQ2ValueActions();
  }
  else
  {
    emergencyMQ2ValueActions();
  }
  activateBuzzerIfNeeded();
}

void setupSerial()
{
  Serial.begin(MONITOR_SPEED);
  while (!Serial)
    ;
  delay(SERIAL_DELAY);
  Serial.println();
  Serial.print("SmartGas >> Serial setup with speed: ");
  Serial.println(MONITOR_SPEED);
}

void setupRGBLed()
{
  Serial.print("SmartGas >> Setting up the RGB LED, pins: ");
  Serial.print("Red: ");
  Serial.print(PIN_LED_RED);
  Serial.print(", Green: ");
  Serial.print(PIN_LED_GREEN);
  Serial.print(", Blue: ");
  Serial.println(PIN_LED_BLUE);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
}

void blueLedOn()
{
  analogWrite(PIN_LED_RED, 0);
  analogWrite(PIN_LED_GREEN, 0);
  analogWrite(PIN_LED_BLUE, 255);
}

void connectWiFi()
{
  Serial.print("SmartGas >> Connecting to ");
  Serial.print(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print(ID.c_str());
  Serial.println(" connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void createMQTTClient()
{
  Serial.println("SmartGas >> Creating MQTT client");
  client.setServer(BROKER.c_str(), PORT);
  client.setCallback(clientBuzzerCallback);
  reconnectMQTTClient();
}

void clientBuzzerCallback(char *topic, byte *payload, unsigned int length)
{
  String response;
  for (int i = 0; i < length; i++)
  {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(BUZZER_TOPIC.c_str());
  Serial.print("]: ");
  if (response == BUZZER_OFF_MESSAGE && isBuzzerActive)
  {
    Serial.println(response);
    stopBuzzerWarning();
    publishBuzzerStatus(response.c_str());
    userDeactivatedBuzzer = true;
  }
  else
  {
    Serial.println("Invalid message received. Expected: OFF");
  }
}

void publishBuzzerStatus(const char *status)
{
  client.publish(BUZZER_STATUS_TOPIC.c_str(), status);
}

void stopBuzzerWarning()
{
  noTone(PIN_BUZZER);
  isBuzzerActive = false;
}

void reconnectMQTTClient()
{
  while (!client.connected())
  {
    Serial.println("SmartGas >> Attempting MQTT connection...");
    if (client.connect(CLIENT_NAME.c_str()))
    {
      Serial.print("Connected to Broker: ");
      Serial.println(BROKER.c_str());
      client.subscribe(BUZZER_TOPIC.c_str(), 1);
    }
    else
    {
      Serial.print("Retying in 5 seconds - failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void persistMQTTConnection()
{
  reconnectMQTTClient();
  client.loop();
  delay(PERSIST_MQTT_CONNECTION_DELAY);
}

void setupMQ2()
{
  Serial.print("SmartGas >> Warming up the MQ2 sensor on pin: ");
  Serial.print(PIN_MQ2_ANALOG);
  Serial.print(" for ");
  Serial.print(MQ2_WARM_UP_TIME);
  Serial.println(" ms");
  delay(MQ2_WARM_UP_TIME);
}

void setupBuzzer()
{
  Serial.print("SmartGas >> Setting up the buzzer on pin: ");
  Serial.print(PIN_BUZZER);
  Serial.println(" with a frequency of 5000 Hz and a resolution of 8 bits on channel 0");
  ledcSetup(0, 5000, 8);
  ledcAttachPin(PIN_BUZZER, 0);
}

int readMQ2Value()
{
  return analogRead(PIN_MQ2_ANALOG);
}

void showGasValue(int gasValue)
{
  Serial.print("Gas value: ");
  Serial.println(gasValue);
  publishGasValue(gasValue);
  delay(GAS_READ_VALUE_DELAY);
}

bool isMQ2ValueGood(int gasValue)
{
  return gasValue < GOOD_GAS_VALUE;
}

void goodMQ2ValueActions()
{
  greenLedOn();
  reduceTimeAfterWarning();
  if (timeAfterWarning == 0)
  {
    if (isBuzzerActive)
    {
      doBuzzerDeactivation();
      userDeactivatedBuzzer = false;
    }
    if (isServoActive)
    {
      publishServoStatus(SERVO_OFF_MESSAGE);
      isServoActive = false;
    }
  }
}

void publishGasValue(int gasValue)
{
  char gasValueStr[10];
  sprintf(gasValueStr, "%d", gasValue);
  client.publish(MQ2_TOPIC.c_str(), gasValueStr);
}

void greenLedOn()
{
  analogWrite(PIN_LED_RED, 0);
  analogWrite(PIN_LED_GREEN, 255);
  analogWrite(PIN_LED_BLUE, 0);
}

void reduceTimeAfterWarning()
{
  if (timeAfterWarning > 0)
  {
    timeAfterWarning--;
  }
}

void doBuzzerDeactivation()
{
  if (isBuzzerActive)
  {
    isBuzzerActive = false;
    userDeactivatedBuzzer = false;
    publishBuzzerStatus(BUZZER_OFF_MESSAGE);
  }
}

void publishServoStatus(const char *status)
{
  client.publish(SERVO_TOPIC.c_str(), status);
}

bool isMQ2ValueWarning(int gasValue)
{
  return gasValue >= GOOD_GAS_VALUE && gasValue < WARNING_GAS_VALUE;
}

void warningMQ2ValueActions()
{
  reduceTimeAfterWarning();
  orangeLedOn();
}

void orangeLedOn()
{
  analogWrite(PIN_LED_RED, 255);
  analogWrite(PIN_LED_GREEN, 165);
  analogWrite(PIN_LED_BLUE, 0);
}

void emergencyMQ2ValueActions()
{
  redLedOn();
  increaseTimeAfterWarning();
  if (timeAfterWarning > TIME_TO_ACTIVATE_ALERTS)
  {
    doBuzzerActivation();
    publishServoStatus(SERVO_ON_MESSAGE);
    isServoActive = true;
  }
}

void redLedOn()
{
  analogWrite(PIN_LED_RED, 255);
  analogWrite(PIN_LED_GREEN, 0);
  analogWrite(PIN_LED_BLUE, 0);
}

void increaseTimeAfterWarning()
{
  timeAfterWarning++;
}

void doBuzzerActivation()
{
  if (!isBuzzerActive && !userDeactivatedBuzzer)
  {
    publishBuzzerStatus(BUZZER_ON_MESSAGE);
    isBuzzerActive = true;
  }
}

void doBuzzerWarning()
{
  tone(PIN_BUZZER, melody[0], melodyDurations[0]);
}

void activateBuzzerIfNeeded()
{
  if (isBuzzerActive)
  {
    doBuzzerWarning();
  }
}