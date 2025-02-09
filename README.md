# Astama_Detection
This project integrates an ESP32 microcontroller with multiple sensors, including a MEMS INMP441 microphone, to detect and predict asthma in humans. The system captures respiratory sounds, air quality, temperature, humidity, heart rate, and oxygen saturation, which are critical indicators of asthma. The MEMS microphone, placed on the chest, monitors respiratory operations to detect abnormalities in breathing patterns. Real-time data collection and processing are achieved using an RTOS-based approach, ensuring efficient multitasking. The collected data is used to train a machine learning (ML) model on the Edge Impulse platform, enabling the system to predict asthma conditions. The results are displayed on a 1.3-inch OLED screen, and alerts are provided through a buzzer in critical situations.
![image](https://github.com/Saifali4604/Asthma_Detection_on_ESP32_using_ML/blob/main/Gallery/IMG_20250116_013132.jpg)

[Watch the Demo Video on LinkedIn](https://www.linkedin.com/posts/mohammad-saifali-shaikh-87922b2b4_rtos-edgeml-embeddedsystems-activity-7294403019264196608-YoVi?utm_source=share&utm_medium=member_desktop&rcm=ACoAAFCh1LEBXEJIP3p8ofVr8EcSpXVme1eKLgw)


## Components Used:
1. ### ESP32 (ESP-WROOM-32):
   The central processing unit that collects sensor data, processes it, and forwards it to Edge Impulse for training the ML model.
2. ### MEMS Microphone (INMP441):
   Monitors respiratory sounds when placed on the chest, enabling the system to detect abnormal breathing patterns indicative of asthma.
3. ### Gas Sensor:
    Measures air quality and pollutants that may trigger asthma.
4. ### DHT11/DHT22 Sensor:
   Captures temperature and humidity, environmental factors that significantly affect asthma conditions.
5. ### MAX30100 Pulse Oximeter:
   Monitors heart rate and oxygen saturation, physiological indicators of asthma.
6. ### 1.3-inch OLED Display (ST7789 Driver):
   Displays vital information such as respiratory patterns, temperature, humidity, heart rate, and asthma detection status.
7. ### Edge Impulse Platform:
   Used to train a machine learning model using collected sensor data for asthma detection.

<p align='center'>
    <img src="./Gallery/ezgif.com-video-to-gif-converter.gif" alt="drawing" width="800"/>
</p>

## Functionality of the Code:
### Respiratory Monitoring:
   The INMP441 microphone continuously captures respiratory sounds.
The data is processed to identify abnormalities, such as wheezing or irregular breathing patterns.

### Data Collection:
   Sensors collect real-time data on gas levels, temperature, humidity, heart rate, and oxygen saturation.

   
All data is synchronized using RTOS to ensure efficient multitasking.

### Real-Time Operating System (RTOS):
   RTOS manages tasks such as data acquisition, sound processing, and display updates.
High-priority tasks like respiratory monitoring are processed without delay.

### Machine Learning Model:
   The respiratory and environmental data is forwarded to the Edge Impulse platform for training.
The trained model, capable of detecting asthma-related patterns, is deployed on the ESP32 for real-time predictions.

### Prediction and Alerts:
   The ML model analyzes respiratory sounds and environmental data in real-time.
If abnormal patterns or triggers are detected, the buzzer sounds an alert, and the display shows a warning.

### Display Interface:
   The OLED screen displays vital data, including respiratory status, temperature, humidity, heart rate, and asthma detection status.
The interface is intuitive and user-friendly.

### Data Forwarding:
   The ESP32 is connected to Edge Impulse through a provided code interface.
Collected data is uploaded for training and refining the ML model.


# Software 
## python Installation
Download the latest version Python from [https://www.python.org/downloads/] for Windows / linux.

Run the installer and make sure to check the box that says Add Python to PATH during the installation process.

Once installed, restart the Command Prompt and try running:
``` python --version ```

## Node.js Installation
Install Node.js v20 or above on your host computer, using [https://nodejs.org/en/] 
install it selecting the option: Automatic install the necessary

## Edge Impulse CLI
### Windows:

Install WSL on your Windows machine, To install WSL, open PowerShell as an Administrator and run:
``` wsl --install ```


restart the Computer


open PowerShell as an Administrator and run:
``` wsl ```
follow the Linux setup.

## Linux :

```
curl -sL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs
node -v
```

Let's verify the node installation directory:
```
npm config get prefix
```

If it returns /usr/local/, run the following commands to change npm's default directory:
```
mkdir ~/.npm-global
npm config set prefix '~/.npm-global'
echo 'export PATH=~/.npm-global/bin:$PATH' >> ~/.profile
```

```
sudo apt update && sudo apt install -y build-essential
```

```
make --version
```

Install the CLI tools via:

```
npm install -g edge-impulse-cli
```
[https://docs.edgeimpulse.com/docs/tools/edge-impulse-cli/cli-installation]
[https://docs.edgeimpulse.com/docs/tools/edge-impulse-cli/cli-data-forwarder]
### or Arduino CLI
[https://arduino.github.io/arduino-cli/1.1/installation/]

### Data Forwarding
1. Create a Project in Edge Impulse Platform.
2. click to "Connect your device or development board".
3. Upload the Given Code.
4. close the Arduino IDE.
5. Remove and connect the ESP32 to the Laptop.
6. Open CMD/Power shell/ WSL/ Linux.
7. Run
   ```
    edge-impulse-data-forwarder --frequency 50
   ```
8. Sample is been Uploaded.


----
# Telegram Bot

## Step 1: Create a Telegram Bot
1. Open Telegram and search for [BotFather](https://t.me/BotFather).
2. Type ```/newbot``` to create a new bot.
3. Follow the instructions mentioned in a chat.
4. After creation, BotFather will provide you with a Bot Token. Save this token, as you’ll need it later.
5. Type ```/setprivacy```
6. Select the bot, and then select "Disable".
7. Type ```/setcommands```
8. Select the bot, and then type " status - Get all health parameters ".

## Step 2: Get Your Chat ID / Group ID
### private updates
1. Open Telegram and search [IDBot](https://t.me/y9idBot)
2. open it and clic on start.
3. The bot will return the ID, this ID is called Chat ID.
   
### Group updates
1. Add Bot to the Group.
2. Set the bot as an Admin.
3. Open this link in your browser (replace with your bot token):

```https://api.telegram.org/bot<token>/getUpdates```

Ex: ```https://api.telegram.org/bot8151804527:AAHP_kGHTDYdG6ioc3ekMNnbMlE1sezd5nDA/getUpdates```

4. Look for "chat": {"id": -XXXXXXXXX, "title": "Your Group Name"}.
5. XXXXXXXXX represents the chat id.

## Step 3: Set Up Your another ESP32 to Send Messages to Telegram
1. we will be using [witnessmenow’s Universal-Arduino-Telegram-Bot library from GitHub](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot).
Extract the library and add it to the libraries folder path of Arduino IDE.

2. Open ML_telegram code and replace the BOTtoken and CHAT_ID with your token and ID.
3. Replace the Wifi credentials.
4. upload ML_telegram code.
   ![image](https://github.com/Saifali4604/Asthma_Detection_on_ESP32_using_ML/blob/main/Gallery/WhatsApp%20Image%202025-02-02%20at%2003.00.09_aaa3cf97.jpg)

   ![image](https://github.com/Saifali4604/Asthma_Detection_on_ESP32_using_ML/blob/main/Gallery/WhatsApp%20Image%202025-02-02%20at%2003.04.15_3a5625e9.jpg)
