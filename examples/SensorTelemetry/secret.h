#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"
#define TOKEN     "your-project-token"

// Root CA to pin (optional; see README > TLS).
const char* NODRIX_ROOT_CA = R"CERT(
-----BEGIN CERTIFICATE-----
...paste your host's root CA here...
-----END CERTIFICATE-----
)CERT";
