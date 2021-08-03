#ifndef _STEPPERA_H_
#define _STEPPERA_H_

#define STEPPER_A_STEP_N 6
#define STEPPER_A_STEP_PORT PORTB
#define STEPPER_A_STEP_DDR DDRB

#define STEPPER_A_DIRECTION_N 5
#define STEPPER_A_DIRECTION_PORT PORTB
#define STEPPER_A_DIRECTION_DDR DDRB

#define STEPPER_A_ENABLE_N 4
#define STEPPER_A_ENABLE_PORT PORTB
#define STEPPER_A_ENABLE_DDR DDRB

#define STEPPER_A_REFERENCE_N 6
#define STEPPER_A_REFERENCE_PIN PINE
#define STEPPER_A_REFERENCE_PORT PORTE
#define STEPPER_A_REFERENCE_DDR DDRE

#define STEPPER_A_TIMER_TCCRA TCCR1A
#define STEPPER_A_TIMER_TCCRB TCCR1B
#define STEPPER_A_TIMER_TCNT TCNT1
#define STEPPER_A_TIMER_OCRA OCR1A
#define STEPPER_A_TIMER_TIMSK TIMSK1
#define STEPPER_A_TIMER_COMPA_vect TIMER1_COMPA_vect



#define PULSE_LEN 2.9e-6 

double stepper_a_speed_factor;

int8_t stepper_a_moving;
int8_t stepper_a_direction;
int8_t stepper_a_hasPreviouslyMoved;
int8_t stepper_a_hasPreviouslyHomed;
int8_t stepper_a_referencing;
int8_t stepper_a_prescalerState;
int16_t stepper_a_prescalerFactor;

long stepper_a_setpoint;
long stepper_a_position;



void stepper_a_home(void);
void stepper_a_zero(void);
void stepper_a_halt(void);

uint8_t stepper_a_setSetPoint(double);
double stepper_a_getPosition(void);

uint8_t stepper_a_setSpeed(double);
double stepper_a_getSpeed(void);

void stepper_a_setMotionState(int8_t);
void stepper_a_setPrescaler(int8_t);

void stepper_a_init(void);
void stepper_a_loop(USB_ClassInfo_CDC_Device_t*);

StepperSettings_t stepper_a_settings;

#endif