#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint16_t timer_divider = 8; // timer with clock source of 80[MHz] will tick at 10[MHz] now
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 2000 / portTICK_PERIOD_MS;

// Globals
static hw_timer_t *timer = NULL;
static volatile int isr_counter;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

//*************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {

  // ESP-IDF version of a critial section (in an ISR)
  portENTER_CRITICAL_ISR(&spinlock);
  isr_counter++;
  portEXIT_CRITICAL_ISR(&spinlock);

  // Vanilla RTOS version of critical section (in an ISR)
  // UbaseType_t saved_int_status;
  // saved_int_status = taskENTER_CRITICAL_FROM_ISR();
  // isr_counter++;
  // taskEXIT_CRITICAL_FROM_ISR(saved_int_status);
}

//**********************************
// Tasks

// Task: Wait for semaphore and print out ADC value when received
void printValues(void *parameter) {
  while(1) {
    // Count down and print out counter value
    while(isr_counter > 0) {
      Serial.println(isr_counter);
  
      //ESP-IDF version of a critical section (in a task)
      portENTER_CRITICAL(&spinlock);
      isr_counter--;
      portEXIT_CRITICAL(&spinlock);
  
      // Vanilla FreeRTOS version of a critical section (in a task)
      // taskENTER_CRITICAL();
      // isr_counter--;
      // taskEXIT_CRITICAL();
    }
    // Wait 2 seconds while ISR increments counter a few times
    vTaskDelay(task_delay);
  }
}


//**********************************
// Main
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR critical section demo---");

  // Start task to print out values
  xTaskCreatePinnedToCore(printValues,
                          "Print Values",
                          1024,
                          NULL,
                          1,
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
