// Core definitions (assuming dual core ESP32)
static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

// Settings
static const TickType_t time_hog = 200; // [ms] hogging the CPU in task 1
static const TickType_t task_0_delay = 30; // [ms] Time task 0 blocks itself
static const TickType_t task_1_delay = 100; // [ms] Time task 1 blocks itself

// Pins
static const int pin_0 = 12; 
static const int pin_1 = 13; 

// Globals 
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

//******************************
// Functions

// Hogs the processor. Accurate to about 1 second
static void hog_delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 30000; j++) {
      asm("nop");
    }
  }
}


//******************************
// Tasks

// Task in core 0
void doTask0(void *parameter) {
  pinMode(pin_0, OUTPUT); // Configure pin
  // Loop forever
  while(1) {
    //portENTER_CRITICAL(&spinlock);
    digitalWrite(pin_0, !digitalRead(pin_0)); // Toggle LED
    //portEXIT_CRITICAL(&spinlock);
    vTaskDelay(task_0_delay / portTICK_PERIOD_MS);
  }
}

// Task in core 1
void doTask1(void *parameter) {
  pinMode(pin_1, OUTPUT); // Configure pin
  // Loop forever
  while(1) {
    portENTER_CRITICAL(&spinlock); 
    digitalWrite(pin_1, HIGH);
    hog_delay(time_hog);
    digitalWrite(pin_1, LOW);
    portEXIT_CRITICAL(&spinlock);
    vTaskDelay(task_1_delay / portTICK_PERIOD_MS);
  }
}

//******************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

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
