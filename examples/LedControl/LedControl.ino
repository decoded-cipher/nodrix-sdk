// Control the on-board LED from a Nodrix toggle widget bound to the variable "led".
#include <Nodrix.h>
#include "secret.h"

const int LED_PIN = 2;

NODRIX_WRITE("led") {
  digitalWrite(LED_PIN, value.asBool() ? HIGH : LOW);
  Nodrix.send("led", value.asBool());  // echo real state so the dashboard hydrates
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();
}
