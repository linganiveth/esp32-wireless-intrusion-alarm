/*
 * L02_espnow_det.ino  -  Lesson 02: ESP-NOW ping, DETECTOR side (Board 2)
 * Sends PING once per second, measures round-trip time from the PONG,
 * prints loss statistics every 10 pings. Serial Monitor 115200.
 */
#include <WiFi.h>
#include <esp_now.h>

uint8_t HUB_MAC[6] = {0xFC, 0xE8, 0xC0, 0x75, 0xC1, 0x94};   // Board 1 (HUB)

enum : uint8_t { MSG_PING = 1, MSG_PONG = 2 };

typedef struct __attribute__((packed)) {
  uint8_t  type;
  uint32_t seq;
  uint32_t t_send_us;
} PingMsg;

volatile bool rxReady = false;
PingMsg rxMsg;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  if (len != (int)sizeof(PingMsg) || rxReady) return;
  memcpy(&rxMsg, data, sizeof(PingMsg));
  rxReady = true;
}

uint32_t seq = 0, sent = 0, received = 0;
float rttSumMs = 0, rttMaxMs = 0;
bool waiting = false;
uint32_t waitSeq = 0, waitStartMs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init FAILED");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(onRecv);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, HUB_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Add peer FAILED");
    while (true) delay(1000);
  }
  Serial.printf("DET ready on channel %d, pinging HUB every 1 s\n", WiFi.channel());
}

void loop() {
  static uint32_t lastPing = 0;
  uint32_t now = millis();

  // 1. Send a PING every second
  if (now - lastPing >= 1000) {
    lastPing = now;
    if (sent > 0 && sent % 10 == 0) {
      Serial.printf("--- %lu sent, %lu received, loss %.0f %%, avg RTT %.2f ms, max %.2f ms ---\n",
                    (unsigned long)sent, (unsigned long)received,
                    100.0f * (sent - received) / sent,
                    received ? rttSumMs / received : 0.0f, rttMaxMs);
    }
    PingMsg m = { MSG_PING, ++seq, (uint32_t)micros() };
    esp_err_t r = esp_now_send(HUB_MAC, (uint8_t *)&m, sizeof(m));
    sent++;
    waiting = true; waitSeq = m.seq; waitStartMs = now;
    if (r != ESP_OK) Serial.printf("PING #%lu send ERROR %d\n", (unsigned long)m.seq, r);
  }

  // 2. Handle a PONG
  if (rxReady) {
    PingMsg m = rxMsg;
    rxReady = false;
    if (m.type == MSG_PONG) {
      float rttMs = (uint32_t)(micros() - m.t_send_us) / 1000.0f;
      received++;
      rttSumMs += rttMs;
      if (rttMs > rttMaxMs) rttMaxMs = rttMs;
      if (m.seq == waitSeq) waiting = false;
      Serial.printf("PONG #%lu  RTT %.2f ms  (received %lu/%lu)\n",
                    (unsigned long)m.seq, rttMs, (unsigned long)received, (unsigned long)sent);
    }
  }

  // 3. Report a missing PONG after 500 ms
  if (waiting && now - waitStartMs > 500) {
    waiting = false;
    Serial.printf("No PONG for #%lu (timeout 500 ms)\n", (unsigned long)waitSeq);
  }
}
