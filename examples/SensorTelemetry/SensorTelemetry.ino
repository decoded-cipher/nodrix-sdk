// Push sensor readings over a certificate-pinned WebSocket (ESP32). Bind chart or
// gauge widgets to "temperature", "humidity" and "uptime_s".
#include <Nodrix.h>
#include "secret.h"

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Nodrix.setDebug(true);             // log connection and protocol activity to Serial
  Nodrix.setCACert(NODRIX_ROOT_CA);  // pin TLS; omit for the unvalidated default
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();

  if (millis() - lastSend > 5000) {
    lastSend = millis();
    Nodrix.send("temperature", readTempC());
    Nodrix.send("humidity", readHumidity());
    Nodrix.send("uptime_s", (long)(millis() / 1000));
  }
}

float readTempC() { return 20.0 + (float)random(0, 500) / 100.0; }
float readHumidity() { return 40.0 + (float)random(0, 4000) / 100.0; }
