// CAN Node template for sensors (Should be XIAO ESP32-C3s unless i'm stupid or we changed it somewhere)
// If you have any questions my email is cc574924@ohio.edu

// Copy this code for each new node, then make the changes listed below.

// 1: Set NODE_ID (under Node Settings) to your node ID from registry/can_id_registry.csv. It won't compile until you do.
// 2: Make a message function for each message (under Structs / Functions for filling Messages, how to do so listed below)
// 3: Add lines to messages[] table at bottom of section for each new message (Format: { ID, send period in ms, your function })
// 4: Remember to remove the example message

// HOW TO ADD A MESSAGE
//   1. Pick an ID From the GitHub Can-Spec that matches your selected node.                  ----- https://github.com/Bobcat-BAJA/can-spec -----
//      Only use IDs from your own node's slice in the registry.
//   2. Write a fill function like fillSensor() below: put each value into the
//      frame with addOneByte / addTwoBytes / addFourBytes and return how many bytes you used (max 5).
//   3. Add one line to the messages[] table: { ID, send period in ms, your function }.
//   4. Make sure the byte layout matches your message in dbc/baja_daq.dbc (what each byte means, units, scale)
//      so whoever reads the message can decode it.

// ---- Data format ----
//  - Every data frame is 8 bytes max. The first 3 are a header this code fills in for you:
//      byte 0     sequence counter, +1 every send on that ID, wraps 255 -> 0
//      bytes 1-2  microseconds since this node last saw the hub's SYNC (0x080), 0xFFFF = no SYNC yet
//    That leaves 5 bytes for your data (frame bytes 3..7, which is data[0]..data[4] in your fill function)
//  - Multi-byte values go low byte first (the helpers below do this for you, you can look into
//    them if you want to understand what they are doing, but basically it adds a specific amount
//    of bytes
//  - NOTE: Integers only. I'm not sure if that'll ever be a problem, but please try to make sure
//    your inputs are integers. NO FLOATS
//  - NOTE 2: I am trusting that you will properly understand what information you are pumping into
//    the node. If data is malformed, or input in an unexpected way, there could be issues. Like if
//    you try to read one byte from a pin, and it is sending 2, it could lead to issues. Please
//    double check your sensors.
//  - Heartbeat is sent for you on 0x700 + NODE_ID once a second. Don't add it to the table.


// ---- Actual start to the code ----
#include "driver/twai.h"

// ---- Node Settings ----
#define NODE_ID     0x00            // SET THIS: your node ID from the registry (heartbeat goes out on 0x700 + NODE_ID)
#define FW_MAJOR    0               // firmware version, sent in the heartbeat
#define FW_MINOR    1

#if NODE_ID == 0
#error "Set NODE_ID to your node ID from registry/can_id_registry.csv"
#endif

// ---- Node IO Settings ----
// Spec pins. Arduino "USB CDC On Boot" must be Enabled, or Serial takes D7.
#define TX_PIN      ((gpio_num_t)D7)    // D7 (GPIO20) -> CAN TX
#define RX_PIN      ((gpio_num_t)D10)   // D10 (GPIO10) -> CAN RX

#define SYNC_ID     0x080           // hub's SYNC message
#define HEADER_LEN  3               // seq + time since SYNC
#define MAX_DATA    (8 - HEADER_LEN)

// Heartbeat state (spec: 0 booting, 1 OK, 2 warning, 3 fault). Set these from your own code if something goes wrong.
uint8_t  nodeState  = 0;
uint16_t errorFlags = 0;

// Time of the last SYNC we got
bool     syncSeen   = false;
uint32_t lastSyncUs = 0;

// ---- Packing Helpers ----
// write a value into data[] starting at byte position pos,
// then returns the next available byte

// General format for bit shifting is data[n + i] = x >> 8i & 0xFF, where i is byte number in segment and n is byte number in entire message

// These are put next to & 0xFF so that we get only those 8 bits. & 0xFF essentially ANDS the data and 11111111, so we get only 8 bits returned.
// We also shift the data 8 bits using >> 8, so that we get the next bit in the lower 8 bits, so when we AND, we get that specific byte
uint8_t addOneByte(uint8_t *data, uint8_t pos, uint8_t value) {
  // Fills data
  data[pos] = value;
  // Returns size added to pos
  return pos + 1;
}

uint8_t addTwoBytes(uint8_t *data, uint8_t pos, uint16_t value) {
  // Fills data
  data[pos]     = value & 0xFF;
  data[pos + 1] = (value >> 8) & 0xFF;
  // Returns size added to pos
  return pos + 2;
}

uint8_t addThreeBytes(uint8_t *data, uint8_t pos, uint32_t value) {
  // Fills data
  data[pos]     = value & 0xFF;
  data[pos + 1] = (value >> 8)  & 0xFF;
  data[pos + 2] = (value >> 16) & 0xFF;
  // Returns size added to pos
  return pos + 3;
}

uint8_t addFourBytes(uint8_t *data, uint8_t pos, uint32_t value) {
  // Fills data
  data[pos]     = value & 0xFF;
  data[pos + 1] = (value >> 8)  & 0xFF;
  data[pos + 2] = (value >> 16) & 0xFF;
  data[pos + 3] = (value >> 24) & 0xFF;
  // Returns size added to pos
  return pos + 4;
}

// ---- Structs / Functions for filling Messages ----

// An example message is below, simply copy / paste and add data (format explained at top)

// Example struct for filling data. NOTE: THIS ONLY FILLS THE DATA READ FROM THE SENSOR. THE ID AND HEADER (SEQ / TIMESTAMP) ARE HANDLED SEPERATE.
uint8_t fillSensor(uint8_t *data) {
  // pos should always be set to 0 to start
  uint8_t pos = 0;
  // Simply add information per byte size
  pos = addTwoBytes(data, pos, analogRead(A0)); // reads from analog A0
  pos = addOneByte (data, pos, 0);              // placeholder status
  pos = addTwoBytes(data, pos, 0xFFFF);         // placeholder 2-byte value ("not available" is all ones, never 0)
  return pos;                                   // should be 5, because 2 + 1 + 2 = 5. 5 is the max
}

// Message struct. Should realistically never need messed with, if you do, either you or I messed up somewhere
struct Message {
  uint32_t id;                    // unique on the bus; lower = higher priority
  uint32_t periodMs;              // how often to send
  uint8_t (*fill)(uint8_t *data); // writes the data bytes, returns the length (max 5)
  uint32_t lastSent = 0;          // when last was sent
  uint8_t  seq      = 0;          // sequence counter for this ID
};

// One line per message this node sends. IDs must not be used by any other node.
// Format: { ID, send period in ms, your function }
Message messages[] = {
  { 0x600, 100, fillSensor },     // EXAMPLE ONLY, 0x600 is the spare block. Replace with an ID from your slice.
};


// Everything below this point should never need touched unless something goes terribly wrong

// Heartbeat on 0x700 + NODE_ID, no header: node ID, state, error flags, firmware major/minor, uptime (s)
void sendHeartbeat() {
  twai_message_t tx = {};
  tx.identifier = 0x700 + NODE_ID;
  uint8_t pos = 0;
  pos = addOneByte (tx.data, pos, NODE_ID);
  pos = addOneByte (tx.data, pos, nodeState);
  pos = addTwoBytes(tx.data, pos, errorFlags);
  pos = addOneByte (tx.data, pos, FW_MAJOR);
  pos = addOneByte (tx.data, pos, FW_MINOR);
  pos = addTwoBytes(tx.data, pos, millis() / 1000);
  tx.data_length_code = pos;
  twai_transmit(&tx, 0);
}

// Microseconds since the last SYNC, 0xFFFF if we haven't seen one. Capped at 0xFFFE so it never reads as "no SYNC".
uint16_t timeSinceSync() {
  if (!syncSeen) return 0xFFFF;
  uint32_t dt = micros() - lastSyncUs;
  return dt > 0xFFFE ? 0xFFFE : dt;
}

void setup() {

  // Start serial and setup delay
  Serial.begin(115200);
  delay(1000);

  // General config
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t  t = TWAI_TIMING_CONFIG_1MBITS();

  // Only let SYNC (0x080) in, every other frame on the bus gets dropped by the hardware filter.
  // 11-bit IDs sit in the top 11 bits of the filter, hence the << 21
  twai_filter_config_t  f = {
    .acceptance_code = (uint32_t)SYNC_ID << 21,
    .acceptance_mask = ~(0x7FFu << 21),
    .single_filter   = true,
  };

  // Only SYNC comes in, so a small incoming queue is plenty
  g.rx_queue_len = 4;

  // Checks for TWAI installed and twai_start (so we don't break things)
  if (twai_driver_install(&g, &t, &f) != ESP_OK || twai_start() != ESP_OK) {
    Serial.println("CAN init failed");
    while (true) delay(1000);
  }

  // Set up alerts and print that node is ready
  twai_reconfigure_alerts(TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED, NULL);
  nodeState = 1;
  Serial.println("Node ready");
}

void loop() {

  // Grab any SYNC frames and note when they came in
  twai_message_t rx;
  while (twai_receive(&rx, 0) == ESP_OK) {
    if (rx.identifier == SYNC_ID) {
      lastSyncUs = micros();
      syncSeen = true;
    }
  }

  // Heartbeat once a second
  static uint32_t lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 1000) {
    lastHeartbeat = millis();
    sendHeartbeat();
  }

  // Gets number of messages (This was way too hard to figure out)
  int messageCount = sizeof(messages) / sizeof(messages[0]);

  // For each message
  for (int i = 0; i < messageCount; i++) {

    // Message m is assigned to current message
    Message &m = messages[i];

    // Checks to see if it is time to send, if not skip
    if (millis() - m.lastSent < m.periodMs) continue;

    // If is time to send, reset timer and initialize message
    m.lastSent = millis();
    twai_message_t tx = {};

    // Fill your data into a full 8 byte buffer first, so a fill function that writes too much can't run off the frame
    uint8_t buf[8] = {};
    uint8_t len = m.fill(buf);
    if (len > MAX_DATA) {
      Serial.printf("Message 0x%03X has %d data bytes, max is %d. Not sent.\n", (unsigned)m.id, len, MAX_DATA);
      continue;
    }

    // Fill header and message information, data goes after the header
    memcpy(tx.data + HEADER_LEN, buf, len);
    addOneByte (tx.data, 0, m.seq++);
    addTwoBytes(tx.data, 1, timeSinceSync());
    tx.identifier = m.id;
    tx.data_length_code = HEADER_LEN + len;

    // Send message
    twai_transmit(&tx, 0);
  }

  // Tries to handle error recovery
  uint32_t alerts;
  if (twai_read_alerts(&alerts, 0) == ESP_OK) {
    if (alerts & TWAI_ALERT_BUS_OFF)       twai_initiate_recovery();
    if (alerts & TWAI_ALERT_BUS_RECOVERED) twai_start();
  }
}
