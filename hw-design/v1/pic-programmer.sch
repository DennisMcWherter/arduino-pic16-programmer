EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:modules
LIBS:microchip_pic16mcu
LIBS:pic16f57
EELAYER 25 0
EELAYER END
$Descr User 5512 5512
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 1150 4750 0    60   ~ 0
PIC Programmer
Text Notes 4400 4900 0    60   ~ 0
1
$Comp
L PIC16F57-I/P U1
U 1 1 5A6E005D
P 1800 1950
F 0 "U1" H 1900 2900 50  0000 L CNN
F 1 "PIC16F57-I/P" H 1900 2800 50  0000 L CNN
F 2 "" H 1750 2000 50  0001 C CIN
F 3 "" H 1750 2000 50  0001 C CNN
	1    1800 1950
	1    0    0    -1  
$EndComp
$Comp
L Arduino_UNO_R3 A1
U 1 1 5A6E01A8
P 4350 2100
F 0 "A1" H 4150 3150 50  0000 R CNN
F 1 "Arduino_UNO_R3" H 4150 3050 50  0000 R CNN
F 2 "Modules:Arduino_UNO_R3" H 4500 1050 50  0001 L CNN
F 3 "" H 4150 3150 50  0001 C CNN
	1    4350 2100
	1    0    0    1   
$EndComp
$Comp
L GNDREF #PWR?
U 1 1 5A6E0814
P 700 1500
F 0 "#PWR?" H 700 1250 50  0001 C CNN
F 1 "GNDREF" H 700 1350 50  0000 C CNN
F 2 "" H 700 1500 50  0001 C CNN
F 3 "" H 700 1500 50  0001 C CNN
	1    700  1500
	1    0    0    -1  
$EndComp
$Comp
L GNDREF #PWR?
U 1 1 5A6E08C4
P 4350 900
F 0 "#PWR?" H 4350 650 50  0001 C CNN
F 1 "GNDREF" H 4350 750 50  0000 C CNN
F 2 "" H 4350 900 50  0001 C CNN
F 3 "" H 4350 900 50  0001 C CNN
	1    4350 900 
	1    0    0    1   
$EndComp
Wire Wire Line
	4350 1000 4350 900 
Wire Wire Line
	700  1500 900  1500
Text GLabel 4550 3400 3    60   Input ~ 0
5V
Wire Wire Line
	4550 3100 4550 3400
$Comp
L +12V #PWR?
U 1 1 5A6E0F59
P 3050 750
F 0 "#PWR?" H 3050 600 50  0001 C CNN
F 1 "+12V" H 3050 890 50  0000 C CNN
F 2 "" H 3050 750 50  0001 C CNN
F 3 "" H 3050 750 50  0001 C CNN
	1    3050 750 
	1    0    0    -1  
$EndComp
$Comp
L PN2222A Q1
U 1 1 5A6E0F75
P 3150 1450
F 0 "Q1" H 3350 1525 50  0000 L CNN
F 1 "PN2222A" H 3350 1450 50  0000 L CNN
F 2 "TO_SOT_Packages_THT:TO-92_Molded_Narrow" H 3350 1375 50  0001 L CIN
F 3 "" H 3150 1450 50  0001 L CNN
	1    3150 1450
	-1   0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5A6E0FD8
P 3050 1000
F 0 "R1" V 3130 1000 50  0000 C CNN
F 1 "1K" V 3050 1000 50  0000 C CNN
F 2 "" V 2980 1000 50  0001 C CNN
F 3 "" H 3050 1000 50  0001 C CNN
	1    3050 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 2500 3850 2500
Wire Wire Line
	3050 750  3050 850 
Wire Wire Line
	3050 1650 3050 1950
$Comp
L GNDREF #PWR?
U 1 1 5A6E1A5B
P 3050 1950
F 0 "#PWR?" H 3050 1700 50  0001 C CNN
F 1 "GNDREF" H 3050 1800 50  0000 C CNN
F 2 "" H 3050 1950 50  0001 C CNN
F 3 "" H 3050 1950 50  0001 C CNN
	1    3050 1950
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5A6E1B15
P 3500 1450
F 0 "R2" V 3580 1450 50  0000 C CNN
F 1 "4K" V 3500 1450 50  0000 C CNN
F 2 "" V 3430 1450 50  0001 C CNN
F 3 "" H 3500 1450 50  0001 C CNN
	1    3500 1450
	0    1    1    0   
$EndComp
Wire Wire Line
	2600 1250 3050 1250
Wire Wire Line
	3050 1250 3050 1150
Wire Wire Line
	3850 1400 3750 1400
Wire Wire Line
	3750 1400 3750 1450
Wire Wire Line
	3750 1450 3650 1450
Text GLabel 700  900  1    60   Input ~ 0
5V
Wire Wire Line
	700  900  700  1300
Wire Wire Line
	700  1200 900  1200
Wire Wire Line
	700  1300 900  1300
Connection ~ 700  1200
Wire Wire Line
	3850 2300 3050 2300
Wire Wire Line
	3050 2300 3050 2400
Wire Wire Line
	3050 2400 2600 2400
$EndSCHEMATC
