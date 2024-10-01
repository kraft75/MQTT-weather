// For the WiFi connection 
#include <WiFi.h>
// The webserver
#include <WebServer.h>
// Librarys for DHT11 sensor
#include <DHT.h>
#include <DHT_U.h>
#include <DHT118266.h>
#include <Adafruit_Sensor.h>
// LCD 1602 Display Module...
#include <LiquidCrystal_I2C.h>
// ...including I2C 
#include <Wire.h>
// MQTT Protocoll
#include <PubSubClient.h>
// Http Protocol
#include <HTTPClient.h>
// JSON for data exchange
#include <ArduinoJson.h>


// Define DHT type and pin
#define DHTTYPE DHT11
#define DHTPIN 23

// DHT sensor object initialization
DHT dht(DHTPIN, DHTTYPE);
/* LCD settings*/
// Address 0x27 for a 16x2 LCD 
#define LCD_ADDRESS 0x27  // I2C-Adresse für LCD (kann variieren)
#define LCD_COLUMNS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

/* WiFi settings */
// WiFi name
const char* ssid = "YourWifiName"; 
//WiFi password
const char* password = "Your WiFiPWD"; 
// Computer's IP Address
const char* mqtt_server = "YourIPAddress";
// Establishing a TCP client which communicates over TCP/IP
// with other devices (MQTT broker) in the network
WiFiClient espClient;
// Associating the MQTT client (client) with the TCP client (espClient),
// enabling MQTT messages to be sent and received over the Wi-Fi connection
PubSubClient client(espClient);

// It stores the timestamp of the last message that was sent over MQTT.
// It  controls how frequently messages are published.
long lastMsg = 0;
//  It stores the message (temperature, humidity) that will be published to the MQTT broker.
char msg[50];
// Counter for messages
int value = 0;

// OpenWeatherMap API Settings
const char* apiKey = "YourKey";
const char* city = "YourCity";
const char* countryCode = "DE";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 30 seconds (30000)
unsigned long timerDelay = 30000;

// JSON object
String jsonBuffer;

// Global sensor variables
float temperature = 0; 
float humidity = 0; 


void setup() {
  // Initialize I2C and set SDA and SCL pins 
  Wire.begin(21, 22);
  
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Connecting to Wifi
  setup_wifi();

  // Telling the MQTT client which broker to connect to
  client.setServer(mqtt_server, 1883);

  // Print a welcome message
  lcd.setCursor(0, 0);
  lcd.print("Temp & Humid");
  Serial.println("Temp & Humid Monitor");
  delay(2000);  // Wait for 2 seconds

  // Initialize LCD
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.backlight();

  // Initialize DHT sensor
  dht.begin();
}

void setup_wifi() {
  // Connecting the ESP32 with the WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Signals the waiting process for the WiFi connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    ++attempts;
  }

  // Connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed.");
  }
}

void reconnect() {
  // Establishing and maintain a connection between your ESP32 and the MQTT broker
  // The loop ensures that the device (ESP32) keeps attempting to connect to the broker without crashing or exiting the loop prematurely.

  //Check MQTT Connection Status.
  while (!client.connected()) {
    String clientIdPublisher = "ESP32Publisher-" + String(WiFi.macAddress());
    Serial.print("Verbindung zum MQTT-Broker...");
    // Connection to broker
    // "ESP32Publisher" is the client ID
    // It identifies the device (ESP32)  to the broker.
    if (client.connect(clientIdPublisher.c_str())) {
      Serial.println("verbunden");
    } else {
      // Error handling
      Serial.print("Verbindungsfehler, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void sendTemperatureData() {
  // Reading sensor values
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
    
  // Error handling: Sensor's not working
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Fehler beim Lesen des Sensors!");
    return;
  }

  // Publishing temperature
  // The formatted string is stored in the msg character array.
  snprintf(msg, 50, "Temperatur: %.2f°C", temperature);
  Serial.print("Veröffentliche Nachricht: ");
  Serial.println(msg);
  // The temperature value is published to the MQTT topic
  client.publish("Umgebung/Temperatur", msg);

  // Publishing humidity
  snprintf(msg, 50, "Luftfeuchtigkeit: %.2f%%", humidity);
  Serial.print("Veröffentliche Nachricht: ");
  Serial.println(msg);
  client.publish("Umgebung/Luftfeuchtigkeit", msg);

}

String httpGETRequest(const char* serverName) {
  // Client for sending HTTP requesta
  HTTPClient http;

  // Sarting the HTTP Connection.
  // Your server name or URL 
  http.begin(serverName);

  // Send HTTP GET request
  // It returns an HTTP code showing the status 
  // of the request (e.g 200 for success):
  int httpResponseCode = http.GET();

  // Default response as empty object
  String payload = "{}"; 
  // If the request was successful the answer (JSON object) 
  // will be saved as a string
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Stopping the HTTP connection and freeing  resources
  http.end();

  return payload;  
}

void sendWeatherData() {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Checking WiFI connection 
    if(WiFi.status()== WL_CONNECTED) {
      String serverPath = String("http://api.openweathermap.org/data/2.5/weather?q=") + city + "," + countryCode + "&APPID=" + apiKey;

      // The httpGETRequest() function makes a request to OpenWeatherMap and it retrieves
      // a string with a JSON object that contains all the information about the weather 
      //for your city.
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);

      // Create a DynamicJsonDocument to hold the parsed JSON
      DynamicJsonDocument doc(2048); // Adjusting size

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, jsonBuffer);


      // Error handling
      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
      }

      // Access the parsed JSON data
      JsonObject myObject = doc.as<JsonObject>();

      
      Serial.print("JSON object = ");
      Serial.println(myObject);

      // Collect weather data
      float temperatureW = myObject["main"]["temp"].as<float>();  
      float pressure = myObject["main"]["pressure"].as<float>();  
      float humidityW = myObject["main"]["humidity"].as<float>();  
      float windSpeed = myObject["wind"]["speed"].as<float>();  


      // Build a JSON string to publish via MQTT
      float tempC = temperatureW - 273.15;
      String mqttMessage = "{\"temperature\": ";
      mqttMessage += String(tempC, 2);
      mqttMessage += ", \"pressure\": ";
      mqttMessage += String(pressure, 2);
      mqttMessage += ", \"humidity\": ";
      mqttMessage += String(humidityW, 2);
      mqttMessage += ", \"windSpeed\": ";
      mqttMessage += String(windSpeed, 2);
      mqttMessage += "}";

      // Publish the JSON string to the MQTT topic "Umgebung/Wetter"
      client.publish("Umgebung/Wetter", mqttMessage.c_str());
      Serial.print("Published to MQTT: ");
      Serial.println(mqttMessage);

    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void loop() {
  // Reconnect to MQTT broker if not connected
  if (!client.connected()) {
    reconnect();
  }

  //  Ensuring the MQTT client remains connected. It needs to be called frequently
  //  to keep the MQTT communication functioning smoothly.
  client.loop();

  long now = millis();

  // Send sensor data each 10 seconds
  if (now - lastMsg > 10000) {  
    lastMsg = now;
    sendTemperatureData();
    
    // Display on LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Humid: ");
    lcd.print(humidity);
    lcd.print(" ");
  }

  // Send weather data from the server
  sendWeatherData();
}
