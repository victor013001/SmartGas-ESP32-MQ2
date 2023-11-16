#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>
#include <pitches.h>

#pragma once

using namespace std;

#define PIN_MQ2_ANALOG 36 // GPI036
#define PIN_BUZZER 5      // GPI05
#define PIN_LED_RED 4     // GPI04
#define PIN_LED_GREEN 0   // GPI00
#define PIN_LED_BLUE 2    // GPI02
#define QUARTER_NOTE 1000
#define GOOD_GAS_VALUE 500
#define WARNING_GAS_VALUE 1500
#define TIME_TO_ACTIVATE_ALERTS 5
#define WIFI_CONNECTION_DELAY 500
#define GAS_READ_VALUE_DELAY 500
#define MQ2_WARM_UP_TIME 20000
#define PERSIST_MQTT_CONNECTION_DELAY 500
#define RECONNECTION_DELAY 5000
#define SERIAL_DELAY 1000
#define MONITOR_SPEED 9600

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int timeAfterWarning = 0;
bool isBuzzerActive = false;
bool userDeactivatedBuzzer = false;
bool isServoActive = false;

const float melody[] = {NOTE_E4};

const int melodyDurations[] = {QUARTER_NOTE};

const char *SSID = "";
const char *PASSWORD = "";

const string ID = "smargas_esp32_001";
const string ID_ESP32_SERVO = "smargas_esp32_002";
const string CLIENT_NAME = ID + "SmartGas client";

const string BROKER = "192.168.204.206";
const int PORT = 1883;

const string BUZZER_TOPIC = "UdeA/SmartGas/Buzzer/" + ID;
const string BUZZER_STATUS_TOPIC = BUZZER_TOPIC + "/Status";
const string MQ2_TOPIC = "UdeA/SmartGas/MQ2/" + ID;
const string SERVO_TOPIC = "UdeA/SmartGas/Servo/" + ID_ESP32_SERVO;

const char *BUZZER_ON_MESSAGE = "ON";
const char *BUZZER_OFF_MESSAGE = "OFF";
const char *SERVO_ON_MESSAGE = "ON";
const char *SERVO_OFF_MESSAGE = "OFF";