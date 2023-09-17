// Core definitions (assuming dual core ESP32)
static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

// Settings
static const TickType_t time_hog = 200; // [ms] hoggin the CPU

//*****************************
// Functions

// Hogs the processor. Accurate to about 1 second
static void hog_delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 40000; j++) {
      asm("nop");
    }
  }
}

//****************************
// Tasks

// Task L (low priority)
void doTaskL(void *paramter) {
  TickType_t timestamp;
  char str[20];

  //Loop forever
  while(1) {

    // Print something to serial
    sprintf(str, "Task L, Core %i\r\n", xPortGetCoreID());
    Serial.print(str);

    // Hog the processor for a while doing nothing
    hog_delay(time_hog);
  }
}

// Task H (high priority)
void doTaskH(void *parameter) {
  TickType_t timestamp;
  char str[20];

  // Loop forever
  while(1) {
    // Print something to serial
    sprintf(str, "Task H, Core %i\r\n", xPortGetCoreID());
    Serial.print(str);
  
    // Hod the processor for a while doing nothing
    hog_delay(time_hog);
  }
}

//****************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Priority inheritance demo---");

  // Start tasks
  xTaskCreatePinnedToCore(doTaskL,
                          "Task L",
                          2048,
                          NULL,
                          1,
                          NULL,
                          tskNO_AFFINITY);

   xTaskCreatePinnedToCore(doTaskH,
                           "Task H",
                           2048,
                           NULL,
                           2,
                           NULL,
                           tskNO_AFFINITY);

  vTaskDelete(NULL);                      
}

void loop() {
  // Execution shouldn't get here

}
