// Control the on-board LED from a Nodrix toggle widget bound to the variable "led".
#include <Nodrix.h>
#include "secret.h"

NODRIX_WRITE("led") {
  digitalWrite(LED_BUILTIN, value.asBool() ? HIGH : LOW);
  Nodrix.send("led", value.asBool());  // echo real state so the dashboard hydrates
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();
}
