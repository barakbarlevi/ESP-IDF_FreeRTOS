/**
 * FreeRTOS Heap Demo
 * 
 * One task reads from Serial, constructs a message buffer, and the second
 * prints the message to the console.
 * 
 * Date: December 5, 2020
 * Author: Shawn Hymel
 * License: 0BSD
 */

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint8_t buf_len = 255;

// Globals
static char *msg_ptr = NULL;
static volatile uint8_t msg_flag = 0;

//************************************
//Tasks

//Task: read message from serial buffer
void readSerial(void *parameter) {
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  // Clear the whole buffer
  memset(buf, 0, buf_len);

  // Loop forever
  while(1) {
    //Read characters from serial
    if (Serial.available() > 0) {
      c = Serial.read();

      // Store recieved character to buffer if not over buffer limit
      if (idx < buf_len -1) {
        buf[idx] = c;
        idx++;
      }

      // Create a message buffer for print task
      if (c == '\n') {
        // The last character in the string is '\n', so we need
        // To replace it with '\0' to make it null-terminated
        buf[idx -1] = '\0';

        // Try to allocate memory and copy over message.
        // If message buffer is still in use, ignore 
        // The entire message.

        if (msg_flag == 0) {
          msg_ptr = (char *)pvPortMalloc(idx * sizeof(char));

          // If malloc returns 0 (out of memory), throw and error and reset
          configASSERT(msg_ptr);

          // Copy message
          memcpy(msg_ptr, buf, idx);

          // Notify other task that message is ready
          msg_flag = 1;
        }

        // Reset receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      }
    }
  }
}

// Task: print message whenever flag is set and free buffer
void printMessage(void *parameter) {
  while(1) {
    // Wait for flag to be set and print message
    if (msg_flag == 1) {
      Serial.println(msg_ptr);

      // Give amount of free heap memory
      Serial.print("Free heap (bytes): ");
      Serial.println(xPortGetFreeHeapSize());

      // Free buffer, set pointer to null and clear flag
      vPortFree(msg_ptr);
      msg_ptr = NULL;
      msg_flag = 0;
    }
  }
}

//************************************
// Main (runs as its own task with priority 1 on core 1

void setup() {
  // Configure serial
  Serial.begin(115200);

  // Wait a moment to start so we don't miss serial output
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Heap demo---");
  Serial.println("Enter a string");

  // Start serial recieve task
  xTaskCreatePinnedToCore(readSerial,
                          "Read Serial",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Start serial print task
  xTaskCreatePinnedToCore(printMessage,
                          "Print Message",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Delete setup and loop task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}
