/*
 * Stepper.h
 *
 * Created: 21.06.2021 12:04:21
 *  Author: Waxrek
 */ 


#define PULSE_LEN 2.75e-6 
#define STEPPER_A_SETTINGS_START_ADDR 0x00
#define STEPPER_B_SETTINGS_START_ADDR 0x20
typedef struct 
{
		double steps_per_unit;				//spu
		double speed_default;				//vdf
		uint8_t speed_max_enabled;			//vlm
	    double speed_max;					//vmx
		
		uint8_t position_limit_enabled;		//plm
		double position_max;				//pmx
		double position_min;				//pmn
	    double reference_offset;			//rof
		uint8_t invert_direction;			//din
		uint8_t polarity_reference_switch;	//rpl

	
	}StepperSettings_t;