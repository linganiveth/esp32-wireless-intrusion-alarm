// packet.h - shared ESP-NOW packet format (MASTER COPY)
// Copy this file unchanged into detector_node/ and hub/ (Arduino sketches
// can only include files from their own folder).
#pragma once
#include <stdint.h>
#include <stddef.h>

enum MsgType : uint8_t {
  MSG_HEARTBEAT    = 0,
  MSG_MOTION       = 1,
  MSG_DOOR_OPEN    = 2,
  MSG_DOOR_CLOSED  = 3,
  MSG_TAMPER       = 4,
  MSG_ACK          = 5,
  MSG_TAMPER_CLEAR = 6
};

const uint8_t NODE_HUB = 0;
const uint8_t NODE_DET = 1;

typedef struct __attribute__((packed)) {
  uint8_t  node_id;     // sender: 0 hub, 1 detector
  uint8_t  type;        // MsgType
  uint16_t seq;         // event number; an ACK echoes the event's seq
  uint16_t battery_mV;  // 0 on USB power
  uint32_t uptime_ms;   // sender's millis() when the event happened
  uint16_t crc;         // CRC-16/CCITT over the first 10 bytes
} Packet;               // 12 bytes

inline uint16_t crc16_ccitt(const uint8_t *d, size_t n) {
  uint16_t c = 0xFFFF;
  while (n--) {
    c ^= (uint16_t)(*d++) << 8;
    for (int i = 0; i < 8; i++) c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
  }
  return c;
}
inline void packetSeal(Packet &p)        { p.crc = crc16_ccitt((const uint8_t *)&p, sizeof(Packet) - 2); }
inline bool packetValid(const Packet &p) { return p.crc == crc16_ccitt((const uint8_t *)&p, sizeof(Packet) - 2); }

inline const char *msgName(uint8_t t) {
  switch (t) {
    case MSG_HEARTBEAT:    return "HEARTBEAT";
    case MSG_MOTION:       return "MOTION";
    case MSG_DOOR_OPEN:    return "DOOR OPEN";
    case MSG_DOOR_CLOSED:  return "DOOR CLOSED";
    case MSG_TAMPER:       return "TAMPER";
    case MSG_ACK:          return "ACK";
    case MSG_TAMPER_CLEAR: return "LID CLOSED";
    default:               return "UNKNOWN";
  }
}
