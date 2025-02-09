#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "DHT.h"
#include "MAX30100_PulseOximeter.h" //max30100
#include <Wire.h> 
#include <driver/i2s.h> // mic
#include <SPI.h> //display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "image.h"

/////////////////////////////////////////////////////////////////////
//If breathing signals fluctuate too much
//Increase the high-pass filter factor
const float highPassFactor = 0.99;

// If the system is too sensitive (detecting noise instead of breathing), then increse both
//If the system is not detecting weak breathing properly,Then Decrease both
int s1 = 1200, s2 = 500;

#define REPORTING_PERIOD_MS  400
#define d2 2
#define mqsensor 34
#define TFT_CS   19   // Chip select pin
#define TFT_DC   4  // Data/Command pin
#define TFT_RST  5  // Reset pin (can be set to -1 if not used)
#define DHTPIN  27 // 17
#define FREQUENCY_HZ 50
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))

static unsigned long last_interval_ms = 0;

uint32_t tsLastReport = 0;
int heart,spo2, mq, heart_d, respLevel;
float t, h, rms, body_temp;

// Mutex for TFT display access
SemaphoreHandle_t tftMutex ;

int16_t i2sData[1024];
int32_t sum = 0;
int R = 1;
int sensitivity = 90; // Sensitivity variable (0 to 100)

#define I2S_WS 15    // I2S word select pin
#define I2S_SCK 14   // I2S clock pin
#define I2S_SD 32    // I2S data pin

// I2S configuration
const i2s_config_t i2s_config = {
  .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive mode
  .sample_rate = 44100,                             // Sample rate
  .bits_per_sample = i2s_bits_per_sample_t(16),     // Bits per sample
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,      // Only left channel
  .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,         // Interrupt level 1
  .dma_buf_count = 8,                               // Number of DMA buffers
  .dma_buf_len = 1024,                              // DMA buffer length
  .use_apll = false,
  .tx_desc_auto_clear = false,
  .fixed_mclk = 0
};

// I2S pin configuration
const i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_SCK,
  .ws_io_num = I2S_WS,
  .data_out_num = I2S_PIN_NO_CHANGE,
  .data_in_num = I2S_SD
};

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
DHT dht(DHTPIN, DHT11);
PulseOximeter pox;

void Edge_impulse(void *pvParameters);
TaskHandle_t Edge_impulse_Handler;

void Lungs_monitor(void *pvParameters);
TaskHandle_t Lungs_monitor_Handler;

void Sensor_values(void *pvParameters);
TaskHandle_t Sensor_values_Handler;

void max30100_value(void *pvParameters);
TaskHandle_t max30100_value_Handler;

void TFT_display(void *pvParameters);
TaskHandle_t TFT_display_Handler;

void onBeatDetected(){
}

void setup() {
  Serial.begin(115200);
  Serial.println(esp_get_free_heap_size());
  
  tftMutex = xSemaphoreCreateMutex();
  
  xTaskCreatePinnedToCore(Edge_impulse, "Edge Impulse", 2048, NULL, 1, &Edge_impulse_Handler, 0);
  xTaskCreatePinnedToCore(Lungs_monitor, "Lungs monitor", 2048, NULL, 1, &Lungs_monitor_Handler, 0);
  xTaskCreatePinnedToCore(max30100_value, "MAX30100", 2048, NULL, 1, &max30100_value_Handler, 1);
  xTaskCreatePinnedToCore(Sensor_values, "Sensor values", 2048, NULL, 1, &Sensor_values_Handler, 1);
  xTaskCreatePinnedToCore(TFT_display, "TFT display", 2048, NULL, 1, &TFT_display_Handler, 1);
}

/////////////////////////////////////////////////// DHT11 and MQ ///////////////////////////////////////////////////

void Sensor_values(void *pvParameters) {
  dht.begin();
  int mq_v, j;
  float temp;
  for (;;) {
    h = dht.readHumidity();
    temp = temp + dht.readTemperature();
    //Get the ultrasonic sensor values
    mq_v = mq_v + analogRead(mqsensor);
    j++;
    if(j == 10){
      t = (temp/10);
      mq = mq_v/10 ;
      mq = map(mq, 0, 4095, 0, 100);
      mq_v = 0;
      j=0;
      temp =0;
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay of 0.5s
    // UBaseType_t stackLeft = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("Task sensor Stack Left: ");
    // Serial.println(stackLeft);
  }  
}

/////////////////////////////////////////////////// max30100 ///////////////////////////////////////////////////

void max30100_value(void *pvParameters) {
  Serial.print("Initializing pulse oximeter..");
  // Initialize sensor
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_20_8MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  for (;;) {
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      int ht = pox.getHeartRate();
      int sp = pox.getSpO2();
      //Sp02
      if(sp >= 100){
        spo2 = 99;
      }
      else if(heart == 0){
        spo2 = 0;
        body_temp = 0;
      }
      else{
        spo2 = sp + 2;
      }
      //Heart Rate
      if(ht >= 100){
        heart = 99;
      }
      else if(ht >= 49){
        heart = ht;
      }
      else {
        heart = 0;
      }
      //animation
      if (heart > 0) {
          heart_d = 1;
          body_temp = 97.0 + (random(0, 18) / 10.0); // Generates values between 97.0 to 98.7
      }
      tsLastReport = millis();
    }
  }  
}

/////////////////////////////////////////////////// MEMS microphone ///////////////////////////////////////////////////

// Sound Animation 
void displayNoiseIndication(int noiseLevel) {
  // Acquire mutex to access TFT display
  if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
    int barWidth = map(noiseLevel, 0, 100, 0, 240); // Map noise level percentage to pixel width
    // Clear previous noise indication display
    tft.fillRect(0, 195, 240, 60, ST77XX_BLACK);
    // Draw noise level bar
    for (int i = 0; i < barWidth; ++i) {
      if (i < barWidth * 0.2) {
        tft.drawFastVLine(i, 200, 30, ST77XX_GREEN);
      } else if (i < barWidth * 0.5) {
        tft.drawFastVLine(i, 200, 30, ST77XX_YELLOW);
      } else if (i < barWidth * 0.8) {
        tft.drawFastVLine(i, 200, 30, ST77XX_ORANGE);
      } else {
        tft.drawFastVLine(i, 200, 30, ST77XX_RED);
      }
    }
    // Release mutex after TFT display access
    xSemaphoreGive(tftMutex);
  }
}

void Lungs_monitor(void *pvParameters) {  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  float prevFilteredValue = 0;      // For high-pass filter
  size_t bytesRead;

  for (;;) {
    i2s_read(I2S_NUM_0, i2sData, sizeof(i2sData), &bytesRead, portMAX_DELAY);

    sum = 0;
    int sampleCount = bytesRead / 2; // Each sample is 16 bits (2 bytes)
    for (int i = 0; i < sampleCount; ++i) {
      int16_t sample = i2sData[i]; // Read each 16-bit sample

      // Apply high-pass filter to remove low-frequency noise
      float filteredSample = sample - (highPassFactor * prevFilteredValue);
      prevFilteredValue = filteredSample;

      // Sum the absolute values for RMS calculation
      sum += abs(filteredSample);
    }

    // Calculate RMS
    rms = sum / sampleCount;
    

    // Map RMS to a respiratory activity level
    int maxRMS = map(sensitivity, 0, 100, s1, s2); // Adjust based on observed breathing amplitude
    respLevel = map(rms, 0, maxRMS, 0, 100);
    respLevel = constrain(respLevel, 0, 100); // Constrain to 0-100%
    
    displayNoiseIndication(respLevel);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

 
/////////////////////////////////////////////////// Edge Impulse ///////////////////////////////////////////////////

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
      Serial.print(body_temp);
      Serial.print('\t');
      Serial.print(mq);
      Serial.print('\t');
      Serial.println(respLevel);
      // UBaseType_t stackLeft = uxTaskGetStackHighWaterMark(NULL);
      // Serial.print("Task edge Stack Left: ");
      // Serial.println(stackLeft);
    } 
  }
}

/////////////////////////////////////////////////// Display ///////////////////////////////////////////////////

// Heart Animation
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

// Display the constant content
void staticdisplay(){
  tft.fillScreen(ST77XX_BLACK);

  // Draw logo bars of Edge Impulse
  tft.fillRect(0, 0, 240, 45, ST77XX_WHITE);
  tft.fillRoundRect(0+10, 7, 45, 7, 10, 0x059D); 
  tft.fillCircle(2+10, 10, 5, 0x059D);    
  tft.fillRoundRect(0+10, 19, 30, 7, 10, 0x07E0);    
  tft.fillRoundRect(35+10, 19, 15, 7, 10, 0xFDA0);    
  tft.fillRoundRect(0+10, 31, 55, 7, 10, 0xF800); 
  tft.fillCircle(2+10, 34, 5, 0xF800);    
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(65+10, 15);
  tft.print("EDGE IMPULSE");

  // Draw temperature section
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(10, 54);
  tft.print("Temp: ");

  // Draw humidity section
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 84);
  tft.print("Humidity: ");

  // Draw SpO2 section
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(10, 114);
  tft.print("SpO2: ");

  // Draw heart rate section
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 144);
  tft.print("Heart: ");

  // Draw MQ sensor section
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setTextSize(2);
  tft.setCursor(10, 174);
  tft.print("MQ: ");

  drawHeart(195, 145, 20, ST77XX_RED);

}

// Display the content 
void updateDisplay() {
  static float prevTemp = -1;
  static float prevHumidity = -1;
  static int prevSpo2 = -1;
  static int prevHeart = -1;
  static int prevMQ = -1;
  // Update temperature only if it changes
  if (t != prevTemp) {
     if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
      tft.fillRect(90, 54, 140, 20, ST77XX_BLACK); // Clear only the old value
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2.5);
      tft.setCursor(90, 54);
      tft.print(t);
      tft.print(" C");
      prevTemp = t;
     xSemaphoreGive(tftMutex);
    }
  }

  // Update humidity only if it changes
  if (h != prevHumidity) {
     if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
      tft.fillRect(120, 84, 110, 20, ST77XX_BLACK); // Clear only the old value
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2.5);
      tft.setCursor(120, 84);
      tft.print(h);
      tft.print(" %");
      prevHumidity = h;
     xSemaphoreGive(tftMutex);
    }
  }

  // Update SpO2 only if it changes
  if (spo2 != prevSpo2) {
    if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
      tft.fillRect(90, 114, 80, 20, ST77XX_BLACK); // Clear only the old value
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2.5);
      tft.setCursor(90, 114);
      tft.print(spo2);
      tft.print(" %");
      prevSpo2 = spo2;
     xSemaphoreGive(tftMutex);
    }
  }

  // Update heart rate only if it changes
  if (heart != prevHeart) {
    if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
      tft.fillRect(90, 144, 84, 20, ST77XX_BLACK); // Clear only the old value
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2.5);
      tft.setCursor(90, 144);
      tft.print(heart);
      tft.print(".bpm");
      prevHeart = heart;
      xSemaphoreGive(tftMutex);
    }
  }

  // Update MQ sensor only if it changes
  if (mq != prevMQ) {
    if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
      tft.fillRect(90, 174, 80, 20, ST77XX_BLACK); // Clear only the old value
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2.5);
      tft.setCursor(90, 174);
      tft.print(mq);
      tft.print(" %");
      prevMQ = mq;
      xSemaphoreGive(tftMutex);
    }
  }
}

// Display Main Task
void TFT_display(void *pvParameters) {
  if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
    tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixels
    tft.setRotation(3);
    tft.fillRect(0, 0, 240, 240, ST77XX_BLACK);
    tft.drawBitmap(0,0,RV_logo,250,250,ST77XX_WHITE);
    delay(2000);
    staticdisplay(); // only once
    xSemaphoreGive(tftMutex);
  }
  int a = 0;
  unsigned long last = 0;

  for (;;) {
    updateDisplay(); // Only when the new value is present

    if (heart_d > 0) { // Heartbeat animation
      if (millis() > last + 50) {
        if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
          drawHeart(195, 145, (a == 1) ? 20 : 15, ST77XX_WHITE);
          drawHeart(195, 145, (a == 1) ? 15 : 20, ST77XX_RED);
          a = !a;
          xSemaphoreGive(tftMutex);
        }
        last = millis();
      }
      heart_d = 0;
    }
    // UBaseType_t stackLeft = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("Task display Stack Left: ");
    // Serial.println(stackLeft);
  }
}

void loop(){}
