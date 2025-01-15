# Astama_Detection

![image](https://github.com/Saifali4604/Asthma_Detection_on_ESP32_using_ML/blob/main/IMG_20250116_013132.jpg)


# Software Method 1
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

# Method 2
If you have issues installing the CLI you can also collect data from fully-supported development boards directly using recent versions of Google Chrome and Microsoft Edge. 


Collecting data from real devices is the first step of any embedded machine learning solution, and to make this even easier we’re adding support for Web Serial to the Edge Impulse Studio.


This means that you can now collect data from any fully-supported development board straight from your browser, without any other tools installed on your computer.


To get started just open the latest version of Google Chrome or Microsoft Edge, go to the data acquisition page in the Studio, and click ’Connect using WebUSB’.


Select your development board, and you’re done.


You can now collect data from any of the built-in sensors, see a visual preview on boards with a camera, and verify your model.

[https://edgeimpulse.com/blog/collect-sensor-data-straight-from-your-web-browser/]
