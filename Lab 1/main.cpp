#include "mbed.h"

InterruptIn userButton(BUTTON1);

DigitalOut led3(PG_13);
DigitalOut led4(PG_14);

Ticker ticker_1;

int state = 0; // default state on startup

void SetState() {
  if (state != 1) {
    state = 1;
  } else {
    state = 2;
  }
}

int lastLED = 0;
void Toggle() { 
    if (state == 0) {
        led4 = !led4;
    }

    else if (state == 1) {
        if (led4 == 1) {led4 = 0; led3 = 1;}
        else {led4 = 1; led3 = 0;}
    }

    else if (state == 2) {
        if (led4 == 1) lastLED = 4;
        if (led3 == 1) lastLED = 3;

        if (led4 == 1 || led3 == 1) { led4 = 0; led3 = 0; }

        else if (led4 == 0 && led3 == 0) {
            if (lastLED == 4) led3 = 1;
            if (lastLED == 3) led4 = 1;
        }
    }
}

int main() {
  userButton.fall(&SetState);
  __enable_irq();

  ticker_1.attach(&Toggle, 1000ms);

  while (true) {
    /*// start with LEDs off to switch between phases cleanly  
    led3 = 0;
    led4 = 0;
    if (state == 0) {
      led4 = 1;
      thread_sleep_for(1000);
      led4 = 0;
      thread_sleep_for(1000);
    } else if (state == 1) {
      led3 = 0;
      led4 = 1;
      thread_sleep_for(1000);
      led3 = 1;
      led4 = 0;
      thread_sleep_for(1000);
    } else {
      led4 = 1;
      thread_sleep_for(1000);
      led4 = 0;
      thread_sleep_for(1000);
      led3 = 1;
      thread_sleep_for(1000);
      led3 = 0;
      thread_sleep_for(1000);
    }*/
  }
}

