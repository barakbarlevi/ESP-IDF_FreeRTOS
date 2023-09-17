// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
enum {BUF_SIZE = 5}; // Size of buffer array
static const int num_prod_tasks = 5; // Number of producers
static const int num_cons_tasks = 2; // Number of consumers
static const int num_writes = 3; // Num times each producer writes to buf

// Globals
static int buf[BUF_SIZE]; // Shared buffer
static int head = 0; // Writing index to buffer
static int tail = 0; // Reading index to buffer
static SemaphoreHandle_t bin_sem; // Waits for parameter to be read
static SemaphoreHandle_t mutex; // Locks access to the buffer and serial
static SemaphoreHandle_t sem_empty; // Counts number of emptry slots in bug
static SemaphoreHandle_t sem_filled; // Countsnumber of filled slots in buf

//********************************************
// Tasks

// Task: Producer - write a given number of times to shared buffer
void producer(void *parameter) {
  
  // Copy the parameter into a local var
  int num = *(int *)parameter;

  // Release the bin sem
  xSemaphoreGive(bin_sem);

  // Fill shared buffer with task number
  for (int i =0; i < num_writes; i++) {
    // Wait for empty slot in buffer to be available
    xSemaphoreTake(sem_empty, portMAX_DELAY);

    // Lock critical section with a mutex
    xSemaphoreTake(mutex, portMAX_DELAY);

    // Critical section (accessing shared buffer)
    buf[head] = num;
    head = (head + 1) % BUF_SIZE;

    xSemaphoreGive(mutex);

    // Signal to consumer tasks that a slot in the buffer has been filled
    xSemaphoreGive(sem_filled);
    
  }

  // Delete self task
  vTaskDelete(NULL);
}

// Task: Consumer - continuously read from shared buffer
void consumer (void *parameter) {
  int val;

  // Read from buffer
  while(1) {
    // Wait for at least one slot in buffer to be filled
    xSemaphoreTake(sem_filled, portMAX_DELAY);

    // Lock Critical section (accessing shared buffer and serial)
    xSemaphoreTake(mutex, portMAX_DELAY);
    val = buf[tail];
    tail = (tail +1) % BUF_SIZE;
    Serial.println(val);
    xSemaphoreGive(mutex);

    // Signal to producer thread that a slot in the buffer is free    
    xSemaphoreGive(sem_empty);
  }
}

//********************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  char task_name[12];

  // Configure serial and wait a moment
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Semaphore solution---");

  // Create mutexes and semaphores before starting tasks
  bin_sem = xSemaphoreCreateBinary();
  mutex = xSemaphoreCreateMutex();
  sem_empty = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE );
  sem_filled = xSemaphoreCreateCounting(BUF_SIZE, 0);

  
  // Start producer tasks (wait for each to read argument)
  for (int i = 0; i < num_prod_tasks; i++) {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(producer,
                            task_name,
                            1024,
                            (void *)&i,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  // Start consumer tasks
  for (int i = 0; i < num_cons_tasks; i++) {
    sprintf(task_name, "Consumer %i", i);
    xTaskCreatePinnedToCore(consumer,
                            task_name,
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
  }

  // Notify that all tasks have been created
  xSemaphoreTake(mutex, portMAX_DELAY);
  Serial.println("All tasks created!");
  xSemaphoreGive(mutex);
}

void loop() {
  
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
