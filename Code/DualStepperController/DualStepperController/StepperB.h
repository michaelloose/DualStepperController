#ifndef _STEPPERB_H_
#define _STEPPERB_H_

#define STEPPER_B_STEP_N 7
#define STEPPER_B_STEP_PORT PORTC
#define STEPPER_B_STEP_DDR DDRC

#define STEPPER_B_DIRECTION_N 6
#define STEPPER_B_DIRECTION_PORT PORTD
#define STEPPER_B_DIRECTION_DDR DDRD

#define STEPPER_B_ENABLE_N 7
#define STEPPER_B_ENABLE_PORT PORTB
#define STEPPER_B_ENABLE_DDR DDRB

#define STEPPER_B_REFERENCE_N 7
#define STEPPER_B_REFERENCE_PIN PIND
#define STEPPER_B_REFERENCE_PORT PORTD
#define STEPPER_B_REFERENCE_DDR DDRD

#define STEPPER_B_TIMER_TCCRA TCCR3A
#define STEPPER_B_TIMER_TCCRB TCCR3B
#define STEPPER_B_TIMER_TCNT TCNT3
#define STEPPER_B_TIMER_OCRA OCR3A
#define STEPPER_B_TIMER_TIMSK TIMSK3
#define STEPPER_B_TIMER_COMPA_vect TIMER3_COMPA_vect
#define PULSE_LEN 2.9e-6 

double stepper_b_speed_factor;

int8_t stepper_b_moving;
int8_t stepper_b_direction;
int8_t stepper_b_hasPreviouslyMoved;
int8_t stepper_b_hasPreviouslyHomed;
int8_t stepper_b_referencing;
int8_t stepper_b_prescalerState;
int16_t stepper_b_prescalerFactor;

long stepper_b_setpoint;
long stepper_b_position;



void stepper_b_home(void);
void stepper_b_zero(void);
void stepper_b_halt(void);

uint8_t stepper_b_setSetPoint(double);
double stepper_b_getPosition(void);

uint8_t stepper_b_setSpeed(double);
double stepper_b_getSpeed(void);

void stepper_b_setMotionState(int8_t);
void stepper_b_setPrescaler(int8_t);

void stepper_b_init(void);
void stepper_b_loop(USB_ClassInfo_CDC_Device_t*);

StepperSettings_t stepper_b_settings;

#endif