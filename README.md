# PIC18F45K22 Digital Thermometer (LM35 + ADC + Multiplexed 7-Segment Display)

This project implements a fully functional digital thermometer using the PIC18F45K22, the LM35 temperature sensor, and a multiplexed 7-segment display. The system measures temperature in real time, supports Celsius/Fahrenheit switching, displays and drives LO/MED/HI indicator LEDs based on programmable thresholds.
Features

## Accurate ADC Temperature Conversion

LM35 output amplified to match ADC range.

ADC configured with FVR = 1.024 V reference.

Left-justified 8-bit readout using the formula:

T(°C) = 102.4 × ADRESH / 256


## Interrupt-Driven System

High-frequency timer ISR handles multiplexed 7-segment display refresh.

RB0 external interrupt used for instantaneous Celsius ↔ Fahrenheit toggling.


## Three-Level Temperature Control Output

LO LED (RB5): < 20 °C

MED LED (RB6): 20–30 °C

HI LED (RB7): > 30 °C

Full Proteus + MPLAB Workflow


## Hardware Overview

Microcontroller: PIC18F45K22

Temperature Sensor: LM35

Display: 4-digit multiplexed common-anode 7-segment

RB0 pushbutton for unit toggle

LO/MED/HI LEDs on RB5–RB7
