// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
TickType_t cs_wait = 250; // Time spent in critical section [ms]
TickType_t med_wait = 5000; // Rime medium task spends working [ms]

// Globals
static SemaphoreHandle_t lock;

//***************************************
// Tasks

// Task L (low priority)
void doTaskL(void *parameter) {
  TickType_t timestamp;

  // Loop forever
  while(1) {
    // Take lock
    Serial.println("Task L trying to take lock..");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    xSemaphoreTake(lock, portMAX_DELAY);

    // Mention how long we spend waiting for a lock
    Serial.print("Task L got lock. Spent ");
    Serial.print((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp);
    Serial.println(" [ms] waiting for the lock. Doing some work..");

    // Hog the processor for a while doing nothing
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while( (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < cs_wait);

    // Release lock
    Serial.println("Task L releasing lock");
    xSemaphoreGive(lock);

    // Go to sleep
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// Task M (medium priority)
void doTaskM(void *parameter) {

  //Loop forever
  while(1) {
    TickType_t timestamp;
    // Hog the processor for a while doing nothing
    Serial.println("Task M doing some work..");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while( (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < med_wait);

    // Go to sleep
    Serial.println("Task M done");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
  }
}

// Task H (high priority)
void doTaskH(void *parameter) {
  TickType_t timestamp;

  // Loop forever
  while(1) {
    // Take lock
    Serial.println("Task H trying to take lock..");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    xSemaphoreTake(lock, portMAX_DELAY);

    // Mention how long we spend waiting for a lock
    Serial.print("Task H got lock. Spent ");
    Serial.print((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp);
    Serial.println(" [ms] waiting for the lock. Doing some work..");

    // Hog the processor for a while doing nothing
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while( (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < cs_wait);

    // Release lock
    Serial.println("Task H releasing lock");
    xSemaphoreGive(lock);

    // Go to sleep
    vTaskDelay(500 / portTICK_PERIOD_MS); 
  }
}

//***************************************
// Main (runs as its own task with priorit 1 on core 1)

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Priority Inversion Demo---");

  // Create semaphores and mutexes before starting tasks
  //lock = xSemaphoreCreateBinary();
  //xSemaphoreGive(lock); // Make sure bin sem starts at 1

  lock = xSemaphoreCreateMutex();

  // The order of starting the tasks matters to force priority inversion

  // Start task L
  xTaskCreatePinnedToCore(doTaskL,
                          "Task L",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Introduce 1 delay to force priority inversion
  vTaskDelay(15 / portTICK_PERIOD_MS);

  // Start task H
  xTaskCreatePinnedToCore(doTaskH,
                          "Task H",
                          1024,
                          NULL,
                          3,
                          NULL,
                          app_cpu);
  // Start task M
  xTaskCreatePinnedToCore(doTaskM,
                          "Task M",
                          1024,
                          NULL,
                          2,
                          NULL,
                          app_cpu);

  // Delete "setup and loop"
  vTaskDelete(NULL);

}

void loop() {
  // Execution should never get here

}
