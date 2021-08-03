#include "StepperController.hpp"

void serialEvent()
{
  uint8_t data[bSize] = {};

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