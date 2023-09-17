// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
//TickType_t mutex_timeout = 1000 / portTICK_PERIOD_MS;

// Globals

static SemaphoreHandle_t mutex_1;
static SemaphoreHandle_t mutex_2;

//****************************************
// Tasks

// Task A (high priority)
void doTaskA(void *parameter) {
  // Loop forever
  while(1) {

    // Take mutex 1 (introduce wait to force deadlock)
    xSemaphoreTake(mutex_1, portMAX_DELAY);
    Serial.println("Task A took mutex 1");
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // Take mutex 2
    xSemaphoreTake(mutex_2, portMAX_DELAY);
    Serial.println("Task A took mutex 2");

    // Critical section protected by 2 mutexes
    Serial.println("Task A doing some work");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Give back mutexes
    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);

    // Wait to let the other task execute
    Serial.println("Task A going to sleep");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// Task B (low priority)
void doTaskB(void *parameter) {
  while(1) {

    // Take mutex 2 (introduce wait to force deadlock)
    xSemaphoreTake(mutex_2, portMAX_DELAY);
    // Avoid deadlock: take mutex 1
    Serial.println("Task B took mutex 2");
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // Take mutex 1
    xSemaphoreTake(mutex_1, portMAX_DELAY);
    // Avoid deadlock: take mutex 2
    Serial.println("Task B took mutex 1");

    // Critical section protected by 2 mutexes
    Serial.println("Task B doing some work");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Give back mutexes, always in reverse order to taking them
    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);

    // Wait to let the other task execute
    // We should never get here for deadlock demo
    Serial.println("Should never get here");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

//****************************************
// Main (runs as its own task with priority 1 on core 1

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS deadlock demo---");

  // Create mutexes before starting tasks
  mutex_1 = xSemaphoreCreateMutex();
  mutex_2 = xSemaphoreCreateMutex();

  // Start task A (high priority)
  xTaskCreatePinnedToCore(doTaskA,
                          "Task A",
                          1024,
                          NULL,
                          2,
                          NULL,
                          app_cpu);
  
  // Start task B (low priority)
  xTaskCreatePinnedToCore(doTaskB,
                          "Task B",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

 //vTaskDelete(NULL);
 

}

void loop() {
  // put your main code here, to run repeatedly:

}
