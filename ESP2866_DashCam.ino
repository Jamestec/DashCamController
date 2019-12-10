#include "ESP8266WiFi.h"
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <sys/time.h>

#include "login.h"

#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 600       //Time ESP32 will go to sleep (in seconds)

#define CONTROL_PIN 5 // D1
#define VERBOSE 1
#define PRINT_NEXT_SLEEP 1

void setup(){
  Serial.begin(74880);
}

void loop(){
  if (VERBOSE) Serial.printf("Turning on relay for %d seconds", TIME_TO_SLEEP);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);
  delay(TIME_TO_SLEEP * 1000);

  doWork();
}

void doWork() {
  if (VERBOSE) Serial.println("Scanning WiFi...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    if (VERBOSE) Serial.printf("%d. %s\n", i + 1, WiFi.SSID(i).c_str());
    if (WiFi.SSID(i).equals(ssid)) {
      if (VERBOSE) Serial.printf("Found %s\n", ssid);
      doDeepSleep(TIME_TO_SLEEP);
    }
  }
  if (VERBOSE) Serial.printf("Couldn't find %s\n", ssid);
}

void doDeepSleep(int timeSeconds) {
  unsigned long sleepTime = timeSeconds * uS_TO_S_FACTOR;
  if (VERBOSE) Serial.printf("Setup D1 Mini to sleep for every %d Seconds (%lu micro-seconds)\n", timeSeconds, sleepTime);

  if (VERBOSE) Serial.println("Starting deep sleep");
  Serial.flush();
  //Go to sleep now
  ESP.deepSleep(sleepTime);
}
