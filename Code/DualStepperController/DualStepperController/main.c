/*
* main.c
*
* Created: 6/11/2021 10:40:07 AM
*  Author: Waxrek
*/
#include "headers.h"





static RingBuffer_t RxBuffer;
static uint8_t RxBufferData[RX_BUFFER_LEN];


/** LUFA CDC Class driver interface configuration and state information. This structure is
*  passed to all CDC Class driver functions, so that multiple instances of the same class
*  within a device can be differentiated from one another.
*/
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
	.Config =
	{
		.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
		.DataINEndpoint           =
		{
			.Address          = CDC_TX_EPADDR,
			.Size             = CDC_TXRX_EPSIZE,
			.Banks            = 1,
		},
		.DataOUTEndpoint =
		{
			.Address          = CDC_RX_EPADDR,
			.Size             = CDC_TXRX_EPSIZE,
			.Banks            = 1,
		},
		.NotificationEndpoint =
		{
			.Address          = CDC_NOTIFICATION_EPADDR,
			.Size             = CDC_NOTIFICATION_EPSIZE,
			.Banks            = 1,
		},
	},
};



int main (void) {


	testCounter = 0;
	driveState = DRV_STATE_RDY;
	
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	
	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	USB_Init();

	GlobalInterruptEnable();
	
	
	DDRB |= (1 << 0);
	DDRD |= (1 << 5);
	
	PORTB |= (1<<0);    //Bit setzen, LED AUS
	PORTD |= (1<<5);    //Bit setzen, LED AUS
	//PORTD &= ~(1<<5);    //Bit setzen, LED AN
	
	// Initialize the buffer with the created storage array
	RingBuffer_InitBuffer(&RxBuffer, RxBufferData, RX_BUFFER_LEN);
	
	stepper_a_init();
	stepper_b_init();

	while(1){

		/*
		testCounter++;
		
		if(testCounter > 20000){
			testCounter = 0;
			PORTB |= (1<<0);    //Bit setzen, LED AUS
			PORTD &= ~(1<<5);    //Bit loeschen, LED AN
		}
		
		else if(testCounter > 10000){
			// halbe sekunde warten
			PORTB &= ~(1<<0);   // Bit loeschen, LED AN
			PORTD |= (1<<5);    //Bit setzen, LED AUS
		}
		//fputs((char*)&testCounter, &USBSerialStream);
		*/
		
		
		stepper_a_loop(&VirtualSerial_CDC_Interface);
		stepper_b_loop(&VirtualSerial_CDC_Interface);

		putCDCtoBuffer(&VirtualSerial_CDC_Interface, &RxBuffer);
		checkBufferForCommands(&VirtualSerial_CDC_Interface, &RxBuffer);
		
		/*
		if(commandState == CMD_STATE_RDY_TO_EXEC){
			pCommandBuffer = commandBuffer;
			char nextChar = *pCommandBuffer++;
			
			while(nextChar != 0x00){
				CDC_Device_SendByte(&VirtualSerial_CDC_Interface, nextChar);
				nextChar = *pCommandBuffer++;
			}
			commandState = CMD_STATE_EXECUTED;
		}
		*/
		
		handleCommands(&VirtualSerial_CDC_Interface);
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
	
	return 0;
}






/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	//LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}


/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** CDC class driver callback function the processing of changes to the virtual
*  control lines sent from the host..
*
*  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
*/
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo)
{
	/* You can get changes to the virtual CDC lines in this callback; a common
	use-case is to use the Data Terminal Ready (DTR) flag to enable and
	disable CDC communications in your application when set to avoid the
	application blocking while waiting for a host to become ready and read
	in the pending data from the USB endpoints.
	*/
	//bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
}
