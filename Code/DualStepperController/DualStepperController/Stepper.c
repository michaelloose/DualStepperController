/*
 * Stepper.c
 *
 * Created: 05.07.2021 10:47:16
 *  Author: Waxrek
 */ 

#include "headers.h"


void driveHalt(){
	stepper_a_halt();
	stepper_b_halt();
}
void emergencyStop(void){
	driveState = DRV_STATE_ESTOP;
	//Enable auf LOW setzen
	STEPPER_A_ENABLE_PORT |= (1<<STEPPER_A_ENABLE_N);
	STEPPER_B_ENABLE_PORT |= (1<<STEPPER_B_ENABLE_N);
	driveHalt();
}
void emergencyStopRelease(void){
	driveState = DRV_STATE_RDY;
	//Enable auf HIGH setzen
	STEPPER_A_ENABLE_PORT &= ~(1<<STEPPER_A_ENABLE_N);
	STEPPER_B_ENABLE_PORT &= ~(1<<STEPPER_B_ENABLE_N);
}