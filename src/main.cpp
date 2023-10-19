#include "config.h"

void setupSerial();
void setupRGBLed();
void blueLedOn();
void connectWiFi();
void createMQTTClient();
void clientBuzzerCallback(char *topic, byte *payload, unsigned int length);
void publishBuzzerStatus(const char *status);
void reconnectMQTTClient();
void setupMQ2();
void setupServo();
int readMQ2Value();
void persistMQTTConnection();
void showGasValue(int gasValue);
void publishGasValue(int gasValue);
bool isMQ2ValueGood(int gasValue);
void goodMQ2ValueActions();
void greenLedOn();
void reduceTimeAfterWarning();
void doBuzzerDeactivation();
void doServoDeactivation();
bool isMQ2ValueWarning(int gasValue);
void warningMQ2ValueActions();
void orangeLedOn();
void emergencyMQ2ValueActions();
void redLedOn();
void increaseTimeAfterWarning();
void doBuzzerActivation();
void doServoActivation();
void activateBuzzerIfNeeded();
void doBuzzerWarning();
void stopBuzzerWarning();

void setup()
{
  setupSerial();
  setupRGBLed();
  blueLedOn();
  connectWiFi();
  createMQTTClient();
  setupMQ2();
  setupServo();
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
  Serial.println("SmartGas >> Setting up the serial");
  Serial.begin(MONITOR_SPEED);
  while (!Serial)
    ;
  delay(SERIAL_DELAY);
}

void setupRGBLed()
{
  Serial.println("SmartGas >> Setting up the RGB LED");
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
  if (response == BUZZER_OFF_MESSAGE)
  {
    publishBuzzerStatus(response.c_str());
    Serial.println(response);
    isBuzzerActive = false;
  }
  else
  {
    Serial.println("Invalid message received. Expected: OFF");
  }
}

void publishBuzzerStatus(const char *status)
{
  // How to send QoS 1?
  client.publish(BUZZER_STATUS_TOPIC.c_str(), status);
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
  // Is this for persistent connection?
  reconnectMQTTClient();
  client.loop();
  delay(PERSIST_MQTT_CONNECTION_DELAY);
}

void setupMQ2()
{
  Serial.println("SmartGas >> Warming up the MQ2 sensor");
  delay(MQ2_WARM_UP_TIME);
}

void setupServo()
{
  Serial.println("SmartGas >> Setting up the servo motor");
  servoMotor.attach(PIN_SERVO, 500, 2400);
  servoMotor.write(0);
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
    doBuzzerDeactivation();
    doServoDeactivation();
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

void doServoDeactivation()
{
  if (currentServoPosition == 180 && timeAfterWarning == 0)
  {
    Serial.println("SmartGas >> Deactivating the servo motor");
    for (int pos = SERVO_MAX_POSITION; pos >= 0; pos -= 1)
    {
      servoMotor.write(pos);
      delay(SERVO_DELAY);
    }
    currentServoPosition = 0;
  }
}

bool isMQ2ValueWarning(int gasValue)
{
  return gasValue >= GOOD_GAS_VALUE && gasValue < WARNING_GAS_VALUE;
}

void warningMQ2ValueActions()
{
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
    doServoActivation();
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
  if (!isBuzzerActive)
  {
    publishBuzzerStatus(BUZZER_ON_MESSAGE);
  }
  if (!userDeactivatedBuzzer)
  {
    isBuzzerActive = true;
  }
}

void doServoActivation()
{
  if (currentServoPosition == 0)
  {
    Serial.println("SmartGas >> Activating the servo motor");
    for (int pos = 0; pos <= SERVO_MAX_POSITION; pos += 1)
    {
      servoMotor.write(pos);
      delay(SERVO_DELAY);
    }
    currentServoPosition = SERVO_MAX_POSITION;
  }
}

void activateBuzzerIfNeeded()
{
  if (isBuzzerActive)
  {
    doBuzzerWarning();
  }
  else
  {
    stopBuzzerWarning();
  }
}

void doBuzzerWarning()
{
  tone(PIN_BUZZER, melody[0], melodyDurations[0]);
}

void stopBuzzerWarning()
{
  noTone(PIN_BUZZER);
}
