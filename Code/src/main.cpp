//#include <Arduino.h>
#include "stepper.hpp"

//Maximum Size of a Serial Transmission
#define bSize 12

//Name of Device
#define controller_name "Radar Motor Controller V1.0"

//Pins attached
#define turntable_pin_enable 2
#define turntable_pin_step 3
#define turntable_pin_direction 4
#define turntable_pin_reference 8

#define turntable_speed_max 14
//#define turntable_steps_per_unit 1000
#define turntable_steps_per_unit 2000
#define turntable_home_offset 4.5

#define lineardrive_pin_enable 5
#define lineardrive_pin_step 6
#define lineardrive_pin_direction 7
#define lineardrive_pin_reference 9

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
  TCCR1B |= (1 << WGM12); //WGM12 setzen
  TIMSK1 |= (1 << OCIE1A); // Timer Compare Interrupt aktivieren
  
  //Standardgeschwindigkeit einstellen
  turntable.setSpeed(10);
  lineardrive.setSpeed(100);
  sei();


}

void serialEvent()
{
  byte data[bSize] = {};

  Serial.readBytesUntil(0xA, data, bSize);
  //First Element is t
  if ((data[0] == 0x54) || (data[0] == 0x74))
  {
    //Second Element is s
    if ((data[1] == 0x53) || (data[1] == 0x73))
    {
      //Third Element is =
      if (data[2] == 0x3D)
      {
        //TS=
        //Extract Value from the Command
        int i = 3;
        String valuestr = "";
        do
        {
          valuestr += char(data[i]);
          i++;
        } while (i < bSize - 1 || data[i] == 0);
        //Convert to Float
        float value = valuestr.toFloat();

        if (turntable.setSpeed(value))
          //Fehler beim Ausführen
          Serial.println("TS?");
        else
          //Befehl Erkannt
          Serial.println("TS=");
      }
      //Third Element is ?
      else if (data[2] == 0x3F)
      {
        //TS?
        Serial.print("TS=");
        Serial.println(turntable.getSpeed());
      }
      else
      {
        Serial.println("?");
      }
    }
    //Second Element is p
    else if ((data[1] == 0x50) || (data[1] == 0x70))
    {
      //Third Element is =
      if (data[2] == 0x3D)
      {
        //TP=
        //Extract Value from the Command
        int i = 3;
        String valuestr = "";
        do
        {
          valuestr += char(data[i]);
          i++;
        } while (i < bSize - 1 || data[i] == 0);
        //Convert to Float
        if (turntable.setSetPoint(valuestr.toFloat()))
          Serial.println("TP?");
        else
          //Befehl Erkannt
          Serial.println("TP=");
      }
      //Third Element is ?
      else if (data[2] == 0x3F)
      {
        //return the current position as a float
        Serial.print("TP=");
        Serial.println(turntable.getValue());
      }
      else
        Serial.println("?");
    }
    //Second Element is z
    else if ((data[1] == 0x5A) || (data[1] == 0x7A))
    {
      turntable.zero();
      Serial.println("TZ=");
    }
    //Second Element is h
    else if ((data[1] == 0x48) || (data[1] == 0x68))
    {
      turntable.home();
      Serial.println("TH=");
    }

    else
    {
      Serial.println("?");
    }
  }

  //LINEAR DRIVE
  //First Element is l
  else if ((data[0] == 0x4C) || (data[0] == 0x6C))
  {
    //Second Element is s
    if ((data[1] == 0x53) || (data[1] == 0x73))
    {
      //Third Element is =
      if (data[2] == 0x3D)
      {
        //LS=
        //Extract Value from the Command
        int i = 3;
        String valuestr = "";
        do
        {
          valuestr += char(data[i]);
          i++;
        } while (i < bSize - 1 || data[i] == 0);
        //Convert to Float
        float value = valuestr.toFloat();

        if (lineardrive.setSpeed(value))
          Serial.println("LS?");
        else
          //Befehl Erkannt
          Serial.println("LS=");
      }
      //Third Element is ?
      else if (data[2] == 0x3F)
      {
        //LS?
        Serial.print("LS=");
        Serial.println(lineardrive.getSpeed());
      }
      else
      {
        Serial.println("?");
      }
    }
    //Second Element is p
    else if ((data[1] == 0x50) || (data[1] == 0x70))
    {
      //Third Element is =
      if (data[2] == 0x3D)
      {
        //TP=
        //Extract Value from the Command
        int i = 3;
        String valuestr = "";
        do
        {
          valuestr += char(data[i]);
          i++;
        } while (i < bSize - 1 || data[i] == 0);
        //Convert to Float

        if (lineardrive.setSetPoint(valuestr.toFloat()))
          Serial.println("LP?");
        else
          //Befehl Erkannt
          Serial.println("LP=");
        ;
      }
      //Third Element is ?
      else if (data[2] == 0x3F)
      //return the current position as a float
      {
        Serial.print("LP=");
        Serial.println(lineardrive.getValue());
      }

      else
      {
        Serial.println("?");
      }
    }
    //Second Element is z
    else if ((data[1] == 0x5A) || (data[1] == 0x7A))
    {
      lineardrive.zero();
      Serial.println("LZ=");
    }
    //Second Element is h
    else if ((data[1] == 0x48) || (data[1] == 0x68))
    {
      lineardrive.home();
      Serial.println("LH=");
    }

    else
    {
      Serial.println("?");
    }
  }
}

void loop()
{

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
