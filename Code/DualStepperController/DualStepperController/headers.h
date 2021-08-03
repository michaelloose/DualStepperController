/*
 * IncFile1.h
 *
 * Created: 16.06.2021 10:22:23
 *  Author: Waxrek
 */ 

//#define F_CPU 16000000UL //Liegt in Project -> Toolchain -> AVR/GNU C Compiler -> Symbols


#include "Descriptors.h"
#include "Stepper.h"
#include "StepperA.h"
#include "StepperB.h"

#ifndef HEADERS_H_
#define HEADERS_H_



	/* Includes: */
	#include <avr/io.h>
	#include <avr/wdt.h>
	#include <avr/power.h>
	#include <avr/interrupt.h>
	#include <avr/eeprom.h>
	//#include <util/delay.h>
	#include <stdlib.h>

	#include <string.h>
	#include <stdio.h>
	

	//#include <LUFA/Drivers/Board/LEDs.h>
	#include <LUFA/Drivers/USB/USB.h>
	#include <LUFA/Drivers/Misc/RingBuffer.h>
	//#include <LUFA/Platform/Platform.h>
	
	#define CMD_LEN_MAX 12
	#define RX_BUFFER_LEN 128
	
	#define CMD_STATE_EXECUTED 0
	#define CMD_STATE_RDY_TO_WRITE 1
	#define CMD_STATE_RDY_TO_EXEC 2
	
	#define DRV_STATE_RDY 0
	#define DRV_STATE_ESTOP 1
	#define DRV_STATE_PARAM 2
	
	
	/* Macros: */
	/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
	//#define LEDMASK_USB_NOTREADY      LEDS_LED1

	/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
	//#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

	/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
	//#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

	/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
	//#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)

	//#define _SFR_MEM8(mem_addr) (*(volatile uint8_t *)(mem_addr))
	//#define _SFR_MEM16(mem_addr)(*(volatile uint16_t *)(mem_addr))
	
	/* Function Prototypes: */

	void EVENT_USB_Device_Connect(void);
	void EVENT_USB_Device_Disconnect(void);
	void EVENT_USB_Device_ConfigurationChanged(void);
	void EVENT_USB_Device_ControlRequest(void);
	
	void putCDCtoBuffer(USB_ClassInfo_CDC_Device_t*, RingBuffer_t*);
	void checkBufferForCommands(USB_ClassInfo_CDC_Device_t*, RingBuffer_t*);
	void handleCommands(USB_ClassInfo_CDC_Device_t*);
	
	void driveHalt(void);
	void emergencyStop(void);
	void emergencyStopRelease(void);
	
	uint8_t driveState;
	
	void double2str(char*, double);
	
	uint16_t testCounter;	
	uint8_t commandState;
	char commandBuffer[CMD_LEN_MAX+1];
	char* pCommandBuffer;
#endif