// Battery node: wake, report over HTTP, apply pending control, sleep.
#include <Nodrix.h>
#if defined(ESP32)
  #include <esp_sleep.h>
#endif

#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"
#define TOKEN     "your-project-token"

#define SLEEP_SECONDS 300

void setup() {
  Nodrix.beginHTTP(WIFI_SSID, WIFI_PASS, HOST, TOKEN);

  Nodrix.send("temperature", readTempC());
  Nodrix.send("battery", readBatteryPct());
  Nodrix.flush();
  Nodrix.poll();

#if defined(ESP32)
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_SECONDS * 1000000ULL);
  esp_deep_sleep_start();
#else
  ESP.deepSleep((uint64_t)SLEEP_SECONDS * 1000000ULL);  // wire GPIO16 to RST to wake
#endif
}

void loop() {}

float readTempC() { return 21.5; }
int readBatteryPct() { return 87; }
