#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint16_t timer_divider = 80; // timer with clock source of 80[MHz] will tick at 10[MHz] now
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 2000 / portTICK_PERIOD_MS;

// Pins
static const int adc_pin = A0;
// Globals
static hw_timer_t *timer = NULL;
static volatile uint16_t val;
static SemaphoreHandle_t bin_sem = NULL;

//*************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {

  BaseType_t task_woken = pdFALSE;
  // Perform action (read from ADC)
  val = analogRead(adc_pin);

  // Give semaphore to tell task that new value is ready
  xSemaphoreGiveFromISR(bin_sem, &task_woken);

  // Vanilla FreeRTOS - Exit from ISR
  // portYIELD_FROM_ISR(task_woken);

  // ESP-IDF - Exit from ISR
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

//**********************************
// Tasks

// Task: Wait for semaphore and print out ADC value when received
void printValues(void *parameter) {
  while(1) {
    // Loop forever, wait for semaphore, and print value
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.println(val);   
  }
}


//**********************************
// Main (runs as its own task with priority 1 on core 1)
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR buffer demo---");

  // Create semaphore before it is used (in task or ISR)
  bin_sem = xSemaphoreCreateBinary();

  // Force reboot if we can't create the semaphore
  if (bin_sem == NULL) {
    Serial.print("Could not create semaphore");
    ESP.restart();
  }

  // Start task to print out values
  xTaskCreatePinnedToCore(printValues,
                          "Print Values",
                          1024,
                          NULL,
                          2,
                          NULL,
                          app_cpu);

  // Create and start timer
  timer = timerBegin(0, timer_divider, true);

  // Provide ISR to timer
  timerAttachInterrupt(timer, &onTimer, true);

  // At what count should ISR trigger
  timerAlarmWrite(timer, timer_max_count, true);

  // Allow ISR to trigger
  timerAlarmEnable(timer);

  // Delete "setup and loop" task
  vTaskDelete(NULL);

}

void loop() {
  // Execution shouldn't get here

}
