/**
   PostHTTPClient.ino
    Created on: 21.11.2016
*/
//Wifi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <DHT.h>

/* this can be run with an emulated server on host:
        cd esp8266-core-root-dir
        cd tests/host
        make ../../libraries/ESP8266WebServer/examples/PostServer/PostServer
        bin/PostServer/PostServer
   then put your PC's IP address in SERVER_IP below, port 9080 (instead of default 80):
*/
//#define SERVER_IP "10.0.1.7:9080" // PC address with emulation on host
#define SERVER_IP "192.168.1.42"

#ifndef STASSID
#define STASSID "Snelyus"
#define STAPSK  "Kiril2003"
#endif

//Temperature sensor
#define DHTPIN D2 //pin gpio 2 in sensor
#define DHTTYPE DHT22 // DHT 22 Change this if you have a DHT11

DHT dht(DHTPIN, DHTTYPE);

float t, h;
unsigned long previousMillis = 0; // will store last temp was read
const long interval = 2000; // interval at which to read sensor

void setup() {
  //Setup for the Wifi
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  //Setup for the temperature sensor
  // Initialize device.
  dht.begin();
 
}

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor
    previousMillis = currentMillis;

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    h = dht.readHumidity(); // Read humidity (percent)
    t = dht.readTemperature(); // Read temperature as C
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

void readSensorData() {
  // Reading from DHT
  gettemperature();
  Serial.print("temp:");
  Serial.println(t);
  Serial.print("humi:");
  Serial.println(h);
}

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    readSensorData();

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://192.168.0.105:8000/api/device/postSensorData"); //HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");

    int httpCode = http.POST("{\"deviceKey\": \"device-kitchen\",\"sensorData\": {\"Temperature\": " +  String(t)+", \"Humidity\":"  + String(h) + "}, \"userid\": 1}");
    
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }


  delay(60000);
}
