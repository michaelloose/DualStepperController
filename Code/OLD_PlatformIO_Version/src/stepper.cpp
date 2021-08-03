#include "stepper.hpp"

Stepper::Stepper(byte step, byte direction, byte enable, byte reference, int stepper_speed_max, float stepper_home_offset, double stepper_steps_per_unit, byte TCCRA_address, byte TCCRB_address, byte TCNT_address, byte OCRA_address, byte TIMSK_address)
{
    pin_step = step;
    pin_direction = direction;
    pin_enable = enable;
    pin_reference = reference;
    speed_max = stepper_speed_max;
    steps_per_unit = stepper_steps_per_unit;
    home_offset = stepper_home_offset;

    //speed_factor = (uint16_t)(ClockSpeed / (steps_per_unit * 64));

    TCCRA = TCCRA_address;
    TCCRB = TCCRB_address;
    TCNT = TCNT_address;
    OCRA = OCRA_address;
    TIMSK = TIMSK_address;

    pinMode(pin_enable, OUTPUT);
    pinMode(pin_step, OUTPUT);
    pinMode(pin_direction, OUTPUT);
    pinMode(pin_reference, INPUT);
    digitalWrite(pin_direction, LOW);
    

    //Interrupts deaktivieren
    cli();
    // Register mit 0 initialisieren

    //TCCR3A = 0; //Register für PWM
    _SFR_MEM8(TCCRA) = 0;
    //TCCR3B = 0; //Register für Prescaler
    _SFR_MEM8(TCCRB) = 0;
    //TCNT3 = 0;
    _SFR_MEM16(TCNT) = 0; //Counter Register
    _SFR_MEM8(TIMSK) = 0;
    
    _SFR_MEM8(TCCRB) |= (1 << 4); //WGM13 setzen. Aus Irgend einem Grund Behebt das das Problem, dass nur das Low Byte vom OCRA Register beschrieben wird

    //_SFR_MEM16(OCRA) = speed_factor / 10; // Output Compare Register vorbelegen (Standardverfahrgeschwindigkeit) //Nicht mehr erforderlich, läuft über die Hauptroutine

    _SFR_MEM8(TIMSK) |= (1 << OCIE1A); // Timer Compare Interrupt aktivieren
    //Interrupts aktivieren
    sei();
    //Antrieb aktivieren
    digitalWrite(pin_enable, LOW);
}

boolean Stepper::checkPos()
{

    //In welchem Zustand befindet sich der Referenzierungsvorgang?
    switch (referencingState)
    {
    case 1:

        if (digitalRead(pin_reference))
        {
            stopMotion();
            referencingState = 0;

            position = home_offset * steps_per_unit;
            setpoint = 0;
            
            //Wenn Offset vorhanden: Vorgang noch nicht abgeschlossen!
            if(position != setpoint)
                return 1;

        }
        else
        {
            startMotion(false);
            return 1;
        }
        break;

    case 2:
        //Befand sich der antrieb bei aufgerufener Referenzierung im Fangbereich des Endschalters?
        //Ist der Endschalter noch aktiv?
        if (digitalRead(pin_reference))
        {
            startMotion(true);
            return 1;            
        }
        else
        {
            stopMotion();
            referencingState = 1;
            return 1;
        }
        break;

    default:
        //Überprüfe ob Istwert = Sollwert

        //moving = position != setpoint;
        //direction = position < setpoint;
        if(position != setpoint){            
                startMotion(position < setpoint);
                return 1;
        }
        else{
                stopMotion();
        }
        break;
    }

//
//    if (moving)
//        return 1;
    return 0;
}
void Stepper::home()
{
    hasPreviouslyMoved = true;
    hasPreviouslyHomed = true;

    if (digitalRead(pin_reference))
    {
        //Status2: Antrieb befindet sich im Fangbereich des Endschalters
        referencingState = 2;
    }

    else{
        //Status 1: Antrieb befindet sich nicht im Fangbereich des Endschalters
        referencingState = 1;
    }
}

void Stepper::zero()
{
    position = 0;
    setpoint = 0;
}

void Stepper::doStep()
{
    if(position == setpoint){
        moving = true;
        stopMotion();
    }
    digitalWrite(pin_step, HIGH);
    digitalWrite(pin_step, LOW);
    position += direction ? 1 : -1;
}

void Stepper::startMotion(bool _direction)
{
    if(!moving){
        direction = _direction;
        //Signal für die Bewegungsrichtung setzen
        digitalWrite(pin_direction, _direction);
        //Set Prescaler to previous Value
        setPrescaler(prescalerState); 
        moving = true;
    }

}

void Stepper::stopMotion()
{
    if(moving){
        //Disable Timer
        setPrescaler(0);
        moving = false;  
    }
}

float Stepper::getValue()
{
    return position / steps_per_unit;
}

int Stepper::setSetPoint(double value)
{
    hasPreviouslyMoved = true;
    setpoint = value * steps_per_unit;

    //moving = position != setpoint;
    direction = position < setpoint;
    digitalWrite(pin_direction, direction);
    return 0;
}



int Stepper::setSpeed(double value)
{
    //Check if entered Value is Valid
    if (value > 0 && value <= speed_max)
    {
        int prescalerFactor = 0;


        if ((value * steps_per_unit) > 245)
        {
            //No Prescaler
            prescalerState = 1;
            prescalerFactor = 1;
        }
        else if ((value * steps_per_unit) > 31)
        {
            //Prescaler=8
            prescalerState = 2;
            prescalerFactor = 8;
        }
        //Die Fälle ab hier sind eigentlich uninteressant, da sich das Teil eh nie so langsam bewegen wird, aber der Vollständigkeit halber nehm ichs mal auf
        else if ((value * steps_per_unit) > 4)
        {
            //Prescaler=64
            prescalerState = 3;
            prescalerFactor = 64;
        }
        else if ((value * steps_per_unit) > 1)
        {
            //Prescaler=256
            prescalerState = 4;
            prescalerFactor = 256;
        }
        else
        {
            //Prescaler=1024
            prescalerState = 5;
            prescalerFactor = 1024;
        }

        speed_factor = ClockSpeed / (steps_per_unit * prescalerFactor);
                                              //Subtracting the Length of one Output Pulse (~2.9us per Step) in TimerRegister Units: Used to compensate the Length of the Pulse, so the Speed is accurate
        uint16_t ocr = (uint16_t)((speed_factor / value)-(2.9e-6 * (ClockSpeed / (double)prescalerFactor)));
        //_SFR_MEM16(OCRA) = (uint16_t)ocr;
        //Disable Interrupts
        cli();
        _SFR_MEM8(OCRA+1) = (uint8_t)(ocr>>8);
        _SFR_MEM8(OCRA) = (uint8_t)ocr;
        //Enable Interrupts
        sei();
        /*
        //Debugging Ausgabe

        Serial.println("ocr: ");
        Serial.println((uint8_t)(ocr>>8), HEX);
        Serial.println((uint8_t)ocr, HEX);
        

        //_SFR_MEM8(TCCRB) = 4;
        Serial.print("SPF: ");
        Serial.println(speed_factor);

        //int ocr_readback = _SFR_MEM8(OCRA) | (_SFR_MEM8(OCRA+1)<<8);
        //int ocr_readback = _SFR_MEM16(OCRA);

        Serial.print("OCR: ");
        Serial.println(_SFR_MEM16(OCRA), HEX);
        Serial.println(_SFR_MEM8(OCRA), HEX);
        Serial.println(_SFR_MEM8(OCRA+1), HEX);

        Serial.print("Prescaler: ");
        Serial.println(_SFR_MEM8(TCCRB), BIN);
        */


        return 0;
    }
    else
    {
        return 1;
    }
}
float Stepper::getSpeed()
{
    /*
    Serial.print("SPF: ");
    Serial.println(speed_factor);

    Serial.print("OCR: ");
    Serial.println(_SFR_MEM16(OCRA));

    Serial.print("Prescaler: ");
    Serial.println(_SFR_MEM8(TCCRB));
    
    Serial.print("TCCRA: ");
    Serial.println(_SFR_MEM8(TCCRA), BIN);
    Serial.print("TCCRB: ");
    Serial.println(_SFR_MEM8(TCCRB), BIN);
    Serial.print("TIMSK: ");
    Serial.println(_SFR_MEM8(TIMSK), BIN);
    Serial.print("TIFR1: ");
    Serial.println(_SFR_MEM8(TIFR1), BIN);
    Serial.print("TIFR3: ");
    Serial.println(_SFR_MEM8(TIFR3), BIN);
    */

    return speed_factor / _SFR_MEM16(OCRA);
}

void Stepper::setPrescaler(byte state){
  
     //Disable Interrupts
        cli();
        switch(state){
            case 1:
                //No Prescaler
                //CS10
                _SFR_MEM8(TCCRB) |= (1 << 0);//1
                //CS11
                _SFR_MEM8(TCCRB) &=~(1 << 1);//0
                //CS12
                _SFR_MEM8(TCCRB) &=~(1 << 2);//0
                break;
            case 2:
                //Prescaler=8
                //CS10
                _SFR_MEM8(TCCRB) &=~(1 << 0);
                //CS11
                _SFR_MEM8(TCCRB) |= (1 << 1);
                //CS12
                _SFR_MEM8(TCCRB) &=~(1 << 2);
                break;
            case 3:
                //Prescaler=64
                //CS10
                _SFR_MEM8(TCCRB) |= (1 << 0);
                //CS11
                _SFR_MEM8(TCCRB) |= (1 << 1);
                //CS12
                _SFR_MEM8(TCCRB) &=~(1 << 2);
                break;
            case 4:
                //Prescaler=256
                //CS10
                _SFR_MEM8(TCCRB) &=~(1 << 0);
                //CS11
                _SFR_MEM8(TCCRB) &=~(1 << 1);
                //CS12
                _SFR_MEM8(TCCRB) |= (1 << 2);
                break;
            case 5:
                //Prescaler=1024
                //CS10
                _SFR_MEM8(TCCRB) |= (1 << 0);
                //CS11
                _SFR_MEM8(TCCRB) &=~(1 << 1);
                //CS12
                _SFR_MEM8(TCCRB) |= (1 << 2);
                break;
            default:
                //Disable Timer, dont dave prescaler State
                //CS10
                _SFR_MEM8(TCCRB) &=~(1 << 0);//0
                //CS11
                _SFR_MEM8(TCCRB) &=~(1 << 1);//0
                //CS12
                _SFR_MEM8(TCCRB) &=~(1 << 2);//0
                break;      
        }
        //Enable Interrupts
        sei();
}