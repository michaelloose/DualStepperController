#include <Arduino.h>




#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "binary.h"


#include "definitions.h"
#include "stepper.hpp"



//#include "serial.hpp"
void serialEvent(void);

#ifndef SYMBOL
#define SYMBOL
extern Stepper turntable;
extern Stepper lineardrive;
  

//extern Stepper turntable(turntable_pin_step, turntable_pin_direction, turntable_pin_enable, turntable_pin_reference, turntable_speed_max, turntable_home_offset, turntable_steps_per_unit, timer1_TCCRA, timer1_TCCRB, timer1_TCNT, timer1_OCRA, timer1_TIMSK);
//extern Stepper lineardrive(lineardrive_pin_step, lineardrive_pin_direction, lineardrive_pin_enable, lineardrive_pin_reference, lineardrive_speed_max, lineardrive_home_offset, lineardrive_steps_per_unit, timer3_TCCRA, timer3_TCCRB, timer3_TCNT, timer3_OCRA, timer3_TIMSK);
//extern Stepper turntable(byte, byte, byte, byte, int, float, double, byte, byte, byte, byte, byte);
//extern Stepper lineardrive(byte, byte, byte, byte, int, float, double, byte, byte, byte, byte, byte);

#endif
