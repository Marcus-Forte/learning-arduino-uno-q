#include <Arduino.h>
#include <zephyr/kernel.h>

#define STACK_SIZE 1024
#define THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensor_thread_stack, STACK_SIZE);
struct k_thread sensor_thread_data;

void sensor_task(void *arg1, void *arg2, void *arg3) {
  while (true) {
    // High-frequency deterministic polling (e.g., IMU via SPI/I2C)
    // ... code here ...

    k_msleep(10); // Sleep 10ms, yielding CPU to other threads
  }
}

const auto LED = LED3_B; // Use the built-in red LED on the Arduino board

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(LED, OUTPUT);
  // Spawn the background thread
  k_thread_create(&sensor_thread_data, sensor_thread_stack,
                  K_THREAD_STACK_SIZEOF(sensor_thread_stack), sensor_task, NULL,
                  NULL, NULL, THREAD_PRIORITY, 0, K_NO_WAIT);
}

void loop() {
  // Your main code here
  delay(1000);
  digitalWrite(LED, HIGH);
  Serial.println("High!");
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("Low!");
}