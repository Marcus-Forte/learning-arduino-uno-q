#include <Arduino.h>

void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  
  // Wait for the serial port to connect. 
  // This is often necessary for boards with native USB (like the UNO Q)
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("Arduino UNO Q serial initialized!");
}

void loop() {
  // Print a message to the serial monitor:
  Serial.println("Hello from the MCU!");

  // Toggle LED and print its state
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("LED: ON");
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("LED: OFF");
  
  // Wait for 500 milliseconds:
  delay(1000);
}