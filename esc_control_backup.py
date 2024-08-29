import wiringpi
import time
import sys
import tty
import termios

# Constants for ESC control
ESC_PIN = 21  # wPi 21, Physical pin 32, GPIO 62
STEP_PULSE = 1  # 10µs step for manual adjustment
STEP_DELAY = 0.02  # 20ms delay between each step

# Pulse width constants
NEUTRAL_PULSE = 190  # 1900µs neutral
MIN_PULSE = 100      # 1000µs min (full CW speed)
MAX_PULSE = 260      # 2600µs max (full CCW speed)

def setup():
    wiringpi.wiringPiSetup()
    wiringpi.pinMode(ESC_PIN, wiringpi.PWM_OUTPUT)
    wiringpi.pwmSetMode(ESC_PIN, wiringpi.PWM_MODE_MS)
    wiringpi.pwmSetClock(ESC_PIN, 192)
    wiringpi.pwmSetRange(ESC_PIN, 2000)

def set_throttle(pulse_width):
    print(f"Setting pulse width to: {pulse_width * 10}µs")
    wiringpi.pwmWrite(ESC_PIN, pulse_width)
    time.sleep(STEP_DELAY)

def getch():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def calibrate_esc():
    print("Starting ESC calibration...")
    print("1. Disconnect the battery from the ESC and press any key to continue.")
    getch()
    
    print("Setting maximum pulse width...")
    set_throttle(MAX_PULSE)
    time.sleep(2)
    
    print("2. Connect the battery to the ESC. You should hear two beeps. Press any key to continue.")
    getch()
    
    print("Setting minimum pulse width...")
    set_throttle(MIN_PULSE)
    time.sleep(2)
    
    print("3. You should hear a confirming tone. Press any key to continue.")
    getch()
    
    print("Setting neutral pulse width...")
    set_throttle(NEUTRAL_PULSE)
    time.sleep(2)
    
    print("Calibration complete. The ESC is now ready for bidirectional operation.")

def main():
    setup()
    current_pulse = NEUTRAL_PULSE

    print("ESC Control Program")
    print("C: Calibrate ESC")
    print("W: Increase speed CCW")
    print("S: Decrease speed / Increase speed CW")
    print("X: Stop (neutral)")
    print("Q: Quit")

    while True:
        cmd = getch().lower()

        if cmd == 'c':
            calibrate_esc()
            current_pulse = NEUTRAL_PULSE
        elif cmd == 'w':
            if current_pulse < MAX_PULSE:
                current_pulse += STEP_PULSE
                set_throttle(current_pulse)
        elif cmd == 's':
            if current_pulse > MIN_PULSE:
                current_pulse -= STEP_PULSE
                set_throttle(current_pulse)
        elif cmd == 'x':
            current_pulse = NEUTRAL_PULSE
            set_throttle(current_pulse)
        elif cmd == 'q':
            print("Quitting. Setting throttle to neutral.")
            set_throttle(NEUTRAL_PULSE)
            break
        else:
            print(f"Unknown command: {cmd}")

        print(f"Current pulse width: {current_pulse * 10}µs")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nProgram interrupted. Setting throttle to neutral.")
        wiringpi.pwmWrite(ESC_PIN, NEUTRAL_PULSE)
    finally:
        wiringpi.pinMode(ESC_PIN, wiringpi.INPUT)  # Reset the pin to INPUT mode
