#include "credentials.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <stdio.h>
#include <time.h>

// debug output
#define USE_SERIAL Serial

// pin assign
#define LED_BUILTIN 2
#define SENSOR_HOT_PIN 23
#define SENSOR_COLD_PIN 22

// constants
#define JST 9 * 3600
#define PHASE_OPEN 0
#define PHASE_HIGH 1
#define PHASE_LOW 2
#define PHASE_HIT 3
#define REFRACTORY_PERIOD 1
#define OPEN_REFRACTORY_PERIOD 10
#define MILLIMETER_PAR_STEP 134.0 // 670mm / 5step

// wifi struct
WiFiMulti wifiMulti;

// variables
unsigned long prevTimeHigh = 0;
unsigned long prevTimeLow = 0;
unsigned long prevTimeHit = 0;
unsigned long intervalTime = 0;
unsigned long hits = 0;
unsigned int phase = 0;
unsigned int prevSensor = HIGH;

/*
 * GPIO change handler
 */
void IRAM_ATTR handleInterrupt() {

  // read sensor signal
  int sensor = digitalRead(SENSOR_COLD_PIN);

  // indicate sensor status to builtin led
  digitalWrite(LED_BUILTIN, !sensor);

  // get current time
  unsigned long currentTime = millis();

  // phase progression
  if (phase == PHASE_OPEN) {
    if (prevSensor == LOW && sensor == HIGH &&
        (currentTime - prevTimeHit) >= OPEN_REFRACTORY_PERIOD) {
      prevTimeHigh = currentTime;
      phase = PHASE_HIGH;
    }
  } else if (phase == PHASE_HIGH) {
    if (prevSensor == HIGH && sensor == LOW &&
        (currentTime - prevTimeHigh) >= REFRACTORY_PERIOD) {
      prevTimeLow = currentTime;
      phase = PHASE_LOW;
    }
  } else if (phase == PHASE_LOW) {
    if (prevSensor == LOW && sensor == HIGH &&
        (currentTime - prevTimeLow) >= REFRACTORY_PERIOD) {
      intervalTime = currentTime - prevTimeHit;
      prevTimeHit = currentTime;
      phase = PHASE_HIT;
      hits++;
    }
  } else if (phase == PHASE_HIT) {
    if (prevSensor == HIGH && sensor == LOW &&
        (currentTime - prevTimeHit) >= REFRACTORY_PERIOD) {
      phase = PHASE_OPEN;
    }
  }

  // save current sensor signal
  prevSensor = sensor;
}

// build http post message
void buildMessage(char *buf, float distance) {
  time_t ts;
  ts = time(NULL);
  USE_SERIAL.printf("[SENSOR] ts=%d\n", ts);
  USE_SERIAL.printf("[SENSOR] distance=%f\n", distance);
  sprintf(buf, "{\"esp32\":{\"ts\":%d,\"distance\":%f}}", ts, distance);
}

// post message with HTTPS or HTTP
void postHttp(const char *url, const char *data) {
  HTTPClient http;

  USE_SERIAL.print("[HTTP] begin...\n");

  http.begin(url);                             // HTTP or HTTPS
  http.setAuthorization(AUTH_USER, AUTH_PASS); // basic auth

  USE_SERIAL.print("[HTTP] POST...\n");
  USE_SERIAL.print(data);

  int httpCode = http.POST(data);

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      USE_SERIAL.println(payload);
    }
  } else {
    USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n",
                      http.errorToString(httpCode).c_str());
  }

  http.end();
}

// setup device gpio, wifi, variables
void setup() {
  // init serial
  USE_SERIAL.begin(115200);

  // init pin mode
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR_COLD_PIN, INPUT_PULLUP);
  pinMode(SENSOR_HOT_PIN, OUTPUT);

  // set low senser hot pin
  digitalWrite(SENSOR_HOT_PIN, LOW);

  // wait for wifi module setup
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  // connect wifi
  wifiMulti.addAP(WIFI_SSID, WIFI_SECRET);
  while (wifiMulti.run() != WL_CONNECTED) {
    USE_SERIAL.printf("[WiFi] Wait for connection.\n");
    delay(1000);
  }
  USE_SERIAL.printf("[WiFi] connected.\n");

  // set ntp
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  // init variables
  prevSensor = digitalRead(SENSOR_COLD_PIN);
  prevTimeHigh = millis();
  prevTimeLow = prevTimeHigh;
  prevTimeHit = prevTimeHigh;
  hits = 0;

  // weight for sensor phase
  delay(OPEN_REFRACTORY_PERIOD);

  // start sensor intterupt
  attachInterrupt(digitalPinToInterrupt(SENSOR_COLD_PIN), handleInterrupt,
                  CHANGE);
}

void loop() {
  // message buffer
  char buf[100];

  if ((wifiMulti.run() == WL_CONNECTED)) {
    // wifi connected

    // calc distance. hits is global variable.
    float cumsumDistanceMeter = MILLIMETER_PAR_STEP * hits / 1000.0;

    // debug output
    USE_SERIAL.print("[SENSOR] cumsumDistanceMeter=");
    USE_SERIAL.println(cumsumDistanceMeter);

    // build http body message
    buildMessage(buf, cumsumDistanceMeter);

    // post to api
    postHttp(API_URL, buf);

    // loop weight
    delay(1 * 60 * 1000);

  } else {
    delay(1000);
  }
}
