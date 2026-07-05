# Nodrix Arduino Library

Arduino library for connecting ESP32 / ESP8266 hardware to
[Nodrix](https://nodrix.live). It hides WiFi, TLS, the WebSocket/HTTP transport,
JSON, and the control/telemetry protocol behind a small API — you write device
logic, not plumbing.

```cpp
#include <Nodrix.h>
#include "secret.h"

const int LED_PIN = 2;

NODRIX_WRITE("led") {                          // cloud writes "led" -> this runs
  digitalWrite(LED_PIN, value.asBool() ? HIGH : LOW);
  Nodrix.send("led", value.asBool());          // report state back
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Nodrix.begin(WIFI_SSID, WIFI_PASS, HOST, TOKEN);
}

void loop() {
  Nodrix.run();
}
```

## Installation

**Arduino IDE** — install these from the Library Manager, then add this library
(Library Manager, or drop the folder into `Arduino/libraries/`):

- ArduinoJson (v7)
- WebSockets by Markus Sattler

**PlatformIO** — add to `platformio.ini`:

```ini
lib_deps =
  bblanchon/ArduinoJson@^7
  links2004/WebSockets@^2.6
  https://github.com/decoded-cipher/nodrix-sdk.git
```

**ESP-IDF** — from the
[ESP Component Registry](https://components.espressif.com/components/decoded-cipher/nodrix),
in a project that uses arduino-esp32 as a component:

```
idf.py add-dependency "decoded-cipher/nodrix"
```

## Supported boards

Any **ESP32** (including S2, S3, C3, C6, H2) or **ESP8266** board — e.g. XIAO
ESP32-C3/S3, Arduino Nano ESP32, Seeed ESP32 boards, NodeMCU, Wemos D1 mini.

Boards that reach WiFi through a coprocessor (Pico W, UNO R4 WiFi, WiFiNINA) and
cellular modules (A9G, SIM7000) are not supported yet.

## Configuration

Put your WiFi and Nodrix credentials at the top of the sketch:

```cpp
#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"   // Nodrix host, no https://
#define TOKEN     "your-project-token"         // Settings -> Tokens
```

Or keep them in a `secret.h` next to the sketch and `#include "secret.h"` — the
`LedControl` and `SensorTelemetry` examples do this.

### Multiple networks

Register fallbacks with `addAP()` before `begin()`. The strongest reachable
network is used, and the device fails over between them:

```cpp
Nodrix.addAP("home-ssid", "home-pass");
Nodrix.addAP("office-ssid", "office-pass");
Nodrix.begin(HOST, TOKEN);   // no ssid/pass — uses the list above
```

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

Acks are automatic. A handler may run more than once for the same command across
a reconnect, so keep it idempotent (setting a pin is).

## Sending telemetry

```cpp
Nodrix.send("temperature", 22.5);
Nodrix.send("online", true);
Nodrix.send("status", "ok");
```

Calls stage a metric and coalesce into a single frame on the next `run()`
(WebSocket) or `flush()`/`poll()` (HTTP). Overloads cover `bool`, `int`, `long`,
`float`, `double`, `const char*`, and `String`.

## Events

Fire a named event to trigger a server-side automation:

```cpp
Nodrix.event("door_opened");
```

## Transports

- **WebSocket — `begin()`** — always-on. Control arrives instantly; call
  `Nodrix.run()` every `loop()`. Best for lights, relays, anything cloud-driven.
- **HTTP — `beginHTTP()`** — for battery / deep-sleep nodes. `send()` + `flush()`
  to POST readings, `poll()` to fetch and apply pending control, then sleep.

## TLS

By default certificates are not validated — the simplest path to get running. To
pin a certificate, call one before `begin()`:

- `Nodrix.setCACert(pem)` — ESP32, WebSocket and HTTP.
- `Nodrix.setFingerprint(fp)` — ESP8266 HTTP mode (SHA-1 fingerprint).

Pin the **root CA**, not the server (leaf) certificate — the leaf rotates every
~90 days, the root does not. Your host already serves its root as the last block
of the TLS chain; pull that block straight into `secret.h`:

```sh
openssl s_client -showcerts -servername yourproject.workers.dev \
  -connect yourproject.workers.dev:443 </dev/null 2>/dev/null \
| awk '/BEGIN CERTIFICATE/{c++; b=""} {b=b $0 ORS} /END CERTIFICATE/{last=b} END{printf "\n%s", last}'
```

For Cloudflare `*.workers.dev` that returns GTS Root R4 — the one shipped in the
`SensorTelemetry` example. A custom domain returns its own root; the command is
the same.

On ESP8266 the WebSocket transport is always unvalidated; use ESP32 for a pinned
WebSocket, or ESP8266 HTTP mode with a fingerprint — but a fingerprint pins the
leaf, so it must be refreshed on each rotation; prefer a root CA where you can.

## API reference

| | |
| --- | --- |
| `addAP(ssid, pass)` | Register a WiFi network before `begin()` |
| `begin(ssid, pass, host, token[, port])` | Connect over WebSocket |
| `begin(host, token[, port])` | WebSocket, using the `addAP()` networks |
| `beginHTTP(ssid, pass, host, token[, port])` | HTTP mode |
| `beginHTTP(host, token[, port])` | HTTP mode, using the `addAP()` networks |
| `run()` | WebSocket: pump the socket and flush telemetry; call every `loop()` |
| `poll()` | HTTP: send readings, fetch and apply control; call on wake |
| `send(var, value)` | Stage a metric (`bool`/`int`/`long`/`float`/`double`/`const char*`/`String`) |
| `flush()` | Transmit staged metrics now |
| `event(name)` / `event(name, payload)` | Fire a server-side event |
| `connected()` | Whether the link is up |
| `onConnect(cb)` / `onDisconnect(cb)` | Connection callbacks |
| `setInsecure()` | Skip certificate validation (default) |
| `setCACert(pem)` | Pin a root CA (ESP32) |
| `setFingerprint(fp)` | Pin a SHA-1 fingerprint (ESP8266 HTTP) |
| `setDebug(on)` | Log connection and protocol activity to Serial |
| `NODRIX_WRITE("var") { ... }` | Handle a cloud write; `value` is in scope |

## Examples

- **LedControl** — toggle the on-board LED (`secret.h`).
- **HomeLights** — two independently controlled lights/relays.
- **MultiWiFi** — connect through several WiFi networks with failover.
- **SensorTelemetry** — DHT11 temperature/humidity over a cert-pinned socket (`secret.h` holds the CA).
- **DeepSleepSensor** — HTTP mode: read a DHT11, report, apply control, deep sleep.

`SensorTelemetry` and `DeepSleepSensor` read a DHT11 — install the **DHT sensor
library** by Adafruit from the Library Manager.

## Debug logging

Call `Nodrix.setDebug(true)` before `begin()` to print connection and protocol
activity to Serial (call `Serial.begin()` first).