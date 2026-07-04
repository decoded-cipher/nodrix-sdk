// Battery node: wake, read a DHT11, report over HTTP, apply pending control, sleep.
// Needs the "DHT sensor library" by Adafruit (Library Manager).
#include <Nodrix.h>
#include <DHT.h>
#if defined(ESP32)
  #include <esp_sleep.h>
#endif

#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"
#define TOKEN     "your-project-token"

#define DHT_PIN  4
#define DHT_TYPE DHT11
#define SLEEP_SECONDS 300

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  dht.begin();
  Nodrix.beginHTTP(WIFI_SSID, WIFI_PASS, HOST, TOKEN);

  float tempC = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (!isnan(tempC)) Nodrix.send("temperature", tempC);
  if (!isnan(humidity)) Nodrix.send("humidity", humidity);
  Nodrix.flush();
  Nodrix.poll();

#if defined(ESP32)
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_SECONDS * 1000000ULL);
  esp_deep_sleep_start();
#else
  ESP.deepSleep((uint64_t)SLEEP_SECONDS * 1000000ULL);  // wire GPIO16 to RST to wake
#endif
}

void loop() {}
