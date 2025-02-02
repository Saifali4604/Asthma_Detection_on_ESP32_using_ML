#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define Button 0  // ESP32 Boot button (GPIO 0)
#define D2 2  // Built in LED

// Your Telegram Bot Token & Chat ID
#define BOTtoken "8129554371:AAHJaJF4PlQAcAUehriYkgrjcuBypQCjdLA"
//#define CHAT_ID "-1002487084414"  // Your Group Chat ID
#define CHAT_ID "7166901221"  // Saif's private Chat ID
// Your WiFi credentials
const char* ssid = "Saif";
const char* password = "a1234567";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastCheck = 0;
const int checkInterval = 1000;

int w;

String keyboard = "[[\"Health Status\"]]";
String t, h, spo2, hR, body_temp, mq, rms, respLevel;

void Uart_SerialTask(void *pvParameters);
TaskHandle_t Uart_SerialTask_Handler;

void wifisetup()
{
    if(WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Connecting to WiFi...");
      digitalWrite(D2, LOW);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(200);
      }
      w=0;
    }
    else
    {
      digitalWrite(D2, HIGH);
      if(w==0)
      {
        Serial.println("\n✅ WiFi Connected!");
        client.setInsecure();
        bot.sendMessageWithReplyKeyboard(CHAT_ID, "✅ ESP32 is now connected! Click the button below:", "", keyboard, true);
        w=1;
      }
    }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  pinMode(D2, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }
  wifisetup();
  
  xTaskCreatePinnedToCore(Uart_SerialTask, "Uart_SerialTask", 2048, NULL, 1, &Uart_SerialTask_Handler, 1); //Serial works in core 1
}

//////////////////////////////////////////////////// Uart Data /////////////////////////////////////////////

void Uart_SerialTask(void *pvParameters) {
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
  for(;;){
    if (Serial1.available()) {
      String currentData = Serial1.readStringUntil('\n');  // Read until newline character
      int index1 = currentData.indexOf('\t');
      int index2 = currentData.indexOf('\t', index1 + 1);
      int index3 = currentData.indexOf('\t', index2 + 1);
      int index4 = currentData.indexOf('\t', index3 + 1);
      int index5 = currentData.indexOf('\t', index4 + 1);
      int index6 = currentData.indexOf('\t', index5 + 1);
      int index7 = currentData.indexOf('\t', index6 + 1);
      // Extract each value based on the tabs
      t = currentData.substring(0, index1);
      h = currentData.substring(index1 + 1, index2);
      spo2 = currentData.substring(index2 + 1, index3);
      hR = currentData.substring(index3 + 1, index4);
      body_temp = currentData.substring(index4 + 1, index5);
      mq = currentData.substring(index5 + 1, index6);
      rms = currentData.substring(index6 + 1, index7);
      respLevel = currentData.substring(index7 + 1);
    }
    
  }
}

//////////////////////////////////////////////////// Telegram /////////////////////////////////////////////

void handleNewMessages(int numNewMessages) {// Function to handle received messages
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.print("Received Message: ");
    Serial.println(text);

    if (text == "Health Status" || text == "/status@Astama_v2_bot" || text == "/status") {
      String sensorData = "🌡 Temperature: " + String(t) + "°C\n" +
                          "💧 Humidity: " + String(h) + "%\n" +
                          "🌬 Air Quality: " + String(mq) + "\n" +
                          "❤️ Heart Rate: " + String(hR) + " bpm\n" +
                          "🩸 SpO2: " + String(spo2) + "%\n" +
                          "🤒 Body Temperature: " + String(body_temp) + "°F\n" +
                          "💨 Breathing Rate: " + String(respLevel) + " breaths/min";
      bot.sendMessage(chat_id, sensorData, "");
      t = ""; h = ""; spo2 = ""; hR = ""; body_temp = ""; mq = ""; rms = ""; respLevel = "";
    }
  }
}

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
//////////////////////////////////////////////////// Normal Code /////////////////////////////////////////////

void loop() {
  wifisetup();
  Telegram();
  if (digitalRead(Button) == LOW) {
    // Button pressed, turn on D2 LED
    digitalWrite(D2, LOW);
    Serial.println("📤 Sending test message to Telegram...");
    bool sent = bot.sendMessage(CHAT_ID, "⚠️ Astama detected! Hurry up and take action!");

    if (sent) {
      Serial.println("✅ Message sent successfully!");
    } 
    else {
      Serial.println("❌ Failed to send message! Check Bot Token & Chat ID.");
    }
  } 
}
