# ESP32S3 EYE Camera Receiver Module README

## Overview

This repository contains the code for a receiver module using ESP-NOW protocol on an ESP32S3 EYE board, primarily aimed at capturing, processing, and storing images. It utilizes the ESP32 camera module for image capture, processes the image data to extract grayscale pixel values, and saves the images to an SD card. Additionally, it employs a WS2812 LED as a visual indicator for camera operations.

## Features

- **ESP-NOW Communication**: Receives commands wirelessly to trigger image capture.
- **Image Capture & Storage**: Utilizes the ESP32 camera to take photos and save them on an SD card.
- **Image Processing**: Processes captured images to extract and display grayscale pixel values.
- **Visual Feedback**: Uses a WS2812 LED to indicate the status of camera operations.

## Hardware Requirements

- ESP32S3 EYE board
- SD card for image storage
- WS2812 LED for visual feedback

## Software Requirements

- Arduino IDE or compatible ESP-IDF environment
- Required Libraries:
  - `esp_now.h` and `WiFi.h` for ESP-NOW communication
  - `esp_camera.h` for camera functionality
  - Custom libraries for WS2812 LED (`ws2812.h`) and SD card operations (`sd_read_write.h`)

## Installation

1. **Prepare the Environment**: Install the Arduino IDE or set up the ESP-IDF environment.
2. **Install Dependencies**: Ensure all required libraries are available in your development environment. This may involve downloading external libraries or configuring your environment to include them.
3. **Load the Sketch**: Open the provided code in your Arduino IDE or ESP-IDF project.
4. **Configure Camera Model**: Verify that the `CAMERA_MODEL_ESP32S3_EYE` define statement matches your ESP32 camera model.
5. **Compile and Upload**: Compile the code and upload it to your ESP32S3 EYE board.

## Usage

1. **Initial Setup**: Power on the ESP32S3 EYE board. The device initializes the SD card, WS2812 LED, and camera module. Successful initialization is indicated by a specific color on the WS2812 LED.
2. **Receiving Commands**: The module continuously listens for ESP-NOW messages.
3. **Taking Pictures**: Upon receiving a command with the `takePicture` flag set, the device captures an image, saves it to the SD card, processes it to extract grayscale values, and provides visual feedback via the WS2812 LED.
4. **Viewing Captured Images**: Access the SD card to view saved images and their corresponding grayscale value processing results.

## Function Descriptions

- `void processGrayscaleImage(const camera_fb_t *fb)`: Processes the image to calculate and display grayscale pixel values.
- `void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)`: Callback function for processing received ESP-NOW messages and triggering image capture.
- `int cameraSetup(void)`: Initializes the camera with predefined settings.
- `void setup()`: Initializes components and registers the ESP-NOW receive callback.
- `void loop()`: Main loop with minimal activity, primarily waiting for incoming ESP-NOW messages.

## Notes

- Ensure the camera model defined matches your hardware.
- The camera setup may need adjustments based on specific requirements or hardware variations.
- The ESP-NOW communication range and reliability depend on environmental conditions and hardware setup.
- This code is designed for demonstration purposes and may require optimization for production use.

For detailed documentation on the ESP32 camera module, ESP-NOW protocol, and WS2812 LED configuration, refer to the respective official documentation and libraries.
