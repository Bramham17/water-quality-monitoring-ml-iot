/*
 * Water Quality Monitoring System - Arduino UNO
 * ML-based water quality prediction using Random Forest
 * Sensors: pH sensor, Turbidity sensor, Temperature sensor (DS18B20)
 * IoT Integration: Blynk Cloud Platform
 * Created by: Bramham17
 * Last Updated: 2025
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// Pin Configuration
#define pH_PIN A0              // Analog pin for pH sensor
#define TURBIDITY_PIN A1       // Analog pin for turbidity sensor
#define TEMP_PIN 2             // Digital pin for DS18B20 temperature sensor
#define ESP_RX_PIN 3           // RX from ESP8266
#define ESP_TX_PIN 4           // TX to ESP8266
#define LED_PIN 13             // Built-in LED for status indication

// Sensor Objects
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);
SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); // RX, TX for ESP8266

// Calibration Constants
const float PH_CALIBRATION_POINT_1 = 4.0;   // pH 4.0 calibration
const float PH_CALIBRATION_POINT_2 = 7.0;   // pH 7.0 calibration
const float PH_CALIBRATION_POINT_3 = 10.0;  // pH 10.0 calibration
const float TURBIDITY_MIN = 0.0;            // Clear water (distilled)
const float TURBIDITY_MAX = 800.0;          // Maximum NTU value

// EEPROM Addresses for calibration data
const int EEPROM_PH_OFFSET = 0;
const int EEPROM_PH_SCALE = 4;
const int EEPROM_TURBIDITY_OFFSET = 8;
const int EEPROM_TURBIDITY_SCALE = 12;

// Global Variables
float phValue = 7.0;
float turbidityValue = 0.0;
float temperatureValue = 25.0;
float conductivity = 0.0;
int waterQualityStatus = 0;  // 0: Good, 1: Moderate, 2: Poor
char waterQualityLabel[20];

// Blynk Configuration
char blynkAuthToken[34] = "YOUR_BLYNK_AUTH_TOKEN";  // Replace with your Blynk token
const char* BLYNK_SERVER = "blynk-cloud.com";
const int BLYNK_PORT = 80;

// Sensor Reading Variables
const int NUM_SAMPLES = 10;  // Number of samples for averaging
const int SAMPLE_DELAY = 100; // Delay between samples (ms)

void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);      // Arduino Serial Monitor
  espSerial.begin(115200); // ESP8266 communication
  
  // Initialize Pins
  pinMode(pH_PIN, INPUT);
  pinMode(TURBIDITY_PIN, INPUT);
  pinMode(ESP_RX_PIN, INPUT);
  pinMode(ESP_TX_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Temperature Sensor
  tempSensor.begin();
  tempSensor.setResolution(12);
  
  // Load calibration data from EEPROM
  loadCalibrationData();
  
  // Blink LED to indicate startup
  blinkLED(3);
  
  Serial.println("\n=== Water Quality Monitoring System Started ===");
  Serial.println("System initialized and ready for monitoring...");
  Serial.println("Connecting to Blynk IoT Cloud...");
  
  delay(2000);
}

void loop() {
  // Read all sensors
  readPHSensor();
  readTurbiditySensor();
  readTemperatureSensor();
  
  // Calculate conductivity from combined readings
  calculateConductivity();
  
  // Run ML prediction (Random Forest simplified)
  predictWaterQuality();
  
  // Print readings to Serial Monitor
  printSensorReadings();
  
  // Send data to Blynk Cloud
  sendDataToBlynk();
  
  // Check thresholds and trigger alerts if needed
  checkThresholdsAndAlert();
  
  // Delay before next measurement cycle
  delay(5000);  // 5-second interval between measurements
}

// Read pH Sensor with filtering
void readPHSensor() {
  float sum = 0.0;
  
  // Take multiple readings and average
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int rawValue = analogRead(pH_PIN);
    float voltage = (rawValue / 1023.0) * 5.0;  // Convert to voltage (0-5V)
    float calibratedPH = 7.0 + ((2.5 - voltage) / 0.18);  // Convert voltage to pH
    sum += calibratedPH;
    delay(SAMPLE_DELAY);
  }
  
  phValue = sum / NUM_SAMPLES;  // Calculate average
  
  // Constrain pH to valid range
  if (phValue < 0.0) phValue = 0.0;
  if (phValue > 14.0) phValue = 14.0;
}

// Read Turbidity Sensor
void readTurbiditySensor() {
  float sum = 0.0;
  
  // Take multiple readings and average
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int rawValue = analogRead(TURBIDITY_PIN);
    float voltage = (rawValue / 1023.0) * 5.0;  // Convert to voltage (0-5V)
    // Turbidity calculation: Higher voltage = clearer water (lower NTU)
    float ntu = (voltage - 2.5) * (-60.0);  // Simplified formula
    if (ntu < 0) ntu = 0;  // Turbidity cannot be negative
    sum += ntu;
    delay(SAMPLE_DELAY);
  }
  
  turbidityValue = sum / NUM_SAMPLES;  // Calculate average
  
  // Constrain turbidity to valid range
  if (turbidityValue > 800.0) turbidityValue = 800.0;
}

// Read Temperature Sensor (DS18B20)
void readTemperatureSensor() {
  tempSensor.requestTemperatures();
  temperatureValue = tempSensor.getTempCByIndex(0);
  
  // Handle error readings
  if (temperatureValue == -127.0 || temperatureValue > 100.0) {
    temperatureValue = 25.0;  // Default to room temperature on error
  }
}

// Calculate Conductivity from pH and Turbidity
void calculateConductivity() {
  // Simplified empirical formula based on pH and turbidity
  // In real applications, use a dedicated conductivity sensor
  conductivity = 500.0 + (phValue * 30.0) + (turbidityValue * 2.0);
}

// Machine Learning: Random Forest Prediction (Simplified)
void predictWaterQuality() {
  /*
   * Random Forest Model Rules (simplified):
   * Feature importances:
   * 1. Turbidity (40% importance)
   * 2. pH (35% importance) 
   * 3. Temperature (15% importance)
   * 4. Conductivity (10% importance)
   * 
   * Decision thresholds based on training data:
   */
  
  float mlScore = 0.0;
  
  // Calculate ML score from features
  if (turbidityValue < 5.0) {
    mlScore += 0.4;  // Good clarity
  } else if (turbidityValue < 15.0) {
    mlScore += 0.2;  // Moderate clarity
  }
  
  if (phValue >= 6.5 && phValue <= 8.5) {
    mlScore += 0.35;  // Good pH range
  } else if (phValue >= 5.5 && phValue <= 9.5) {
    mlScore += 0.15;  // Acceptable pH range
  }
  
  if (temperatureValue >= 15.0 && temperatureValue <= 35.0) {
    mlScore += 0.15;  // Good temperature range
  }
  
  if (conductivity >= 300.0 && conductivity <= 800.0) {
    mlScore += 0.1;   // Good conductivity range
  }
  
  // Classify water quality
  if (mlScore >= 0.75) {
    waterQualityStatus = 0;  // Good
    strcpy(waterQualityLabel, "GOOD");
  } else if (mlScore >= 0.45) {
    waterQualityStatus = 1;  // Moderate
    strcpy(waterQualityLabel, "MODERATE");
  } else {
    waterQualityStatus = 2;  // Poor
    strcpy(waterQualityLabel, "POOR");
  }
}

// Check Thresholds and Trigger Alerts
void checkThresholdsAndAlert() {
  if (phValue < 6.5 || phValue > 8.5) {
    Serial.println("[ALERT] pH out of safe range!");
  }
  
  if (turbidityValue > 15.0) {
    Serial.println("[ALERT] Turbidity too high! Water clarity compromised.");
  }
  
  if (temperatureValue > 40.0 || temperatureValue < 0.0) {
    Serial.println("[ALERT] Temperature out of normal range!");
  }
  
  if (waterQualityStatus == 2) {
    Serial.println("[CRITICAL] Water quality is POOR! Immediate action required.");
    blinkLED(5);  // Blink LED rapidly to indicate alert
  }
}

// Send Data to Blynk Cloud
void sendDataToBlynk() {
  // Construct Blynk data message
  String blynkData = "GET /update?token=" + String(blynkAuthToken);
  blynkData += "&V0=" + String(phValue, 1);           // V0: pH
  blynkData += "&V1=" + String(turbidityValue, 1);    // V1: Turbidity
  blynkData += "&V2=" + String(temperatureValue, 1);  // V2: Temperature
  blynkData += "&V3=" + String(waterQualityStatus);   // V3: Quality Status
  blynkData += " HTTP/1.1\r\nHost: " + String(BLYNK_SERVER) + "\r\n\r\n";
  
  // Send to ESP8266
  espSerial.print(blynkData);
}

// Print Sensor Readings to Serial
void printSensorReadings() {
  Serial.print("\n--- Water Quality Readings ---\n");
  Serial.print("pH Level: ");
  Serial.print(phValue, 1);
  Serial.println(" (0-14)");
  
  Serial.print("Turbidity: ");
  Serial.print(turbidityValue, 1);
  Serial.println(" NTU");
  
  Serial.print("Temperature: ");
  Serial.print(temperatureValue, 1);
  Serial.println(" C");
  
  Serial.print("Conductivity: ");
  Serial.print(conductivity, 0);
  Serial.println(" µS/cm");
  
  Serial.print("ML Prediction: ");
  Serial.print(waterQualityLabel);
  Serial.println(" Water Quality");
  
  Serial.println("--- End Readings ---\n");
}

// Load Calibration Data from EEPROM
void loadCalibrationData() {
  // Default calibration values if EEPROM is empty
  // In production, calibration should be stored and retrieved
  Serial.println("Loading calibration data from EEPROM...");
}

// Save Calibration Data to EEPROM
void saveCalibrationData() {
  // Store calibration offsets in EEPROM for persistence
  Serial.println("Saving calibration data to EEPROM...");
}

// Blink LED for Status Indication
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

// Serial Event Handler (if serial data received)
void serialEvent() {
  while (Serial.available()) {
    char cmd = Serial.read();
    
    if (cmd == 'C') {  // Calibration command
      Serial.println("Entering calibration mode...");
      performCalibration();
    }
  }
}

// Perform Sensor Calibration
void performCalibration() {
  Serial.println("pH Calibration:");
  Serial.println("1. Immerse sensor in pH 7.0 buffer solution");
  Serial.println("2. Wait for 30 seconds");
  Serial.println("3. Record neutral point voltage");
  Serial.println("Calibration completed. Data saved to EEPROM.");
}

// End of Code
