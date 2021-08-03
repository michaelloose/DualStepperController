/*
* StepperA.c
*
* Created: 05.07.2021 11:06:45
*  Author: Waxrek
*/

#include "headers.h"



void stepper_a_init(){
	
	
	eeprom_read_block(&stepper_a_settings, (void*)STEPPER_A_SETTINGS_START_ADDR, sizeof(stepper_a_settings));
	stepper_a_moving = 0;
	stepper_a_direction = 0;
	stepper_a_hasPreviouslyMoved = 0;
	stepper_a_hasPreviouslyHomed = 0;
	stepper_a_referencing = 0;
	stepper_a_prescalerState = 0;

	stepper_a_setpoint = 0;
	stepper_a_position = 0;

	
	
	//Ausgänge definieren
	STEPPER_A_STEP_DDR |= (1 << STEPPER_A_STEP_N);
	STEPPER_A_DIRECTION_DDR |= (1 << STEPPER_A_DIRECTION_N);
	STEPPER_A_ENABLE_DDR |= (1 << STEPPER_A_ENABLE_N);
	//Eingang definieren
	STEPPER_A_REFERENCE_DDR &= ~(1 << STEPPER_A_REFERENCE_N);
	
	//Enable auf HIGH setzen
	STEPPER_A_ENABLE_PORT &= ~(1<<STEPPER_A_ENABLE_N);
	
	//Register fuer Timer setzen
	//Interrupts deaktivieren
	cli();
	// Register mit 0 initialisieren

	STEPPER_A_TIMER_TCCRA = 0b00000000; //Register für PWM
	STEPPER_A_TIMER_TCCRB = 0b00011000;//Register für Prescaler: //WGM12 setzen, WGM13 setzen. Aus Irgend einem Grund Behebt das Setzen von WGM13 das Problem, dass nur das Low Byte vom OCRA Register beschrieben wird
	STEPPER_A_TIMER_TCNT = 0b00000000; //Counter Register
	STEPPER_A_TIMER_TIMSK = 0b00000010;

	sei();
	stepper_a_setSpeed(stepper_a_settings.speed_default);
}

void stepper_a_loop(USB_ClassInfo_CDC_Device_t* pInterface){
// 	if (stepper_a_settings.polarity_reference_switch != ((STEPPER_A_REFERENCE_PIN & (1 << STEPPER_A_REFERENCE_N)) == (1 << STEPPER_A_REFERENCE_N)))
// 	PORTD &= ~(1<<5);    //Bit setzen, LED AN
// 
// 	else
// 	PORTD |= (1<<5);    //Bit setzen, LED AUS
	
	//In welchem Zustand befindet sich der Referenzierungsvorgang?
	switch (stepper_a_referencing)
	{
		case 1:
		//Antrieb bei aufgerufener Referenzierung ausserhalb des Fangbereiches des Endschalters
		
		//Überprüfe ob Istwert = Sollwert
		if(stepper_a_position != stepper_a_setpoint){
			stepper_a_setMotionState((stepper_a_position < stepper_a_setpoint)? 1: -1);
		}
		else{
			stepper_a_setMotionState(0);
			stepper_a_referencing = 0;
		}


		break;
		
		case 2:
		//Antrieb bei aufgerufener Referenzierung ausserhalb des Fangbereiches des Endschalters
		if (stepper_a_settings.polarity_reference_switch != ((STEPPER_A_REFERENCE_PIN & (1 << STEPPER_A_REFERENCE_N)) == (1 << STEPPER_A_REFERENCE_N)))
		{
			
			stepper_a_referencing = 1;

			stepper_a_position = stepper_a_settings.reference_offset * stepper_a_settings.steps_per_unit;
			stepper_a_setpoint = 0;
			
		}
		else
		{
			stepper_a_setMotionState(-1);
		}
		break;

		case 3:
		//Befand sich der antrieb bei aufgerufener Referenzierung im Fangbereich des Endschalters?
		//Ist der Endschalter noch aktiv?
		if (stepper_a_settings.polarity_reference_switch != ((STEPPER_A_REFERENCE_PIN & (1 << STEPPER_A_REFERENCE_N)) == (1 << STEPPER_A_REFERENCE_N)))
		{
			stepper_a_setMotionState(1);
		}
		else
		{
			stepper_a_setMotionState(0);
			stepper_a_referencing = 2;

		}
		break;

		default:
		//Überprüfe ob Istwert = Sollwert
		if(stepper_a_position != stepper_a_setpoint){
			stepper_a_setMotionState((stepper_a_position < stepper_a_setpoint)? 1: -1);
		}
		else{
			stepper_a_setMotionState(0);
		}
		break;
	}

	if ((stepper_a_moving == 0) && (stepper_a_hasPreviouslyMoved == 1))
	{
		

		if (stepper_a_hasPreviouslyHomed && !stepper_a_referencing)
		{
			stepper_a_hasPreviouslyMoved = 0;
			stepper_a_hasPreviouslyHomed = 0;
			CDC_Device_SendString(pInterface, "AH!\r");
		}
		else if(!stepper_a_hasPreviouslyHomed)
		{
			stepper_a_hasPreviouslyMoved = 0;
			CDC_Device_SendString(pInterface, "AP!\r");
		}
	}
	
}

void stepper_a_home(){
	stepper_a_hasPreviouslyMoved = 1;
	stepper_a_hasPreviouslyHomed = 1;

	if (stepper_a_settings.polarity_reference_switch != ((STEPPER_A_REFERENCE_PIN & (1 << STEPPER_A_REFERENCE_N)) == (1 << STEPPER_A_REFERENCE_N)))
	{
		//Status2: Antrieb befindet sich im Fangbereich des Endschalters
		stepper_a_referencing = 3;
		
		
		
	}

	else{
		//Status 1: Antrieb befindet sich nicht im Fangbereich des Endschalters
		stepper_a_referencing = 2;
		
	}
}
void stepper_a_zero(){
	stepper_a_setpoint = 0;
	stepper_a_position = 0;
}

void stepper_a_halt(){
	stepper_a_referencing = 0;
	stepper_a_setpoint = stepper_a_position;
}

uint8_t stepper_a_setSetPoint(double value){
	//Prüfen ob Wert in erlaubtem Bereich falls Positionsbegrenzung aktiviert
	if(stepper_a_settings.position_limit_enabled && ((value > stepper_a_settings.position_max)||(value < stepper_a_settings.position_min)))
	return 1;
	else
	{
		stepper_a_hasPreviouslyMoved = 1;
		stepper_a_setpoint = value * stepper_a_settings.steps_per_unit;
	}
	
	return 0;
}

double stepper_a_getPosition(){
	return (double)stepper_a_position / stepper_a_settings.steps_per_unit;
}


uint8_t stepper_a_setSpeed(double value){
	//Befehl nur akzeptieren wenn Wer
	if (value > 0 && ((value <= stepper_a_settings.speed_max)||stepper_a_settings.speed_max_enabled))
	{

		if ((value * stepper_a_settings.steps_per_unit) > 245)
		{
			//No Prescaler
			stepper_a_prescalerState = 1;
			stepper_a_prescalerFactor = 1;
		}
		else if ((value * stepper_a_settings.steps_per_unit) > 31)
		{
			//Prescaler=8
			stepper_a_prescalerState = 2;
			stepper_a_prescalerFactor = 8;
		}
		//Die Fälle ab hier sind eigentlich uninteressant, da sich das Teil eh nie so langsam bewegen wird, aber der Vollständigkeit halber nehm ichs mal auf
		else if ((value * stepper_a_settings.steps_per_unit) > 4)
		{
			//Prescaler=64
			stepper_a_prescalerState = 3;
			stepper_a_prescalerFactor = 64;
		}
		else if ((value * stepper_a_settings.steps_per_unit) > 1)
		{
			//Prescaler=256
			stepper_a_prescalerState = 4;
			stepper_a_prescalerFactor = 256;
		}
		else
		{
			//Prescaler=1024
			stepper_a_prescalerState = 5;
			stepper_a_prescalerFactor = 1024;
		}

		stepper_a_speed_factor = (double)F_CPU / ((double)stepper_a_settings.steps_per_unit * (double)stepper_a_prescalerFactor);
		//Subtracting the Length of one Output Pulse (~2.9us per Step) in TimerRegister Units: Used to compensate the Length of the Pulse, so the Speed is accurate
		
		//uint16_t ocr = (uint16_t)((stepper_a_speed_factor / value)-(PULSE_LEN * ((double)F_CPU / (double)stepper_a_prescalerFactor)));
		uint16_t ocr = (uint16_t)(stepper_a_speed_factor / value);

		//_SFR_MEM16(OCRA) = (uint16_t)ocr;
		//Disable Interrupts
		cli();
		//_SFR_MEM8(OCRA+1) = (uint8_t)(ocr>>8);
		//_SFR_MEM8(OCRA) = (uint8_t)ocr;
		STEPPER_A_TIMER_TCNT = 0;
		STEPPER_A_TIMER_OCRA = ocr;
		//Enable Interrupts
		sei();

		return 0;
	}
	else
	{
		return 1;
	}
	
}
double stepper_a_getSpeed(){
	return (stepper_a_speed_factor / (double)STEPPER_A_TIMER_OCRA);

	//return (stepper_a_speed_factor / (double)STEPPER_A_TIMER_OCRA) - ( (double)stepper_a_prescalerFactor / (PULSE_LEN*(double)F_CPU));
}

void stepper_a_setMotionState(int8_t state){
	if(state < 0 && (!stepper_a_moving || stepper_a_direction)){
		stepper_a_direction = 0;
		
		if(stepper_a_settings.invert_direction)
		STEPPER_A_DIRECTION_PORT |= (1<<STEPPER_A_DIRECTION_N);    //Direction Pin High
		else
		STEPPER_A_DIRECTION_PORT &= ~(1<<STEPPER_A_DIRECTION_N);    //Direction Pin Low
		
		stepper_a_setPrescaler(stepper_a_prescalerState);
		stepper_a_moving = 1;

		
	}
	else if(state > 0 && (!stepper_a_moving || !stepper_a_direction)){
		stepper_a_direction = 1;
		if(stepper_a_settings.invert_direction)
		STEPPER_A_DIRECTION_PORT &= ~(1<<STEPPER_A_DIRECTION_N);    //Direction Pin Low
		else
		STEPPER_A_DIRECTION_PORT |= (1<<STEPPER_A_DIRECTION_N);    //Direction Pin High
		stepper_a_setPrescaler(stepper_a_prescalerState);
		stepper_a_moving = 1;

	}
	else if(state == 0 && stepper_a_moving){
		stepper_a_setPrescaler(0);
		stepper_a_moving = 0;
	}
}


void stepper_a_setPrescaler(int8_t state){
	
	//Disable Interrupts
	cli();
	switch(state){
		case 1:
		//No Prescaler
		//CS10
		STEPPER_A_TIMER_TCCRB |= (1 << 0);//1
		//CS11
		STEPPER_A_TIMER_TCCRB &=~(1 << 1);//0
		//CS12
		STEPPER_A_TIMER_TCCRB &=~(1 << 2);//0
		break;
		case 2:
		//Prescaler=8
		//CS10
		STEPPER_A_TIMER_TCCRB &=~(1 << 0);
		//CS11
		STEPPER_A_TIMER_TCCRB |= (1 << 1);
		//CS12
		STEPPER_A_TIMER_TCCRB &=~(1 << 2);
		break;
		case 3:
		//Prescaler=64
		//CS10
		STEPPER_A_TIMER_TCCRB |= (1 << 0);
		//CS11
		STEPPER_A_TIMER_TCCRB |= (1 << 1);
		//CS12
		STEPPER_A_TIMER_TCCRB &=~(1 << 2);
		break;
		case 4:
		//Prescaler=256
		//CS10
		STEPPER_A_TIMER_TCCRB &=~(1 << 0);
		//CS11
		STEPPER_A_TIMER_TCCRB &=~(1 << 1);
		//CS12
		STEPPER_A_TIMER_TCCRB |= (1 << 2);
		break;
		case 5:
		//Prescaler=1024
		//CS10
		STEPPER_A_TIMER_TCCRB |= (1 << 0);
		//CS11
		STEPPER_A_TIMER_TCCRB &=~(1 << 1);
		//CS12
		STEPPER_A_TIMER_TCCRB |= (1 << 2);
		break;
		default:
		//Disable Timer, dont dave prescaler State
		//CS10
		STEPPER_A_TIMER_TCCRB &=~(1 << 0);//0
		//CS11
		STEPPER_A_TIMER_TCCRB &=~(1 << 1);//0
		//CS12
		STEPPER_A_TIMER_TCCRB &=~(1 << 2);//0
		break;
	}
	//Enable Interrupts
	sei();
	
}



ISR(STEPPER_A_TIMER_COMPA_vect){
	//PORTB ^= (1<<0);    //Bit setzen, LED TOGGLE
	//PORTD ^= (1<<5);    //Bit loeschen, LED TOGGLE
	
	STEPPER_A_TIMER_TCNT = 0;
	STEPPER_A_STEP_PORT |= (1<<STEPPER_A_STEP_N);    //Step Pin High
	
	if(stepper_a_position == stepper_a_setpoint){
		//stepper_a_moving = 1;
		stepper_a_setMotionState(0);
	}
	stepper_a_position += stepper_a_direction ? 1 : -1;
	STEPPER_A_STEP_PORT &= ~(1<<STEPPER_A_STEP_N);    //Step Pin Low

	
}