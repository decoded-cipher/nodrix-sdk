// Push sensor readings to Nodrix over the always-on WebSocket. Bind chart / gauge
// widgets to "temperature", "humidity" and "uptime_s".
#include <Nodrix.h>
#include "secret.h"

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();

  if (millis() - lastSend > 5000) {
    lastSend = millis();
    Nodrix.send("temperature", readTempC());
    Nodrix.send("humidity", readHumidity());
    Nodrix.send("uptime_s", (long)(millis() / 1000));
    // The three metrics coalesce into one telemetry frame on the next run().
  }
}

// Replace with real sensor reads.
float readTempC() { return 20.0 + (float)random(0, 500) / 100.0; }
float readHumidity() { return 40.0 + (float)random(0, 4000) / 100.0; }
