# Astama_Detection

# Software Installation
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


restart
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


