#pragma once
#include <string>
#include <pitches.h>

using namespace std;

#define PIN_MQ2_ANALOG 36 // GPI036
#define PIN_BUZZER 5      // GPI05
#define PIN_LED_RED 4     // GPI04
#define PIN_LED_GREEN 0   // GPI00
#define PIN_LED_BLUE 2    // GPI02
#define PIN_SERVO 18      // GPIO18

#define QUARTER_NOTE 1000
#define GOOD_GAS_VALUE 500
#define WARNING_GAS_VALUE 1500
#define TIME_TO_ACTIVATE_ALERTS 10

const float melody[] = {NOTE_E4};

const int melodyDurations[] = {QUARTER_NOTE};

// WiFi credentials
const char *SSID = "ssid";
const char *PASSWORD = "password";

// MQTT settings
const string ID = "id";

const string BROKER = "ip";
const string CLIENT_NAME = ID + "client";

const string TOPIC = "topictree";