/*
 * detector_node.ino  -  Wireless Home Security System, DETECTOR (Board 2)
 * Senses motion (PIR), door (reed) and tamper (lid switch); sends each event
 * to the hub over ESP-NOW with sequence number + CRC, waits for an ACK,
 * retries up to 3 times, and sends a heartbeat every 5 s.
 * Serial Monitor 115200. Files: packet.h, debounced_input.h (same folder).
 */
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "packet.h"
#include "debounced_input.h"

// ---------------- Configuration ----------------
uint8_t HUB_MAC[6] = {0xFC, 0xE8, 0xC0, 0x75, 0xC1, 0x94};   // Board 1 (HUB) STA MAC
const uint8_t  WIFI_CHANNEL   = 1;      // must match the hub's access-point channel
const int      PIN_PIR        = 27;
const int      PIN_REED       = 26;     // LOW = door closed
const int      PIN_TAMPER     = 25;     // LOW = lid closed
const int      PIN_LED        = 2;      // blinks on every transmission
const uint32_t DEBOUNCE_MS    = 50;
const uint32_t PIR_WARMUP_MS  = 30000;
const uint32_t HEARTBEAT_MS   = 5000;
const uint32_t ACK_TIMEOUT_MS = 100;
const uint8_t  MAX_RETRIES    = 3;

// ---------------- Types and globals (all before any function) ----------------
struct TxItem { uint8_t type; uint32_t eventMs; };
const int QLEN = 8;
TxItem   txq[QLEN];
int      qHead = 0, qCount = 0;

Packet   inFlight;
bool     waitingAck = false;
uint8_t  tries = 0;
uint32_t lastSendMs = 0;
uint16_t nextSeq = 1;
uint32_t statOk = 0, statFailed = 0, statRetries = 0;

volatile bool     pirRise = false;
volatile uint32_t pirRiseMs = 0;
volatile bool     ackReady = false;
volatile uint16_t ackSeq = 0;

bool     pirReady = false;
uint32_t lastHeartbeatMs = 0, ledOffMs = 0;

DebouncedInput reed   = {PIN_REED,   "DOOR", "CLOSED", "OPEN",           HIGH, HIGH, 0, 0};
DebouncedInput tamper = {PIN_TAMPER, "LID ", "CLOSED", "OPEN -> TAMPER", HIGH, HIGH, 0, 0};

// ---------------- Interrupt and radio callbacks ----------------
void IRAM_ATTR onPirRise() {
  pirRise = true;
  pirRiseMs = millis();
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  if (len != (int)sizeof(Packet)) return;
  Packet p;
  memcpy(&p, data, sizeof(Packet));
  if (!packetValid(p) || p.type != MSG_ACK) return;
  ackSeq = p.seq;
  ackReady = true;
}

// ---------------- Transmit queue ----------------
void enqueue(uint8_t type, uint32_t eventMs) {
  if (qCount == QLEN) {
    Serial.printf("[%8lu ms] QUEUE FULL - %s dropped\n", (unsigned long)millis(), msgName(type));
    return;
  }
  txq[(qHead + qCount) % QLEN] = {type, eventMs};
  qCount++;
}

void transmit() {
  esp_now_send(HUB_MAC, (uint8_t *)&inFlight, sizeof(inFlight));
  lastSendMs = millis();
  digitalWrite(PIN_LED, HIGH);
  ledOffMs = lastSendMs + 30;
}

void popFront() {
  qHead = (qHead + 1) % QLEN;
  qCount--;
}

void serviceTx(uint32_t now) {
  if (ackReady) {
    uint16_t s = ackSeq;
    ackReady = false;
    if (waitingAck && s == inFlight.seq) {
      waitingAck = false;
      statOk++;
      if (inFlight.type != MSG_HEARTBEAT)
        Serial.printf("[%8lu ms] ACK  %-11s seq=%u  event->ACK %lu ms (tries %u)\n", (unsigned long)now,
                      msgName(inFlight.type), inFlight.seq, (unsigned long)(now - inFlight.uptime_ms), tries);
      popFront();
    }
  }
  if (waitingAck && now - lastSendMs >= ACK_TIMEOUT_MS) {
    if (tries < 1 + MAX_RETRIES) {
      tries++;
      statRetries++;
      Serial.printf("[%8lu ms] RETRY %-10s seq=%u (try %u)\n", (unsigned long)now, msgName(inFlight.type), inFlight.seq, tries);
      transmit();
    } else {
      waitingAck = false;
      statFailed++;
      Serial.printf("[%8lu ms] SEND FAILED %s seq=%u - hub not answering\n", (unsigned long)now, msgName(inFlight.type), inFlight.seq);
      popFront();
    }
  }
  if (!waitingAck && qCount > 0) {
    TxItem &it = txq[qHead];
    inFlight.node_id    = NODE_DET;
    inFlight.type       = it.type;
    inFlight.seq        = nextSeq++;
    inFlight.battery_mV = 0;
    inFlight.uptime_ms  = it.eventMs;
    packetSeal(inFlight);
    tries = 1;
    waitingAck = true;
    if (it.type != MSG_HEARTBEAT)
      Serial.printf("[%8lu ms] TX   %-11s seq=%u\n", (unsigned long)now, msgName(it.type), inFlight.seq);
    transmit();
  }
}

// ---------------- Setup and loop ----------------
void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PIR, INPUT);
  initInput(reed);
  initInput(tamper);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init FAILED"); while (true) delay(1000); }
  esp_now_register_recv_cb(onRecv);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, HUB_MAC, 6);
  peer.channel = WIFI_CHANNEL;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) { Serial.println("Add peer FAILED"); while (true) delay(1000); }

  attachInterrupt(digitalPinToInterrupt(PIN_PIR), onPirRise, RISING);

  // Tell the hub the current door and lid state at start-up
  enqueue(reed.stable == LOW ? MSG_DOOR_CLOSED : MSG_DOOR_OPEN, millis());
  enqueue(tamper.stable == LOW ? MSG_TAMPER_CLEAR : MSG_TAMPER, millis());

  Serial.printf("DETECTOR ready on channel %u. Door %s, lid %s. PIR warming up %lu s\n", WIFI_CHANNEL,
                reed.stable == LOW ? "CLOSED" : "OPEN", tamper.stable == LOW ? "CLOSED" : "OPEN",
                (unsigned long)(PIR_WARMUP_MS / 1000));
}

void loop() {
  uint32_t now = millis();

  if (!pirReady && now >= PIR_WARMUP_MS) {
    pirReady = true;
    Serial.printf("[%8lu ms] PIR ready\n", (unsigned long)now);
  }
  if (pirRise) {
    noInterrupts();
    pirRise = false;
    uint32_t t = pirRiseMs;
    interrupts();
    if (pirReady) {
      Serial.printf("[%8lu ms] MOTION detected\n", (unsigned long)t);
      enqueue(MSG_MOTION, t);
    }
  }
  if (updateInput(reed, now, DEBOUNCE_MS))
    enqueue(reed.stable == LOW ? MSG_DOOR_CLOSED : MSG_DOOR_OPEN, now);
  if (updateInput(tamper, now, DEBOUNCE_MS))
    enqueue(tamper.stable == LOW ? MSG_TAMPER_CLEAR : MSG_TAMPER, now);

  if (now - lastHeartbeatMs >= HEARTBEAT_MS) {
    lastHeartbeatMs = now;
    enqueue(MSG_HEARTBEAT, now);
  }

  serviceTx(now);
  if (ledOffMs && (int32_t)(now - ledOffMs) >= 0) { digitalWrite(PIN_LED, LOW); ledOffMs = 0; }
}
