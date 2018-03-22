/*
   Function called to setup the connection to the Wi-Fi Access Point
*/

#include <ESP8266WiFi.h>    // https://github.com/esp8266/Arduino (GNUv2.1 licence)
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient (no licence)

#define DEBUG
#define TLS
#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wi-Fi: Access Point SSID and password
const char*       AP_SSID           = "[Redacted]";
const char*       AP_PASSWORD       = "[Redacted]";

// MQTT: client ID, broker IP address, port, username & password
const char*       MQTT_CLIENT_ID    = "[Redacted]";
const char*       MQTT_SERVER_IP    = "192.168.1.10";
#ifdef TLS
const uint16_t    MQTT_SERVER_PORT  = 8883;
#else
const uint16_t    MQTT_SERVER_PORT  = 1883;
#endif
const char*       MQTT_USERNAME     = "[Redacted]";
const char* MQTT_PASSWORD = "[Redacted]";

// TLS: The fingerprint of the MQTT broker certificate (SHA1)
#ifdef TLS
// openssl x509 -fingerprint -in  <certificate>.crt
const char*       CA_FINGERPRINT    = "[Redacted]";
// openssl x509 -subject -in  <certificate>.crt
const char*       CA_SUBJECT        = "[Redacted]";
#endif

// Fixed IP address: IP address, IP gateway, subnet, dns
const IPAddress   IP                (192, 168,   1, 102);
const IPAddress   IP_GATEWAY        (192, 168,   1,   1);
const IPAddress   IP_SUBNET         (255, 255, 255,   0);
const IPAddress   IP_DNS            (192, 168, 1, 1);

// WiFiFlientSecure instead of WiFiClient, for SSL/TLS support
#ifdef TLS
WiFiClientSecure  g_wifiClient;
#else
WiFiClient  g_wifiClient;
#endif
PubSubClient g_mqttClient(g_wifiClient);


void setupWifi() {
  delay(10);
  // attempt to connect to the Wi-Fi AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  // define the fixed IP address
  WiFi.config(IP, IP_GATEWAY, IP_SUBNET, IP_DNS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

#ifdef DEBUG
  Serial.println(F("INFO: Client is now connected to the Wi-Fi AP"));
  Serial.print(F("INFO: IP address: "));
  Serial.println(WiFi.localIP());
#endif

#ifdef TLS
  verifyFingerprint();
#endif
}

#ifdef TLS
void verifyFingerprint() {
  if (!g_wifiClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
#ifdef DEBUG
    Serial.println(F("ERROR: The connection failed to the secure MQTT server"));
#endif
    return;
  }

  if (g_wifiClient.verify(CA_FINGERPRINT, CA_SUBJECT)) {
#ifdef DEBUG
    Serial.println(F("INFO: The connection is secure"));
#endif
  } else {
#ifdef DEBUG
    Serial.println(F("ERROR: The given certificate does't match"));
#endif
  }
}
#endif

void reconnect() {
  while (!g_mqttClient.connected()) {
    if (g_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: The connection failed with the MQTT broker"));
      Serial.print("ERROR: rc: ");
      Serial.println(g_mqttClient.state());
      delay(5000);
#endif
    }
  }
}

void callback(char* p_topic, byte* p_payload, unsigned int p_length) {

}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println(F("\nINFO: The Wi-Fi module is starting..."));
#endif
  setupWifi();
}

void loop() {
  // keep the MQTT client connected to the broker
  if (!g_mqttClient.connected()) {
    reconnect();
  }
  g_mqttClient.loop();
}
