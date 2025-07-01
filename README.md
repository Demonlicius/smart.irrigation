# smart.irrigation

# Sensor Network with LoRa and ESP-NOW

Sensor network project using ESP32 and Arduino for the collection and transmission of environmental data.

## Components

### 1. Receiver-Arduino-LoRa  
End device that receives data via LoRa and stores it in a CSV file.

### 2. Main-Node-ESP32  
Intermediate node that collects data from sensor nodes via ESP-NOW and transmits it via LoRa.

### 3. Sensor-Node-ESP32  
Nodes equipped with sensors to measure soil temperature, soil moisture, and air humidity and temperature, and send data via ESP-NOW.
