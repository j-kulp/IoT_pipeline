#include <WiFi.h>  // WiFi library for ESP32
#include <PubSubClient.h>  // MQTT client library for ESP32
#include <ArduinoJson.h>  // Library for JSON handling
#include <Adafruit_BME280.h>  // BME280 sensor library for reading temperature, humidity, and pressure

// Define the device name and sensor name
const char* device = "DEVICE_NAME";
const char* sensorName = "BME280";

// WiFi credentials for connecting to the local network
const char* ssid = "YOUR_WIFI";  // WiFi SSID
const char* password = "WIFI_PW";  // WiFi password

// MQTT server details (replace with your actual server details)
const char* mqtt_server = "IP_ADDRESS";  // MQTT broker IP address or domain name
const int mqtt_port = 1883;  // Default MQTT port (1883 for non-secure connection)
const char* mqttUser = "MQTT_USERNAME";  // MQTT username (if applicable)
const char* mqttPassword = "MQTT_PW";  // MQTT password (if applicable)
const char* mqtt_topic = "sensor/data";  // Topic where the sensor data will be published

// BME280 sensor SPI interface pins (configured for ESP32)
#define BME_CS 5    // Chip select pin (green wire)
#define BME_SCK 18  // Clock pin (yellow wire)
#define BME_MOSI 23 // Master Out Slave In pin (blue wire)
#define BME_MISO 19 // Master In Slave Out pin (orange wire)

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);  // SPI connection setup

WiFiClient espClient;  // Create an instance of WiFiClient
PubSubClient client(espClient);  // Create an instance of PubSubClient for MQTT

// Variables for timing and error handling
unsigned long lastPublishTime = 0;  // Tracks the last publish time
const unsigned long publishInterval = 10000;  // Interval between publishes (10 seconds)

void setup_wifi() {
  delay(10);  // Short delay before starting Wi-Fi connection
  Serial.print("Connecting to Wi-Fi...");  // Notify user the ESP32 is attempting to connect

  WiFi.begin(ssid, password);  // Start connecting to Wi-Fi network

  // Wait until the ESP32 is connected to the Wi-Fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  // Wait 500 ms before checking again
    Serial.print(".");  // Print dots while connecting
  }

  // Once connected, display the local IP address
  Serial.println("Wi-Fi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until the MQTT client is connected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Try to connect to the MQTT broker
    if (client.connect("Client", mqttUser, mqttPassword)) {
      Serial.println("connected");  // Successfully connected to MQTT broker
      // You can subscribe to topics or perform actions after connection
    } else {
      // Failed to connect, print error and retry after 5 seconds
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud rate
  bme.begin();  // Initialize the BME280 sensor
  setup_wifi();  // Connect to Wi-Fi
  client.setServer(mqtt_server, mqtt_port);  // Set the MQTT server details
}

void loop() {
  // Ensure the MQTT client is connected, otherwise attempt reconnection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Keep the MQTT client processing any incoming messages

  unsigned long currentMillis = millis();  // Get the current time in milliseconds

  // Check if it's time to publish data (every 10 seconds in this case)
  if (currentMillis - lastPublishTime >= publishInterval) {
    lastPublishTime = currentMillis;  // Update the last publish time

    // Read data from the BME280 sensor
    float humidity = bme.readHumidity();
    float temperature = bme.readTemperature();
    float pressure = (bme.readPressure() / 100.0F);  // Convert pressure from Pa to hPa

    // Check if the sensor readings are valid
    if (isnan(humidity) || isnan(temperature) || isnan(pressure)) {
      Serial.println("Failed to read from BME sensor!");  // Print error message if reading fails
      return;  // Skip publishing if the sensor data is invalid
    }

    // Create a JSON object to store the sensor data and metadata
    StaticJsonDocument<200> doc;
    doc["deviceID"] = device;  // Device name or ID
    doc["sensorName"] = sensorName;  // Sensor name (e.g., BME280)
    doc["temperature"] = temperature;  // Temperature data
    doc["humidity"] = humidity;  // Humidity data
    doc["pressure"] = pressure;  // Pressure data

    // Serialize the JSON object into a character buffer
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);  // Convert the JSON object to a string

    // Publish the sensor data to the MQTT broker
    Serial.print("Publishing message: ");
    Serial.println(jsonBuffer);  // Print the message to serial for debugging
    if (client.publish(mqtt_topic, jsonBuffer)) {
      Serial.println("Message published successfully!");  // Notify if the message was successfully published
    } else {
      Serial.println("Failed to publish message.");  // Notify if the publish fails
    }
  }
}
