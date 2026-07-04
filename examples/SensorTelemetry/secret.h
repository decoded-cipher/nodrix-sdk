#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-password"
#define HOST      "yourproject.workers.dev"
#define TOKEN     "your-project-token"

// Root CA (PEM) for your host — fetch it with openssl (see README > TLS).
const char* NODRIX_ROOT_CA = R"CERT(
-----BEGIN CERTIFICATE-----
...paste the root CA here...
-----END CERTIFICATE-----
)CERT";
