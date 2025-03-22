#include <ESP8266WiFi.h>  // For ESP8266
#include <OneWire.h>
#include <espnow.h>

// List of peer MAC addresses (Replace with actual addresses)
uint8_t peerAddresses[][6] = {
  {0xDC, 0x4F, 0x22, 0x60, 0x9F, 0xE7},  // node 1
};
const int numPeers = sizeof(peerAddresses) / sizeof(peerAddresses[0]);


typedef struct SensorDataResponse {
  float temperature;
  String moisture;
} SensorDataResponse;

typedef struct SensorDataRequest {
  bool requestPing;
} SensorDataRequest;

SensorDataResponse sensorDataResponse;
SensorDataRequest sensorDataRequest;


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  delay(2000);

  // Serial.swap();           // Use GPIO2 (TX) & GPIO13 (RX) for Serial1
  Serial1.begin(9600);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Add all peers
  for (int i = 0; i < numPeers; i++) {
    int status = esp_now_add_peer(peerAddresses[i], ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    Serial.print(status);
  }
}


void loop() {
    delay(30000);
    sensorDataRequest.requestPing = true;
    
    for (int i = 0; i < numPeers; i++) {
      esp_now_send(peerAddresses[i], (uint8_t *)&sensorDataRequest, sizeof(sensorDataRequest));
      for (int j = 0; j < 6; j++) Serial.printf("%02X:", peerAddresses[i][j]);
      Serial.print(" -> ");
    }
}

void printTemperature(float temperatureC) {
  Serial.print("T: ");
  Serial.print(temperatureC);
  Serial.println("℃");
}

void printMoiatureValue(String wet) {
  Serial.println(wet);
}

void printMACAddress() {
  Serial.print("ESP8266 MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Send Status: ");
  Serial.println(sendStatus == 0 ? "Success" : "Fail");
}

void sendDataToSerial(float temperature, String moistureLevel) {
  Serial.println("Sending dat to serial");
  Serial1.print(temperature);
  Serial1.print("\n");  // Newline to indicate end of message

  Serial.print("Temperature Sent: ");

  Serial1.print(moistureLevel);
  Serial1.print("\n");
  Serial.println("Sent data to serial");
}

//TODO: support to multiple seonsor node data
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&sensorDataResponse, incomingData, sizeof(sensorDataResponse));
  Serial.print("Received: ");
  printTemperature(sensorDataResponse.temperature);
  printMoiatureValue(sensorDataResponse.moisture);
}
