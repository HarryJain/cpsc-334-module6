/* CPSC 334 - Module 6 - ESP-NOW Installation Template
   Date: 3 December 2021
   Author: Harry Jain <https://github.com/HarryJain>
   Purpose: ESP-NOW RSSI proximity measurement to trigger art installations with a ESP32 remote
   Description: This sketch consists of the template for installations that are triggered by the remotes.
   References:
   a. https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/ESPNow/Basic
   b. https://github.com/arvindr21
   << This Remote >>
   Flow: Installation
   Step 1 : Initialize ESP-NOW on Installation and set it in STA mode
   Step 2 : Start scanning for ESP32 Remotes (we have added a prefix of `Spectre` to the SSID of each Remote for easy setup)
   Step 3 : Once found, add Remotes as peers
   Step 4 : Register for send callback
   Step 5 : Start transmitting RSSI data from installation to nearby Remotes
   Flow: Remote
   Step 1 : Initialize ESP-NOW on Remote
   Step 2 : Update the SSID of Remote with a prefix of `Spectre`
   Step 3 : Set Remote in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, set the vibration accordingly
*/

// Include esp_now and WiFi libraries for mesh networking
#include <esp_now.h>
#include <WiFi.h>


// Set the maximum RSSI that starts the installation
#define RSSI_LIMIT 40
// LED for indicating the start of an installation
#define LED 14


// Connection and debugging constants
#define CHANNEL 1
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0


// Global constant for the total number of remotes
#define REMOTE_COUNT 4

// Global list of remote objects
esp_now_peer_info_t remotes[REMOTE_COUNT];

// MAC addresses of remote ESP32s
String devices[REMOTE_COUNT] = {"E8:68:E7:30:54:69", "E8:68:E7:30:61:4D", "E8:68:E7:30:5B:C9", "E8:68:E7:30:2A:A9"};


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


// Global RSSI list to store proximity to each remote
uint8_t rssis[REMOTE_COUNT] = {0, 0, 0, 0};

// Scan for remotes in AP mode
void ScanForRemotes() {
  int8_t scanResults = WiFi.scanNetworks();
  
  // Reset on each scan
  bool remoteFound = 0;
  bool remoteClose = 0;
  for (int i = 0; i < REMOTE_COUNT; i++) {
    memset(&remotes[i], 0, sizeof(remotes[i]));
  }
  Serial.println("");

  if (scanResults == 0) {
    Serial.println("No WiFi devices in AP Mode found");
  } else {
    Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      if (PRINTSCANRESULTS) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      }
      delay(10);
      
      // If the current device starts with `Spectre`, it is a remote
      if (SSID.indexOf("Spectre") == 0) {
        int remote_index = 0;
        
        // SSID of interest
        Serial.println("Found a Remote.");
        Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");

        // Resolve MAC address to device index
        for (int j = 0; j < REMOTE_COUNT; j++) {
          if (BSSIDstr == devices[j]) {
            remote_index = j;
            break;
          }
        }
        
        // Get BSSID => Mac Address of the Remote
        int mac[6];
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            remotes[remote_index].peer_addr[ii] = (uint8_t) mac[ii];
          }
        }

        // Store the absolute value of the RSSI in the according index of the array
        rssis[remote_index] = abs(WiFi.RSSI(i));

        // Pick a channel for connecting to the remote
        remotes[remote_index].channel = CHANNEL;

        // Do not encrypt the ESP Now connection
        remotes[remote_index].encrypt = 0;

        // Signal that a remote has been found
        remoteFound = 1;
      }
    }
  }

  // If remotes are found, trigger an installation if any are close
  if (remoteFound) {
    Serial.println("Remote Found, processing..");
    for (int i = 0; i < REMOTE_COUNT; i++) {
      if (rssis[i] < RSSI_LIMIT && rssis[i] > 0) {
        Serial.println("Start installation");
        digitalWrite(LED, HIGH);
        // PUT INSTALLATION LOOPING CODE HERE
        /*
         * 
         */
        // Signal that a device is in triggering range
        remoteClose = 1;
        break;
      }
    }
  } else {
    Serial.println("Remote Not Found, trying again.");
  }

  // Turn off the LED if no remotes are in triggering range
  if (remoteClose == 0) {
    digitalWrite(LED, LOW);
  }

  // Clean up WiFi scan RAM
  WiFi.scanDelete();
}


// Check if the remotes are already paired with the installation
//  If not, pair them
void manageRemotes(bool paired[]) {
  // Loop through all the remotes and pair them if they are unpaired
  for (int i = 0; i < REMOTE_COUNT; i++) {
    if (remotes[i].channel == CHANNEL) {
      if (DELETEBEFOREPAIR) {
        deletePeer(i);
      }
  
      Serial.print("Remote Status: ");
      // Check if the peer exists
      bool exists = esp_now_is_peer_exist(remotes[i].peer_addr);
      if (exists) {
        // Remote already paired.
        Serial.println("Already Paired");
        paired[i] = true;
      } else {
        // Remote not paired, attempt pair
        esp_err_t addStatus = esp_now_add_peer(&remotes[i]);
        if (addStatus == ESP_OK) {
          // Pair success
          Serial.println("Pair success");
          paired[i] = true;
        } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
          // How did we get so far!!
          Serial.println("ESPNOW Not Init");
          paired[i] = false;
        } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
          Serial.println("Invalid Argument");
          paired[i] = false;
        } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
          Serial.println("Peer list full");
          paired[i] = false;
        } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
          Serial.println("Out of memory");
          paired[i] = false;
        } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
          Serial.println("Peer Exists");
          paired[i] = true;
        } else {
          Serial.println("Not sure what happened");
          paired[i] = false;
        }
      }
    } else {
      // No remotes found to process
      // Serial.println("No remote found to process");
      paired[i] = false;
    }
  }
}


void deletePeer(int remote_index) {
  esp_err_t delStatus = esp_now_del_peer(remotes[remote_index].peer_addr);
  Serial.print("Remote Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println("Success");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}


// Send RSSI data to remote with remote_index
void sendData(int remote_index) {
  const uint8_t *peer_addr = remotes[remote_index].peer_addr;
  Serial.print("Sending: "); Serial.println(rssis[remote_index]);
  esp_err_t result = esp_now_send(peer_addr, &rssis[remote_index], sizeof(rssis[remote_index]));
  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW not Init.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}


// Callback when data is sent from installation to remote
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println(WiFi.RSSI());
}


void setup() {
  Serial.begin(115200);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  
  // This is the mac address of the pupper show in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  
  // Initialize ESPNow with a fallback logic
  InitESPNow();
  
  // Once ESPNow is successfully initialized, we will register for Send CB to
  //  get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  pinMode(LED, OUTPUT);
  // PUT INSTALLATION SETUP CODE HERE
  /*
   * 
   */
}

void loop() {
  // In the loop we scan for remotes
  ScanForRemotes();
  
  // Add remotes as peers if they have not been added already
  bool isPaired[REMOTE_COUNT] = {0};
  manageRemotes(isPaired);

  for (int i = 0; i < REMOTE_COUNT; i++) {
    if (isPaired[i]) {
      // If pair success or already paired
      // Send data to device
      sendData(i);
    } else {
      // Remote pair failed
      // Serial.print("Remote "); Serial.print(i); Serial.print("pair failed!");
    }
  }

  // wait for 1ms to run the logic again
  delay(1);
}
