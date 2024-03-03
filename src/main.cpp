// receiver code
#include <esp_now.h>
#include <WiFi.h>

#include "Arduino.h"
#include "esp_camera.h"
#define CAMERA_MODEL_ESP32S3_EYE // Make sure to define the correct camera model
#include "camera_pins.h"
#include "ws2812.h"
#include "sd_read_write.h"

//FRAME_SIZE FRAMESIZE_QQVGA
#define WIDTH 160
#define HEIGHT 120    

// Define a structure for your message
typedef struct struct_message {
  int takePicture; // Signal to take a picture
} struct_message;

struct_message myData;

// Function to convert a pixel's value to a hex string (for grayscale images)
// This function needs to be adapted if working with actual RGB values
String toHex(uint8_t value) {
    char hexStr[3];
    sprintf(hexStr, "%02X", value);
    return String(hexStr);
}

// Function to process the image buffer, calculate, and store pixel hex values in a 1D array
String* processRGBImageToHex(const camera_fb_t *fb, size_t &len) {
    static String *hexValues = nullptr; // Static pointer to hold hex values between captures
    static size_t previousLen = 0; // Keep track of the previous allocation size

    uint8_t *buf = fb->buf;
    size_t pixelCount = fb->width * fb->height; // Calculate the total number of pixels

    // Check if we need to reallocate memory (only if the new image size is different)
    if (pixelCount != previousLen) {
        // Free previously allocated memory if it exists
        if (hexValues != nullptr) {
            delete[] hexValues;
            hexValues = nullptr;
        }

        // Allocate new memory for the current image
        hexValues = new String[pixelCount];

        // Update the tracking variable to the new length
        previousLen = pixelCount;
    }

    // Check if memory allocation was successful
    if (hexValues == nullptr) {
        Serial.println("Failed to allocate memory for hex values");
        return nullptr;
    }

    // Convert and copy each pixel's value into the array as a hex string
    for (size_t i = 0; i < pixelCount; i++) {
        hexValues[i] = toHex(buf[i]);
    }

    len = pixelCount; // Update the length to the actual number of processed pixels
    // Return the pointer to the array containing the hex values
    return hexValues;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Data received: ");
  Serial.println(len);
  if(myData.takePicture == 1){
    Serial.println("Triggering action to take picture.");
    ws2812SetColor(3); // Optional: Use an LED to indicate photo capture
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if (fb != NULL) {
      int photo_index = readFileNum(SD_MMC, "/camera");
      if(photo_index != -1) {
        String path = "/camera/" + String(photo_index) + ".jpg";
        writejpg(SD_MMC, path.c_str(), fb->buf, fb->len);
        Serial.println("Picture taken and saved.");
      }
      esp_camera_fb_return(fb);
    } else {
      Serial.println("Camera capture failed.");
    }
    size_t len = 19200;
    processRGBImageToHex(fb, len); // 160 pixels (width) * 120 pixels (height) = 19,200 pixels
    ws2812SetColor(2); // Reset LED color or turn off
    myData.takePicture = 0; // Reset the flag
  }
}


// cameraSetup remains unchanged, ensure it's configured for your specific camera model

int cameraSetup(void) {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // GRAYSCALE
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  // for larger pre-allocated frame buffer.
  if(psramFound()){
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return 0;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  s->set_vflip(s, 0); // flip it back
  s->set_brightness(s, 1); // up the brightness just a bit
  s->set_saturation(s, 0); // lower the saturation

  Serial.println("Camera configuration complete!");
  return 1;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  // Initialize SD card, WS2812 LED, and camera
  ws2812Init();
  sdmmcInit();
  createDir(SD_MMC, "/camera");
  listDir(SD_MMC, "/camera", 0);
  if(cameraSetup() == 1) {
    ws2812SetColor(2); // Indicate camera setup success
  } else {
    ws2812SetColor(1); // Indicate camera setup failure
    return; // Abort setup if camera initialization fails
  }
  
  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // No need to repeatedly check the flag in loop() since action is triggered directly in the callback
  delay(1000); // Just a delay to keep the loop from spinning too fast
}
