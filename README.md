# Zoo Catapult v3.4 - Control System

## 🚀 Overview
This repository contains the dual-microcontroller software for the Zoo Catapult project. The system automates the tensioning, loading, and launching of animal enrichment payloads.

## 🛠 Hardware Architecture
* **Primary Logic:** Arduino Mega 2560
* **Communication:** ESP32-CAM (Asynchronous Serial)
* **Actuators:** Stepper Motor (Auger), Linear Actuator (Lock), Servo (Rotator)

## 📁 Repository Structure
* `Mega_Control_System/`: Main state machine, motor timing, and safety interrupts.
* `ESP32_Wireless_UI/`: WiFi Access Point and Web Server implementation.
* `Documentation/`: PDF schematics and UI screenshots.

## 💻 How to Install
1. **Mega:** Open `Mega_Control_System.ino` in Arduino IDE. Install `AccelStepper` and `Servo` libraries. Upload to Mega 2560.
2. **ESP32:** Open `ESP32_Wireless_UI.ino`. Ensure the ESP32 board manager is installed. Upload to ESP32-CAM.
3. **Connect:** Ensure common ground between both controllers and 12V power supply.
