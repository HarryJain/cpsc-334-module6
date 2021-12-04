/* CPSC 334 - Module 6 - ESP-NOW Remotes
   Date: 3 December 2021
   Author: Harry Jain <https://github.com/HarryJain>
   Purpose: ESP-NOW RSSI proximity measurement to trigger art installations with a ESP32 remote
   Description: This sketch consists of the code for the "spectre" remotes to interact with the various installations.
   References:
   a. https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/ESPNow/Basic
   b. https://github.com/arvindr21
   << This Remote >>
   Flow: Installation
   Step 1 : Initialize ESP-NOW on Installation and set it in STA mode
   Step 2 : Start scanning for ESP32 Remotes (we have added a prefix of `spectre` to the SSID of each Remote for easy setup)
   Step 3 : Once found, add Remotes as peers
   Step 4 : Register for send callback
   Step 5 : Start transmitting RSSI data from installation to nearby Remotes
   Flow: Remote
   Step 1 : Initialize ESP-NOW on Remote
   Step 2 : Update the SSID of Remote with a prefix of `spectre`
   Step 3 : Set Remote in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor
*/


// Include esp_now and WiFi libraries for mesh networking
#include <esp_now.h>
#include <WiFi.h>


#define BUZZ_LIMIT 50

// Set the channel for ESP-NOW
#define CHANNEL 1

// Control pin for the buzzer motor
#define BUZZER_PIN 13

// Set PWM properties
#define FREQ 5000
#define BUZZER_CHANNEL 0
#define RESOLUTION 8


// Store the number of remotes and the MAC addresses of the remotes in AP-mode to get the current remote index
#define REMOTE_COUNT 4
String devices[REMOTE_COUNT] = {"E8:68:E7:30:54:69", "E8:68:E7:30:61:4D", "E8:68:E7:30:5B:C9", "E8:68:E7:30:2A:A9"};
const char *names[REMOTE_COUNT] = {"Spectre_1", "Spectre_2", "Spectre_3", "Spectre_4"};
const char *passwds[REMOTE_COUNT] = {"Spectre_1_Password", "Spectre_2_Password", "Spectre_3_Password", "Spectre_4_Password"};
// Variables to store the device index and name
int remote_index = 0;

char current_installation[18];
int rssi_val = 0;
int buzz_val = 0;


// Initialize ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}


// Configure AP SSID
void configDeviceAP() {
  const char *SSID = names[remote_index];
  bool result = WiFi.softAP(SSID, passwds[remote_index], CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}


void setup() {
  // Set upload speed
  Serial.begin(115200);
  
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // Print the MAC address of this remote in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());

  // Resolve MAC address to device index
  for (size_t i = 0; i < REMOTE_COUNT; i++) {
    if (devices[i] == WiFi.softAPmacAddress()) {
      remote_index = i;
      Serial.print("Remote Index: "); Serial.println(remote_index);
    }
  }
  
  // Configure device AP mode
  configDeviceAP();
  
  // Initialize ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully initialized, register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);

  // Configure buzzer motor PWM functionalities
  ledcSetup(BUZZER_CHANNEL, FREQ, RESOLUTION);
  // Attach the channel to the GPIO to be controlled
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
}


// Callback when data is received from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println(WiFi.RSSI());
  Serial.println("");

  Serial.println(current_installation);

  // Check if the remote is close to the exhibit that sent data
  if (*data < BUZZ_LIMIT && *data > 0) {
    // If it is an update from the currently-connected installation, modify the rssi and buzz values accordingly
    if (macStr == current_installation) {
      rssi_val = *data;
      buzz_val = sqrt(abs(*data - BUZZ_LIMIT) * 1000);
    // If it is from another installation, update the rssi and buzz values and current installation only if it is closer than the current installation
    } else if (*data < rssi_val || buzz_val == 0) {
      Serial.println("Changing installation");
      rssi_val = *data;
      buzz_val = sqrt(abs(*data - BUZZ_LIMIT) * 1000);
      for (int i = 0; i < 17; i++) {
        current_installation[i] = macStr[i];
      }
      current_installation[17] = '\0';
    }
  // If the remote loses connection to the currently-connected installation, stop buzzing
  } else {
    bool current = true;
    for (int i = 0; i < 17; i++) {
      if (current_installation[i] != macStr[i]) {
        current = false;
      }
    }
    if (current) {
      rssi_val = *data;
      buzz_val = 0;
    }
  }

  // Write the current buzz value to the relevant pin
  ledcWrite(BUZZER_CHANNEL, buzz_val);
}


void loop() {
  // Chill
}
