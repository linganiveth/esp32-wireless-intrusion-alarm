/*
 * L05_hub_io.ino  -  Lesson 05: hub hardware test (HUB, Board 1)
 * I2C scan + OLED, 4 status LEDs, 4x4 keypad, siren via BC547.
 * Libraries: "Adafruit SSD1306" (installs Adafruit GFX), "Keypad" by Mark Stanley.
 * Serial Monitor 115200.
 *
 * Keys:  any key -> shown on Serial + OLED
 *        A -> toggle ARMED LED     C -> toggle all LEDs
 *        B -> siren beep 300 ms    D -> siren ON/OFF (hold on for the HT4 voltage measurement)
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

// ---------- Pins (docs/02_specification) ----------
const int PIN_SIREN = 25;                       // -> 1 kOhm -> BC547 base
const int LED_PINS[4] = {26, 27, 14, 13};       // READY, ARMED, ALARM, FAULT
const char *LED_NAMES[4] = {"READY", "ARMED", "ALARM", "FAULT"};
const int PIN_SDA = 21, PIN_SCL = 22;

// ---------- OLED ----------
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool oledOk = false;

// ---------- Keypad ----------
const byte ROWS = 4, COLS = 4;
char keymap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {19, 18, 23, 17};          // keypad connector pins 1-4
byte colPins[COLS] = {16, 4, 32, 33};           // keypad connector pins 5-8
Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

// ---------- State ----------
bool armedLed = false, allLeds = false, sirenLatched = false;
uint32_t beepUntil = 0;
uint16_t keyCount = 0;

void i2cScan() {
  Serial.println("I2C scan:");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  device at 0x%02X\n", addr);
      found++;
    }
  }
  if (!found) Serial.println("  none found - check SDA=21, SCL=22, VCC, GND");
}

void oledShow(const char *line1, const char *line2) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("HUB I/O TEST");
  oled.setTextSize(2);
  oled.setCursor(0, 18);
  oled.println(line1);
  oled.setTextSize(1);
  oled.setCursor(0, 48);
  oled.println(line2);
  oled.display();
}

void setLeds(bool on) {
  for (int i = 0; i < 4; i++) digitalWrite(LED_PINS[i], on ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIN_SIREN, OUTPUT);
  digitalWrite(PIN_SIREN, LOW);                 // safe default: siren off
  for (int i = 0; i < 4; i++) { pinMode(LED_PINS[i], OUTPUT); digitalWrite(LED_PINS[i], LOW); }

  Wire.begin(PIN_SDA, PIN_SCL);
  i2cScan();
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.println(oledOk ? "OLED OK at 0x3C" : "OLED NOT found at 0x3C");
  oledShow("READY", "press any key");

  Serial.println("LED walk:");
  for (int i = 0; i < 4; i++) {                 // each LED on for 400 ms
    digitalWrite(LED_PINS[i], HIGH);
    Serial.printf("  %s (GPIO%d) ON\n", LED_NAMES[i], LED_PINS[i]);
    delay(400);
    digitalWrite(LED_PINS[i], LOW);
  }
  Serial.println("Siren chirp 100 ms");
  digitalWrite(PIN_SIREN, HIGH); delay(100); digitalWrite(PIN_SIREN, LOW);
  Serial.println("Ready: press keys (A armed LED, B beep, C all LEDs, D siren on/off)");
}

void loop() {
  uint32_t now = millis();
  char k = keypad.getKey();
  if (k) {
    keyCount++;
    Serial.printf("Key '%c'  (count %u)\n", k, keyCount);
    char l1[8]; snprintf(l1, sizeof(l1), "Key %c", k);
    char l2[24];
    switch (k) {
      case 'A': armedLed = !armedLed; digitalWrite(LED_PINS[1], armedLed); snprintf(l2, sizeof(l2), "ARMED LED %s", armedLed ? "on" : "off"); break;
      case 'B': beepUntil = now + 300; snprintf(l2, sizeof(l2), "siren beep"); break;
      case 'C': allLeds = !allLeds; setLeds(allLeds); snprintf(l2, sizeof(l2), "all LEDs %s", allLeds ? "on" : "off"); break;
      case 'D': sirenLatched = !sirenLatched; snprintf(l2, sizeof(l2), "siren %s", sirenLatched ? "ON" : "off"); break;
      default:  snprintf(l2, sizeof(l2), "keys pressed: %u", keyCount); break;
    }
    oledShow(l1, l2);
  }
  bool sirenOn = sirenLatched || (int32_t)(beepUntil - now) > 0;   // non-blocking beep
  digitalWrite(PIN_SIREN, sirenOn ? HIGH : LOW);
}
