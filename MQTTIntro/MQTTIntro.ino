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
const char* ssid = "YourRouter"; 
//WiFi password
const char* password = "YourWifiPassword"; 
// Computer's IP Address
const char* mqtt_server = "YourIP";
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

// Initialize the server on standard port
WebServer server(80); 

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
    Serial.print("Verbindung zum MQTT-Broker...");
    // Connection to broker
    // "ESP32Publisher" is the client ID
    // It identifies the device (ESP32)  to the broker.
    if (client.connect("ESP32Publisher")) {
      Serial.println("verbunden");
    } else {
      // Error handling
      Serial.print("Verbindungsfehler, rc=");
      Serial.print(client.state());
      delay(5000);
    }
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

  // Each 10 seconds
  if (now - lastMsg > 10000) {  
    lastMsg = now;

    // Reading sensor values
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
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


}
