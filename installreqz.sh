#!/bin/sh
#ESP-IDF Prerequisite installer
#Exofense - IOT team
echo "Detecting Python version: "
python -V
echo "Starting..."
sudo apt update && sudo apt upgrade -y
echo "Setting alternative: python3 to python"
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 1
echo "Installing pip3:"
sudo apt install -y python3-pip
echo "Setting alternative: pip3 to pip"
sudo update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 1
echo "Installing prerequisites for ESP-IDF:"
sudo apt install -y git wget flex bison gperf python-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util
echo "Installed."
echo "Creating esp directory:"
mkdir ~/esp
echo "Switching to ~/esp"
cd ~/esp
echo "Cloning:"
git clone --recursive https://github.com/espressif/esp-idf.git
echo "ESP-IDF repository cloned. Setting up compiler and other prerequisites:"
cd esp-idf
. ./install.sh
pip install --upgrade pip
cd ../..
echo "Completed."
echo "Please refer to the README.md for further instructions."
