This project demonstrates the communication process in an IoT network using MQTT. One ESP32 device acts as a client publisher, sending sensor data (temperature and humidity) to the broker. Additionally, it retrieves weather data via HTTP from OpenWeatherMap.org and publishes it to the broker under a different topic.

Another ESP32 device subscribes to these data topics from the broker.

For detailed instructions on how to retrieve data from the internet, please refer to this website:
Random Nerd Tutorials - ESP32 HTTP GET OpenWeatherMap

This project illustrates the fundamental principles of an MQTT-based network. It is thoroughly commented to encourage beginners to experiment with the code for learning purposes.
