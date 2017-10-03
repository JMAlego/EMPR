# EMPR - Embedded Systems Project

***Computer Science BEng/MEng Stage 2 EMPR (Embedded Systems Project)***

*Team Members: Jacob Allen & Kia Alderson (Currently)*

## Contents

- [1. Mini Projects](#1-mini-projects)
  - [1.1. Mini Project 1](#11-mini-project-1)
  - [1.2. Mini Project 2](#12-mini-project-2)
  - [1.3. Mini Project 3](#13-mini-project-3)
- [2. Main Project](#2-main-project)

## 1. Mini Projects

### 1.1. Mini Project 1

LED and Serial communication project. Must fulfil the following:

 - Print:  `"Starting count"` on the terminal screen.
 - Display the 4-bit (0-15) number on the LEDs with each number displayed for about 1 second under the control of timer-based interrupts.
 - Simultaneously display the 0-15 count on the terminal screen in decimal, hexadecimal and binary.
 - Print:  `"Finished count"` on the terminal screen.

### 1.2. Mini Project 2

I2C, LCD and Keypad project. Must fulfil the following:

 - Print: the number I2C devices connected, followed by a pause of 1 second.
 - Display `"Hello World"` then `"Hello [newline] world"` on the LCD with a pause of 1 second and a clear LCD screen after each of the two print operations.
 - Display characters on the LCD as they are typed on the keypad.
 - Optional extra: Mini calculator.

### 1.3. Mini Project 3

ADC, DAC and PWM project. Must fulfil the following:

 - Read and print out an analogue voltage via the ADC (Max v3.3).
 - Generate a sine wave using the DAC, the amplitude and frequency should be varied over time (every 10 seconds).
 - Mirror an input sine wave using the ADC to read and the DAC to write.
 - Output a waveform on the PWM, using single edge mode and cycling through the full range of changes in pulse width 5 times. A single cycle from minimum pulse width to maximum should take 5 seconds and should be displayed on the oscillscope. It should use some form of timer on the MCU to change the pulse width at regular intervals, not a delay loop.
 - Optionally: Build a distance measuring device, using a Sharp GP2Y0A21YK0F infra-red range sensor. Printing the sensors output voltage and/or measured distance to the LCD screen.

## 2. Main Project

*Currently unknown.*
