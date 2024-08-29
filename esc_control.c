#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define ESC_PIN 21         // WiringPi pin 21 (GPIO 62, Physical pin 32)
#define STEP_PULSE 1       // 10µs step for manual adjustment
#define STEP_DELAY 20      // Delay between each step in milliseconds

// Pulse width constants
#define NEUTRAL_PULSE 190  // 1900µs neutral
#define MIN_PULSE 100      // 1000µs min (full CW speed)
#define MAX_PULSE 260      // 2600µs max (full CCW speed)

void setThrottle(int pulseWidth) {
    printf("Setting pulse width to: %dµs\n", pulseWidth * 10);
    pwmWrite(ESC_PIN, pulseWidth);
    delay(STEP_DELAY);
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void calibrateESC() {
    printf("Starting ESC calibration...\n");
    printf("Please disconnect the battery from the ESC.\n");
    printf("Waiting 10 seconds for battery disconnection...\n");
    delay(10000);  // Wait for 10 seconds

    printf("Setting maximum pulse width...\n");
    setThrottle(MAX_PULSE);

    printf("Please connect the battery to the ESC now.\n");
    printf("Proceeding with calibration in 5 seconds...\n");
    delay(5000);  // Wait for 5 seconds

    printf("Battery assumed connected. Continuing calibration...\n");
    delay(2000);  // Wait for ESC to initialize

    printf("Setting minimum pulse width...\n");
    setThrottle(MIN_PULSE);
    delay(2000);

    printf("Setting neutral pulse width...\n");
    setThrottle(NEUTRAL_PULSE);
    delay(2000);

    printf("Calibration complete. The ESC is now ready for bidirectional operation.\n");
}

int main(void) {
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed\n");
        return 1;
    }

    pinMode(ESC_PIN, PWM_OUTPUT);
    pwmSetMode(ESC_PIN, PWM_MODE_MS);
    pwmSetClock(ESC_PIN, 192);
    pwmSetRange(ESC_PIN, 2000);

    int current_pulse = NEUTRAL_PULSE;
    
    calibrateESC();
    current_pulse = NEUTRAL_PULSE;

    printf("ESC Control Program\n");
    printf("W: Increase speed CW\n");
    printf("S: Increase speed CCW\n");
    printf("X: Stop (neutral)\n");
    printf("Q: Quit\n");

    while (1) {
        if (kbhit()) {
            char cmd = getchar();
            cmd = tolower(cmd);

            switch(cmd) {
                case 'w':  // Now CW
                    if (current_pulse > MIN_PULSE) {
                        current_pulse -= STEP_PULSE;
                        setThrottle(current_pulse);
                    } else {
                        printf("Already at maximum CW speed.\n");
                    }
                    break;
                case 's':  // Now CCW
                    if (current_pulse < MAX_PULSE) {
                        current_pulse += STEP_PULSE;
                        setThrottle(current_pulse);
                    } else {
                        printf("Already at maximum CCW speed.\n");
                    }
                    break;
                case 'x':
                    current_pulse = NEUTRAL_PULSE;
                    setThrottle(current_pulse);
                    printf("Stopped. Motor at neutral position.\n");
                    break;
                case 'q':
                    printf("Quitting. Setting throttle to neutral.\n");
                    setThrottle(NEUTRAL_PULSE);
                    return 0;
                default:
                    printf("Unknown command: %c\n", cmd);
            }

            printf("Current pulse width: %dµs\n", current_pulse * 10);
        }

        usleep(10000); // 10ms delay to reduce CPU usage
    }

    return 0;
}
