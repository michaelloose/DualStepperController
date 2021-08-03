#include "StepperController.hpp"

Stepper turntable(turntable_pin_step, turntable_pin_direction, turntable_pin_enable, turntable_pin_reference, turntable_speed_max, turntable_home_offset, turntable_steps_per_unit, timer1_TCCRA, timer1_TCCRB, timer1_TCNT, timer1_OCRA, timer1_TIMSK);
Stepper lineardrive(lineardrive_pin_step, lineardrive_pin_direction, lineardrive_pin_enable, lineardrive_pin_reference, lineardrive_speed_max, lineardrive_home_offset, lineardrive_steps_per_unit, timer3_TCCRA, timer3_TCCRB, timer3_TCNT, timer3_OCRA, timer3_TIMSK);

void setup()
{

  Serial.begin(115200);
  Serial.println(controller_name);

  //Alle Register vom Timer 1 Löschen. Das ist Notwendig, weil der Arduino Bootloader sich den Timer selbst anders konfiguriert und diese Änderungen rückgängig gemacht werden. Ohne den Bootloader entfallen diese Zeilen

  cli();
  TIFR1 = 0;
  TIFR3 = 0;

  //Register für PWM
  TCCR1A = 0;
  //Register für Prescaler
  TCCR1B = 0;
  //TCCR3B = 0;
  //Counter Register
  TCNT1 = 0;
  TIMSK1 = 0;
  //TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12);  //WGM12 setzen
  TIMSK1 |= (1 << OCIE1A); // Timer Compare Interrupt aktivieren

  //Standardgeschwindigkeit einstellen
  turntable.setSpeed(10);
  lineardrive.setSpeed(100);
  sei();
}
void loop()
{
  if (serialEventRun)
    serialEventRun();

  //Führt das Polling der beiden Antriebe aus und gibt nach Erfolgreich beendeter Bewegung Rückmeldung
  if ((turntable.checkPos() == false) && (turntable.hasPreviouslyMoved == true))
  {
    turntable.hasPreviouslyMoved = false;

    if (turntable.hasPreviouslyHomed)
    {
      turntable.hasPreviouslyHomed = false;
      Serial.println("TH!");
    }
    else
    {
      Serial.println("TP!");
    }
  }

  if ((lineardrive.checkPos() == false) && (lineardrive.hasPreviouslyMoved == true))
  {
    lineardrive.hasPreviouslyMoved = false;

    if (lineardrive.hasPreviouslyHomed)
    {
      lineardrive.hasPreviouslyHomed = false;
      Serial.println("LH!");
    }
    else
    {
      Serial.println("LP!");
    }
  }

  if (Serial.available() > 3)
    serialEvent();
}

ISR(TIMER1_COMPA_vect)
{
  //if (turntable.moving)
  //{
  TCNT1 = 0;
  turntable.doStep();
  //}
}

ISR(TIMER3_COMPA_vect)
{
  //if (lineardrive.moving)
  //{
  TCNT3 = 0;
  lineardrive.doStep();
  //}
}
