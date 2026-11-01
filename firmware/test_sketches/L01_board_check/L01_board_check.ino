/*
 * L01_board_check.ino  -  Lesson 01: ESP32 setup and board identity
 * Project : Wireless Home Security System (25ES604 ESD)
 * Board   : ESP32 Dev Module (ESP32-WROOM-32 DevKit V1)
 * Purpose : Prove the toolchain works, print chip info and the Wi-Fi STA MAC
 *           address (needed later as the ESP-NOW peer address), blink the LED.
 * Check   : Serial Monitor at 115200 baud shows the report every 5 s and the
 *           on-board LED blinks once per second.
 */
#include <WiFi.h>
#include "esp_mac.h"          // esp_read_mac()

const int LED_PIN = 2;        // On-board LED on most DevKit V1 boards (verify on your board)

void printReport() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);   // factory MAC from eFuse, works before Wi-Fi starts

  Serial.println();
  Serial.println("=== L01 board check ===");
  Serial.printf("Chip   : %s rev %d, %d cores, %lu MHz\n",
                ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("Flash  : %lu MB\n", (unsigned long)(ESP.getFlashChipSize() / (1024UL * 1024UL)));
  Serial.printf("STA MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("C array: {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("LED    : blinking on GPIO2");
}

void setup() {
  Serial.begin(115200);
  delay(500);                 // give the USB-serial bridge time to open the port
  pinMode(LED_PIN, OUTPUT);
  WiFi.mode(WIFI_STA);        // station mode: the mode ESP-NOW will use later
  printReport();
}

void loop() {
  static uint32_t lastBlink = 0, lastReport = 0;
  static bool ledOn = false;
  uint32_t now = millis();

  if (now - lastBlink >= 500) {          // non-blocking blink: 1 Hz
    lastBlink = now;
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  }
  if (now - lastReport >= 5000) {        // repeat report so you never miss it
    lastReport = now;
    printReport();
  }
}
