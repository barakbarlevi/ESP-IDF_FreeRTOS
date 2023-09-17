// Core definitions (assuming dual core ESP32)
static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

// Settings
static const uint32_t task_0_delay = 500; //[ms] Time task 0 blocks itself

// Pins
static const int pin_1 = 13; // LED pin

// Globals 
static SemaphoreHandle_t bin_sem;

//******************************
// Tasks

// Task in core 0
void doTask0(void *parameter) {
  pinMode(pin_1, OUTPUT); // Configure pin

  // Loop forever
  while(1) {
    xSemaphoreGive(bin_sem); // Notify other task
    vTaskDelay(task_0_delay / portTICK_PERIOD_MS);
  }
}

// Task in core 1
void doTask1(void *parameter) {

  // Loop forever
  while(1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY); // Wait for semaphore
    digitalWrite(pin_1, !digitalRead(pin_1)); // Toggle LED
  }
}

//******************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  bin_sem = xSemaphoreCreateBinary();

  // Start task 0 on core 0
  xTaskCreatePinnedToCore(doTask0,
                          "Task 0",
                          1024,
                          NULL,
                          1,
                          NULL,
                          pro_cpu);

  // Start task 1 on core 1
  xTaskCreatePinnedToCore(doTask1,
                          "Task 1",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

 // Delete "setup and loop" task
 vTaskDelete(NULL);                        
}

void loop() {
  // Should never get here

}
