<!-- Generate Readme file for this project -->

# Water Pump Controller - ESP8266 + LoRA

<!-- Badges - Arduino, C++, PlatformIO, ESP8266 -->
![Arduino CI](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![C++ CI](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![PlatformIO CI](https://img.shields.io/static/v1?label=Build&message=PlatformIO&color=orange&style=for-the-badge)
![ESP8266 CI](https://img.shields.io/static/v1?label=ESP8266&message=NodeMCU&logo=espressif&color=red&style=for-the-badge)
![License](https://img.shields.io/github/license/amjed-ali-k/WaterPumpAutomation?style=for-the-badge)
[![Bitcoin](https://img.shields.io/badge/Bitcoin-F7931A?logo=bitcoin&logoColor=fff&style=for-the-badge
)](bitcoin://bc1qum3cr9cj7lxkyqgsljzphcugaprr3thhk5yl8r)

## Description

This project is a water pump controller that uses a ESP8266 and a LoRA module to communicate with a LoRA gateway. The controller is able to receive commands from the gateway to turn on/off the water pump and send back the status of the pump.

Motor control is done using a relay module. The controller also has a switch to manually turn on/off the pump. The status of the pump is displayed on a OLED display.

It also has a lock feature that prevents the pump from being turned on from the controller. Motor auto locks if it turned on morethan finite times.  The lock can be cleared using the switch from reciever side.

### Why locking the motor?

The motor is being used to pump water from a well inside the woods, which also is very far away from the institution where I'm working. The well is filled with rainwater and the motor needs periodic maintenance. By implementing this feature, the motor can only be turned on from the controller a few times before it locks. When it is locked, a person has to go to the well and clear the lock using the switch. This prevents the motor from being unattended for a long period. Also, it privents from being turned on by mistake and also prevents the motor from being turned on when it is not needed.

## Hardware

- ESP8266
- LoRA module
- Relay module
- 12V power supply
- SSD1306 OLED display
- Switch

## Software

- PlatformIO
- Arduino framework
- VSCode

## Libraries

- LoRa by Sandeep Mistry (<https://github.com/sandeepmistry/arduino-LoRa>)
- Adafruit SSD1306 (<<https://github.com/adafruit/Adafruit_SSD1306>)
- Adafruit GFX (<<https://github.com/adafruit/Adafruit-GFX-Library>)
