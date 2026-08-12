#include "Arduino_RouterBridge.h"
#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <array>

// Matrix is 8x13 pixels.

Arduino_LED_Matrix matrix;

volatile uint32_t g_set_matrix_calls = 0;

void set_led_state(bool state);
void set_matrix(std::array<uint8_t, 104> pixels);

uint8_t logo[104] = {1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0,
                     0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
                     0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

};

void setup() {
  pinMode(LED3_R, OUTPUT);
  pinMode(LED3_B, OUTPUT);

  digitalWrite(LED3_B, LOW); // Turn off the blue LED initially

  matrix.begin();

  matrix.setGrayscaleBits(1);
  matrix.draw(logo);

  Bridge.begin();
  Bridge.provide_safe("set_led_state", set_led_state);
  Bridge.provide_safe("set_matrix", set_matrix);
}

void loop() {
  // Keep loop responsive so provide_safe callbacks can execute quickly.
  delay(1);
}

void set_led_state(bool state) {
  // LOW state means LED is ON
  digitalWrite(LED3_B, state ? LOW : HIGH);
}

void set_matrix(std::array<uint8_t, 104> pixels) {
  ++g_set_matrix_calls;

  matrix.draw(pixels.data());

  // Clear debug latch on successful draw.
  digitalWrite(LED3_R, HIGH);
}