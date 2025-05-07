#include <SD.h>
#include <SPI.h>
#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// Pin definitions
const int HX711_DOUT = 4;     // HX711 DT
const int HX711_SCK = 5;      // HX711 SCK
const int LED_RED = 2;
const int LED_GREEN = 3;
const int SD_CS = 10;         // SD card Chip Select (Uno SPI: 10 for CS)

// Global variables
HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);
float loadCellReading;
float timeCounter = 0;
bool sdInitialized = false;
File dataFile;

void setup() {
  Serial.begin(57600);
  delay(100);

  // Initialize LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Initialize SD card
  pinMode(SD_CS, OUTPUT);
  if (SD.begin(SD_CS)) {
    Serial.println("SD card initialized.");
    sdInitialized = true;
    if (SD.exists("Data.txt")) {
      SD.remove("Data.txt");
    }
  } else {
    Serial.println("ERROR: SD card failed to initialize.");
    sdInitialized = false;
  }

  // Initialize Load Cell
  LoadCell.begin();
  bool tare = true;
  unsigned long stabilizingTime = 2000;
  LoadCell.start(stabilizingTime, tare);
  LoadCell.setCalFactor(213.11);  // Replace with your calibration factor
}

void loop() {
  if (!sdInitialized) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    return;
  }

  // SD card is ready
  dataFile = SD.open("Data.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("ERROR: Unable to open Data.txt.");
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    return;
  }

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);

  // Main logging loop
  while (true) {
    LoadCell.update();
    loadCellReading = LoadCell.getData();

    if (loadCellReading >= 2.0) {
      timeCounter += 0.1;

      dataFile.print(timeCounter);
      dataFile.print(",");
      dataFile.println(loadCellReading);

      delay(100);  // Log every 100 ms
    }

    if (loadCellReading <= -2.0) {
      dataFile.close();
      Serial.println("Logging ended. File closed.");
      sdInitialized = false;
      return;
    }
  }
}
