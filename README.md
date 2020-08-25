# Emcoturn_toolchanger
Toolchanger PCB and firmware for Emcoturn 120/220 lathe

This is a PCB to control the 8-position tool turrent for the EmcoTurn 120 and 220 lathe. Currently it uses modbus over RS485 communication.

Specifications:
* Teensy 3.2 for the control logic
* 24VDC power supply. Internal 5V 2A switching regulator
* TI DRV8876 to control the maxon DC motor in the turret
* Locked position detected by stall current of motor
* 4 additional IO's on the board for free to program functions
