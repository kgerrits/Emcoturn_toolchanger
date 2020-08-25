# Emcoturn_toolchanger
Toolchanger PCB and firmware for Emcoturn 120/220 lathe

This is a PCB to control the 8-position tool turrent for the EmcoTurn 120 and 220 lathe. Currently it uses modbus over RS485 communication.

Specifications:
* Teensy 3.2 for the control logic
* 24VDC power supply. Internal 5V 2A switching regulator
* TI DRV8876 to control the maxon DC motor in the turret
* Locked position detected by stall current of motor
* 4 additional IO's on the board for free to program functions

![Emcoturn toolchanger picture](https://github.com/kgerrits/Emcoturn_toolchanger/blob/master/pitcures/213F2B1F-4403-42C3-B792-9A6939B8880E.jpg)


Testrun of the PCB:

[![Alt text](https://img.youtube.com/vi/sjd6qg67e3A/0.jpg)](https://www.youtube.com/watch?v=sjd6qg67e3A)
