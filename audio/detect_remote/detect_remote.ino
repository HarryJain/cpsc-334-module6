// Include esp_now and WiFi libraries for mesh networking
#include <esp_now.h>
#include <WiFi.h>

// Global copy of remote
esp_now_peer_info_t remote;
#define CHANNEL 1
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

// Initialize ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    // Serial.println("ESPNow Init Success");
  }
  else {
    // Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

uint8_t rssi = 0;
// Scan for remotes in AP mode
void ScanForRemote() {
  int8_t scanResults = WiFi.scanNetworks();
  
  // Reset on each scan
  bool remoteFound = 0;
  memset(&remote, 0, sizeof(remote));

  // Serial.println("");
  if (scanResults == 0) {
    Serial.println("No WiFi devices in AP Mode found");
  } else {
    // Serial.print("Found "); // Serial.print(scanResults); // Serial.println(" devices ");
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
      
      // Check if the current device starts with `Spectre`
      if (SSID.indexOf("Spectre") == 0) {
        // SSID of interest
        // Serial.println("Found a Remote.");
        // Serial.print(i + 1); // Serial.print(": "); // Serial.print(SSID); // Serial.print(" ["); // Serial.print(BSSIDstr); // Serial.print("]"); // Serial.print(" ("); // Serial.print(rssi); // Serial.print(")"); // Serial.println("");
        // Get BSSID => Mac Address of the Remote
        int mac[6];
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            remote.peer_addr[ii] = (uint8_t) mac[ii];
          }
        }

        if (i == 0 || abs(WiFi.RSSI(i)) < rssi) {
          rssi = abs(WiFi.RSSI(i));
        }

        // Pick a channel for connecting to the remote
        remote.channel = CHANNEL;

        // Do not encrypt the ESP Now connection
        remote.encrypt = 0;

        remoteFound = 1;
      }
      else {
        // Serial.println("FALSE");
      }
    }
  }

  if (remoteFound) {
    // Serial.println("Remote Found, processing..");
    //Serial.println("TRUE");
  } else {
    // Serial.println("Remote Not Found, trying again.");
    //Serial.println("FALSE");
  }

  // Clean up WiFi scan RAM
  WiFi.scanDelete();
}

// Check if the remote is already paired with the puppet show
//  If not, pair them
bool manageRemote() {
  if (remote.channel == CHANNEL) {
    if (DELETEBEFOREPAIR) {
      deletePeer();
    }

    // Serial.print("Remote Status: ");
    // Check if the peer exists
    bool exists = esp_now_is_peer_exist(remote.peer_addr);
    if (exists) {
      // Remote already paired.
      // Serial.println("Already Paired");
      return true;
    } else {
      // Remote not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&remote);
      if (addStatus == ESP_OK) {
        // Pair success
        // Serial.println("Pair success");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        // Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        // Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        // Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        // Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        // Serial.println("Peer Exists");
        return true;
      } else {
        // Serial.println("Not sure what happened");
        return false;
      }
    }
  } else {
    // No remote found to process
    // Serial.println("No Remote found to process");
    return false;
  }
}

void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(remote.peer_addr);
  // Serial.print("Remote Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    // Serial.println("Success");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    // Serial.println("ESPNOW Not Init");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    // Serial.println("Invalid Argument");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    // Serial.println("Peer not found.");
  } else {
    // Serial.println("Not sure what happened");
  }
}

// Send RSSI data to remote
void sendData() {
  const uint8_t *peer_addr = remote.peer_addr;
  // Serial.print("Sending: "); // Serial.println(rssi);
  esp_err_t result = esp_now_send(peer_addr, &rssi, sizeof(rssi));
  // Serial.print("Send Status: ");
  if (result == ESP_OK) {
    // Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    // Serial.println("ESPNOW not Init.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    // Serial.println("Invalid Argument");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    // Serial.println("Internal Error");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    // Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    // Serial.println("Peer not found.");
  } else {
    // Serial.println("Not sure what happened");
  }
}

// Callback when data is sent from puppet show to remote
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.print("Last Packet Sent to: "); // Serial.println(macStr);
  // Serial.print("Last Packet Send Status: "); // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  // Serial.println(WiFi.RSSI());
}

void setup() {
  Serial.begin(9600);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  
  // This is the mac address of the pupper show in Station Mode
  // Serial.print("STA MAC: "); // Serial.println(WiFi.macAddress());
  
  // Initialize ESPNow with a fallback logic
  InitESPNow();
  
  // Once ESPNow is successfully initialized, we will register for Send CB to
  //  get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  // In the loop we scan for remote
  ScanForRemote();
  // If remote is found, it would be populate in `remote` variable
   
  // We will check if `remote` is defined and then we proceed further
  if (remote.channel == CHANNEL) { // check if remote channel is defined
    // `remote` is defined
    // Add remote as peer if it has not been added already
    bool isPaired = manageRemote();
    if (isPaired) {
      Serial.println(rssi);
      if (rssi < 45) {
        Serial.println("TRUE");
      }
      else {
        Serial.println("FALSE");
      }
      // Pair success or already paired
      // Send data to device
      sendData();
    } else {
      Serial.println("FALSE");
      // Remote pair failed
      // Serial.println("Remote pair failed!");
    }
  }
  else {
    // No remote found to process
    Serial.println("FALSE");
  }

  // wait for 100ms to run the logic again
  delay(100);
}
