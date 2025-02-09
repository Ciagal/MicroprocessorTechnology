# MicroprocessorTechnology

Weather Station Project on FRDM-KL05Z Board
The goal of this project was to create a weather station using the FRDM-KL05Z board with the MKL05Z4 microcontroller and the SHT35 sensor for measuring temperature and humidity. The project also includes displaying the results on an LCD1602 screen and implementing an alarm system that responds to values of temperature or humidity exceeding predefined thresholds.

Used Components

FRDM-KL05Z board with MKL05Z4 microcontroller
LCD1602 display
SHT35 sensor for measuring temperature and humidity
Matrix keypad for adjusting settings
Project Structure
The project consists of several source files, each serving a specific function:

main.c – The main file responsible for device initialization and interrupt handling. It contains functions for alarm signaling and the main loop of the program. In the main loop, sensor readings are periodically taken, alarms are checked, and results are displayed on the LCD screen.
sht35.c / sht35.h – Files containing functions to handle the SHT35 sensor, including reading and transmitting data.
lcd1602.c / lcd1602.h – A library for controlling the LCD1602 display.
leds.c / leds.h – A library for configuring LEDs, used in the alarm system.
i2c.c / i2c.h – Manages communication via the I²C bus.
klaw.c / klaw.h – Files for handling the matrix keypad, allowing user interaction.
Hardware Connections

LCD1602 display connections to the FRDM-KL05Z board:

SCL (PTB3) and SDA (PTB4) connect the board to the display.
Power: +5V (VCC) and GND (GND).
SHT35 sensor connections to the FRDM-KL05Z board:

SCL (PTB3) and SDA (PTB4) for I²C communication.
Power: +3V3 (VCC) and GND (GND).
Matrix keypad connections:

The keypad is connected to pins PTA7, PTA10, PTA11, PTA12 and GND.
System Operation
Once the device is powered on, the current temperature and humidity values are displayed on the screen. If the values are within acceptable ranges, they are shown without blinking, and no LEDs light up. If a threshold is exceeded (e.g., the temperature is too high or low), the displayed value starts blinking, and the LED signals an alarm:

Red LED will indicate a temperature alarm.
Blue LED will indicate a humidity alarm.
If both thresholds are exceeded (temperature and humidity), the system will first trigger a temperature alarm 5 times, then a humidity alarm 5 times.

Temperature and humidity can be adjusted by either touching the sensor with a finger or blowing on it. Additionally, the matrix keypad allows the user to change alarm thresholds:

Pressing S1 switches the system to configuration mode.
Using S4, the user selects the value they want to change (e.g., minimum temperature).
S2 and S3 buttons allow the user to decrease or increase the selected value.
Summary
The weather station project on the FRDM-KL05Z board enables monitoring of temperature and humidity, provides alarm signaling when thresholds are exceeded, and allows easy configuration of these thresholds using the matrix keypad. The LCD1602 display presents the results clearly, and the LEDs provide additional visual alarm notifications.
