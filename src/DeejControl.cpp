#include "DeejControl.h"


const int SCALE_FACTOR = 2; 
const int MAX_VALUE = 100;
const int MIN_VALUE = 0;

int numSliders = 1;
int* sliderValues = nullptr;   
int* previousValues = nullptr;
bool* mutedStates = nullptr;
String* sliderNames = nullptr;

const int numMaxSliders = 20; // Maximum number of sliders supported

volatile int encoderDelta1 = 0;   
volatile int encoderDelta2 = 0;   
bool buttonPressed = false;
int currentSlider = 0;

unsigned long lastChangeTime = 0;   
unsigned long writeInterval = 10000; // 10 seconds wait after last change
bool dataDirty = false;
int* lastSavedValues = nullptr;  
bool* lastSavedMuted = nullptr;  
int* lastSavedPreviousValues = nullptr;

// For long-press on second encoder
unsigned long encoder2PressStart = 0;
bool encoder2LongPressActive = false;



void IRAM_ATTR encoder1ISR() {
    if (digitalRead(ENCODER1_CLK) == digitalRead(ENCODER1_DT))
        encoderDelta1 = -SCALE_FACTOR; 
    else
        encoderDelta1 = SCALE_FACTOR;  
}

void IRAM_ATTR encoder2ISR() {
    if (digitalRead(ENCODER2_CLK) == digitalRead(ENCODER2_DT))
        encoderDelta2 = -1; 
    else
        encoderDelta2 = 1;  
}

bool valuesAreDifferent() {
    for (int i = 0; i < numSliders; i++) {
        if (sliderValues[i] != lastSavedValues[i]) return true;
        if (mutedStates[i] != lastSavedMuted[i]) return true;
        if (previousValues[i] != lastSavedPreviousValues[i]) return true;
    }
    return false;
}

void markDataSaved() {
    for (int i = 0; i < numSliders; i++) {
        lastSavedValues[i] = sliderValues[i];
        lastSavedMuted[i] = mutedStates[i];
        lastSavedPreviousValues[i] = previousValues[i];
    }
    dataDirty = false;
}

void readSerial() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        Serial.println("Received: " + input);

        // Example: #steam.exe|64|0;#SpotifyWidgetProvider.exe|100|0;#SystemSounds|50|0;
        int sessionIdx = 1; // Start from 1 to preserve Master slider at index 0
        int start = 0;
        while (start < input.length() && sessionIdx < numMaxSliders) {
            int hashPos = input.indexOf('#', start);
            if (hashPos == -1) break;
            int pipe1 = input.indexOf('|', hashPos);
            int pipe2 = input.indexOf('|', pipe1 + 1);
            int semi = input.indexOf(';', pipe2 + 1);

            if (pipe1 == -1 || pipe2 == -1 || semi == -1) break;

            String name = input.substring(hashPos + 1, pipe1);
            int vol = input.substring(pipe1 + 1, pipe2).toInt();
            int muted = input.substring(pipe2 + 1, semi).toInt();

            // Never overwrite Master slider (index 0)
            sliderNames[sessionIdx] = name;
            sliderValues[sessionIdx] = constrain(vol, MIN_VALUE, MAX_VALUE);
            mutedStates[sessionIdx] = (muted != 0);
            if (!mutedStates[sessionIdx]) {
                previousValues[sessionIdx] = sliderValues[sessionIdx];
            }

            sessionIdx++;
            start = semi + 1;
        }
        // Only update numSliders if at least one session was parsed (besides Master)
        if (sessionIdx > 1) {
            numSliders = sessionIdx;
        }
        // Debug: Print updated arrays to Serial
        Serial.println("Updated slider arrays:");
        for (int i = 0; i < numSliders; i++) {
            Serial.print("Slider ");
            Serial.print(i);
            Serial.print(": Name=");
            Serial.print(sliderNames[i]);
            Serial.print(", Value=");
            Serial.print(sliderValues[i]);
            Serial.print(", Muted=");
            Serial.print(mutedStates[i] ? "true" : "false");
            Serial.print(", Previous=");
            Serial.println(previousValues[i]);
        }
    }
}

extern void displayError(const char* line1, const char* line2);

void initDeejControl() {
    
    sliderValues = new int[numMaxSliders];
    previousValues = new int[numMaxSliders];
    mutedStates = new bool[numMaxSliders];
    sliderNames = new String[numMaxSliders];
    lastSavedValues = new int[numMaxSliders];
    lastSavedMuted = new bool[numMaxSliders];
    lastSavedPreviousValues = new int[numMaxSliders];
    
    sliderNames[0] = "Master";
    sliderValues[0] = 100;
    previousValues[0] = 100;
    mutedStates[0] = false;
    lastSavedValues[0] = 100;
    lastSavedMuted[0] = false;
    lastSavedPreviousValues[0] = 100;
    

    readSerial(); // Read initial values from serial
    pinMode(ENCODER1_CLK, INPUT_PULLUP);
    pinMode(ENCODER1_DT, INPUT_PULLUP);
    pinMode(ENCODER1_SW, INPUT_PULLUP);
    pinMode(ENCODER2_CLK, INPUT_PULLUP);
    pinMode(ENCODER2_DT, INPUT_PULLUP);
    pinMode(ENCODER2_SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENCODER1_CLK), encoder1ISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER2_CLK), encoder2ISR, CHANGE);

    Serial.println("Deej Control Initialized");
}


void runDeejControl() {
    bool valueChanged = false;

    // Check long-press on ENCODER2_SW
    if (digitalRead(ENCODER2_SW) == LOW && !encoder2LongPressActive) {
        encoder2PressStart = millis();
        encoder2LongPressActive = true;
    } else if (digitalRead(ENCODER2_SW) == HIGH && encoder2LongPressActive) {
        encoder2LongPressActive = false;
    }

    // If pressed > 10s
    if (encoder2LongPressActive && (millis() - encoder2PressStart > 10000)) {
        encoder2LongPressActive = false;
        Serial.println("Long press detected. Entering WiFi setup mode.");
        
        return;
    }

    // Adjust Slider Values
    if (encoderDelta1 != 0) {
        if (mutedStates[currentSlider]) {
            // Unmute before adjusting
            sliderValues[currentSlider] = previousValues[currentSlider];
            mutedStates[currentSlider] = false;
        }

        sliderValues[currentSlider] = constrain(sliderValues[currentSlider] + encoderDelta1, MIN_VALUE, MAX_VALUE);
        encoderDelta1 = 0;
        valueChanged = true;
    }

    // Change Slider Selection
    static int encoder2Accum = 0;
    if (encoderDelta2 != 0) {
        encoder2Accum += encoderDelta2;
        encoderDelta2 = 0;
        // Only change slider if accumulated movement exceeds threshold (e.g., 2 steps)
        const int threshold = 2;
        if (abs(encoder2Accum) >= threshold) {
            int steps = encoder2Accum / threshold;
            currentSlider = (currentSlider + steps + numSliders) % numSliders;
            encoder2Accum -= steps * threshold;
        }
    }

    // Check Mute/Unmute Button
    if (digitalRead(ENCODER1_SW) == LOW && !buttonPressed) {
        delay(200);  // Debounce
        if (mutedStates[currentSlider]) {
            sliderValues[currentSlider] = previousValues[currentSlider];
            mutedStates[currentSlider] = false;
        } else {
            previousValues[currentSlider] = sliderValues[currentSlider];
            sliderValues[currentSlider] = 0;
            mutedStates[currentSlider] = true;
        }
        buttonPressed = true;
        valueChanged = true;
    } else if (digitalRead(ENCODER1_SW) == HIGH) {
        buttonPressed = false;
    }

    // Update Display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, 15);
    u8g2.print(sliderNames[currentSlider] + " (" + String(currentSlider + 1) + "/" + String(numSliders) + ")");

    int barWidth = map(sliderValues[currentSlider], 0, 100, 0, 105); 
    u8g2.drawFrame(0, 63 - 15 - 2, 100, 15);
    u8g2.drawBox(0, 63 - 15 - 2, barWidth, 15);

    if (mutedStates[currentSlider]) {
        u8g2.setCursor(115, 59);
        u8g2.print("M");
    } else {
        u8g2.setCursor(105, 59);
        u8g2.print(String(sliderValues[currentSlider]));
    }

    u8g2.sendBuffer();

    // Send only the active slider value to Serial
    String output = "#" + sliderNames[currentSlider] + "|" + String(sliderValues[currentSlider]) + "|" + String(mutedStates[currentSlider] ? 1 : 0) + ";";
    Serial.println(output);
    
    // Read serial input for updates
    readSerial();  
}
