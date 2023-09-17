
#include <stdlib.h> // Needed for atoi()

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint8_t buf_len = 20;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals

static int led_delay = 500; // ms, initial value

//*****************************************
// Tasks

// Task: Blinking LED at rate set by the global variable led_delay
void toggleLED(void *parameter) {
  while(1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
  }
}

// Task: Read from serial terminal
// For non-arduino, we would need to replace Serial with own UART code.
void readSerial(void *parameter) {
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  // Clear the whole buffer
  memset(buf, 0, buf_len);

  // Loop forever
  while (1) {
    // Read characters from serial
    if (Serial.available() > 0) {
      c = Serial.read();

      // Update delay variable and reset buffer if we get a newline character
      if (c == '\n') {
        led_delay = atoi(buf);
        Serial.print("Updated LED delay to: ");
        Serial.println(led_delay);
        memset(buf, 0, buf_len);
        idx = 0;
      } else {

        // Only append if index is not over message limit
        if (idx < buf_len -1) {
          buf[idx] = c;
          idx++;
        }
      }
    }
  }
}

//*****************************************
// Main

void setup() {
  //Configure pin
  pinMode(led_pin, OUTPUT);

  // Configure serial and wait a second
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Multi-task LED demo");
  Serial.println("Enter a number in [ms] to change LED delay");

  // Start blink task
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              toggleLED,    // Function to be called
              "Toggle LED", // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS
              NULL,         // Parameter to pass
              1,            // Task priority
              NULL,         // Task handle
              app_cpu);     //  Run on one core for demo puposes (ESP32 only)

  // Start serial read task
  xTaskCreatePinnedToCore(readSerial,     // Function to call
                          "Read Serial",  // Name of task
                          1024,           // Stack size
                          NULL,           // Parameter to pass
                          1,              // Task priority
                          NULL,           // Task handle
                          app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}
