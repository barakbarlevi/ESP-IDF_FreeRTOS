// Settings
static const uint16_t timer_divider = 80; // timer with clock source of 80[MHz] will tick at 1[MHz] now
static const uint64_t timer_max_count = 1000000;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals
static hw_timer_t *timer = NULL;

//*************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {

  // Toggle LED
  int pin_state = digitalRead(led_pin);
  digitalWrite(led_pin, !pin_state);
}


//**********************************
// Main
void setup() {
  // Configure LED pin
  pinMode(led_pin, OUTPUT);

  // Create and start timer
  timer = timerBegin(0, timer_divider, true);

  // Provide ISR to timer
  timerAttachInterrupt(timer, &onTimer, true);

  // At what count should ISR trigger
  timerAlarmWrite(timer, timer_max_count, true);

  // Allow ISR to trigger
  timerAlarmEnable(timer);

}

void loop() {
  // put your main code here, to run repeatedly:

}
