// Two-light home automation node. Bind two toggle widgets to the variables
// "light_living" and "light_bedroom" on the dashboard.
#include <Nodrix.h>
#include "secret.h"

const int LIGHT_LIVING = 16;
const int LIGHT_BEDROOM = 17;

// Many relay boards are active-LOW (LOW = on) — swap these if yours is.
const int LIGHT_ON = HIGH;
const int LIGHT_OFF = LOW;

NODRIX_WRITE("light_living") {
  digitalWrite(LIGHT_LIVING, value.asBool() ? LIGHT_ON : LIGHT_OFF);
  Nodrix.send("light_living", value.asBool());
}

NODRIX_WRITE("light_bedroom") {
  digitalWrite(LIGHT_BEDROOM, value.asBool() ? LIGHT_ON : LIGHT_OFF);
  Nodrix.send("light_bedroom", value.asBool());
}

void setup() {
  pinMode(LIGHT_LIVING, OUTPUT);
  pinMode(LIGHT_BEDROOM, OUTPUT);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();
}
