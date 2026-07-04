// Battery sensor node: wake, report over HTTP, apply any pending control, sleep.
// Uses HTTP mode (beginHTTP) instead of the always-on socket to save power.
#include <Nodrix.h>
#include "secret.h"

#define SLEEP_SECONDS 300

void setup() {
  Serial.begin(115200);
  Nodrix.beginHTTP(WIFI_SSID, WIFI_PASS, HOST, TOKEN);

  Nodrix.send("temperature", readTempC());
  Nodrix.send("battery", readBatteryPct());
  Nodrix.flush();  // POST the batch

  Nodrix.poll();   // fetch + apply pending control, then ack

  // ESP.deepSleep takes microseconds. On ESP8266 wire GPIO16 to RST to wake.
  ESP.deepSleep((uint64_t)SLEEP_SECONDS * 1000000ULL);
}

void loop() {}

// Replace with real sensor reads.
float readTempC() { return 21.5; }
int readBatteryPct() { return 87; }
