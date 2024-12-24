# ESP32 MQTT, FastAPI, InfluxDB and Grafana Data Pipeline

This project involves using an **ESP32** to collect sensor data (e.g., from DHT11 or BME280 sensors), send it to an MQTT broker, and insert the data into an **InfluxDB** database. 
The data is visualized in **Grafana** and can be tested via a **FastAPI** endpoint.

The project divides into two sections: the **ESP32 MQTT publisher** (```ESP32_MQTT_BM.ino```), and the **MQTT subscriber that pushes to InfluxDB** (```app.py```)

![images/pipeline-diagram.png](https://github.com/j-kulp/portfolio/blob/e5d8aaf0b053426ca691c5e1b8806b83e4c09623/images/flow.png)

I suggest following the order of installation here, starting with the ESP32 before finally moving on to the MQTT subscriber section.

## Requirements

### Hardware
- **ESP32 microcontroller**
  - **Sensors** like DHT11 or BME280 for environmental data
- **MQTT Broker** (can be a cloud or local broker, e.g., Mosquitto)

### Software & Libraries

**ESP32**:
Arduino IDE or PlatformIO
Libraries:
  - **WiFi** (comes with the ESP32 Arduino core)
  - **PubSubClient** (for MQTT communication)
  - **ArduinoJson** (for JSON handling)
  - **Adafruit BME280** (for sensor data collection)

**Backend**:
**Python 3.7+**
- **FastAPI** (for serving the API)
- **InfluxDB** (for storing sensor data)
- **paho-mqtt** (for connecting to the MQTT broker)
- **InfluxDB Client** (for writing data to InfluxDB)

**Frontend**:
- **Grafana** (for visualization)

  
=====================================================================

=====================================================================

## Installation and Setup - ESP32 with C++

### Hardware Requirements
- ESP32 development board (e.g., ESP32 DevKit v1)
- BME280 sensor (can be connected via SPI)
- Jumper wires for connections
- Power supply for the ESP32


#### Wiring Diagram

|ESP32 Pin  |	BME280 Pin | Description |
|-----------|------------|---------|
|GPIO 5	| CS (Chip Select) |	SPI Chip Select |
|GPIO 18 | SCK (Clock) |	SPI Clock |
|GPIO 23 |	MOSI	| SPI MOSI (Master Out Slave In) |
|GPIO 19 | MISO |	SPI MISO (Master In Slave Out) |

Note: The ESP32 uses SPI to communicate with the BME280 sensor in this project. Ensure the wiring matches the above table.

### Software Requirements
Arduino IDE (or PlatformIO)
Libraries:
  - **WiFi** (comes with the ESP32 Arduino core)
  - **PubSubClient** (for MQTT communication)
  - **ArduinoJson** (for JSON handling)
  - **Adafruit BME280** (for sensor data collection)

You can install the necessary libraries through the Arduino Library Manager or by using the Library Manager in the Arduino IDE.

### Installation and Setup

### 1: Install Arduino IDE and Configure ESP32
- Install the **Arduino IDE**.
- Install the ESP32 board support in the Arduino IDE:
  - Open Arduino IDE
  - Go to File > Preferences and add the following URL to the "Additional Boards Manager URLs" field:
    ``` https://dl.espressif.com/dl/package_esp32_index.json ```
  - Go to Tools > Board > Boards Manager, search for ESP32, and install it.

### 2: Install Required Libraries
  - Open Arduino IDE > Sketch > Include Library > Manage Libraries.
  - Install the following libraries:
    - **WiFi** (for connecting the ESP32 to a Wi-Fi network)
    - **PubSubClient** (for MQTT communication)
    - **ArduinoJson** (for JSON data handling)
    - **Adafruit BME280** (for reading data from the BME280 sensor)

### 3: Upload the Code to the ESP32
- Connect the ESP32 board to your computer via USB.
- Open the provided code in the Arduino IDE.
- Configure the Wi-Fi and MQTT settings:
  - Set your Wi-Fi SSID and password in the code.
  - Set your MQTT broker's IP_ADDRESS, username, and password.
- Select the appropriate Port and ESP32 board from Tools > Board.
- Upload the code to the ESP32.

### 4: Monitor the Serial Output
- Open the Serial Monitor in Arduino IDE to view connection status and data being published to the MQTT broker.
- Set the baud rate to 115200 for correct serial communication.

### 5. MQTT Setup
You can use any MQTT broker, such as Mosquitto, HiveMQ, or a cloud-based service like AWS IoT or ThingSpeak. I used Mosquitto.
The default topic where the sensor data is published is ```sensor/data```, but you can change this in the code.

Example MQTT Message:

``` json
{
  "deviceID": "DEVICE_NAME",
  "sensorName": "BME280",
  "temperature": 25.3,
  "humidity": 60.5,
  "pressure": 1012.3
}
```

### Customizing the Code
- **Device Name**: Update the device variable to your preferred device name.
- **Sensor Type**: The current code is configured for a BME280 sensor, but it can easily be adapted to work with other sensors by modifying the sensorName and sensor-specific code.
- **Publish Interval**: Adjust the publishInterval to control how often data is sent to the MQTT broker.

### Troubleshooting

- **ESP32 Not Connecting to Wi-Fi**: 
  - Double-check the SSID and password in the code.
  - Ensure your ESP32 is within range of the Wi-Fi network and has adequate power.
  
- **MQTT Connection Issues**:
  - Ensure the MQTT broker is running and accessible.
  - Verify the MQTT username and password are correct.
  - Check the MQTT topic name to ensure the ESP32 is publishing to the correct one.

- **Sensor Reading Failures**:
  - Ensure the BME280 sensor is correctly wired.
  - Verify the sensor is powered properly (3.3V and GND).
  - Check for any loose connections or wiring issues.


=====================================================================

=====================================================================

## Installation and Setup - Backend/Laptop in Python

### 1. Install Mosquitto MQTT Broker

Mosquitto is an open-source MQTT broker that you can run locally or on a server to handle messages between the ESP32 and the FastAPI server. Here's how to install and start the service:

- **Download and install Mosquitto**:
  - You can download the latest version of Mosquitto from the official site: [Mosquitto Download](https://mosquitto.org/download/).
  - Follow the installation instructions for your operating system.

- **Start the Mosquitto MQTT Broker**:
  Once Mosquitto is installed, open a terminal/command prompt and run the following command to start the broker:

  ```bash
  net start mosquitto
  ```
  This command starts the Mosquitto MQTT broker on your local machine.

### 2. Install InfluxDB
InfluxDB is a time-series database that will store the sensor data from the ESP32. To set up InfluxDB, follow these steps:

- **Download and install InfluxDB**:
  - Download the latest version of InfluxDB from the official site: [InfluxDB Download](https://www.influxdata.com/influxdata-downloads/)
  - Follow the installation instructions for your operating system.

- Start the InfluxDB service: Navigate to the directory where InfluxDB is installed and run the following command to start InfluxDB:

  ``` bash
  cd C:\Program Files\InfluxData\influxdb
  influxd
  ```
  
  This starts the InfluxDB service, and it will listen for incoming requests on the default port (8086).
  You can access the InfluxDB service and dashboard at http://localhost:8086

### 3. Install and Run FastAPI Server
The FastAPI server handles the incoming MQTT messages, processes them, and writes the data to InfluxDB.

- Install FastAPI and dependencies: Install FastAPI and the necessary libraries using pip:

``` bash
pip install fastapi paho-mqtt influxdb-client uvicorn
```

- Set up the FastAPI project: Clone or create a directory for your FastAPI app and save the provided Python code (e.g., app.py) inside the project folder.
  - Modify the code to match your MQTT and InfluxDB configurations (e.g., MQTT username/password, InfluxDB token).

- Start the FastAPI server: In the FastAPI project directory, run the following command to start the server using Uvicorn:

``` bash
cd C:\Users\...\MQTT
uvicorn app:app --reload
```

This will start the FastAPI server on http://localhost:8000.

### 4. Install and Run Grafana
Grafana is used to visualize the sensor data stored in InfluxDB.

- Download and install Grafana:
  - Download the latest open-source version of Grafana from the official site [Grafana Download](https://grafana.com/grafana/download?pg=oss-graf&plcmt=resources&edition=oss)
  - Follow the installation instructions for your operating system.

- Start the Grafana service: After installing Grafana, navigate to the installation directory and run the following command:

``` bash
cd C:\Program Files\GrafanaLabs\grafana\bin
grafana-server.exe
```

This will start the Grafana server, and you can access the Grafana dashboard at http://localhost:3000.

- Log in to Grafana: Use the default credentials (admin/admin) to log in to Grafana. You will be prompted to change the password.

- Add InfluxDB as a data source: Once logged in to Grafana, follow these steps to add InfluxDB as a data source:
  - Go to Configuration > Data Sources > Add data source.
  - Select InfluxDB from the list.
  - Set the URL to http://localhost:8086.
  - Provide your InfluxDB Token, and choose the bucket (Test or the bucket name you created in InfluxDB).
  - Click Save & Test to verify the connection.

- Create a dashboard in Grafana: After adding InfluxDB as a data source, you can create a dashboard to visualize your sensor data:
  - Go to Create > Dashboard and add new panels to display metrics such as temperature, humidity, etc.

### Troubleshooting
#### Common Issues:
- MQTT connection issues:
  - Security may flag 64b install, try 32b.
  - Verify that your broker IP, username, and password are correctly configured.
- InfluxDB connection errors: Double-check the InfluxDB URL, token, and bucket settings.
- FastAPI server errors: Check the logs for details, ensure all dependencies are installed, and make sure the correct IP addresses/ports are configured.

## Results
You should have something that starts to resemble this!

![images/pipeline-diagram.png](https://github.com/j-kulp/portfolio/blob/e5d8aaf0b053426ca691c5e1b8806b83e4c09623/images/grafana.png)


=====================================================================

=====================================================================

## License
This project is licensed under the MIT License - see the LICENSE file for details.
