This project clarifies the communication procedure in a IoT network via MQTT.
An ESP32 device functions as a client publisher sending sensor data (temperature, humidity)
to the broker. It also requests via HTTP weather data from penweathermap.org and sends it additional
to the broker with a different topic.
Another ESP32 subscribes the data from the broker

This project demonstrates the basic principal of an MQTT based network.

It is well commented in order to motivate beginners to copy the code for learning purposes.
