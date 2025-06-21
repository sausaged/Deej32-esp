#include <Arduino.h>
#include <SPIFFS.h>
#include <U8g2lib.h>
#include "DeejControl.h" // Handles Deej slider control

// Pin definitions
const int ENCODER1_CLK = 3; // Define CLK pin for encoder 1
const int ENCODER1_DT = 10; // Define DT pin for encoder 1
const int ENCODER1_SW = 8; // Define SW pin for encoder 1

const int ENCODER2_CLK = 4; // Define CLK pin for encoder 2
const int ENCODER2_DT = 6; // Define DT pin for encoder 2
const int ENCODER2_SW = 5; // Define SW pin for encoder 2

const int OLED_SCL = 1; // Define SCL pin for OLED
const int OLED_SDA = 2; // Define SDA pin for OLED

const bool useTxPowerControl = true; // If you're using an ESP32 C3 Supermini and experiencing WiFi connection issues, set this to true.


U8G2_SSD1315_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);



void displayError(const char* line1, const char* line2) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 15, line1);
    u8g2.drawStr(0, 35, line2);
    u8g2.sendBuffer();
}

void setup() {
    Serial.begin(115200);
    u8g2.begin();

    delay(500); // Allow time for serial monitor to open

    // Initialize Deej Slider Control
    initDeejControl();
}

void loop() {
    
    runDeejControl();  // If WiFi is disabled, only run Deej logic 
}