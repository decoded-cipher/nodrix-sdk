// Connect through several WiFi networks — the strongest reachable one is used,
// with automatic failover between them.
#include <Nodrix.h>

const int LED_PIN = 2;

#define HOST  "yourproject.workers.dev"
#define TOKEN "your-project-token"

NODRIX_WRITE("led") {
  digitalWrite(LED_PIN, value.asBool() ? HIGH : LOW);
  Nodrix.send("led", value.asBool());
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Nodrix.addAP("home-ssid", "home-pass");
  Nodrix.addAP("office-ssid", "office-pass");
  Nodrix.begin(HOST, TOKEN);  // no ssid/pass — uses the networks above
}

void loop() {
  Nodrix.run();
}
