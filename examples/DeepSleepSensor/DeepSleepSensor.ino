// Battery node: wake, report over HTTP, apply pending control, sleep.
#include <Nodrix.h>

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

  // ESP8266: wire GPIO16 to RST to wake.
  ESP.deepSleep((uint64_t)SLEEP_SECONDS * 1000000ULL);
}

void loop() {}

float readTempC() { return 21.5; }
int readBatteryPct() { return 87; }
