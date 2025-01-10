#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "DHT.h"

#define d2 2
static uint8_t DHTPIN = 23;

DHT dht(DHTPIN, DHT11);

void Sensor_values(void *pvParameters);
TaskHandle_t Sensor_values_handler;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(green1, OUTPUT);
  
  //xTaskCreatePinnedToCore(ESP_NOW_Task, "ESP NOW", 2048, NULL, 1, &ESP_NOW_Task_Handler, 0);
  xTaskCreate(Sensor_values, "Sensor values", 2048, NULL, 1, &Sensor_values_handler);
}

void Sensor_values(void *pvParameters) {
  for (;;) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    serial_print();
  }  
}
void serial_print(){
  Serial.print("temp:");
  Serial.print(t);
  Serial.print(" :: ");
  Serial.print("Hum:");
  Serial.print(h);
  Serial.print(" :: ");
  Serial.print("spio2:");
  Serial.print(s);
  Serial.print(" :: ");
  Serial.print("Heart:");
  Serial.print(hr);
  Serial.print(" :: ");
  Serial.print("MQ:");
  Serial.println(mq);
}

void loop(){}
