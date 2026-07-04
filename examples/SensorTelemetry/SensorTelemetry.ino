// Push DHT11 temperature and humidity over a certificate-pinned WebSocket. Bind
// chart or gauge widgets to "temperature", "humidity" and "uptime_s".
// Needs the "DHT sensor library" by Adafruit (Library Manager).
#include <Nodrix.h>
#include <DHT.h>
#include "secret.h"

#define DHT_PIN  4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Nodrix.setDebug(true);
  Nodrix.setCACert(NODRIX_ROOT_CA);  // pin TLS; omit for the unvalidated default
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();

  if (millis() - lastSend > 5000) {
    lastSend = millis();
    float tempC = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(tempC) || isnan(humidity)) return;
    Nodrix.send("temperature", tempC);
    Nodrix.send("humidity", humidity);
    Nodrix.send("uptime_s", (long)(millis() / 1000));
  }
}
