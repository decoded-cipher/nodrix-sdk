// A sketch that can replace itself over the air. The version below is what the
// board reports and what Nodrix compares against the firmware assigned to it —
// upload the compiled .ino.bin under this exact string.
#include <Nodrix.h>

#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"
#define TOKEN     "your-project-token"

#define FIRMWARE_VERSION "1.0.0"

const int LED_PIN = 2;
unsigned long lastReport = 0;
bool updateRequested = false;

// Bind a button widget to "check_update". Applying an update blocks and then
// restarts the board, so it runs from loop() rather than in this handler.
NODRIX_WRITE("check_update") {
  if (value.asBool()) updateRequested = true;
}

NODRIX_WRITE("led") {
  digitalWrite(LED_PIN, value.asBool() ? HIGH : LOW);
  Nodrix.send("led", value.asBool());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Nodrix.setDebug(true);  // prints what the update does, step by step
  Nodrix.setFirmwareVersion(FIRMWARE_VERSION);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();

  if (updateRequested) {
    updateRequested = false;
    Nodrix.checkForUpdate();
  }

  // Somewhere to watch the version change after an update lands.
  if (millis() - lastReport > 10000) {
    lastReport = millis();
    Nodrix.send("firmware", FIRMWARE_VERSION);
    Nodrix.send("uptime_s", (long)(millis() / 1000));
  }
}
