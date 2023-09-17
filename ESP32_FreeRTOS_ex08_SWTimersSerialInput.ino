#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const TickType_t dim_delay = 5000 / portTICK_PERIOD_MS;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals
static TimerHandle_t one_shot_timer = NULL;


//************************************
// Callbacks

// Turn off LED when timer expires
void autoDimmerCallback(TimerHandle_t xTimer) {
  digitalWrite(led_pin, LOW);
}

//***********************************
// Tasks

// Task: Echo things back to serial port, turn on LED while entering input
void doCLI(void *paramter) {
  char c;
  // Configure LED pin
  pinMode(led_pin, OUTPUT);

  while(1) {

    // See if there's an input in the input serial buffer
    if (Serial.available() > 0) {
      // If so, echo everything back to the serial port
      c = Serial.read();
      Serial.print(c);

      // Turn on the LED
      digitalWrite(led_pin, HIGH);

      // Start timer (if timer is already running, this will act
      // as xTimerReset() instead.

      xTimerStart(one_shot_timer, portMAX_DELAY);
    }
  }
}

//***********************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  // Configure serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS LED Dimmer Software Timer---");

  // Create a one-shot timer
  one_shot_timer = xTimerCreate("One-shot timer",
                                dim_delay,
                                pdFALSE,
                                (void *)0,
                                autoDimmerCallback);

 // Start CLI task
 xTaskCreatePinnedToCore(doCLI,
                         "Do CLI",
                         1024,
                         NULL,
                         1,
                         NULL,
                         app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);

}

void loop() {
  // Execution should never get here

}
