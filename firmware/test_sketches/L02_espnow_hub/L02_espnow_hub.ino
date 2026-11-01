/*
 * L02_espnow_hub.ino  -  Lesson 02: ESP-NOW ping, HUB side (Board 1)
 * Receives PING from the detector and answers with PONG (same seq + timestamp).
 * Serial Monitor 115200.
 */
#include <WiFi.h>
#include <esp_now.h>

uint8_t DET_MAC[6] = {0xD0, 0xEF, 0x76, 0x33, 0xEE, 0x44};   // Board 2 (DET)

enum : uint8_t { MSG_PING = 1, MSG_PONG = 2 };

typedef struct __attribute__((packed)) {
  uint8_t  type;        // MSG_PING or MSG_PONG
  uint32_t seq;         // ping number
  uint32_t t_send_us;   // detector's micros() when the PING left
} PingMsg;

// Filled by the receive callback (Wi-Fi task), consumed in loop()
volatile bool rxReady = false;
PingMsg rxMsg;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  if (len != (int)sizeof(PingMsg) || rxReady) return;   // wrong size, or last one not handled yet
  memcpy(&rxMsg, data, sizeof(PingMsg));                // copy only: keep callbacks short
  rxReady = true;
}

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
  memcpy(peer.peer_addr, DET_MAC, 6);
  peer.channel = 0;          // 0 = use the current Wi-Fi channel
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Add peer FAILED");
    while (true) delay(1000);
  }
  Serial.printf("HUB ready on channel %d, waiting for PING from DET\n", WiFi.channel());
}

void loop() {
  if (rxReady) {
    PingMsg m = rxMsg;
    rxReady = false;
    if (m.type == MSG_PING) {
      PingMsg reply = m;
      reply.type = MSG_PONG;                       // echo seq and timestamp back
      esp_err_t r = esp_now_send(DET_MAC, (uint8_t *)&reply, sizeof(reply));
      Serial.printf("PING #%lu from DET -> PONG %s\n",
                    (unsigned long)m.seq, r == ESP_OK ? "sent" : "send ERROR");
    }
  }
}
