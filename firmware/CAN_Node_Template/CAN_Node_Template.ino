// CAN Node template for sensors (Should be XIAO ESP32-C3s unless i'm stupid or we changed it somewhere)
// If you have any questions my email is cc574924@ohio.edu

// Copy this code for each new node, then make the changes listed below.

// 1: Make a message function for each message (under Structs / Functions for filling Messages, how to do so listed below)
// 2: Add lines to messages[] table at bottom of section for each new message (Format: { ID, send period in ms, your function, 0 })
// 3: Remember to remove the example message

// HOW TO ADD A MESSAGE
//   1. Pick an ID From the GitHub Can-Spec that matches your selected node.                  ----- https://github.com/Bobcat-BAJA/can-spec -----
//   2. Write a fill function like fillSensor() below: put each value into the
//      frame with addOneByte / addTwoByte / addFourByte and return how many bytes you used (max 8).
//   3. Add one line to the messages[] table: { ID, send period in ms, your function, 0 }.
//   4. Write down the byte layout (what each byte means, units, scale) for whoever
//      reads the message.

// ---- Data format ----
//  - Up to 8 bytes per frame (data[0]..data[7]), actual content should be the same as spec
//  - Multi-byte values go low byte first (the helpers below do this for you, you can look into
//    them if you want to understand what they are doing, but basically it adds a specific amount
//    of bytes
//  - NOTE: Integers only. I'm not sure if that'll ever be a problem, but please try to make sure
//    your inputs are integers. NO FLOATS
//  - NOTE 2: I am trusting that you will properly understand what information you are pumping into
//    the node. If data is malformed, or input in an unexpected way, there could be issues. Like if
//    you try to read one byte from a pin, and it is sending 2, it could lead to issues. Please
//    double check your sensors.


// ---- Actual start to the code ----
#include "driver/twai.h"

// ---- Node IO Settings ----
#define TX_PIN      GPIO_NUM_5      // D3 -> CAN TX
#define RX_PIN      GPIO_NUM_4      // D2 -> CAN RX

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

// Two example messages are below, simply copy / paste and add data (format explained at top)

// Example struct for filling data. NOTE: THIS ONLY FILLS THE DATA READ FROM THE SENSOR. THE ID AND TIMESTAMPS ARE HANDLED SEPERATE.
uint8_t fillSensor(uint8_t *data) {
  // pos should always be set to 0 to start
  uint8_t pos = 0;
  // Simply add information per byte size
  pos = addTwoBytes (data, pos, analogRead(A0)); // reads from analog A0
  pos = addOneByte  (data, pos, 0);        // placeholder status
  pos = addFourBytes(data, pos, 123456);   // placeholder 4-byte value
  return pos;                         // should be 7, because 2 + 1 + 4 = 7
}

// Heartbeat: uptime in seconds
uint8_t fillHeartbeat(uint8_t *data) {
  // Just adds two bytes to data and returns the value
  return addTwoBytes(data, 0, millis() / 1000);
}

// Message struct. Should realistically never need messed with, if you do, either you or I messed up somewhere
struct Message {
  uint32_t id;                    // unique on the bus; lower = higher priority
  uint32_t periodMs;              // how often to send
  uint8_t (*fill)(uint8_t *data); // writes the data bytes, returns the length
  uint32_t lastSent;              // when last was sent. Should initialize at 0
};

// One line per message this node sends. IDs must not be used by any other node.
// Format: { ID, send period in ms, your function, 0 }
Message messages[] = {
  { 0x100, 100,  fillSensor,    0 },
  { 0x700, 1000, fillHeartbeat, 0 },
};
 

// Everything below this point should never need touched unless something goes terribly wrong
void setup() {
  
  // Start serial and setup delay
  Serial.begin(115200);
  delay(1000);
 
  // General config
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t  t = TWAI_TIMING_CONFIG_1MBITS();
  twai_filter_config_t  f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Don't read incoming frames, so incoming queue is limited
  g.rx_queue_len = 1;

  // Checks for TWAI installed and twai_start (so we don't break things)
  if (twai_driver_install(&g, &t, &f) != ESP_OK || twai_start() != ESP_OK) {
    Serial.println("CAN init failed");
    while (true) delay(1000);
  }
  
  // Set up alerts and print that node is ready
  twai_reconfigure_alerts(TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED, NULL);
  Serial.println("Node ready");
}
 
void loop() {

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
    
    // Fill message information
    tx.identifier = m.id;
    tx.data_length_code = m.fill(tx.data);

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