#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions
#define RFID_SS_PIN    5
#define RFID_RST_PIN   4
#define SD_SS_PIN      15
#define BUZZER_PIN     16

// SPI pins for HSPI (SD card)
#define SD_SCK         14
#define SD_MISO        12
#define SD_MOSI        13

// RFID setup (uses default VSPI)
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// SD card via HSPI
SPIClass hspi(HSPI);

// RTC
RTC_DS3231 rtc;

// Variables
String currentDate = "";
unsigned long lastAnimationTime = 0;
int animationFrame = 0;

// Web Server
WebServer server(80);
const char *ssid = "ESP32-Attendance";
const char *password = "12345678";
File uploadFile;

// Graphics
const unsigned char logo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x70, 0x01, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe3, 0x80, 0x00, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x78, 0x01, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe7, 0x80, 0x00, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x78, 0x01, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe7, 0x80, 0x00, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x78, 0x01, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe7, 0x80, 0x00, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x78, 0x01, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe7, 0x80, 0x00, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x78, 0x01, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe7, 0x80, 0x00, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x79, 0xff, 0xe7, 0x9f, 0xff, 0xcf, 0xff, 0xe7, 0x83, 0xfe, 0x3f, 0xff, 0x9f, 0xff, 0xc0,
  0x00, 0x79, 0xff, 0xe7, 0x9c, 0x03, 0xcf, 0x00, 0xe7, 0x83, 0xfe, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x79, 0xff, 0xe7, 0x9c, 0x03, 0xcf, 0x00, 0xe7, 0x83, 0xfe, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x79, 0xff, 0xe7, 0x9c, 0x03, 0xcf, 0x00, 0xe7, 0x83, 0xfe, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x79, 0xff, 0xe7, 0x9c, 0x03, 0xcf, 0x00, 0xe7, 0x83, 0xfe, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x79, 0xff, 0xe7, 0x9e, 0x03, 0xcf, 0x00, 0xe7, 0x83, 0xfe, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9f, 0xff, 0xcf, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9f, 0xff, 0xcf, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9f, 0xff, 0xcf, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9f, 0xff, 0xcf, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9f, 0xff, 0xcf, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9f, 0xff, 0xcf, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x3e, 0x0f, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x3e, 0x0f, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x1f, 0x0f, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9e, 0x1f, 0x0f, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x0f, 0x8f, 0x00, 0xe7, 0x80, 0x0e, 0x38, 0x07, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x0f, 0x8f, 0xff, 0xe7, 0xff, 0xfe, 0x3f, 0xff, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x07, 0xcf, 0xff, 0xe7, 0xff, 0xfe, 0x3f, 0xff, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x07, 0xcf, 0xff, 0xe7, 0xff, 0xfe, 0x3f, 0xff, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9c, 0x03, 0xcf, 0xff, 0xe7, 0xff, 0xfe, 0x3f, 0xff, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9e, 0x03, 0xcf, 0xff, 0xe3, 0xff, 0xfe, 0x3f, 0xff, 0x9c, 0x03, 0xc0,
  0x00, 0x78, 0x00, 0x07, 0x9e, 0x01, 0xcf, 0xff, 0xe7, 0xff, 0xfe, 0x3f, 0xff, 0x9c, 0x03, 0xc0,
  0x00, 0x30, 0x00, 0x03, 0x0c, 0x00, 0xc7, 0xff, 0xe3, 0xff, 0xfe, 0x3f, 0xff, 0x1c, 0x01, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  // Your logo bitmap data here
};

const unsigned char pirogon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00,
  0x1f, 0xfc, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x0c, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00,
  0x0c, 0x0d, 0x7f, 0xbf, 0xd8, 0x07, 0xf7, 0xf8, 0x0d, 0xfd, 0x7f, 0xbf, 0xd9, 0xf7, 0xf7, 0xf8,
  0x0d, 0xfb, 0x71, 0xb0, 0xd9, 0xf6, 0x36, 0x18, 0x0c, 0x03, 0x7f, 0xb0, 0xd8, 0x36, 0x36, 0x18,
  0x0c, 0x03, 0x77, 0x30, 0xd8, 0x36, 0x16, 0x18, 0x0c, 0x03, 0x63, 0xbf, 0xdf, 0xf7, 0xf6, 0x18,
  0x0c, 0x03, 0x61, 0xbf, 0xdf, 0xf7, 0xf6, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
  // Your pirogon bitmap data here
};

void setup() {
  Serial.begin(115200);

  // Initialize components
  pinMode(BUZZER_PIN, OUTPUT);
  Wire.begin(21, 22);
  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();

  // Show splash screen
  showSplashScreen();
  delay(2000);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // Setup WiFi
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Hotspot Created!");
  display.setCursor(0, 15);
  display.print("SSID: ");
  display.println(ssid);
  display.setCursor(0, 30);
  display.print("IP: ");
  display.println(IP);
  display.display();
  delay(2000);

  // RTC

  //rtc.adjust(DateTime(2025, 4, 24, 10, 30, 30)); // YYYY, MM, DD, HH, MM, SS;
  //Serial.println("RTC time manually set.");

  if (!rtc.begin()) {
    showError("RTC not found!");
    while (1);
  }

  // Update current date
  DateTime now = rtc.now();
  currentDate = formatDate(now);

  // SD Card
  hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS_PIN);
  if (!SD.begin(SD_SS_PIN, hspi)) {
    showError("SD card error!");
    while (1);
  }

  // Create header if file doesn't exist
  if (!SD.exists("/attendance.csv")) {
    File logFile = SD.open("/attendance.csv", FILE_WRITE);
    if (logFile) {
      logFile.println("UID,Name,Date,TimeIn,TimeOut");
      logFile.close();
    }
  }

  // Setup Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/download", HTTP_GET, handleFileDownload);
  server.on("/upload", HTTP_POST, []() {
    server.send(200); // Respond to POST
  }, handleFileUpload);
  server.on("/delete", HTTP_GET, handleFileDelete);
  server.on("/list", HTTP_GET, handleFileList);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Debug SD card at startup
  debugSDCard();

  showWelcomeScreen();
  delay(1000);
  display.clearDisplay();
}

void loop() {
  server.handleClient();

  // Animation when idle
  if (millis() - lastAnimationTime > 300) {
    animationFrame = (animationFrame + 1) % 4;
    drawIdleAnimation(animationFrame);
    lastAnimationTime = millis();
  }

  // Look for new card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = getUID();
  String name = getEmployeeName(uid);

  if (name == "") {
    showUnknownCardScreen(uid);
    beepError();
  } else {
    DateTime now = rtc.now();
    String today = formatDate(now);
    String timeStr = formatTime(now);

    // Check if it's a new day
    if (today != currentDate) {
      currentDate = today;
    }

    // Check if this UID has an open check-in (no check-out) for today
    bool hasOpenCheckIn = hasOpenAttendance(uid, today);

    if (hasOpenCheckIn) {
      // Update the check-out time
      if (updateTimeOut(uid, today, timeStr)) {
        showCheckOutScreen(name, timeStr, today);
      } else {
        showError("Update failed");
      }
    } else {
      // Create new check-in record
      logAttendance(uid, name, today, timeStr, "");
      showCheckInScreen(name, timeStr, today);
    }

    beepSuccess();
  }

  rfid.PICC_HaltA();
  delay(1500);
}

// Check if employee has open attendance record
bool hasOpenAttendance(String uid, String date) {
  File file = SD.open("/attendance.csv");
  if (!file) return false;

  bool found = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    // Skip empty lines or header
    if (line.length() == 0 || line.startsWith("UID")) continue;

    // Parse the line
    int comma1 = line.indexOf(',');
    int comma2 = line.indexOf(',', comma1 + 1);
    int comma3 = line.indexOf(',', comma2 + 1);
    int comma4 = line.indexOf(',', comma3 + 1);

    if (comma1 == -1 || comma2 == -1 || comma3 == -1) continue;

    String lineUID = line.substring(0, comma1);
    String lineDate = line.substring(comma2 + 1, comma3);
    String lineTimeOut = (comma4 != -1) ? line.substring(comma4 + 1) : "";

    if (lineUID == uid && lineDate == date && lineTimeOut.length() == 0) {
      found = true;
      break;
    }
  }
  file.close();
  return found;
}

// Update time-out in attendance record
bool updateTimeOut(String uid, String date, String timeOut) {
  File tempFile = SD.open("/temp.csv", FILE_WRITE);
  if (!tempFile) return false;

  File logFile = SD.open("/attendance.csv");
  if (!logFile) {
    tempFile.close();
    return false;
  }

  bool updated = false;

  // Read header
  if (logFile.available()) {
    String header = logFile.readStringUntil('\n');
    tempFile.println(header);
  }

  // Process each line
  while (logFile.available()) {
    String line = logFile.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) continue;

    // Parse the line
    int comma1 = line.indexOf(',');
    int comma2 = line.indexOf(',', comma1 + 1);
    int comma3 = line.indexOf(',', comma2 + 1);
    int comma4 = line.indexOf(',', comma3 + 1);

    if (comma1 == -1 || comma2 == -1 || comma3 == -1) {
      tempFile.println(line);
      continue;
    }

    String lineUID = line.substring(0, comma1);
    String lineDate = line.substring(comma2 + 1, comma3);
    String lineTimeOut = (comma4 != -1) ? line.substring(comma4 + 1) : "";

    // Check if this is the record we want to update
    if (lineUID == uid && lineDate == date && lineTimeOut.length() == 0 && !updated) {
      // Update the time-out
      String updatedLine = line.substring(0, comma4 + 1) + timeOut;
      tempFile.println(updatedLine);
      updated = true;
    } else {
      // Copy the line as-is
      tempFile.println(line);
    }
  }

  logFile.close();
  tempFile.close();

  if (updated) {
    SD.remove("/attendance.csv");
    SD.rename("/temp.csv", "/attendance.csv");
  } else {
    SD.remove("/temp.csv");
  }

  return updated;
}

// Web Server Handlers
// Updated handleRoot function with proper form
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Attendance System</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body {font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5;}";
  html += "h1 {color: #333;}";
  html += ".container {background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1);}";
  html += "a {color: #0066cc; text-decoration: none;}";
  html += "a:hover {text-decoration: underline;}";
  html += "button {background-color: #4CAF50; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer;}";
  html += "button:hover {background-color: #45a049;}";
  html += "table {width: 100%; border-collapse: collapse; margin-top: 20px;}";
  html += "th, td {border: 1px solid #ddd; padding: 8px; text-align: left;}";
  html += "th {background-color: #f2f2f2;}";
  html += "form {margin-top: 20px;}";
  html += ".status {margin-top: 10px; padding: 10px; border-radius: 4px;}";
  html += ".success {background-color: #dff0d8; color: #3c763d;}";
  html += ".error {background-color: #f2dede; color: #a94442;}";
  html += "</style>";
  html += "<script>";
  html += "function showStatus() {";
  html += "  const urlParams = new URLSearchParams(window.location.search);";
  html += "  const uploadStatus = urlParams.get('upload');";
  html += "  if (uploadStatus === 'success') {";
  html += "    document.getElementById('uploadStatus').innerHTML = ";
  html += "      '<div class=\"status success\">File uploaded successfully!</div>';";
  html += "  } else if (uploadStatus === 'error') {";
  html += "    document.getElementById('uploadStatus').innerHTML = ";
  html += "      '<div class=\"status error\">File upload failed!</div>';";
  html += "  }";
  html += "}";
  html += "</script>";
  html += "</head><body onload='showStatus()'>";
  html += "<div class='container'>";
  html += "<h1>Attendance System</h1>";
  html += "<p>Connected to: " + String(ssid) + "</p>";

  // Status div (messages will be inserted here by JavaScript)
  html += "<div id='uploadStatus'></div>";

  // File upload form
  html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
  html += "<h3>Upload File</h3>";
  html += "<input type='file' name='file' required>";
  html += "<button type='submit' id='uploadButton'>Upload</button>";
  html += "</form>";

  // File list table
  html += "<div class='file-list'>";
  html += "<h3>SD Card Files:</h3>";
  html += "<table><tr><th>Name</th><th>Size</th><th>Actions</th></tr>";

  File root = SD.open("/");
  while (File file = root.openNextFile()) {
    String fileName = file.name();
    if (!file.isDirectory()) {
      html += "<tr>";
      html += "<td>" + fileName + "</td>";
      html += "<td>" + formatFileSize(file.size()) + "</td>";
      html += "<td>";
      html += "<a href='/download?file=" + fileName + "'>Download</a> | ";
      html += "<a href='/delete?file=" + fileName + "' onclick='return confirm(\"Are you sure?\")'>Delete</a>";
      html += "</td>";
      html += "</tr>";
    }
    file.close();
  }
  root.close();

  html += "</table></div>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// Helper function to format file sizes
String formatFileSize(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0, 1) + " KB";
  } else {
    return String(bytes / (1024.0 * 1024.0), 1) + " MB";
  }
}

void handleFileDownload() {
  String filename = server.arg("file");
  if (filename.length() == 0 || filename.indexOf("..") != -1) {
    server.send(400, "text/plain", "Invalid filename");
    return;
  }

  File file = SD.open("/" + filename);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  if (file.isDirectory()) {
    server.send(400, "text/plain", "Cannot download directory");
    file.close();
    return;
  }

  server.sendHeader("Content-Type", "application/octet-stream");
  server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
  server.sendHeader("Connection", "close");

  server.setContentLength(file.size());
  server.send(200);

  byte buf[256];
  int sent;
  while (file.available()) {
    sent = file.read(buf, 256);
    server.client().write(buf, sent);
  }

  file.close();
}


// Updated handleFileUpload function
void handleFileUpload() {
  static File uploadFile;
  static String uploadedFilename;

  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("\nUpload Start: %s\n", upload.filename.c_str());
    uploadedFilename = upload.filename;
    if (!uploadedFilename.startsWith("/")) uploadedFilename = "/" + uploadedFilename;

    // Close any existing file and remove if exists
    if (uploadFile) uploadFile.close();
    if (SD.exists(uploadedFilename)) SD.remove(uploadedFilename);

    uploadFile = SD.open(uploadedFilename, FILE_WRITE);
    if (!uploadFile) {
      Serial.println("SD Open Failed");
      return;
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      size_t written = uploadFile.write(upload.buf, upload.currentSize);
      if (written != upload.currentSize) {
        Serial.println("Write Mismatch");
      }
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    bool success = false;

    if (uploadFile) {
      uploadFile.flush();
      uploadFile.close();

      // Critical: Wait for SD card to complete writes
      delay(250);

      // Verification
      File testFile = SD.open(uploadedFilename);
      if (testFile) {
        success = true;
        testFile.close();
        Serial.printf("Upload Verified: %d bytes\n", testFile.size());
      }
    }

    // Debug output
    Serial.printf("Final Status: %s\n", success ? "SUCCESS" : "FAILED");

    // Force client disconnect before redirect
    server.client().stop();

    // Immediate redirect (302 Found)
    server.sendHeader("Location", "/?upload=" + String(success ? "success" : "error"));
    server.send(302, "text/plain", "");
  }
  else {
    if (uploadFile) uploadFile.close();
    server.sendHeader("Location", "/?upload=error");
    server.send(302, "text/plain", "");
  }
}

void handleFileDelete() {
  String filename = server.arg("file");
  if (filename.length() == 0 || filename.indexOf("..") != -1) {
    server.send(400, "text/plain", "Invalid filename");
    return;
  }

  if (SD.remove("/" + filename)) {
    server.send(200, "text/plain", "File deleted");
  } else {
    server.send(500, "text/plain", "Delete failed");
  }
}

void handleFileList() {
  String html = "<html><head><title>File List</title>";
  html += "<style>body {font-family: Arial; margin: 20px;}";
  html += "table {border-collapse: collapse; width: 100%;}";
  html += "th, td {border: 1px solid #ddd; padding: 8px; text-align: left;}";
  html += "th {background-color: #f2f2f2;}</style></head>";
  html += "<body><h1>SD Card Files</h1>";
  html += "<table><tr><th>Name</th><th>Size</th><th>Action</th></tr>";

  File root = SD.open("/");
  while (File file = root.openNextFile()) {
    String fileName = file.name();
    html += "<tr>";
    html += "<td>" + fileName + "</td>";
    html += "<td>" + formatFileSize(file.size()) + "</td>";
    if (!file.isDirectory()) {
      html += "<td><a href='/download?file=" + fileName + "'>Download</a></td>";
    } else {
      html += "<td></td>";
    }
    html += "</tr>";
    file.close();
  }
  root.close();

  html += "</table>";
  html += "<p><a href='/'>Back to main page</a></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}


// Add this function to debug SD card contents
void debugSDCard() {
  Serial.println("\nSD Card Contents:");
  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open root directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    Serial.print(file.name());
    if (file.isDirectory()) {
      Serial.println("/");
    } else {
      Serial.print("\t");
      Serial.println(file.size(), DEC);
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
}

// Add this function to verify uploads
void verifyUpload(String filename) {
  File file = SD.open(filename);
  if (file) {
    Serial.print("Verified ");
    Serial.print(filename);
    Serial.print(" - Size: ");
    Serial.println(file.size());
    file.close();
  } else {
    Serial.print("Failed to verify ");
    Serial.println(filename);
  }
}

// Display Functions
void showSplashScreen() {
  display.clearDisplay();
  display.drawBitmap(0, 0, logo, 128, 64, SH110X_WHITE);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.display();
}

void showWelcomeScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Draw border
  display.drawRect(0, 0, 124, 64, SH110X_WHITE);

  // Title
  display.setCursor(15, 5);
  display.println("WELCOME TO");

  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("ATTENDANCE");
  display.setCursor(30, 40);
  display.println("SYSTEM");
  display.display();
}

void showCheckInScreen(String name, String time, String date) {
  display.clearDisplay();

  // Draw border with animation
  for (int i = 0; i < 5; i++) {
    display.drawRect(i, i, 124 - 2 * i, 64 - 2 * i, SH110X_WHITE);
    display.display();
    delay(50);
  }

  // Title
  display.fillRect(0, 0, 128, 12, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setCursor(40, 3);
  display.println("CHECK IN");
  display.setTextColor(SH110X_WHITE);

  // Employee info
  display.setCursor(10, 20);
  display.print("Name: ");
  display.println(name);

  display.setCursor(10, 35);
  display.print("Time: ");
  display.println(time);

  display.setCursor(10, 45);
  display.print("Date:");
  display.println(date);

  // Status icon
  display.fillCircle(110, 50, 8, SH110X_WHITE);
  display.fillCircle(110, 50, 4, SH110X_BLACK);

  display.display();
}

void showCheckOutScreen(String name, String time, String date) {
  display.clearDisplay();

  // Draw border with animation
  for (int i = 0; i < 5; i++) {
    display.drawRect(i, i, 128 - 2 * i, 64 - 2 * i, SH110X_WHITE);
    display.display();
    delay(50);
  }

  // Title
  display.fillRect(0, 0, 128, 12, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setCursor(35, 3);
  display.println("CHECK OUT");
  display.setTextColor(SH110X_WHITE);

  // Employee info
  display.setCursor(10, 20);
  display.print("Name: ");
  display.println(name);

  display.setCursor(10, 35);
  display.print("Time: ");
  display.println(time);
  display.setCursor(10, 45);
  display.print("Date:");
  display.println(date);

  // Status icon
  display.fillCircle(110, 50, 8, SH110X_WHITE);
  display.drawLine(107, 47, 113, 53, SH110X_BLACK);
  display.drawLine(107, 53, 113, 47, SH110X_BLACK);

  display.display();
}

void showUnknownCardScreen(String uid) {
  display.clearDisplay();

  // Draw border
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);

  // Title
  display.fillRect(0, 0, 128, 12, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setCursor(30, 3);
  display.println("UNKNOWN CARD");
  display.setTextColor(SH110X_WHITE);

  // UID
  display.setCursor(10, 25);
  display.print("UID: ");
  display.println(uid);

  // Message
  display.setCursor(10, 40);
  display.println("Not registered");

  // Warning icon
  display.fillTriangle(110, 45, 105, 55, 115, 55, SH110X_WHITE);
  display.drawChar(110, 47, '!', SH110X_BLACK, SH110X_WHITE, 1);

  display.display();
}

void drawIdleAnimation(int frame) {
  display.clearDisplay();

  // Draw border
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);

  // Title
  display.setCursor(30, 5);
  display.println("READY TO SCAN");

  int x = 5 + frame * 30;

  display.drawBitmap(x, 25, pirogon, 64, 16, SH110X_WHITE); // 64x16 logo centered

  // RFID icon (kept at bottom right)
  display.drawRect(90, 50, 30, 12, SH110X_WHITE);
  display.fillRect(100, 45, 10, 5, SH110X_WHITE);

  display.display();
}

// Helper Functions
String formatDate(DateTime dt) {
  char buf[11];
  sprintf(buf, "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());
  return String(buf);
}

String formatTime(DateTime dt) {
  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
  return String(buf);
}

String getUID() {
  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidStr += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

String getEmployeeName(String uid) {
  File file = SD.open("/employees.csv");
  if (!file) return "";

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;
    int comma = line.indexOf(',');
    String storedUID = line.substring(0, comma);
    String name = line.substring(comma + 1);
    if (storedUID.equalsIgnoreCase(uid)) {
      file.close();
      return name;
    }
  }
  file.close();
  return "";
}

void logAttendance(String uid, String name, String date, String timeIn, String timeOut) {
  File logFile = SD.open("/attendance.csv", FILE_APPEND);
  if (logFile) {
    logFile.printf("%s,%s,%s,%s,%s\n",
                   uid.c_str(),
                   name.c_str(),
                   date.c_str(),
                   timeIn.c_str(),
                   timeOut.c_str());
    logFile.close();
  }
}

void showError(String message) {
  display.clearDisplay();
  display.fillRect(0, 0, 128, 16, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setCursor(40, 4);
  display.println("ERROR");
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 30);
  display.println(message);
  display.display();
  beepError();
}

void beepSuccess() {
  tone(BUZZER_PIN, 1000, 200);
  delay(50);
  tone(BUZZER_PIN, 1500, 200);
}

void beepError() {
  tone(BUZZER_PIN, 400, 500);
  delay(100);
  tone(BUZZER_PIN, 300, 500);
}
