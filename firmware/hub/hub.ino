/*
 * hub.ino  -  Wireless Home Security System, HUB (Board 1)
 * Receives detector events over ESP-NOW (CRC check, duplicate filter, ACK),
 * runs the 5-state alarm machine, keypad PIN arm/disarm with lockout,
 * detector supervision (offline after 15 s), siren + LEDs + OLED, and a
 * phone web page on its own Wi-Fi access point.
 * Libraries: Adafruit SSD1306 (+ GFX), Keypad (Mark Stanley). Serial 115200.
 * Files: packet.h (same folder).
 */
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <stdarg.h>
#include "packet.h"

// ---------------- Configuration ----------------
uint8_t DET_MAC[6] = {0xD0, 0xEF, 0x76, 0x33, 0xEE, 0x44};   // Board 2 (DET) STA MAC
const uint8_t  WIFI_CHANNEL   = 1;
const char    *AP_SSID        = "HomeGuard-Hub";
const char    *AP_PASS        = "homeguard123";              // 8+ characters
const char    *PIN_CODE       = "1234";                      // change before the demo if you like
const uint32_t EXIT_DELAY_MS  = 30000;
const uint32_t ENTRY_DELAY_MS = 15000;
const uint32_t OFFLINE_MS     = 15000;                       // 3 missed heartbeats
const uint32_t LOCKOUT_MS     = 60000;
const uint8_t  MAX_WRONG_PINS = 3;
const uint32_t SIREN_MAX_MS   = 180000;                      // 3 min

const int PIN_SIREN = 25;
const int LED_READY = 26, LED_ARMED = 27, LED_ALARM = 14, LED_FAULT = 13;

// ---------------- Types and globals (all before any function) ----------------
enum AlarmState : uint8_t { ST_DISARMED, ST_EXIT_DELAY, ST_ARMED, ST_ENTRY_DELAY, ST_ALARM };
const char *STATE_NAMES[] = {"DISARMED", "EXIT DELAY", "ARMED", "ENTRY DELAY", "ALARM"};

const int RXQ = 8;
Packet        rxq[RXQ];
volatile int  rxHead = 0, rxTail = 0;           // producer: radio callback, consumer: loop()
volatile uint32_t rxDropped = 0, rxBadCrc = 0;

struct LogEntry { uint32_t ms; char text[48]; };
const int LOGN = 20;
LogEntry logbuf[LOGN];
int logNext = 0, logCount = 0;

AlarmState state = ST_DISARMED;
uint32_t   stateSinceMs = 0;
bool       sirenTimedOut = false;

bool     detOnline = false, everHeard = false;
uint32_t lastHeardMs = 0;
uint16_t lastSeq = 0;
bool     haveSeq = false;
uint32_t dupCount = 0;
bool     doorOpen = false, lidOpen = false;
char     lastEvent[40] = "none";

char     pinBuf[5] = "";
uint8_t  pinLen = 0, wrongCount = 0;
uint32_t lockUntilMs = 0;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool oledOk = false;
uint32_t lastOledMs = 0;

const byte ROWS = 4, COLS = 4;
char keymap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {19, 18, 23, 17};
byte colPins[COLS] = {16, 4, 32, 33};
Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

WebServer server(80);

// ---------------- Log ----------------
void addLog(const char *fmt, ...) {
  char text[48];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(text, sizeof(text), fmt, ap);
  va_end(ap);
  uint32_t now = millis();
  logbuf[logNext].ms = now;
  strncpy(logbuf[logNext].text, text, sizeof(logbuf[logNext].text) - 1);
  logbuf[logNext].text[sizeof(logbuf[logNext].text) - 1] = '\0';
  logNext = (logNext + 1) % LOGN;
  if (logCount < LOGN) logCount++;
  Serial.printf("[%8lu ms] %s\n", (unsigned long)now, text);
}

// ---------------- Radio ----------------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  if (len != (int)sizeof(Packet)) return;
  Packet p;
  memcpy(&p, data, sizeof(Packet));
  if (!packetValid(p)) { rxBadCrc++; return; }
  int next = (rxTail + 1) % RXQ;
  if (next == rxHead) { rxDropped++; return; }   // queue full
  rxq[rxTail] = p;
  rxTail = next;
}

void sendAck(uint16_t seq) {
  Packet a;
  a.node_id = NODE_HUB;
  a.type = MSG_ACK;
  a.seq = seq;
  a.battery_mV = 0;
  a.uptime_ms = millis();
  packetSeal(a);
  esp_now_send(DET_MAC, (uint8_t *)&a, sizeof(a));
}

// ---------------- Alarm state machine ----------------
void setState(AlarmState s, const char *reason) {
  state = s;
  stateSinceMs = millis();
  sirenTimedOut = false;
  if (s == ST_ALARM) digitalWrite(PIN_SIREN, HIGH);          // siren on before the ACK goes out
  if (s == ST_DISARMED) digitalWrite(PIN_SIREN, LOW);
  addLog("STATE -> %s (%s)", STATE_NAMES[s], reason);
}

bool pinMatches(const char *entered) { return strcmp(entered, PIN_CODE) == 0; }

// action: 'A' = arm, '#' = disarm / cancel
void handlePin(char action, const char *entered, const char *source) {
  uint32_t now = millis();
  if ((int32_t)(lockUntilMs - now) > 0) {
    addLog("%s: keypad locked", source);
    return;
  }
  if (!pinMatches(entered)) {
    wrongCount++;
    addLog("%s: wrong PIN (%u/%u)", source, wrongCount, MAX_WRONG_PINS);
    if (wrongCount >= MAX_WRONG_PINS) {
      wrongCount = 0;
      lockUntilMs = now + LOCKOUT_MS;
      addLog("KEYPAD LOCKED for %lu s", (unsigned long)(LOCKOUT_MS / 1000));
    }
    return;
  }
  wrongCount = 0;
  if (action == 'A') {
    if (state != ST_DISARMED) { addLog("%s: already %s", source, STATE_NAMES[state]); return; }
    if (lidOpen)  { addLog("Cannot arm: detector lid open"); return; }
    if (!detOnline) { addLog("Cannot arm: detector offline"); return; }
    setState(ST_EXIT_DELAY, source);
  } else {
    if (state == ST_DISARMED) { addLog("%s: already DISARMED", source); return; }
    setState(ST_DISARMED, source);
  }
}

void handleEvent(const Packet &p) {
  uint32_t now = millis();
  lastHeardMs = now;
  everHeard = true;
  if (!detOnline) { detOnline = true; addLog("Detector ONLINE"); }

  if (haveSeq && p.seq == lastSeq) {      // retry of a packet we already handled
    dupCount++;
    sendAck(p.seq);
    return;
  }
  lastSeq = p.seq;
  haveSeq = true;

  if (p.type != MSG_HEARTBEAT) {
    snprintf(lastEvent, sizeof(lastEvent), "%s", msgName(p.type));
    addLog("EVENT %s (seq %u)", msgName(p.type), p.seq);
  }
  switch (p.type) {
    case MSG_MOTION:
      if (state == ST_ARMED) setState(ST_ALARM, "motion");
      break;
    case MSG_DOOR_OPEN:
      doorOpen = true;
      if (state == ST_ARMED) setState(ST_ENTRY_DELAY, "door opened");
      break;
    case MSG_DOOR_CLOSED:
      doorOpen = false;
      break;
    case MSG_TAMPER:
      lidOpen = true;
      if (state != ST_ALARM) setState(ST_ALARM, "tamper");
      break;
    case MSG_TAMPER_CLEAR:
      lidOpen = false;
      break;
    default:
      break;
  }
  sendAck(p.seq);                          // ACK after the decision: ACK means "handled"
}

void tick(uint32_t now) {
  uint32_t inState = now - stateSinceMs;
  if (state == ST_EXIT_DELAY && inState >= EXIT_DELAY_MS) setState(ST_ARMED, "exit delay over");
  if (state == ST_ENTRY_DELAY && inState >= ENTRY_DELAY_MS) setState(ST_ALARM, "entry delay expired");
  if (state == ST_ALARM && !sirenTimedOut && inState >= SIREN_MAX_MS) {
    sirenTimedOut = true;
    addLog("Siren stopped after 3 min (still ALARM)");
  }
  if (everHeard && detOnline && now - lastHeardMs > OFFLINE_MS) {
    detOnline = false;
    addLog("DETECTOR OFFLINE (no heartbeat 15 s)");
  }
}

// ---------------- Outputs ----------------
void updateOutputs(uint32_t now) {
  uint32_t inState = now - stateSinceMs;
  bool siren = false;
  if (state == ST_ALARM && !sirenTimedOut) siren = true;
  else if ((state == ST_EXIT_DELAY || state == ST_ENTRY_DELAY) && (inState % 1000) < 40) siren = true;   // countdown chirp
  else if (everHeard && !detOnline && (now % 10000) < 40) siren = true;                               // fault chirp
  digitalWrite(PIN_SIREN, siren ? HIGH : LOW);

  bool blink = (now % 500) < 250;
  bool locked = (int32_t)(lockUntilMs - now) > 0;
  digitalWrite(LED_READY, state == ST_DISARMED && detOnline && !lidOpen);
  digitalWrite(LED_ARMED, state == ST_ARMED || ((state == ST_EXIT_DELAY || state == ST_ENTRY_DELAY) && blink));
  digitalWrite(LED_ALARM, state == ST_ALARM);
  digitalWrite(LED_FAULT, (everHeard && !detOnline) || lidOpen || (locked && blink));
}

void updateOled(uint32_t now) {
  if (!oledOk || now - lastOledMs < 250) return;
  lastOledMs = now;
  uint32_t inState = now - stateSinceMs;
  char line[32];
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2);
  oled.setCursor(0, 0);
  oled.println(STATE_NAMES[state]);
  oled.setTextSize(1);
  oled.setCursor(0, 22);
  if (state == ST_EXIT_DELAY)       snprintf(line, sizeof(line), "Leave in %lu s", (unsigned long)((EXIT_DELAY_MS - inState + 999) / 1000));
  else if (state == ST_ENTRY_DELAY) snprintf(line, sizeof(line), "Enter PIN: %lu s", (unsigned long)((ENTRY_DELAY_MS - inState + 999) / 1000));
  else                              snprintf(line, sizeof(line), "Last: %s", lastEvent);
  oled.println(line);
  oled.setCursor(0, 36);
  if (!everHeard)      oled.println("Detector: waiting");
  else if (detOnline)  { snprintf(line, sizeof(line), "Detector: OK %lus", (unsigned long)((now - lastHeardMs) / 1000)); oled.println(line); }
  else                 oled.println("Detector: OFFLINE");
  oled.setCursor(0, 50);
  if ((int32_t)(lockUntilMs - now) > 0) snprintf(line, sizeof(line), "LOCKED %lu s", (unsigned long)((lockUntilMs - now) / 1000));
  else {
    char stars[5] = "";
    for (uint8_t i = 0; i < pinLen; i++) stars[i] = '*';
    stars[pinLen] = '\0';
    snprintf(line, sizeof(line), "PIN: %s", stars);
  }
  oled.println(line);
  oled.display();
}

// ---------------- Keypad ----------------
void serviceKeypad() {
  char k = keypad.getKey();
  if (!k) return;
  if (k >= '0' && k <= '9') {
    if (pinLen < 4) { pinBuf[pinLen++] = k; pinBuf[pinLen] = '\0'; }
  } else if (k == '*') {
    pinLen = 0; pinBuf[0] = '\0';
  } else if (k == 'A' || k == '#') {
    handlePin(k, pinBuf, "Keypad");
    pinLen = 0; pinBuf[0] = '\0';
  }
}

// ---------------- Web page ----------------
void handleRoot() {
  uint32_t now = millis();
  String h;
  h.reserve(3500);
  h += F("<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>HomeGuard</title><script>setInterval(function(){var i=document.querySelector('input');if(!i||(i.value===''&&document.activeElement!==i))location.reload();},2000);</script>"
         "<style>body{font-family:sans-serif;margin:16px;max-width:520px}h1{font-size:22px}"
         ".s{font-size:28px;font-weight:bold;padding:10px;border-radius:8px;background:#eee}"
         ".ALARM{background:#f8d0cc;color:#a00}td{padding:3px 8px}input,button{font-size:18px;margin:4px}</style></head><body>");
  h += F("<h1>HomeGuard hub</h1><div class='s ");
  h += (state == ST_ALARM ? "ALARM" : "");
  h += "'>";
  h += STATE_NAMES[state];
  h += F("</div><p>Detector: ");
  if (!everHeard) h += F("waiting");
  else if (detOnline) { h += F("online (last heard "); h += String((now - lastHeardMs) / 1000); h += F(" s ago)"); }
  else h += F("<b>OFFLINE</b>");
  h += F("<br>Door: ");   h += doorOpen ? "open" : "closed";
  h += F("<br>Detector lid: "); h += lidOpen ? "<b>OPEN (tamper)</b>" : "closed";
  h += F("</p><form action='/cmd'><input name='pin' type='password' inputmode='numeric' maxlength='4' placeholder='PIN'>"
         "<button name='act' value='arm'>Arm</button><button name='act' value='disarm'>Disarm</button></form>"
         "<h2>Last events</h2><table>");
  for (int i = 0; i < logCount; i++) {
    int idx = (logNext - 1 - i + LOGN) % LOGN;
    h += F("<tr><td>");
    h += String((now - logbuf[idx].ms) / 1000);
    h += F(" s ago</td><td>");
    h += logbuf[idx].text;
    h += F("</td></tr>");
  }
  h += F("</table></body></html>");
  server.send(200, "text/html", h);
}

void handleCmd() {
  String pin = server.arg("pin");
  String act = server.arg("act");
  handlePin(act == "arm" ? 'A' : '#', pin.c_str(), "Phone");
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------------- Setup and loop ----------------
void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIN_SIREN, OUTPUT);
  digitalWrite(PIN_SIREN, LOW);                       // DR5: safe default
  pinMode(LED_READY, OUTPUT); pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_ALARM, OUTPUT); pinMode(LED_FAULT, OUTPUT);

  Wire.begin(21, 22);
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS, WIFI_CHANNEL);        // fixes the radio on channel 1 for ESP-NOW too
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init FAILED"); while (true) delay(1000); }
  esp_now_register_recv_cb(onRecv);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, DET_MAC, 6);
  peer.channel = WIFI_CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) { Serial.println("Add peer FAILED"); while (true) delay(1000); }

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();

  stateSinceMs = millis();
  Serial.printf("HUB ready. Wi-Fi '%s' (password %s), open http://%s\n", AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());
  Serial.printf("STA MAC %s  AP MAC %s  channel %u  OLED %s\n", WiFi.macAddress().c_str(), WiFi.softAPmacAddress().c_str(),
                WIFI_CHANNEL, oledOk ? "OK" : "NOT FOUND");
  addLog("Hub started, DISARMED");
}

void loop() {
  while (rxHead != rxTail) {                          // handle every queued packet
    Packet p = rxq[rxHead];
    rxHead = (rxHead + 1) % RXQ;
    if (p.node_id == NODE_DET) handleEvent(p);
  }
  uint32_t now = millis();                            // read AFTER handling packets, so now >= lastHeardMs
  serviceKeypad();
  tick(now);
  updateOutputs(now);
  updateOled(now);
  server.handleClient();
}
