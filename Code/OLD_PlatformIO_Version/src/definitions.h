//Name of Device
#define ClockSpeed 16e6
#define controller_name "Radar Motor Controller V1.0"

//Maximum Size of a Serial Transmission
#define bSize 12

//Pins attached
#define turntable_pin_enable 8
#define turntable_pin_direction 9
#define turntable_pin_step 10
#define turntable_pin_reference 7

#define turntable_speed_max 14
//#define turntable_steps_per_unit 1000
#define turntable_steps_per_unit 2000
#define turntable_home_offset 4.5

#define lineardrive_pin_enable 11
#define lineardrive_pin_direction 12
#define lineardrive_pin_step 13
#define lineardrive_pin_reference 6

#define lineardrive_speed_max 330
#define lineardrive_steps_per_unit 111.11
#define lineardrive_home_offset 0

//Wie viele Mikroschritte einer Einheit ensprechen. Beispiel:
//

/*
TCCR0A
TCCR0B
TCNT0
OCR0A
TIMSK0
_SFR_MEM8(0x6E)

_SFR_IO8(0x24)
__SFR_OFFSET

//Atmega 32U4: Timer1 und Timer3 Benutzen. 16 Bit-Timer
//http://medesign.seas.upenn.edu/index.php/Guides/MaEvArM-timers


*/

//Hier wird ein 16 Bit Timer nur auf 8 Bit Benutzt, das kann man gennauer machen.
#define timer1_TCCRA 0x80
#define timer1_TCCRB 0x81
#define timer1_TCNT 0x84
#define timer1_OCRA 0x88
#define timer1_TIMSK 0x6F

#define timer3_TCCRA 0x90
#define timer3_TCCRB 0x91
#define timer3_TCNT 0x94
#define timer3_OCRA 0x98
#define timer3_TIMSK 0x71

/*
TCCR2A
TCCR2B
TCNT2
OCR2A
TIMSK2
*/
/*
#define timer2_TCCRA 0xB0
#define timer2_TCCRB 0xB1
#define timer2_TCNT 0xB2
#define timer2_OCRA 0xB3
#define timer2_TIMSK 0x70
*/