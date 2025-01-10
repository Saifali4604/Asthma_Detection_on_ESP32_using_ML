#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define green1 15
#define green2 13
#define green3 4
#define green4 16
#define yellow1 18
#define yellow2 19
#define yellow3 21
#define yellow4 22
#define d2 2
int periority=0;
int path_to_choose=0;
int R = 1;
int Ra1, Ra2, Ra3, Ra4;
int r = 0;
int a = 0;
int Ambulance1 = 0;
int Ambulance2 = 0;
int Ambulance3 = 0;
int Ambulance4 = 0;
int Traffic_Max_delay = 2000;
int Higher_limit = 4;
int Mid_limit = 2;
int Lower_limit = 1;
int Traffic_delay_Hold = Traffic_Max_delay;

int Traffic_Mid_delay = (Traffic_Max_delay * 2) / 3;
int Traffic_low_delay = (Traffic_Max_delay * 1) / 3;
int Traffic_delay = Traffic_Max_delay;

int espnow_connect = 0;
int exp_execute = 0;

String send_jsondata;
String recv_jsondata;  
StaticJsonDocument<256> doc;

void Traffic_light(void *pvParameters);
TaskHandle_t Traffic_light_handler;

void Traffic_time_control(void *pvParameters);
TaskHandle_t Traffic_time_control_handler;

void Traffic_time_control2(void *pvParameters);
TaskHandle_t Traffic_time_control_handler2;

EventGroupHandle_t Traffic_lights_time_signal;
EventGroupHandle_t Traffic_lights_time_signal2;

const int BIT_0 = (1 << 0);
const int BIT_1 = (1 << 1);

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS ){
    //Serial.println("Delivery success");
    espnow_connect=1;
  }
  else{
    //Serial.println("Delivery fail");
    espnow_connect=0;
  }
}

void esp_now_send(int value){
  doc["S"] = value;
  serializeJson(doc, send_jsondata); // converting it to JSON
  // Sending it to Master ESP (motor module)
  esp_now_send(broadcastAddress, (uint8_t *) send_jsondata.c_str(), send_jsondata.length());
  //Serial.println(send_jsondata); // printing sent data
  send_jsondata = "";
}

void setup() {
  Serial.begin(115200);
  pinMode(green1, OUTPUT);
  pinMode(green2, OUTPUT);
  pinMode(green3, OUTPUT);
  pinMode(green4, OUTPUT);
  pinMode(yellow1, OUTPUT);
  pinMode(yellow2, OUTPUT);
  pinMode(yellow3, OUTPUT);
  pinMode(yellow4, OUTPUT);
  pinMode(d2, OUTPUT);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() !=  ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    //Serial.println("Failed to add peer");
    return;
  }

  Traffic_lights_time_signal = xEventGroupCreate();
  Traffic_lights_time_signal2 = xEventGroupCreate();
  //xTaskCreatePinnedToCore(ESP_NOW_Task, "ESP NOW", 2048, NULL, 1, &ESP_NOW_Task_Handler, 0);
  xTaskCreate(Traffic_light, "Traffic Light", 2048, NULL, 1, &Traffic_light_handler);
  xTaskCreate(Traffic_time_control, "Traffic Time Control", 2048, NULL, 1, &Traffic_time_control_handler);
  xTaskCreate(Traffic_time_control2, "Traffic Time Control", 2048, NULL, 1, &Traffic_time_control_handler2);
  xEventGroupSetBits(Traffic_lights_time_signal, BIT_1);
}

void Blue_led(){
  if (Ambulance1 == 1 || Ambulance2 == 1 || Ambulance3 == 1 || Ambulance4 == 1){
    digitalWrite(d2,HIGH);
  }
  else{
    digitalWrite(d2,LOW);
  }
    
}
void compare_density(int R1, int R2, int R3, int R4, int Road) {
  switch (Road) {
    case 1:
      if (R2 >= Higher_limit) {
        Traffic_delay_Hold = Traffic_Max_delay;
      } else if (R2 >= Mid_limit) {
        Traffic_delay_Hold = Traffic_Mid_delay;
      } else if (R2 >= Lower_limit) {
        Traffic_delay_Hold = Traffic_low_delay;
      } else {
        // No change
      }
      break;
    case 2:
      if (R3 >= Higher_limit) {
        Traffic_delay_Hold = Traffic_Max_delay;
      } else if (R3 >= Mid_limit) {
        Traffic_delay_Hold = Traffic_Mid_delay;
      } else if (R3 >= Lower_limit) {
        Traffic_delay_Hold = Traffic_low_delay;
      } else {
        // No change
      }
      break;
    case 3:
      if (R4 >= Higher_limit) {
        Traffic_delay_Hold = Traffic_Max_delay;
      } else if (R4 >= Mid_limit) {
        Traffic_delay_Hold = Traffic_Mid_delay;
      } else if (R4 >= Lower_limit) {
        Traffic_delay_Hold = Traffic_low_delay;
      } else {
        // No change
      }
      break;
    case 4:
      if (R1 >= Higher_limit) {
        Traffic_delay_Hold = Traffic_Max_delay;
      } else if (R1 >= Mid_limit) {
        Traffic_delay_Hold = Traffic_Mid_delay;
      } else if (R1 >= Lower_limit) {
        Traffic_delay_Hold = Traffic_low_delay;
      } else {
        // No change
      }
      break;
  }
}

void Traffic_time_control(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal,
      BIT_0,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );
    Traffic_delay = Traffic_delay_Hold;
    Serial.println("Updating New Traffic Delay...");
    xEventGroupSetBits(Traffic_lights_time_signal, BIT_1);
  }
}

void Traffic_time_control2(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal2,
      BIT_0,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );
    int l =R+1;
    if(l==5){
      l=1;
    }
    esp_now_send(l);
    Serial.print("A");
    compare_density(Ra1, Ra2, Ra3, Ra4, R);
    Serial.println("Fetching the No. of cars");
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  char* buff = (char*) incomingData;
  recv_jsondata = String(buff);
  DeserializationError error = deserializeJson(doc, recv_jsondata);
  if (!error) {
      periority = doc["P"];
      path_to_choose = doc["Path"];
      int i = doc["Cam"];
      int S = doc["S"]; // New line to get the status

      if (i == 1) {
        Ra1 = doc["No"];
        Ambulance1 = doc["Ambu"];
        if (S == 1) Ambulance1 = 0; // Update status
      } else if (i == 2) {
        Ra2 = doc["No"];
        Ambulance2 = doc["Ambu"];
        if (S == 1) Ambulance2 = 0; // Update status
      } else if (i == 3) {
        Ra3 = doc["No"];
        Ambulance3 = doc["Ambu"];
        if (S == 1) Ambulance3 = 0; // Update status
      } else if (i == 4) {
        Ra4 = doc["No"];
        Ambulance4 = doc["Ambu"];
        if (S == 1) Ambulance4 = 0; // Update status
      }
  }
  Blue_led();
  Serial.print(recv_jsondata);
  recv_jsondata = "";
}


void Traffic_light(void *pvParameters) {
  for (;;) {
    EventBits_t xBits = xEventGroupWaitBits(
      Traffic_lights_time_signal,
      BIT_1,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );
    if(path_to_choose >= 1){
        R=5;
    }

    switch (R) {
      case 1:
        digitalWrite(green1, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green1, LOW);
        digitalWrite(yellow1, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow1, LOW);
        R = 2;
        break;
      case 2:
        digitalWrite(green2, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green2, LOW);
        digitalWrite(yellow2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow2, LOW);
        R = 3;
        break;
      case 3:
        digitalWrite(green3, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green3, LOW);
        digitalWrite(yellow3, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow3, LOW);
        R = 4;
        break;
      case 4:
        digitalWrite(green4, HIGH);
        xEventGroupSetBits(Traffic_lights_time_signal2, BIT_0);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        digitalWrite(green4, LOW);
        digitalWrite(yellow4, HIGH);
        vTaskDelay(pdMS_TO_TICKS(Traffic_delay / 2));
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        digitalWrite(yellow4, LOW);
        R = 1;
        break;
      case 5:
        digitalWrite(green1, LOW);
        digitalWrite(yellow1, LOW);
        digitalWrite(green2, LOW);
        digitalWrite(yellow2, LOW);
        digitalWrite(green3, LOW);
        digitalWrite(yellow3, LOW);
        digitalWrite(green4, LOW);
        digitalWrite(yellow4, LOW);

        if(path_to_choose == 1){
          digitalWrite(green1, HIGH);
          digitalWrite(yellow1, LOW);
          while(Ambulance1 == 1){
            delay(500);
            int l = 1;
            Blue_led();
            esp_now_send(l);
          }
          digitalWrite(green1, LOW);
          digitalWrite(yellow1, HIGH);

        } else if(path_to_choose == 2){
          digitalWrite(green2, HIGH);
          digitalWrite(yellow2, LOW);
          while(Ambulance2 == 1){
            delay(500);
            int l = 2;
            Blue_led();
            esp_now_send(l);
          }
          digitalWrite(green2, LOW);
          digitalWrite(yellow2, HIGH);

        } else if(path_to_choose == 3){
          digitalWrite(green3, HIGH);
          digitalWrite(yellow3, LOW);
          while(Ambulance3 == 1){
            delay(500);
            int l = 3;
            Blue_led();
            esp_now_send(l);
          }
          digitalWrite(green3, LOW);
          digitalWrite(yellow3, HIGH);

        } else if(path_to_choose == 4){
          digitalWrite(green4, HIGH);
          digitalWrite(yellow4, LOW);
          while(Ambulance4 == 1){
            delay(500);
            int l = 4;
            Blue_led();
            esp_now_send(l);
          }
          digitalWrite(green4, LOW);
          digitalWrite(yellow4, HIGH);

        }
        path_to_choose = 0;
        periority = 0;
        R = 1;
        xEventGroupSetBits(Traffic_lights_time_signal, BIT_0);
        break;
    }
  }
}


void loop(){}
