#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"

// Create instances of the LCD and Touchscreen classes
LCD_DISCO_F429ZI lcd;
TS_DISCO_F429ZI ts;

Timeout timeout1;

DigitalOut LedR(PG_14);

// Define the analog input pin for the LM35 temperature sensor
AnalogIn analog_input(PA_2);

PwmOut motor(PD_14);



// Constants for screen layout
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define BUTTON_HEIGHT 50
#define BUTTON_WIDTH  100
#define BUTTON_Y      (SCREEN_HEIGHT - BUTTON_HEIGHT - 10)

// Global variables
float room_temperature = 0.0;
float temperature_threshold = 0.0;
bool update_display = false; // Flag to indicate when to refresh the display

// Function prototypes
float read_temperature();
void draw_buttons();
void update_lcd();

// Ticker for periodic updates
Ticker ticker;

float duty=0;


int flag=1;


// Periodic update function (called by the ticker)
void periodic_update() {
    // Set flag to update the display
    update_display = true;
}

void start_motor(){
    motor.period_ms(100);
    motor.write(duty);
    if(duty < 0.7){
        duty+=0.01;
        timeout1.attach(start_motor, 1000ms);
    }
}

int main() {
    // Set the reference voltage for the ADC (adjust this value based on your setup)
    analog_input.set_reference_voltage(3); // Assuming 3.3V reference voltage

    // Measure initial room temperature
    room_temperature = read_temperature();
    temperature_threshold = roundf(room_temperature) + 1.0; // Initial threshold is 1°C above room temperature

    // Initialize the LCD
    lcd.Clear(LCD_COLOR_WHITE); // Clear the LCD with a white background
    draw_buttons();

    // Set up a ticker to trigger periodic updates every 500ms
    ticker.attach(&periodic_update, 500ms);

    while (true) {
        // Read current temperature (analog reading in the main loop)
        room_temperature = read_temperature();

        // Print temperature readings and fan status to the serial monitor
        printf("Sensor Temperature: %.2f C, Threshold: %.2f C\n", room_temperature, temperature_threshold);

        // Fan control logic
        if (room_temperature > temperature_threshold) {
            LedR = 1;
            if(flag){
            timeout1.attach(start_motor, 1000ms);
            flag = 0;
            }
        } else {
            LedR=0;
            flag =1;
			duty = 0;
            motor.write(0.0f); // Turn off the fan
            printf("Fan Speed: OFF\n");
        }

        // Update the LCD display if the flag is set
        if (update_display) {
            update_lcd();
            update_display = false; // Reset the flag
        }

        // Read touchscreen state
        TS_StateTypeDef ts_state;
        ts.GetState(&ts_state);

        if (ts_state.TouchDetected) {
            int touch_x = ts_state.X;

            // Check if the left half of the screen is touched ("+" action)
            if (touch_x < SCREEN_WIDTH / 2) {
                // Highlight "+" button by changing its background to green
                lcd.SetTextColor(LCD_COLOR_GREEN);
                lcd.FillRect((SCREEN_WIDTH / 2) - BUTTON_WIDTH - 10, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
                lcd.SetTextColor(LCD_COLOR_BLACK);
                lcd.DisplayChar((SCREEN_WIDTH / 2) - BUTTON_WIDTH - 10 + (BUTTON_WIDTH / 2), BUTTON_Y + (BUTTON_HEIGHT / 2) - 12, '+');

                // Increase threshold by 0.5°C
                temperature_threshold += 0.5;
                printf("Threshold increased to %.2f C\n", temperature_threshold);

                // Restore "+" button after a short delay
                ThisThread::sleep_for(200ms);
                draw_buttons();
            }

            // Check if the right half of the screen is touched ("-" action)
            if (touch_x >= SCREEN_WIDTH / 2) {
                // Highlight "-" button by changing its background to green
                lcd.SetTextColor(LCD_COLOR_GREEN);
                lcd.FillRect((SCREEN_WIDTH / 2) + 10, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
                lcd.SetTextColor(LCD_COLOR_BLACK);
                lcd.DisplayChar((SCREEN_WIDTH / 2) + 10 + (BUTTON_WIDTH / 2), BUTTON_Y + (BUTTON_HEIGHT / 2) - 12, '-');

                // Decrease threshold by 0.5°C
                temperature_threshold -= 0.5;
                printf("Threshold decreased to %.2f C\n", temperature_threshold);

                // Restore "-" button after a short delay
                ThisThread::sleep_for(200ms);
                draw_buttons();
            }
        }

        // Small delay to prevent busy-waiting
        ThisThread::sleep_for(10ms);
    }
}

// Function to read temperature from LM35
float read_temperature() {
    float voltage = analog_input.read_voltage(); // Read voltage from LM35
    return voltage * 100.0; // Convert voltage to temperature (10 mV/°C)
}

// Function to draw buttons
void draw_buttons() {
    // Draw "+" button with red background
    lcd.SetTextColor(LCD_COLOR_RED);
    lcd.FillRect((SCREEN_WIDTH / 2) - BUTTON_WIDTH - 10, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.DisplayChar((SCREEN_WIDTH / 2) - BUTTON_WIDTH - 10 + (BUTTON_WIDTH / 2), BUTTON_Y + (BUTTON_HEIGHT / 2) - 12, '+');

    // Draw "-" button with red background
    lcd.SetTextColor(LCD_COLOR_RED);
    lcd.FillRect((SCREEN_WIDTH / 2) + 10, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.DisplayChar((SCREEN_WIDTH / 2) + 10 + (BUTTON_WIDTH / 2), BUTTON_Y + (BUTTON_HEIGHT / 2) - 12, '-');
}

// Function to update the LCD display
void update_lcd() {
    // Clear the LCD except for the buttons
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.FillRect(0, 0, SCREEN_WIDTH, BUTTON_Y);

    // Display sensor temperature
    lcd.SetFont(&Font24);
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.DisplayStringAt(0, 50, (uint8_t *)"Sensor:", CENTER_MODE);
    char sensor_str[20];
    sprintf(sensor_str, "%.1f C", room_temperature);
    lcd.DisplayStringAt(0, 80, (uint8_t *)sensor_str, CENTER_MODE);

    // Display temperature threshold
    lcd.DisplayStringAt(0, 130, (uint8_t *)"Threshold:", CENTER_MODE);
    char threshold_str[20];
    sprintf(threshold_str, "%.1f C", temperature_threshold);
    lcd.DisplayStringAt(0, 160, (uint8_t *)threshold_str, CENTER_MODE);
}

