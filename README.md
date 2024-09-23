# Treadmill Tracker

This project is an ESP32-based tracker that calculates the running distance using a treadmill's magnetic sensor and sends logs.

## Features

- Measures running distance using a magnet attached to the treadmill's flywheel and a magnetic sensor
- Sends measurement data to a specified URL via HTTP POST (supports HTTPS as well)
- Tested to work with ESP32 development kits
- Easy to build with Arduino IDE

## Treadmills Tested

- ALINCO EXW8023

## Requirements

- ESP32 development kit

  - Example -> example

- Treadmill with built-in magnet and magnetic sensor

  - It is also possible to attach a magnet and a magnetic sensor later with some ingenuity

## How to Use

1. Configure the board settings for the ESP32 development kit, etc., in the Arduino IDE.
1. Open treadmill-tracker/treadmill-tracker.ino in the Arduino IDE.
1. Refer to treadmill-tracker/credentials.example and create treadmill-tracker/credentials.h. Fill in the Wi-Fi settings, logging API URL, and authentication information.
1. Build and upload the code to the ESP32 device.
1. You can check the operation status in the Serial Monitor.

## Configuration

The running distance per flywheel rotation is set by MILLIMETER_PAR_STEP in treadmill-tracker.ino. Set the value calculated by actual measurement on the treadmill you are using.

```
// treadmill-tracker.ino : Line 25
#define MILLIMETER_PAR_STEP 134.0
```

## Data Transmission

Measurement data is sent in JSON format to the specified URL via HTTP POST. It is sent approximately every 60 seconds, but the accuracy of the time interval is not high. A timestamp is also sent, but the ESP32 timestamp is unreliable, so it is recommended to check the time on the logging API side.

```
{"esp32":{"ts":1234567,"distance":123.456}}
```

- ts: Epoch time (seconds)
- distance: Running distance (meters) from when the ESP32 was powered on to the present

## Disclaimer

This project was developed as a personal hobby project and does not provide any warranty. Use at your own risk.

## Rights

All rights, including copyright, are reserved by u-masao.

## GitHub Repository

https://github.com/u-masao/esp32-treadmill-tracker
