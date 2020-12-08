#include <Arduino.h>

#define ClockSpeed 16e6

class Stepper{
public:
  Stepper(byte, byte, byte, byte, int, float, double,  byte,  byte,  byte,  byte,  byte);
  void doStep(void);
  boolean checkPos(void);
  void home(void);
  void zero(void);

  int setSetPoint(double);
  float getValue(void);

  int setSpeed(double);
  float getSpeed(void);

  boolean referencing = 0;
  boolean moving = 0;
  boolean direction = 0;
  boolean hasPreviouslyMoved = 0;
  boolean hasPreviouslyHomed = 0;


private:

  byte TCCRA;
  byte TCCRB;
  byte TCNT;
  byte OCRA;
  byte TIMSK;

  byte pin_step;
  byte pin_direction;
  byte pin_enable;
  byte pin_reference;

  long setpoint = 0;
  long position = 0;

  void startMotion(boolean);
  void stopMotion(void);

  void setPrescaler(byte);
  byte prescalerState = 0;
  byte referencingState = 0;


  int speed_max;
  double steps_per_unit;
  double speed_factor;
  float home_offset;

};