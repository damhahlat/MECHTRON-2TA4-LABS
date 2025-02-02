// Include necessary libraries
#include "LCD_DISCO_F429ZI.h"
#include "mbed.h"
#include <chrono>
#include <cstdlib>
#include <time.h>

// LCD object initialization
LCD_DISCO_F429ZI LCD;

// Buttons
InterruptIn external_button(PA_6, PullUp); // External push button connected to PA6
InterruptIn userButton(BUTTON1); // Onboard button for resetting

// LEDs
DigitalOut greenLED(PG_13);
DigitalOut redLED(PG_14);

// Ticker and timeout and timer
Ticker ticker;
Timeout timeout;
Timer timer;

// Global variables
bool gameOngoing = 0; // To store whether or not the game has been started (onboard press)
int bestTime = -1; // Stores the best time in ms
int currentTime = -1; // Stores the current time in ms

// Function prototypes
void Restart(); // Restarts the entire game
void StartTimer(); // Starts the timer
void StandbyLoop(); // Default 1 Hz blinking
void ButtonClick(); // Actions to perform when the onboard button is clicked

int main() {

    // Seed the rand function with the current time to randomize results of rand
    srand(time(0));

    // LCD setup
    LCD.SetFont(&Font24);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);

    // If the external button is clicked, run the restart function
    external_button.fall(&Restart);

    // If the internal button is clicked, run the ButtonClick function
    userButton.fall(&ButtonClick);
    __enable_irq();

    // Run the standbyloop function every 0.1 second (1 Hz)
    ticker.attach(&StandbyLoop, 100ms);

    while (1) {

        // Prints the best time on the LCD display
        uint8_t text[30];
        LCD.DisplayStringAt(0, 40, (uint8_t *)"Best time", LEFT_MODE);
        sprintf((char *)text, "Secs: %d", bestTime);
        LCD.DisplayStringAt(0, 80, (uint8_t *)&text, LEFT_MODE);

        // Prints the current time on the LCD display
        uint8_t text2[30];
        LCD.DisplayStringAt(0, 160, (uint8_t *)"Current time", LEFT_MODE);
        sprintf((char *)text2, "Secs: %d", currentTime);
        LCD.DisplayStringAt(0, 200, (uint8_t *)&text2, LEFT_MODE);
    }
}

// Restarts the game to defaults
void Restart() {
    LCD.Clear(LCD_COLOR_WHITE);
    gameOngoing = 0;
    bestTime = -1;
    currentTime = -1;
}

// If the onboard button is clicked
void ButtonClick() {

    // If the game isn't running
    if (gameOngoing == 0) {

        // Start the game and turn off the LED
        gameOngoing = 1;
        greenLED = 0;

        // Generate a random time between 1000 and 5000 ms
        float time = rand() % 4000 + 1000;

        // Convert the time into chrono ms for the attach function. Conversion is the courtesy of chatgpt
        std::chrono::milliseconds time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(time/1000));
        timeout.attach(&StartTimer, time_ms);

        // Quit the function
        return;
    }

    // If the game is already running, run the code below

    // Clear the LCD since a new value(s) will be displayed
    LCD.Clear(LCD_COLOR_WHITE);

    // If the button click is after the timer has started (no cheating)
    if (timer.elapsed_time().count() > 0) {

        // Stop the timer, get the elapsed time, and convert it from us to ms
        timer.stop();
        currentTime = timer.elapsed_time().count()/1000;

        // If the current time is better than the best time or the best time is at its default -1, set the current time to the best time
        if (currentTime < bestTime || bestTime == -1) bestTime = currentTime;

        // Reset the timer
        timer.reset();
    }

    // If cheating happens
    else {

        // Set the current time to -1 and stop and reset the timer
        currentTime = -1;
        timer.stop();
        timer.reset();
    }

    // End the game
    gameOngoing = 0;
}

// When the delay for the green LED has been reached, turn it on and start the timer
void StartTimer() {
    greenLED = 1;
    timer.start();
}

// Loop for when the game isn't being played
void StandbyLoop() {

    // Flicker the green LED
    if (gameOngoing == 0) {
        greenLED = !greenLED;
    }
}
