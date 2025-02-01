#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Your WiFi credentials
const char* ssid = "Saif";
const char* password = "a1234567";

// Your Telegram Bot Token & Chat ID
#define BOTtoken "8129554371:AAHJaJF4PlQAcAUehriYkgrjcuBypQCjdLA"
#define CHAT_ID "-1002487084414"  // Your Group Chat ID
//#define CHAT_ID "7166901221"  // Saif's private Chat ID

#define Button 0  // ESP32 Boot button (GPIO 0)
#define LED_PIN 2  

// Secure WiFi client
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Check for new Telegram messages every 1 second
unsigned long lastCheck = 0;
const int checkInterval = 1000;

float temp = 21;   
int hum = 50;  
int mq = 33;
int hR = 83;
int spo2 = 94;
int bodyTemp = 93;
int breathRate = 60;


// Function to handle received messages
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.print("Received Message: ");
    Serial.println(text);

    if (text == "Health Status" || text == "/status@Astama_v2_bot" || text == "/status") {
      String sensorData = "🌡 Temperature: " + String(temp) + "°C\n" +
                          "💧 Humidity: " + String(hum) + "%\n" +
                          "🌬 Air Quality: " + String(mq) + "\n" +
                          "❤️ Heart Rate: " + String(hR) + " bpm\n" +
                          "🩸 SpO2: " + String(spo2) + "%\n" +
                          "🤒 Body Temperature: " + String(bodyTemp) + "°F\n" +
                          "💨 Breathing Rate: " + String(breathRate) + " breaths/min";
      bot.sendMessage(chat_id, sensorData, "");
    }
  }
}

void wifisetup()
{
  if(WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_PIN, LOW);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    client.setInsecure();
    String keyboard = "[[\"Health Status\"]]";
    bot.sendMessageWithReplyKeyboard(CHAT_ID, "✅ ESP32 is now connected! Click the button below:", "", keyboard, true);
  }
  else
  {
    digitalWrite(LED_PIN, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  client.setInsecure();

  // Send test message with the button
  String keyboard = "[[\"Health Status\"]]";
  bot.sendMessageWithReplyKeyboard(CHAT_ID, "✅ ESP32 is now connected! Click the button below:", "", keyboard, true);
}

void loop() {
  // Check for new messages every second
  wifisetup();
  if (millis() - lastCheck > checkInterval) {
    lastCheck = millis();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
  if (digitalRead(Button) == LOW) {
    // Button pressed, turn on D2 LED
    digitalWrite(LED_PIN, LOW);
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

