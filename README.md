# Nodrix

Connect ESP32 / ESP8266 hardware to [Nodrix](https://nodrix.live) in a few lines.
The library hides WiFi, TLS, the WebSocket/HTTP transport, JSON, and the
control/telemetry protocol — you write device logic, not plumbing.

```cpp
#include <Nodrix.h>
#include "secret.h"

NODRIX_WRITE("led") {                        // cloud writes "led" -> this runs
  digitalWrite(LED_BUILTIN, value.asBool());
  Nodrix.send("led", value.asBool());        // report state back
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();
}
```

## Install

Requires two libraries — install them from the Library Manager (or they resolve
automatically in PlatformIO):

- **ArduinoJson** (v7)
- **WebSockets** by Markus Sattler

Then add this library (Arduino Library Manager, or drop the folder into
`Arduino/libraries/`). PlatformIO:

```ini
lib_deps =
  bblanchon/ArduinoJson@^7
  links2004/WebSockets@^2.6
  https://github.com/decoded-cipher/nodrix-sdk.git
```

Boards: ESP32 and ESP8266.

## Configuration

Put your WiFi and Nodrix credentials at the top of the sketch:

```cpp
#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"   // Nodrix host, no https://
#define TOKEN     "your-project-token"         // Settings -> Tokens
```

Or keep them in a `secret.h` next to the sketch and `#include "secret.h"` — the
LedControl and SensorTelemetry examples do this.

## Receiving control

`NODRIX_WRITE("var") { ... }` registers a handler for a variable. When a
dashboard widget (toggle, slider, button, color) writes that variable, the block
runs with `value` in scope. Values are coerced for you:

| method | returns |
| --- | --- |
| `value.asBool()` | `true` for `true`, non-zero, or `"on"`/`"1"`/`"true"`/`"yes"` |
| `value.asInt()` / `asLong()` | integer (parses numeric strings) |
| `value.asFloat()` / `asDouble()` | number |
| `value.asString()` | string |
| `value.isNull()` | no value |

Acks are automatic. Handlers may run more than once for the same command across a
reconnect, so keep them idempotent (setting a pin is).

## Sending telemetry

```cpp
Nodrix.send("temperature", 22.5);
Nodrix.send("online", true);
Nodrix.send("status", "ok");
```

Calls stage a metric and coalesce into a single frame on the next `run()`
(WebSocket) or `flush()`/`poll()` (HTTP). Overloads cover `bool`, `int`, `long`,
`float`, `double`, `const char*`, and `String`.

## Transports

- **WebSocket (`begin`)** — always-on. Control arrives instantly; call
  `Nodrix.run()` every `loop()`. Best for lights, relays, anything cloud-driven.
- **HTTP (`beginHTTP`)** — for battery / deep-sleep nodes. `send()` + `flush()`
  to POST readings, `poll()` to fetch and apply pending control, then sleep.

## Events

Fire a named event to trigger a server-side automation:

```cpp
Nodrix.event("door_opened");
```

## TLS

By default certificates are not validated — the simplest path to get running. To
pin, call one before `begin()`:

- `Nodrix.setCACert(pem)` — ESP32 (CA) / ESP32+ESP8266 for HTTP.
- `Nodrix.setFingerprint(fp)` — ESP8266 SHA-1 fingerprint.

## Examples

- **LedControl** — toggle the on-board LED (`secret.h`).
- **HomeLights** — two independently controlled lights/relays.
- **SensorTelemetry** — periodic readings over a cert-pinned socket (`secret.h` holds the CA).
- **DeepSleepSensor** — HTTP mode: wake, report, apply control, deep sleep.

## Debug logging

Build with `-DNODRIX_DEBUG` to print connection and protocol activity to Serial.

## License

MIT
