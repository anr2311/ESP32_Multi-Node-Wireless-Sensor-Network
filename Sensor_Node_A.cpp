#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP280 bmp; // I2C

const int LED = 23;
const int sensorPin = 4;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x08, 0xB6, 0x1F, 0xBD, 0xEA, 0x20};

// Define variables to store BME280 readings to be sent
float temperature;
float altitude;
float pressure;

// Define variables to store incoming readings
float incomingTemp;
float incomingAlt;
float incomingPres;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float temp;
    float alt;
    float pres;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message BMP280Readings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

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
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTemp = incomingReadings.temp;
  incomingAlt = incomingReadings.alt;
  incomingPres = incomingReadings.pres;
}
 
void setup() {
  
  Serial.begin(115200);
  while ( !Serial ) delay(100);   // wait for native usb
  
  Serial.println(F("BMP280 test"));
  unsigned status;
  
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
                  pinMode(LED, OUTPUT);

  Serial.println(F("DHTxx test!"));
  dht.begin();
 
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
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = true;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

  int LightIntensity = analogRead(sensorPin);

  Serial.println("=========================================");
  Serial.print("Light Intensity : ");
  Serial.println(LightIntensity);
  Serial.println("=========================================");
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  
  getReadings();
 
  // Set values to send
  BMP280Readings.temp = temperature;
  BMP280Readings.alt = altitude;
  BMP280Readings.pres = pressure;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &BMP280Readings, sizeof(BMP280Readings));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

    Serial.println("=======================================================");
    Serial.println("Readings from BMP280 sensor:");
    Serial.print("Temperature : ");
    Serial.println(BMP280Readings.temp);
    Serial.print("Altitude : ");
    Serial.println(BMP280Readings.alt);
    Serial.print("Pressure : ");
    Serial.println(BMP280Readings.pres);
    Serial.println("======================================================="); 
    Serial.println("Readings from DHT11 sensor:");
    Serial.print("Temperature : ");
    Serial.println(t);
    Serial.print("Humidity : ");
    Serial.println(h);
    Serial.print("HeatIndex : ");
    Serial.println(hif);
    Serial.println("======================================================="); 
    
    delay(5000);
}
void getReadings(){
  temperature = bmp.readTemperature();
  altitude = bmp.readAltitude(1013.25);
  pressure = bmp.readPressure();
}