#include "Nodrix.h"

#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
  #include <WiFiMulti.h>
  #define NODRIX_MAKE_TLS(client)                                   \
    WiFiClientSecure client;                                        \
    do { if (_ca) client.setCACert(_ca); else client.setInsecure(); } while (0)
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecureBearSSL.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266WiFiMulti.h>
  #define NODRIX_MAKE_TLS(client)                                   \
    BearSSL::WiFiClientSecure client;                               \
    do { if (_fingerprint) client.setFingerprint(_fingerprint); else client.setInsecure(); } while (0)
#else
  #error "Nodrix supports ESP32 and ESP8266"
#endif

#if defined(ESP32)
static WiFiMulti wifiMulti;
#elif defined(ESP8266)
static ESP8266WiFiMulti wifiMulti;
#endif

#ifdef NODRIX_DEBUG
  #define NODRIX_LOG(...) Serial.printf(__VA_ARGS__)
#else
  #define NODRIX_LOG(...)
#endif

static const int MAX_POINTS = 200;
static const int MAX_KEY_LEN = 64;
static const int MAX_STRING_VALUE = 512;

// Registry head in a function-local static so it's valid during static init.
static NodrixHandlerReg*& registryHead() {
  static NodrixHandlerReg* head = nullptr;
  return head;
}

NodrixHandlerReg::NodrixHandlerReg(const char* v, NodrixWriteFn f)
    : variable(v), fn(f), next(nullptr) {
  NodrixHandlerReg*& head = registryHead();
  next = head;
  head = this;
}

bool NodrixValue::asBool() const {
  if (_v.is<bool>()) return _v.as<bool>();
  if (_v.is<const char*>()) {
    String s = _v.as<const char*>();
    s.trim();
    s.toLowerCase();
    return s == "on" || s == "1" || s == "true" || s == "yes";
  }
  return _v.as<double>() != 0.0;
}

long NodrixValue::asLong() const {
  if (_v.is<const char*>()) return atol(_v.as<const char*>());
  return _v.as<long>();
}

float NodrixValue::asFloat() const {
  if (_v.is<const char*>()) return atof(_v.as<const char*>());
  return _v.as<float>();
}

String NodrixValue::asString() const {
  if (_v.isNull()) return String();
  if (_v.is<const char*>()) return String(_v.as<const char*>());
  String s;
  serializeJson(_v, s);
  return s;
}

NodrixClass Nodrix;

void NodrixClass::addAP(const char* ssid, const char* pass) {
  wifiMulti.addAP(ssid, pass);
  _hasAP = true;
}

void NodrixClass::begin(const char* ssid, const char* pass, const char* host,
                        const char* token, uint16_t port) {
  addAP(ssid, pass);
  begin(host, token, port);
}

void NodrixClass::begin(const char* host, const char* token, uint16_t port) {
  _host = host;
  _token = token;
  _port = port;
  _wsMode = true;
  _wsPath = "/v1/control/ws?token=" + _token;

  connectWiFi();

  _ws.onEvent([](WStype_t t, uint8_t* p, size_t l) { Nodrix._handleWsEvent(t, p, l); });
#if defined(ESP32)
  if (_ca) _ws.beginSslWithCA(_host.c_str(), _port, _wsPath.c_str(), _ca);
  else if (_fingerprint) _ws.beginSSL(_host.c_str(), _port, _wsPath.c_str(), _fingerprint);
  else _ws.beginSSL(_host.c_str(), _port, _wsPath.c_str());
#else
  // ESP8266 WebSocket TLS is unvalidated; fingerprint/CA pinning is HTTP-only there.
  _ws.beginSSL(_host.c_str(), _port, _wsPath.c_str());
#endif
  _ws.setReconnectInterval(5000);
  _ws.enableHeartbeat(15000, 3000, 2);
}

void NodrixClass::beginHTTP(const char* ssid, const char* pass, const char* host,
                            const char* token, uint16_t port) {
  addAP(ssid, pass);
  beginHTTP(host, token, port);
}

void NodrixClass::beginHTTP(const char* host, const char* token, uint16_t port) {
  _host = host;
  _token = token;
  _port = port;
  _wsMode = false;

  connectWiFi();
  _connected = (WiFi.status() == WL_CONNECTED);
  seedControlVars();
  flush();
}

void NodrixClass::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (!_hasAP) {
    NODRIX_LOG("[nodrix] no wifi network set (call addAP)\n");
    return;
  }
  WiFi.mode(WIFI_STA);
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(300);
    NODRIX_LOG(".");
  }
  NODRIX_LOG("\n[nodrix] wifi connected: %s\n", WiFi.localIP().toString().c_str());
}

void NodrixClass::run() {
  if (!_wsMode) return;
  if (WiFi.status() != WL_CONNECTED) wifiMulti.run();
  _ws.loop();
  flush();
}

bool NodrixClass::poll() {
  if (_wsMode) return false;
  if (WiFi.status() != WL_CONNECTED) wifiMulti.run();
  flush();

  String body;
  if (!httpGet("/v1/control", body)) return false;

  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;

  JsonDocument ackDoc;
  JsonArray ids = ackDoc["ids"].to<JsonArray>();
  for (JsonObject c : doc["control"].as<JsonArray>()) {
    const char* id = c["id"] | "";
    const char* variable = c["variable"] | "";
    if (variable[0]) {
      dispatchControl(variable, c["value"]);
      if (id[0]) ids.add(id);
    }
  }

  flush();  // send echoes staged by the handlers
  if (ids.size() > 0) {
    String ackBody;
    serializeJson(ackDoc, ackBody);
    httpPost("/v1/control/ack", ackBody);
  }
  return true;
}

void NodrixClass::_handleWsEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      _connected = true;
      NODRIX_LOG("[nodrix] connected\n");
      onConnected();
      break;
    case WStype_DISCONNECTED:
      _connected = false;
      NODRIX_LOG("[nodrix] disconnected\n");
      if (_onDisconnect) _onDisconnect();
      break;
    case WStype_TEXT: {
      JsonDocument doc;
      if (deserializeJson(doc, payload, length)) return;
      const char* t = doc["type"] | "";
      if (strcmp(t, "control") == 0) {
        const char* id = doc["id"] | "";
        const char* variable = doc["variable"] | "";
        if (variable[0]) {
          dispatchControl(variable, doc["value"]);
          if (id[0]) ackWs(id);
        }
      } else if (strcmp(t, "error") == 0) {
        NODRIX_LOG("[nodrix] server error: %s\n", (const char*)(doc["code"] | ""));
      }
      break;
    }
    default:
      break;
  }
}

void NodrixClass::onConnected() {
  seedControlVars();
  flush();
  if (_onConnect) _onConnect();
}

void NodrixClass::dispatchControl(const char* variable, JsonVariantConst value) {
  bool handled = false;
  for (NodrixHandlerReg* r = registryHead(); r; r = r->next) {
    if (strcmp(r->variable, variable) == 0) {
      r->fn(NodrixValue(value));
      handled = true;
    }
  }
  if (!handled) NODRIX_LOG("[nodrix] unhandled control: %s\n", variable);
}

// The cloud won't queue control writes to a variable it has never seen. Seed
// each handled variable once so control works; re-echo real state on reconnect.
void NodrixClass::seedControlVars() {
  for (NodrixHandlerReg* r = registryHead(); r; r = r->next) {
    if (_ctrlState[r->variable].isNull()) _tx[r->variable] = false;
    else _tx[r->variable] = _ctrlState[r->variable];
  }
}

bool NodrixClass::isControlVar(const char* key) const {
  for (NodrixHandlerReg* r = registryHead(); r; r = r->next)
    if (strcmp(r->variable, key) == 0) return true;
  return false;
}

void NodrixClass::ackWs(const char* id) {
  JsonDocument doc;
  doc["type"] = "ack";
  doc["ids"].to<JsonArray>().add(id);
  String out;
  serializeJson(doc, out);
  _ws.sendTXT(out);
}

bool NodrixClass::validKey(const char* key) const {
  size_t n = strlen(key);
  if (n < 1 || n > (size_t)MAX_KEY_LEN) {
    NODRIX_LOG("[nodrix] key dropped (length): %s\n", key);
    return false;
  }
  return true;
}

// Keep the staged batch within the server's per-frame cap. Returns false if the
// buffer is full and can't be drained (disconnected) — caller drops the metric.
bool NodrixClass::ensureRoom() {
  if ((int)_tx.size() < MAX_POINTS) return true;
  flush();
  if ((int)_tx.size() < MAX_POINTS) return true;
  NODRIX_LOG("[nodrix] telemetry buffer full, dropping metric\n");
  return false;
}

void NodrixClass::send(const char* variable, bool value) {
  if (!validKey(variable) || !ensureRoom()) return;
  _tx[String(variable)] = value;
  if (isControlVar(variable)) _ctrlState[String(variable)] = value;
}

void NodrixClass::send(const char* variable, int value) { send(variable, (long)value); }

void NodrixClass::send(const char* variable, long value) {
  if (!validKey(variable) || !ensureRoom()) return;
  _tx[String(variable)] = value;
  if (isControlVar(variable)) _ctrlState[String(variable)] = value;
}

void NodrixClass::send(const char* variable, float value) { send(variable, (double)value); }

void NodrixClass::send(const char* variable, double value) {
  if (!validKey(variable) || !ensureRoom()) return;
  _tx[String(variable)] = value;
  if (isControlVar(variable)) _ctrlState[String(variable)] = value;
}

void NodrixClass::send(const char* variable, const char* value) {
  if (!validKey(variable) || !ensureRoom()) return;
  String v(value);
  if ((int)v.length() > MAX_STRING_VALUE) v = v.substring(0, MAX_STRING_VALUE);
  _tx[String(variable)] = v;
  if (isControlVar(variable)) _ctrlState[String(variable)] = v;
}

void NodrixClass::send(const char* variable, const String& value) {
  send(variable, value.c_str());
}

void NodrixClass::flush() {
  if (_tx.size() == 0) return;

  if (_wsMode) {
    if (!_connected) return;  // keep staged until the socket is up
    JsonDocument env;
    env["type"] = "telemetry";
    env["metrics"] = _tx.as<JsonObjectConst>();
    String out;
    serializeJson(env, out);
    _ws.sendTXT(out);
    _tx.clear();
  } else {
    JsonDocument env;
    env["metrics"] = _tx.as<JsonObjectConst>();
    String out;
    serializeJson(env, out);
    if (httpPost("/v1/telemetry", out) == 204) _tx.clear();
  }
}

void NodrixClass::event(const char* name) {
  JsonDocument doc;
  if (_wsMode) doc["type"] = "event";
  doc["event"] = name;
  String out;
  serializeJson(doc, out);
  if (_wsMode) _ws.sendTXT(out);
  else httpPost("/v1/events", out);
}

void NodrixClass::event(const char* name, JsonVariantConst payload) {
  JsonDocument doc;
  if (_wsMode) doc["type"] = "event";
  doc["event"] = name;
  doc["payload"] = payload;
  String out;
  serializeJson(doc, out);
  if (_wsMode) _ws.sendTXT(out);
  else httpPost("/v1/events", out);
}

int NodrixClass::httpPost(const char* path, const String& body) {
  NODRIX_MAKE_TLS(client);
  HTTPClient http;
  String url = "https://" + _host + ":" + String(_port) + path;
  if (!http.begin(client, url)) return -1;
  http.addHeader("Authorization", "Bearer " + _token);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  http.end();
  return code;
}

bool NodrixClass::httpGet(const char* path, String& out) {
  NODRIX_MAKE_TLS(client);
  HTTPClient http;
  String url = "https://" + _host + ":" + String(_port) + path;
  if (!http.begin(client, url)) return false;
  http.addHeader("Authorization", "Bearer " + _token);
  int code = http.GET();
  if (code == 200) out = http.getString();
  http.end();
  return code == 200;
}
