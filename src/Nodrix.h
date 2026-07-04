#ifndef NODRIX_H
#define NODRIX_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

// A control value coming down from the cloud. The wire value may be a bool,
// number or string depending on the widget, so everything is coerced.
class NodrixValue {
 public:
  explicit NodrixValue(JsonVariantConst v) : _v(v) {}
  bool asBool() const;
  int asInt() const { return (int)asLong(); }
  long asLong() const;
  float asFloat() const;
  double asDouble() const { return (double)asFloat(); }
  String asString() const;
  bool isNull() const { return _v.isNull(); }

 private:
  JsonVariantConst _v;
};

typedef void (*NodrixWriteFn)(NodrixValue value);

// One NODRIX_WRITE handler. Constructs itself into a global list before setup()
// runs, so no registration call is needed in the sketch.
struct NodrixHandlerReg {
  const char* variable;
  NodrixWriteFn fn;
  NodrixHandlerReg* next;
  NodrixHandlerReg(const char* v, NodrixWriteFn f);
};

#define NODRIX_CONCAT_(a, b) a##b
#define NODRIX_CONCAT(a, b) NODRIX_CONCAT_(a, b)

#define NODRIX_WRITE_IMPL(var, fn, reg)         \
  static void fn(NodrixValue value);            \
  static NodrixHandlerReg reg(var, fn);         \
  static void fn(NodrixValue value)

// NODRIX_WRITE("led") { ... value ... } — runs when the cloud writes that variable.
#define NODRIX_WRITE(var)                                   \
  NODRIX_WRITE_IMPL(var, NODRIX_CONCAT(nodrix_fn_, __COUNTER__), \
                    NODRIX_CONCAT(nodrix_reg_, __COUNTER__))

class NodrixClass {
 public:
  // Always-on: control + telemetry share one WebSocket. Blocks until WiFi is up.
  void begin(const char* ssid, const char* pass, const char* host, const char* token, uint16_t port = 443);
  // Battery/deep-sleep: telemetry over HTTP POST, control fetched by poll().
  void beginHTTP(const char* ssid, const char* pass, const char* host, const char* token, uint16_t port = 443);

  void run();    // WS mode: pump the socket + flush telemetry. Call in loop().
  bool poll();   // HTTP mode: flush telemetry, fetch + apply control, ack. Call on wake.

  void send(const char* variable, bool value);
  void send(const char* variable, int value);
  void send(const char* variable, long value);
  void send(const char* variable, float value);
  void send(const char* variable, double value);
  void send(const char* variable, const char* value);
  void send(const char* variable, const String& value);
  void flush();  // transmit staged telemetry now

  void event(const char* name);
  void event(const char* name, JsonVariantConst payload);

  void onConnect(void (*cb)()) { _onConnect = cb; }
  void onDisconnect(void (*cb)()) { _onDisconnect = cb; }
  bool connected() const { return _connected; }

  // TLS. Default is no cert validation (matches a bare beginSSL). Opt into
  // pinning with one of these before begin()/beginHTTP().
  void setInsecure() { _insecure = true; _ca = nullptr; _fingerprint = nullptr; }
  void setCACert(const char* pem) { _ca = pem; _insecure = false; }
  void setFingerprint(const char* fp) { _fingerprint = fp; _insecure = false; }

  // Internal: forwarded from the WebSocket event callback.
  void _handleWsEvent(WStype_t type, uint8_t* payload, size_t length);

 private:
  void connectWiFi(const char* ssid, const char* pass);
  void onConnected();
  void dispatchControl(const char* variable, JsonVariantConst value);
  void seedControlVars();
  bool isControlVar(const char* key) const;
  void ackWs(const char* id);
  bool validKey(const char* key) const;
  bool ensureRoom();

  int httpPost(const char* path, const String& body);
  bool httpGet(const char* path, String& out);

  WebSocketsClient _ws;
  String _host;
  String _token;
  String _wsPath;
  uint16_t _port = 443;
  bool _wsMode = false;
  bool _connected = false;

  JsonDocument _tx;          // staged telemetry metrics (key -> value)
  JsonDocument _ctrlState;   // last echoed value per control var, for reseed

  void (*_onConnect)() = nullptr;
  void (*_onDisconnect)() = nullptr;

  bool _insecure = true;
  const char* _ca = nullptr;
  const char* _fingerprint = nullptr;
};

extern NodrixClass Nodrix;

#endif  // NODRIX_H
