#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "DHT.h"
#include <MAX30100_PulseOximeter.h>
#include <Wire.h>
#include <SPI.h> 
#include <driver/i2s.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "image.h"

#define REPORTING_PERIOD_MS  500
#define d2 2
#define mqsensor 34
#define TFT_CS   13   // Chip select pin
#define TFT_DC   4  // Data/Command pin
#define TFT_RST  5  // Reset pin (can be set to -1 if not used)

#define FREQUENCY_HZ 50
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))

static unsigned long last_interval_ms = 0;
static uint8_t DHTPIN = 14;
uint32_t tsLastReport = 0;
int heart,spo2, mq, heart_d;
float t,h;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
DHT dht(DHTPIN, DHT11);
PulseOximeter pox;
void onBeatDetected(){
  heart_d = 1;
}

void Edge_impulse(void *pvParameters);
TaskHandle_t Edge_impulse_Handler;

void Sensor_values(void *pvParameters);
TaskHandle_t Sensor_values_Handler;

void max30100_value(void *pvParameters);
TaskHandle_t max30100_value_Handler;

void TFT_display(void *pvParameters);
TaskHandle_t TFT_display_Handler;

void DhT11_sensor(){
  h = dht.readHumidity();
  t = dht.readTemperature();
}

void drawHeart(int x, int y, int size, uint16_t color) {
  // Draw two circles for the top of the heart
  tft.fillCircle(x - size / 2, y - size / 3, size / 2, color);  // Left circle
  tft.fillCircle(x + size / 2, y - size / 3, size / 2, color);  // Right circle

  // Draw a triangle for the bottom of the heart
  int x0 = x - size;   // Left corner
  int y0 = y - size / 3;  // Top of the triangle
  int x1 = x + size;   // Right corner
  int y1 = y - size / 3;
  int x2 = x;          // Bottom point
  int y2 = y + size;
  tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void staticdisplay(){
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, 0, 240, 40, ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2.8);
  tft.setCursor(10, 10);
  tft.println("  Sensor Readings");

  // Draw temperature section
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.print("Temp: ");

  // Draw humidity section
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 80);
  tft.print("Humidity: ");

  // Draw SpO2 section
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(10, 110);
  tft.print("SpO2: ");

  // Draw heart rate section
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 140);
  tft.print("Heart: ");

  // Draw MQ sensor section
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setTextSize(2);
  tft.setCursor(10, 170);
  tft.print("MQ: ");

  drawHeart(195, 145, 20, ST77XX_RED);

  // Draw logo bars of Edge Impulse
  tft.fillRect(0, 195, 240, 50, ST77XX_WHITE);
  tft.fillRoundRect(0+10, 202, 45, 7, 10, 0x059D); 
  tft.fillCircle(2+10, 205, 5, 0x059D);    
  tft.fillRoundRect(0+10, 214, 30, 7, 10, 0x07E0);    
  tft.fillRoundRect(35+10, 214, 15, 7, 10, 0xFDA0);    
  tft.fillRoundRect(0+10, 226, 55, 7, 10, 0xF800); 
  tft.fillCircle(2+10, 229, 5, 0xF800);    
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(65+10, 210);
  tft.print("EDGE IMPULSE");

}

void updateDisplay() {
  static float prevTemp = -1;
  static float prevHumidity = -1;
  static int prevSpo2 = -1;
  static int prevHeart = -1;
  static int prevMQ = -1;
  // Update temperature only if it changes
  if (t != prevTemp) {
    tft.fillRect(90, 50, 140, 20, ST77XX_BLACK); // Clear only the old value
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(90, 50);
    tft.print(t);
    tft.print(" C");
    prevTemp = t;
  }

  // Update humidity only if it changes
  if (h != prevHumidity) {
    tft.fillRect(120, 80, 110, 20, ST77XX_BLACK); // Clear only the old value
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(120, 80);
    tft.print(h);
    tft.print(" %");
    prevHumidity = h;
  }

  // Update SpO2 only if it changes
  if (spo2 != prevSpo2) {
    tft.fillRect(90, 110, 80, 20, ST77XX_BLACK); // Clear only the old value
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(90, 110);
    tft.print(spo2);
    tft.print(" %");
    prevSpo2 = spo2;
  }

  // Update heart rate only if it changes
  if (heart != prevHeart) {
    tft.fillRect(90, 140, 84, 20, ST77XX_BLACK); // Clear only the old value
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(90, 140);
    tft.print(heart);
    tft.print(".bpm");
    prevHeart = heart;
  }

  // Update MQ sensor only if it changes
  if (mq != prevMQ) {
    tft.fillRect(90, 170, 80, 20, ST77XX_BLACK); // Clear only the old value
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(90, 170);
    tft.print(mq);
    tft.print(" %");
    prevMQ = mq;
  }
}




void MQ_sensor(){
  //Get the ultrasonic sensor values
  mq = analogRead(mqsensor);
  mq = map(mq, 0, 4095, 0, 100);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixels
  tft.setRotation(3);
  tft.fillRect(0, 0, 240, 240, ST77XX_BLACK);
  tft.drawBitmap(0,0,RV_logo,250,250,ST77XX_WHITE);
  delay(2000);

  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    // ESP.restart();
  } 
  else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);//increase the intensity of IR light,  in this code current is 14.2 mA
    // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);

  xTaskCreatePinnedToCore(Edge_impulse, "Edge Impulse", 2048, NULL, 1, &Edge_impulse_Handler, 0);
  xTaskCreatePinnedToCore(max30100_value, "MAX30100", 2048, NULL, 1, &max30100_value_Handler, 1);
  xTaskCreatePinnedToCore(Sensor_values, "Sensor values", 2048, NULL, 1, &Sensor_values_Handler, 1);
  xTaskCreatePinnedToCore(TFT_display, "TFT display", 2048, NULL, 1, &TFT_display_Handler, 1);
}

void Sensor_values(void *pvParameters) {
  for (;;) {
    DhT11_sensor();
    MQ_sensor();
    vTaskDelay(pdMS_TO_TICKS(500)); // Delay of 0.5s
  }  
}

void TFT_display(void *pvParameters) {
  staticdisplay(); // only once
  int a = 0;        
  unsigned long last = 0;  
  
  for (;;) {
    updateDisplay(); //only when the new value is present

    if (heart_d > 0) { // heart beat animation.
      if (millis() > last + 50) {    
        drawHeart(195, 145, (a == 1) ? 20 : 15, ST77XX_WHITE);   
        drawHeart(195, 145, (a == 1) ? 15 : 20, ST77XX_RED);
        a = !a;
        last = millis();
      }
      heart_d =0;
    }
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
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay of 100 ms
  }  
}

void Edge_impulse(void *pvParameters) {
  for(;;){
    if (millis() > last_interval_ms + INTERVAL_MS) {
      last_interval_ms = millis();
      Serial.print(t);
      Serial.print('\t');
      Serial.print(h);
      Serial.print('\t');
      Serial.print(spo2);
      Serial.print('\t');
      Serial.print(heart);
      Serial.print('\t');
      Serial.println(mq);
    } 
  }
}

void loop(){}
