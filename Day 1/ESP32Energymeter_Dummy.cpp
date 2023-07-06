#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
// #include <SoftwareSerial.h>

#else
#ifdef ESP32
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif // ESP32
#endif // ESP8266


// Sending data can either be done over MQTT and the PubSubClient
// or HTTPS and the HTTPClient, when using the ESP32 or ESP8266
#define USING_HTTPS false

// Whether the given script is using encryption or not,
// generally recommended as it increases security (communication with the server is not in clear text anymore),
// it does come with an overhead tough as having an encrypted session requires a lot of memory,
// which might not be avaialable on lower end devices.
#define ENCRYPTED false


#if USING_HTTPS
#include <ThingsBoardHttp.h>
#else
#include <ThingsBoard.h>
#endif


// PROGMEM can only be added when using the ESP32 WiFiClient,
// will cause a crash if using the ESP8266WiFiSTAClass instead.

constexpr char WIFI_SSID[] = "estheim@TP_3020";
constexpr char WIFI_PASSWORD[] = "1sampai10";


// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token

constexpr char TOKEN[] = "yhgdpxsb02zxdzlblfm4";

// Thingsboard we want to establish a connection too

constexpr char THINGSBOARD_SERVER[] = "103.175.217.120";


#if USING_HTTPS
// HTTP port used to communicate with the server, 80 is the default unencrypted HTTP port,
// whereas 443 would be the default encrypted SSL HTTPS port
#if ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 443U;
#else
constexpr uint16_t THINGSBOARD_PORT = 80U;
#endif
#else
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port,
// whereas 8883 would be the default encrypted SSL MQTT port
#if ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 8883U;
#else
constexpr uint16_t THINGSBOARD_PORT = 1883U;
#endif
#endif


// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint32_t MAX_MESSAGE_SIZE = 128U;

// Baud rate for the debugging serial connection
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;


constexpr char VOLTAGE_KEY[] = "voltage";
constexpr char CURRENT_KEY[] = "current";


// Initialize underlying client, used to establish a connection
#if ENCRYPTED
WiFiClientSecure espClient;
#else
WiFiClient espClient;
#endif
// Initialize ThingsBoard instance with the maximum needed buffer size
#if USING_HTTPS
ThingsBoardHttp tb(espClient, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT);
#else
ThingsBoard tb(espClient, MAX_MESSAGE_SIZE);
#endif


/// @brief Initalizes WiFi connection,
// will endlessly delay until a connection has been successfully established
void InitWiFi() {

  Serial.println("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been succesfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");

}

/// @brief Reconnects the WiFi uses InitWiFi if the connection has been removed
/// @return Returns true as soon as a connection has been established again
bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void setup() {
  // If analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  // Initalize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(1000);
  InitWiFi();
}

void loop() {
  delay(1000);

  if (!reconnect()) {
    return;
  }

#if !USING_HTTPS
  if (!tb.connected()) {
    // Reconnect to the ThingsBoard server,
    // if a connection was disrupted or has not yet been established
    Serial.printf("Connecting to: (%s) with token (%s)\n", THINGSBOARD_SERVER, TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {

      Serial.println("Failed to connect");

      return;
    }
  }
#endif

  // Uploads new telemetry to ThingsBoard using HTTP.
  // See https://thingsboard.io/docs/reference/http-api/#telemetry-upload-api
  // for more details
  Serial.println("Sending data...");
  tb.sendTelemetryFloat(VOLTAGE_KEY, random(210,225));
  tb.sendTelemetryFloat(CURRENT_KEY, random(1,10));

#if !USING_HTTPS
  tb.loop();
#endif
}