// PicoProbe - I2C Bus Sniffer
// Pico 2W passive I2C analyzer
// Sniffs I2C traffic and outputs decoded transactions

#include <Arduino.h>
#include "i2c_sniffer.h"

// Default pins - change as needed
#define SDA_PIN 4
#define SCL_PIN 5

I2CSniffer sniffer(SDA_PIN, SCL_PIN);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait for serial, max 3s

    Serial.println("\n========================================");
    Serial.println("  PicoProbe - I2C Bus Sniffer");
    Serial.println("  Pico 2W Passive Analyzer");
    Serial.println("========================================");
    Serial.printf("  SDA: GPIO%d\n", SDA_PIN);
    Serial.printf("  SCL: GPIO%d\n", SCL_PIN);
    Serial.println("========================================");
    Serial.println("\nCommands:");
    Serial.println("  r - Reset/clear buffer");
    Serial.println("  s - Show stats");
    Serial.println("  h - Toggle hex/decimal output");
    Serial.println("  p - Pause/resume capture");
    Serial.println("\nWaiting for I2C traffic...\n");

    sniffer.begin();
}

void loop() {
    // Process any captured data
    sniffer.process();

    // Handle serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 'r':
            case 'R':
                sniffer.reset();
                Serial.println("[Reset]");
                break;
            case 's':
            case 'S':
                sniffer.printStats();
                break;
            case 'h':
            case 'H':
                sniffer.toggleHex();
                break;
            case 'p':
            case 'P':
                sniffer.togglePause();
                break;
        }
    }
}
