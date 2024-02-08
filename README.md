# laser_engraver_coolant_switcher

The air coolant is commonly used in laser engraving and cutting to get higher quality.
Normanly you should use LOW air flow during engraving to keep the smoke away of laser lens and HIGH air flow during cutting to have clean cut edges.
This project should help to control coolant supply for a grbl-based laser engraver.

## Description
The coolant switcher consists of two parts: 
- python script, that communicates with Lightburn and detects Laser ON/OFF and Coolant ON/OFF Gcode commands
- ESP32 board, that switches ON/OFF low and high air flow 

The dependency between air flow and Gcode commands is the following:

|Air flow|Gcode command|   Meaning   |
|:------:|:-----------:|:-----------:|
|  LOW   |     M4      |  Laser on   |
|  HIGH  |     M8      | Coolant on  |
|  LOW   |     M9      | Coolant off |
|  OFF   |     M5      |  Laser off  |

## Prerequisites
- Grbl-based laser engraver (for example Ortur Laser Master 3 LE)
- Lightburn software
- WiFi access point
- Aircompressor for air supply
- python3 is installed on your OS

## Used hardware
- ESP32 board
- 0.96 OLED display 128x64 (not required)
- 2pcs of 12VDC pneumatic solenoid valve
- 2pcs of pneumatic throttle valve
- 12VDC power supply
- Custom PCB (see schematics)

## Piping

## Wiring

## How to use
- Start you WiFi access point (smartphone can be used)
- Use your SSID and PASS in `include/project_config.h`
- Build and download ESP32 FW, power on
- Get the ESP32 IP address from your WiFi AP. Use the IP in `coolanter/coolanter.py`: `SWITCHER_IP`
- Connect your laser engraver to WiFi AP. Use its IP in `coolanter/coolanter.py`: `ENGRAVER_IP`
- Start `coolanter/coolanter.py`
- Start Lightburn. Add new grbl device with localhost address (127.0.0.1)
- Enjoy!