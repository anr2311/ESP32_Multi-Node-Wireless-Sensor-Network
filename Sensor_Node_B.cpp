#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>

#define LightPin     33
#define SoundPin     35

#define trigPin      5
#define echoPin      18

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x08, 0xB6, 0x1F, 0xBD, 0xEA, 0x20};

// Define variables to store BME280 readings to be sent
float motion;
float light;
float sound;

int counter = 0;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float motion;
    float light;
    float sound;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message SensorReadings;

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
 
void setup() {
  Serial.begin(115200);
  while ( !Serial ) delay(100);   // wait for native usb

  pinMode(LightPin, INPUT); // Set the signal pin as input
  pinMode(SoundPin, INPUT); // Set the signal pin as input
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
 
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
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;

  // Prints the distance in the Serial Monitor
  Serial.println("=========================================");
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  Serial.println("=========================================");


  light = analogRead(LightPin);

  Serial.println("=========================================");
  Serial.print("Light Intensity : ");
  Serial.println(((100 *(4096 - light) )/ 4096));
  Serial.println("=========================================");

  sound = analogRead(SoundPin);

  Serial.println("=========================================");
  Serial.print("Sound Intensity : ");
  Serial.println(sound *  (330 / 4096.0));
  Serial.println("=========================================");

  if (distanceCm < 50)
  {
    motion = 1;
    counter = 0;
  }
  else
  {
    counter = counter + 1;
    if (counter == 3) motion = 0;
  }
  SensorReadings.motion = motion;
  SensorReadings.light = ((100 *(4096 - light) )/ 4096);
  SensorReadings.sound = (sound *  (330 / 4096.0));

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &SensorReadings, sizeof(SensorReadings));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

    delay(1500);
}