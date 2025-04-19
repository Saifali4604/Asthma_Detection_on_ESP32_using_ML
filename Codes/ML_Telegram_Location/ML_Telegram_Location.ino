//ML lib
#include <astama_detection_inferencing.h>

// Your WiFi credentials
const char* ssid = "Saif";
const char* password = "a1234567";

// Your Telegram Bot Token & Chat ID
#define BOTtoken "8129554371:AAHJaJF4PlQAcAUehriYkgrjcuBypQCjdLA"
#define CHAT_ID "-1002487084414"  // Your Group Chat ID
//#define CHAT_ID "7166901221"  // Saif's private Chat ID+

///////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <EEPROM.h>

#define EEPROM_SIZE 256  // Define EEPROM size
#define Button 0  // ESP32 Boot button (GPIO 0)

static bool debug_nn = false;

int spo2 = 0;
int hR = 0;
int rasp = 0;
int w, B;
int previousB = -1;
float Normal = 0, PostA = 0, PreA = 0;
unsigned long lastCheck = 0;
const int checkInterval = 1000;
String keyboard = "[[\"Health Status\"]]";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void Uart_SerialTask(void *pvParameters);
TaskHandle_t Uart_SerialTask_Handler;

void ML_Task(void *pvParameters);
TaskHandle_t ML_Task_Handler;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //off
  pinMode(Button, INPUT_PULLUP);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }
  wifisetup();
  
  xTaskCreatePinnedToCore(Uart_SerialTask, "Uart_SerialTask", 2048, NULL, 1, &Uart_SerialTask_Handler, 1); //Serial works in core 1
  xTaskCreatePinnedToCore(ML_Task, "ML Task", 4096, NULL, 1, &ML_Task_Handler, 1);
}

//////////////////////////////////////////////////// Telegram Functions /////////////////////////////////////////////

// Storing the location inside EEPROM
void saveLocationToEEPROM(String location) {
  Serial.println("💾 Saving location to EEPROM...");
  
  int len = location.length();
  if (len > EEPROM_SIZE - 2) {  // Ensure it fits
    Serial.println("❌ URL too long to store in EEPROM!");
    return;
  }

  EEPROM.write(0, len);  // Store the length at index 0
  for (int i = 0; i < len; i++) {
    EEPROM.write(i + 1, location[i]);  // Store URL characters
  }
  EEPROM.commit();  // Save changes
  Serial.println("✅ Location saved successfully!");
}

//Reading Location from EEPROM
String readLocationFromEEPROM() {
  Serial.println("📖 Reading location from EEPROM...");
  
  int len = EEPROM.read(0);
  if (len == 0 || len > EEPROM_SIZE - 2) {
    Serial.println("❌ No valid location found in EEPROM!");
    return "";
  }

  String location = "";
  for (int i = 0; i < len; i++) {
    location += char(EEPROM.read(i + 1));
  }

  Serial.println("📍 Saved Location: " + location);
  return location;
}

// Telegram Task
void handleNewMessages(int numNewMessages) {// Function to handle received messages
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.print("Received Message: ");
    Serial.println(text);
    if (text.startsWith("https://maps.app.goo.gl/") && text.length() > 24) { 
      Serial.println("✅ Detected Google Maps short URL!");
      bot.sendMessage(chat_id, "📍 Location received: " + text, "");
      saveLocationToEEPROM(text);  // Save to EEPROM
      bot.sendMessage(chat_id, "✅ Location saved successfully!");
    } 
    else if (text == "Health Status" || text == "/status@Astama_v2_bot" || text == "/status") {
      String sensorData =  "❤️ Heart Rate: " + String(hR) + " bpm\n" +
                        "🩸 SpO2: " + String(spo2) + "%\n" +
                        "💨 Breathing intensity: " + String(rasp) + "";
  bot.sendMessage(CHAT_ID, sensorData, "");
  spo2 = 0; 
  hR = 0; 
  rasp = 0;
    }
  }
}

// Time to Time checking of new Msg in Telegram
void Telegram(){
  if (millis() - lastCheck > checkInterval) { // Check for new messages every second
    lastCheck = millis();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}

//////////////////////////////////////////////////// Normal Functions /////////////////////////////////////////////
// Performs the tasks once Astama Detected
void Astama_Detected(){
  digitalWrite(LED_BUILTIN, HIGH); //OFF
  String storedLocation = readLocationFromEEPROM();
  Serial.println("📤 Sending test message to Telegram...");
  bot.sendMessage(CHAT_ID, "⚠️ Astama detected! Hurry up and take action!");
  String sensorData =  "❤️ Heart Rate: " + String(hR) + " bpm\n" +
                        "🩸 SpO2: " + String(spo2) + "%\n" +
                        "💨 Breathing intensity: " + String(rasp) + "\n" +
                        "📍 Location: " + storedLocation + "";
  bot.sendMessage(CHAT_ID, sensorData, "");
  spo2 = 0; 
  hR = 0; 
  rasp = 0;
}

void Astama_Might(){
  digitalWrite(LED_BUILTIN, HIGH); //OFF
  String storedLocation = readLocationFromEEPROM();
  Serial.println("📤 Sending test message to Telegram...");
  bot.sendMessage(CHAT_ID, "⚠️ Astama Might Occur! Hurry up and take action!");
  String sensorData =  "❤️ Heart Rate: " + String(hR) + " bpm\n" +
                        "🩸 SpO2: " + String(spo2) + "%\n" +
                        "💨 Breathing intensity: " + String(rasp) + "\n" +
                        "📍 Location: " + storedLocation + "";
  bot.sendMessage(CHAT_ID, sensorData, "");
  spo2 = 0; 
  hR = 0; 
  rasp = 0;
}

void wifisetup()
{
    if(WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Connecting to WiFi...");
      digitalWrite(LED_BUILTIN, HIGH); //off
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(200);
      }
      w=0;
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW); //ON
      if(w==0)
      {
        Serial.println("\n✅ WiFi Connected!");
        client.setInsecure();
        bot.sendMessageWithReplyKeyboard(CHAT_ID, "✅ ESP32 is now connected! Click the button below:", "", keyboard, true);
        w=1;
      }
    }
}

//////////////////////////////////////////////////// Uart Data task /////////////////////////////////////////////

void Uart_SerialTask(void *pvParameters) {
  Serial1.begin(115200, SERIAL_8N1, 6, 43); //new rx, default tx Xiao ESP32S3
  //Serial1.begin(115200, SERIAL_8N1, 16, 17); //new rx, default tx ESP32
  for(;;){
    if (Serial1.available()) {
      String currentData = Serial1.readStringUntil('\n');
      int index1 = currentData.indexOf('\t');
      int index2 = currentData.indexOf('\t', index1 + 1);
      spo2 = currentData.substring(0, index1).toInt();
      hR   = currentData.substring(index1 + 1, index2).toInt();
      rasp = currentData.substring(index2 + 1).toInt();
    }
  }
}


//////////////////////////////////////////////////// ML task ////////////////////////////////////////////////////

  /**
  * @brief Return the sign of the number
  * 
  * @param number 
  * @return int 1 if positive (or 0) -1 if negative
  */
  float ei_get_sign(float number) {
    return (number >= 0.0) ? 1.0 : -1.0;
  }
  /**
  * @brief      Get data and run inferencing
  *
  * @param[in]  debug  Get debug info if true
  */


void ML_Task(void *pvParameters) {
  ei_printf("Edge Impulse Inferencing Demo (UART input)\n");
  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
    ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be 3 (spo2, hR, rasp)\n");
    return;
  }

  for(;;){
    uint8_t buf1[64]="idle";
    uint8_t buf2[64]="left&right";
    uint8_t buf3[64]="up&down";
    ei_printf("\nStarting inferencing in 2 seconds...\n");
    delay(2000);
    ei_printf("Sampling...\n");

    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
      uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
      Serial.printf("SPO2: %d | HR: %d | RASP: %d\n", spo2, hR, rasp);
      buffer[ix]     = spo2;
      buffer[ix + 1] = hR;
      buffer[ix + 2] = rasp;
      delayMicroseconds(next_tick - micros());
    }
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
      ei_printf("Failed to create signal from buffer (%d)\n", err);
      continue;
    }

    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", err);
      continue;
    }

    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.):\n",
      result.timing.dsp, result.timing.classification, result.timing.anomaly);

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
      if( ix == 0 ){ Normal = result.classification[ix].value;}
      else if( ix == 1 ){ PostA = result.classification[ix].value;}
      else if( ix == 2 ){ PreA = result.classification[ix].value;}
    }
    Serial.printf("Normal: %.5f | Pre Asthma: %.5f | Post Asthma: %.5f\n", Normal, PreA, PostA);

    #if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf("    anomaly score: %.3f\n", result.anomaly);
    #endif
  }
}

//////////////////////////////////////////////////// Normal task ////////////////////////////////////////////////////

void loop() {
  wifisetup();
  Telegram();
  if (digitalRead(Button) == LOW) {
    Astama_Detected();
  }
}
