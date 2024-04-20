#include <esp_now.h>
#include <WiFi.h>

#define PIRSensor    4

WiFiClient  client;

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <GyverOLED.h>

GyverOLED<SSH1106_128x64> oled;

int person_count = 0;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress1[] = {0xA8, 0x42, 0xE3, 0xC9, 0xE5, 0x50};
uint8_t broadcastAddress2[] = {0x10, 0x06, 0x1C, 0x82, 0x16, 0x98};

// Define variables to store BME280 readings to be sent
float temperature;
float altitude;
float pressure;

// Define variables to store incoming readings
float incomingTemp;
float incomingAlt;
float incomingPres;

// Define variables to store incoming readings
float incomingMotion;
float incomingLight;
float incomingSound;

// Define variables to store incoming readings
float motion;
float light;
float sound;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float temp;
    float alt;
    float pres;
} struct_message;

typedef struct struct_message2 {
    float motion;
    float light;
    float sound;
} struct_message2;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

struct_message2 incomingReadings2;

esp_now_peer_info_t peerInfo;

esp_now_peer_info_t peerInfo2;

void IRAM_ATTR detectsMovement()
{
    Serial.println("=========================================");
    Serial.println( " MOTION DETECTED " );
    Serial.println("=========================================");
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (mac[0] == broadcastAddress1[0])
  {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    
    Serial.print("Bytes received: ");
    Serial.println(len);
    
    incomingTemp = incomingReadings.temp;
    incomingAlt = incomingReadings.alt;
    incomingPres = incomingReadings.pres;

    Serial.println("=====================================");
    Serial.println("Received data: ");
    Serial.print("Temperature : ");
    Serial.println(incomingTemp);
    Serial.print("Altitude : ");
    Serial.println(incomingAlt);
    Serial.print("Pressure : ");
    Serial.println(incomingPres);
    Serial.println("=====================================");
  }
  else
  {
    memcpy(&incomingReadings2, incomingData, sizeof(incomingReadings2));
  
    Serial.print("Bytes received: ");
    Serial.println(len);

    incomingMotion = incomingReadings2.motion;
    incomingLight = incomingReadings2.light;
    incomingSound = incomingReadings2.sound;

    if ((incomingMotion == 1) && (person_count <= 0))
    {
      person_count = 1;
    }
    else if ((incomingMotion == 1) && (person_count >= 1))
    {
      person_count = 0;
    }

    Serial.println("=====================================");
    Serial.println("Received data: ");
    Serial.print("Person : ");
    Serial.println(person_count);
    Serial.print("Light : ");
    Serial.println(incomingLight);
    Serial.print("Sound : ");
    Serial.println(incomingSound);
    Serial.println("=====================================");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode( PIRSensor, INPUT_PULLUP ); // PIR Motion Sensor mode INPUT_PULLUP
  attachInterrupt( digitalPinToInterrupt( PIRSensor ), detectsMovement, FALLING );

  oled.init();
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Register peer2
  memcpy(peerInfo2.peer_addr, broadcastAddress2, 6);
  peerInfo2.channel = 0;  
  peerInfo2.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer 1");
    return;
  }

  if (esp_now_add_peer(&peerInfo2) != ESP_OK){
    Serial.println("Failed to add peer 2");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  updateDisplay();
}

void updateDisplay(){
  // Display Readings on OLED Display
  oled.clear();
  oled.update();  
  oled.home();            
  oled.print("Temp : ");   
  oled.print(incomingTemp);
  oled.println("degC");

  oled.setCursor(0, 1);
  oled.print("Pressure : ");   
  oled.print(incomingPres);
  oled.println("Pa");

  oled.setCursor(0, 2);
  oled.print("Altitude : ");   
  oled.print(incomingAlt);
  oled.println("m");

  oled.setCursor(0, 3);
  oled.print("Person in Room : ");   
  if (person_count >= 1)
  {
    oled.println("Yes");
  }
  else
  {
    oled.println("No");
  }

  oled.setCursor(0, 4);
  oled.print("Light Level : ");
  oled.print(incomingLight);
  oled.println("%");

  oled.setCursor(0, 5);
  oled.print("Sound Level : ");
  oled.print(incomingSound);
  oled.println("%");

  // To be called at the last
  oled.update();

  delay(2500);
}