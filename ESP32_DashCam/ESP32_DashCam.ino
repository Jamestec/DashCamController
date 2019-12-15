#include "WiFi.h"
#include "login.h"

#define uS_TO_S_FACTOR 1000000      // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 60            // Time between checking for WiFi SSID
#define WIFI_DOT_INTERVAL 50        // Milliseconds between checking if WiFi has connected
#define WIFI_DOT_INTERVAL_SEC 0.05  // WIFI_DOT_INTERVAL / 1000
#define WIFI_DOT_LINE 20            // Amount of dots before new line for dots
#define TIMEOUT 200                 // Stops trying to connect to WiFi after TIMEOUT * 0.05 seconds
#define TIME_FIRST_DELAY 600        // Time relay is on when ESP first starts before checking WiFi SSID
#define TIME_TO_RECHECK 10          // Time before rechecking SSID is actually not available

#define CONTROL_PIN 13
#define GPIO_CONTROL_PIN GPIO_NUM_13
#define VERBOSE 1

RTC_DATA_ATTR int found = 1;

void setup(){
  if (VERBOSE) Serial.begin(115200);
}

void loop(){
  if (print_wakeup_reason() == 0) {
    turnOnRelay(TIME_FIRST_DELAY);
  }

  doWork();
}

void doWork() {
  if (isWiFiAvailable()) {
    found = 1;
    turnOffRelay(TIME_TO_SLEEP);
  }
  if (found) {
    found = 0;
    doDeepSleep(TIME_TO_RECHECK);
  } else {
    turnOnRelay(TIME_TO_SLEEP);
  }
}

bool isWiFiAvailable() {
  if (VERBOSE) Serial.print("Connecting to WiFi...");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  int wifiCount = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && wifiCount < TIMEOUT) {
    if (VERBOSE && wifiCount % WIFI_DOT_LINE == 0) Serial.println("");
    if (VERBOSE) Serial.print(".");
    wifiCount += 1;
    digitalWrite(LED_BUILTIN, HIGH);
    delay(WIFI_DOT_INTERVAL);
    digitalWrite(LED_BUILTIN, LOW);
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  if (wifiCount < TIMEOUT) {
    Serial.printf("\n...Connected to %s in %.2f second(s)!\n", ssid, WIFI_DOT_INTERVAL_SEC * wifiCount);
    return true;
  }
  if (VERBOSE) Serial.printf("\nCould not connect to WiFi in %.2f seconds.\n", TIMEOUT * WIFI_DOT_INTERVAL_SEC);
  return false;
}

void turnOnRelay(int timeSeconds) {
  if (VERBOSE) Serial.printf("Turning on relay for %d seconds\n", TIME_TO_SLEEP);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);
  gpio_hold_en(GPIO_CONTROL_PIN);
  doDeepSleep(timeSeconds);
  //delay(timeSeconds * 1000);
}

void turnOffRelay(int timeSeconds) {
  if (VERBOSE) Serial.printf("Turning off relay for %d seconds\n", TIME_TO_SLEEP);
  //pinMode(CONTROL_PIN, OUTPUT);
  //digitalWrite(CONTROL_PIN, HIGH);
  gpio_hold_dis(GPIO_CONTROL_PIN);
  doDeepSleep(timeSeconds);
}

void doDeepSleep(int timeSeconds) {
  unsigned long sleepTime = timeSeconds * uS_TO_S_FACTOR;
  if (VERBOSE) Serial.printf("Setup ESP32 to sleep for every %d Seconds (%lu micro-seconds)\n", timeSeconds, sleepTime);
  esp_sleep_enable_timer_wakeup(sleepTime);
  //Go to sleep now
  if (VERBOSE) Serial.println("Starting deep sleep");
  Serial.flush();
  esp_deep_sleep_start(); 
}

//Function that prints the reason by which ESP32 has been awaken from sleep
int print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/sleep_modes.html#_CPPv418esp_sleep_source_t
    case ESP_SLEEP_WAKEUP_EXT0  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER  : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD  : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP  : Serial.println("Wakeup caused by ULP program"); break;
    case ESP_SLEEP_WAKEUP_GPIO  : Serial.println("Wakeup caused by GPIO (from light sleep)"); break;
    case ESP_SLEEP_WAKEUP_UART  : Serial.println("Wakeup caused by UART (from light sleep)"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break; // ESP_SLEEP_WAKEUP_UNDEFINED
  }
  return wakeup_reason;
}
