#include <WiFi.h>
#include <PubSubClient.h>

/* WiFi settings */
// WiFi name
const char* ssid = "YourWiFIName"; 
//WiFi password
const char* password = "YourPassword"; 
// Computer's IP Address
const char* mqtt_server = "YourIP";

// Establishing a TCP client which communicates over TCP/IP
// with other devices (MQTT broker) in the network
WiFiClient espClient;
// Associating the MQTT client (client) with the TCP client (espClient),
// enabling MQTT messages to be sent and received over the Wi-Fi connection
PubSubClient client(espClient);


void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Connecting to Wifi
  setup_wifi();

  // Telling the MQTT client which broker to connect to
  client.setServer(mqtt_server, 1883);

  // Callback function triggered 
  client.setCallback(callback);

}

void callback(char* topic, byte* payload, unsigned int length) {
  // This function s triggered whenever a message is 
  // received on a subscribed topic. 

  // Print the received message details to the Serial Monitor
  Serial.print("Nachricht empfangen [");  
  // Print the topic the message was received on
  Serial.print(topic);  
  Serial.print("]: ");

  // Loop through the payload (the message content as array of bytes)
  for (int i = 0; i < length; i++) {
    // Convert each byte of the payload to a char and print it
    Serial.print((char)payload[i]);
  }

  // Print a newline after the message has been fully printed
  Serial.println();
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
  // Establish connection between subscriber and broker
  while (!client.connected()) {
    Serial.print("Verbindung zum MQTT-Broker...");
    // Identifying the device (ESP32) to the broker.
    if (client.connect("ESP32Subscriber")) {
      Serial.println("verbunden");
      client.subscribe("Umgebung/Temperatur");
      client.subscribe("Umgebung/Luftfeuchtigkeit");
    } else {
      Serial.print("Verbindungsfehler, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
