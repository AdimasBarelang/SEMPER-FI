#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#define SERVO_PIN 17  // Using PWM0_M2, change if using a different pin
#define MIN_PULSE 100 // Pulse width for 0 degrees (1ms)
#define MAX_PULSE 175 // Pulse width for 110 degrees (1.75ms)
#define STEP_DELAY 20 // Delay between each step (in milliseconds)

void moveServoGradually(int start, int end, int stepDelay) {
    int step = (start < end) ? 1 : -1;
    for (int pulse = start; pulse != end + step; pulse += step) {
        pwmWrite(SERVO_PIN, pulse);
        delay(stepDelay);
    }
}

int main(void) {
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed\n");
        return 1;
    }

    pinMode(SERVO_PIN, PWM_OUTPUT);
    pwmSetClock(SERVO_PIN, 240);  // Clock divisor
    pwmSetRange(SERVO_PIN, 2000); // PWM range

    printf("Moving servo. Press Ctrl+C to exit.\n");

    while(1) {
        // Move slowly from 0 to 119 degrees
        printf("Moving from 0 to 119 degrees...\n");
        moveServoGradually(MIN_PULSE, MAX_PULSE, STEP_DELAY);
        delay(1000);  // Pause at 110 degrees

        // Move slowly from 119 back to 0 degrees
        printf("Moving from 119 to 0 degrees...\n");
        moveServoGradually(MAX_PULSE, MIN_PULSE, STEP_DELAY);
        delay(1000);  // Pause at 0 degrees
    }

    return 0;
}
