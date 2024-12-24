from fastapi import FastAPI
from paho.mqtt import client as mqtt_client
from influxdb_client import InfluxDBClient, Point, WriteOptions
import asyncio
import json

# Create FastAPI app instance
app = FastAPI()

# Set up InfluxDB client with connection details (URL, token, organization)
# The write API is configured with a batch size of 1 for immediate writes
influx_client = InfluxDBClient(url="INFLUXDB_IP_AND_PORT"
                               , token="YOUR_TOKEN"
                               , org="YOUR_ORG")
write_api = influx_client.write_api(write_options=WriteOptions(batch_size=1))

# Variable to store the latest sensor data received from MQTT
latest_sensor_data = None

# Function to flatten nested JSON data into a simple key-value structure
def flatten_json(data, parent_key=''):
    items = {}
    for k, v in data.items():
        # Create a new key for each nested level, joining with underscores
        new_key = f"{parent_key}_{k}" if parent_key else k
        # If the value is a nested dictionary, recurse and flatten it
        if isinstance(v, dict):
            items.update(flatten_json(v, new_key))
        else:
            items[new_key] = v  # Add the flattened key-value pair
    return items

# Function to insert sensor data into InfluxDB as a Point object
# The sensor data is first flattened, then fields are added to the Point
# If the value can be cast to a float, it's stored as a float; otherwise, it's stored as a string
async def insert_data(sensor_data):
    flat_data = flatten_json(sensor_data)  # Flatten the sensor data
    point = Point("sensor_data")  # Create a new Point to hold the sensor data
    for key, value in flat_data.items():
        try:
            point = point.field(key, float(value))  # Attempt to store as float
        except ValueError:
            point = point.field(key, str(value))    # Store as string if not float
    # Write the Point to InfluxDB under the "Test" bucket
    write_api.write(bucket="Test", record=point)

# MQTT message handler function to process incoming messages from the broker
def on_message(client, userdata, msg):
    global latest_sensor_data
    try:
        # Parse the incoming message payload as JSON
        sensor_data = json.loads(msg.payload.decode())
        
        # Update the global variable with the latest sensor data
        latest_sensor_data = sensor_data

        # Insert the parsed data into InfluxDB asynchronously
        asyncio.run(insert_data(sensor_data))
    except Exception as e:
        # Print any errors encountered while parsing the message
        print(f"Error parsing message: {e}")

# MQTT client setup
mqtt_client = mqtt_client.Client()  # Create a new MQTT client instance
mqtt_client.on_message = on_message  # Set the on_message handler to process incoming messages
mqtt_client.username_pw_set("YOUR_MQTT_USERNAME", "YOUR_MQTT_PW")  # Set MQTT username and password
mqtt_client.connect("YOUR_MQTT_BROKER_IP", 1883)  # Connect to the MQTT broker
mqtt_client.subscribe("MQTT/TOPIC")  # Subscribe to the specified MQTT topic
mqtt_client.loop_start()  # Start the MQTT client loop (non-blocking)

# FastAPI endpoint to retrieve the latest sensor data
@app.get("/latest-data")
async def get_latest_data():
    # Return the latest sensor data or a message indicating no data has been received yet
    if latest_sensor_data:
        return {"sensor_data": latest_sensor_data}
    return {"message": "No data received yet."}

# FastAPI shutdown event to close the InfluxDB client when the app stops
@app.on_event("shutdown")
async def shutdown_event():
    influx_client.close()  # Close the InfluxDB client to clean up resources
