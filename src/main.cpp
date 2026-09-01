#include <Arduino.h>
#include <atomic>
#include <OneButton.h>

#define BTN_START_TIMER_IN 12
#define TIMER_LED_OUT 13

std::atomic<uint32_t> isrCounter(0);
volatile bool timerFired = false;

hw_timer_t *timer = NULL;

bool led_state = false;
bool alarm_state = false;

// Функція переривання (ISR)
void IRAM_ATTR onTimer() {
	// Операції ++ та присвоєння для atomic є безпечними (атомарними)
	isrCounter.fetch_add(1, std::memory_order_relaxed);

	if(led_state){
       digitalWrite(TIMER_LED_OUT, LOW);
	   led_state = false;
	}else 
	{
       digitalWrite(TIMER_LED_OUT, HIGH);
	   led_state = true;
	}
	timerFired = true;
}

void startTimer() {
	if (!timer) {
		return;
	}

//	timerFired = false;
	timerAlarmDisable(timer);
//	digitalWrite(TIMER_LED_OUT, HIGH);
	
	Serial.println("Timer started...");
}

void setup() {
	Serial.begin(115200);
	enableLoopWDT();

	pinMode(TIMER_LED_OUT, OUTPUT);
	pinMode(BTN_START_TIMER_IN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_START_TIMER_IN),startTimer,PULLUP);
	digitalWrite(TIMER_LED_OUT, LOW);
	
	// Ініціалізація таймера (ESP32-S3)
	// divider = 80: 80 МГц / 80 = 1 МГц (1 tick = 1 мкс)
	timer = timerBegin(0, 80, true);

	// Прив'язка функції переривання
	timerAttachInterrupt(timer, &onTimer, true);

	// Одноразове спрацювання через 1 000 000 мікросекунд = 1 секунду
	timerAlarmWrite(timer, 1000000, true);
	timerAlarmEnable(timer);

    digitalWrite(TIMER_LED_OUT, HIGH);

	Serial.println("Press the button to start the timer...");
}

void loop() {


	if (timerFired) {
	
	
	

		// Safely read the counter value
		uint32_t currentCount = isrCounter.load();

		Serial.print("Trigger #: ");
		Serial.println(currentCount);
		//Serial.print(" | Time: ");
		//Serial.print(millis());
		//Serial.println(" ms");
	    timerFired = false;
	}

	feedLoopWDT();

}

