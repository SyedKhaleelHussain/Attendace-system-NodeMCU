# Attendace-system-ESP32/Nodemcu

🕒 ESP32 RFID Attendance System 📋
Welcome to the Smart Attendance System project powered by ESP32!
This device uses RFID tags, an OLED display, RTC, and an SD card to securely log employee attendance and even host a web interface for data access! 🌐💡

🔧 Hardware Used

Component	Description
📟 ESP32	Main microcontroller with WiFi & SPI support
🪪 MFRC522	RFID module to scan employee cards
💾 Micro SD Card	Stores all attendance logs & employee data
🕒 DS3231 RTC	Keeps track of accurate date & time
🖥️ OLED Display	Displays messages and animations
🔔 Buzzer	Audio feedback for check-in/out
🔌 Pin Connections
📲 RFID (MFRC522) ➡️ ESP32

MFRC522 Pin	ESP32 Pin
SDA (SS)	GPIO 5
SCK	GPIO 18 (VSPI default)
MOSI	GPIO 23 (VSPI default)
MISO	GPIO 19 (VSPI default)
RST	GPIO 4
GND	GND
VCC	3.3V
💾 SD Card Module ➡️ ESP32 (using HSPI)

SD Card Pin	ESP32 Pin
CS (SS)	GPIO 15
SCK	GPIO 14
MOSI	GPIO 13
MISO	GPIO 12
VCC	3.3V
GND	GND
🕒 RTC Module (DS3231) ➡️ ESP32

DS3231 Pin	ESP32 Pin
SDA	GPIO 21
SCL	GPIO 22
VCC	3.3V
GND	GND
🖥️ OLED Display (SH1106) ➡️ ESP32

OLED Pin	ESP32 Pin
SDA	GPIO 21
SCL	GPIO 22
VCC	3.3V
GND	GND
🔔 Buzzer ➡️ ESP32

Buzzer Pin	ESP32 Pin
+ (Signal)	GPIO 16
- (GND)	GND
🌐 Web Features
📥 Download attendance logs from the browser

📤 Upload new employee files (employees.csv)

🗑️ Delete unwanted files

🧾 View SD card contents

WiFi Access Point is created automatically:

makefile
Copy
Edit
SSID: ESP32-Attendance
Password: 12345678
Access the system by connecting to the AP and navigating to 192.168.4.1 in your browser.

📁 File Structure on SD Card
attendance.csv: Main log file with UID, Name, Date, TimeIn, TimeOut

employees.csv: Map of UID, Name for registered employees

🎯 How It Works
👤 Scan your RFID tag.

🖥️ Name and timestamp are displayed on the OLED.

✅ If already checked in, it records checkout.

📁 Data is stored on SD card in CSV format.

🌍 Access logs from any device via WiFi!

🛠️ Libraries Used
Wire.h, SPI.h, MFRC522.h for RFID

RTClib.h for RTC

Adafruit_SH110X.h and Adafruit_GFX.h for OLED

SD.h, FS.h for SD card

WiFi.h, WebServer.h for hosting the web interface

✅ To-Do Before Uploading
Format your SD card as FAT32

Create employees.csv with format:

objectivec
Copy
Edit
UID123456,John Doe
UID789012,Jane Smith
📸 Screens Include
✅ Check-in screen

📤 Check-out screen

❌ Unknown card screen

💤 Idle animations

⚠️ Error messages
