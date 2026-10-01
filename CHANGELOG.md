# Changelog

## 0.2.0 — 2026-10-02

- Over-the-air updates. `setFirmwareVersion()` names the version a sketch is; it
  must match the version the image was uploaded under. A board updates itself
  when firmware is assigned to it: told over the socket, or through `poll()` in
  HTTP mode, with a six-hourly check as a backstop. `checkForUpdate()` asks now.
  Without a version, updates are skipped.
- Updates are checked against the uploaded image's hash. On ESP32 a new image is
  kept only once it reaches the cloud; one that resets first, or stays offline
  for 15 minutes on a working network, rolls back. A rolled-back or impossible
  update is reported and not offered again.
- Boards identify themselves: a device key (the MAC, or `setDeviceKey()`), the
  chip (`ESP.getChipModel()` on ESP32, or `setChip()`) and the firmware version.
- New `OtaUpdate` example.
- A sketch that blocks in `loop()`, such as a `delay()` blink, connects and stays
  connected for blocks up to about 10 s. Reconnects back off from 5 s to 60 s.
- `poll()` makes all its requests over one TLS connection. ESP32 TLS handshakes
  give up after 15 s; ESP8266 resumes TLS sessions.
- With `setDebug(true)`, a `run()` that blocks for over 500 ms is logged.

## 0.1.1 — 2026-09-16

- Debug output now tells connection failures apart: failed HTTP requests log the
  status code with a reason (token rejected, no access, wrong host, rate limited,
  server error, no connection), a refused WebSocket handshake is reported separately
  from a dropped link, and socket errors are no longer swallowed. All behind
  `setDebug(true)`.
- Install from the Arduino Library Manager, PlatformIO Registry, or ESP Component
  Registry; added ESP-IDF instructions.

## 0.1.0 — 2026-07-05

First release.

- `NODRIX_WRITE("var") { ... }` handlers for cloud control writes, with automatic
  acks and typed value coercion — `asBool()`, `asInt()`, `asLong()`, `asFloat()`,
  `asDouble()`, `asString()`, `isNull()`.
- `Nodrix.send()` telemetry with batching; overloads for `bool`, `int`, `long`,
  `float`, `double`, `const char*`, and `String`.
- Two transports: always-on WebSocket (`begin()` / `run()`) and HTTP polling for
  deep-sleep devices (`beginHTTP()` / `poll()`).
- Multiple WiFi networks with failover via `addAP()`.
- Automatic reconnect, heartbeat, and control-variable seeding.
- Optional TLS pinning — `setCACert()` (ESP32) and `setFingerprint()` (ESP8266
  HTTP); unvalidated `setInsecure()` by default.
- Server-side events via `event()`.
- Runtime debug logging via `setDebug()`.
- Examples: LedControl, HomeLights, MultiWiFi, SensorTelemetry, DeepSleepSensor.
- Supports ESP32 (including S2, S3, C3, C6, H2) and ESP8266.
