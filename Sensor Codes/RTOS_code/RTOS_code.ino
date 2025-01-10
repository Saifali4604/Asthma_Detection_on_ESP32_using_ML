#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "DHT.h"
#include <MAX30100_PulseOximeter.h>
#include <Wire.h>

#define REPORTING_PERIOD_MS  500
#define d2 2
#define sensor A0
static uint8_t DHTPIN = 23;
uint32_t tsLastReport = 0;
int heart,spo2, mq;
float t,h;

DHT dht(DHTPIN, DHT11);
PulseOximeter pox;
void onBeatDetected(){}

void Sensor_values(void *pvParameters);
TaskHandle_t Sensor_values_handler;

void max30100_value(void *pvParameters);
TaskHandle_t max30100_value_Handler;

void DhT11_sensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
}

void serial_print(){
  Serial.print("temp:");
  Serial.print(t);
  Serial.print(" :: ");
  Serial.print("Hum:");
  Serial.print(h);
  Serial.print(" :: ");
  Serial.print("spo2:");
  Serial.print(spo2);
  Serial.print(" :: ");
  Serial.print("Heart:");
  Serial.print(heart);
  Serial.print(" :: ");
  Serial.print("MQ:");
  Serial.println(mq);
}

void MQ_sensor(){
  //Get the ultrasonic sensor values
  mq = analogRead(sensor);
  mq = map(mq, 0, 4095, 0, 100);
}

void setup() {
  Serial.begin(115200);
  pinMode(green1, OUTPUT);
  dht.begin();
  
  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    ESP.restart();
  } 
  else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);//increase the intensity of IR light,  in this code current is 14.2 mA
    // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  
  xTaskCreatePinnedToCore(max30100_value, "MAX30100", 2048, NULL, 1, &max30100_value_Handler, 0);
  xTaskCreate(Sensor_values, "Sensor values", 2048, NULL, 1, &Sensor_values_handler);
}

void Sensor_values(void *pvParameters) {
  for (;;) {
    DhT11_sensor();
    MQ_sensor();
    serial_print();
  }  
}

void max30100_value(void *pvParameters) {
  for (;;) {
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      heart = pox.getHeartRate();
      spo2 = pox.getSpO2();
      tsLastReport = millis();
    }
  }  
}

void loop(){}
