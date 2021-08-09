/*
* Communication.c
*
* Created: 18.06.2021 10:15:25
*  Author: Waxrek
*/

#include "headers.h"

void putCDCtoBuffer(USB_ClassInfo_CDC_Device_t* pInterface, RingBuffer_t* pBuffer){

	/* Only try to read in bytes from the CDC interface if the transmit buffer is not full */
	if (!(RingBuffer_IsFull(pBuffer)))
	{
		int16_t ReceivedByte = CDC_Device_ReceiveByte(pInterface);
		if (!(ReceivedByte < 0)){
			RingBuffer_Insert(pBuffer, ReceivedByte);
			//CDC_Device_SendByte(&VirtualSerial_CDC_Interface, ReceivedByte);
		}
		
	}
}

void checkBufferForCommands(USB_ClassInfo_CDC_Device_t* pInterface, RingBuffer_t* pBuffer){
	uint16_t BufferCount = RingBuffer_GetCount(pBuffer);
	//Clear Buffer when Command was executed
	if (BufferCount && commandState == CMD_STATE_EXECUTED){
		memset(&commandBuffer[0], 0, CMD_LEN_MAX+1);
		pCommandBuffer = commandBuffer;
		commandState = CMD_STATE_RDY_TO_WRITE;
	}
	
	if (BufferCount && commandState == CMD_STATE_RDY_TO_WRITE){
		
		char nextChar;
		while (BufferCount--)
		{
			// Dequeue the already sent byte from the buffer now we have confirmed that no transmission error occurred
			nextChar = (char)RingBuffer_Remove(pBuffer);
			//Byte akzeptieren wenn es sich nicht um ein cr oder lf handelt (terminierungszeichen) und der Befehlspuffer die maximale Länge nicht erreicht hat.
			if ((nextChar != 0x0A) && (nextChar != 0x0D) && ((pCommandBuffer - commandBuffer) < CMD_LEN_MAX)){
				*pCommandBuffer++ = nextChar;
			}
			//Only accept command if it contains anything
			else{
				if((pCommandBuffer - commandBuffer)>0)
				commandState = CMD_STATE_RDY_TO_EXEC;
				break;
			}
		}
	}
}


void handleCommands(USB_ClassInfo_CDC_Device_t* pInterface){
	if(commandState == CMD_STATE_RDY_TO_EXEC)
	{
		char answerString[16] = "?";
		
		//Software-Reset
		if (((commandBuffer[0] == 'R') || (commandBuffer[0] == 'r'))&&((commandBuffer[1] == 'S') || (commandBuffer[1] == 's'))&&((commandBuffer[2] == 'T') || (commandBuffer[2] == 't'))){
			do
			{
				wdt_enable(WDTO_15MS);
				for(;;)
				{
				}
			} while(0);
		}
		
		if (driveState != DRV_STATE_ESTOP)
		{
			
			//Halt
			if (((commandBuffer[0] == 'H') || (commandBuffer[0] == 'h'))&&((commandBuffer[1] == 'L') || (commandBuffer[1] == 'l'))&&((commandBuffer[2] == 'T') || (commandBuffer[2] == 't'))){
				driveHalt();
				strcpy(answerString, "HLT");
			}
			//Not-Halt
			if (((commandBuffer[0] == 'E') || (commandBuffer[0] == 'e'))&&((commandBuffer[1] == 'S') || (commandBuffer[1] == 's'))&&((commandBuffer[2] == 'T') || (commandBuffer[2] == 't'))){
				emergencyStop();
				strcpy(answerString, "EST");
			}

			//Wechsel zu Parametriermodus
			if (((commandBuffer[0] == 'P') || (commandBuffer[0] == 'p'))&&((commandBuffer[1] == 'A') || (commandBuffer[1] == 'a'))&&((commandBuffer[2] == 'R') || (commandBuffer[2] == 'r'))&&((commandBuffer[3] == 'A') || (commandBuffer[3] == 'a'))&&((commandBuffer[4] == 'M') || (commandBuffer[4] == 'm'))){
				driveHalt();
				driveState = DRV_STATE_PARAM;
				strcpy(answerString, "PARAM");
			}
			

			
			
			//Parametrierbefehle
			if(driveState == DRV_STATE_PARAM)
			{
				//EXIT beendet den Parametriermodus
				if (((commandBuffer[0] == 'E') || (commandBuffer[0] == 'e'))&&((commandBuffer[1] == 'X') || (commandBuffer[1] == 'x'))&&((commandBuffer[2] == 'I') || (commandBuffer[2] == 'i'))&&((commandBuffer[3] == 'T') || (commandBuffer[3] == 't'))){
					driveHalt();
					driveState = DRV_STATE_RDY;
					strcpy(answerString, "EXIT");
				}
				
				//Erstes Zeichen bestimmt den Schrittmotor.
				if ((commandBuffer[0] == 'A') || (commandBuffer[0] == 'a') || (commandBuffer[0] == 'B') || (commandBuffer[0] == 'b')){
					
					StepperSettings_t *pSettings;
					void* eeprom_stor_addr;
					if ((commandBuffer[0] == 'A') || (commandBuffer[0] == 'a')){
						pSettings = &stepper_a_settings;
						eeprom_stor_addr = (void*)STEPPER_A_SETTINGS_START_ADDR;
						
					}
					else{
						pSettings = &stepper_b_settings;
						eeprom_stor_addr = (void*)STEPPER_B_SETTINGS_START_ADDR;

					}


					//SPU Steps per Unit
					if (((commandBuffer[1] == 'S') || (commandBuffer[1] == 's'))&&((commandBuffer[2] == 'P') || (commandBuffer[2] == 'p'))&&((commandBuffer[3] == 'U') || (commandBuffer[3] == 'u')))
					{
						if (commandBuffer[4] == '='){
							pSettings->steps_per_unit = atof(commandBuffer+5);
							strcpy(answerString, "SPU=");
						}
						
						if (commandBuffer[4] == '?'){
							char buffer[32];
							double2str(buffer, pSettings->steps_per_unit);
							strcpy(answerString, buffer);
						}
					}
					
					//VLM Geschwindigkeitsbegrenzung
					if (((commandBuffer[1] == 'V') || (commandBuffer[1] == 'v'))&&((commandBuffer[2] == 'L') || (commandBuffer[2] == 'l'))&&((commandBuffer[3] == 'M') || (commandBuffer[3] == 'm')))
					{
						if (commandBuffer[4] == '=')
						{
							if (commandBuffer[5] == '0')
							{
								pSettings->speed_max_enabled = 0;
								strcpy(answerString, "VLM=");
							}
							else if (commandBuffer[5] == '1')
							{
								pSettings->speed_max_enabled = 1;
								strcpy(answerString, "VLM=");
							}
						}
						if (commandBuffer[4] == '?')
						strcpy(answerString, pSettings->speed_max_enabled == 1 ? "1" : "0");
					}
					//VMX Maximalgeschwindigkeit
					if (((commandBuffer[1] == 'V') || (commandBuffer[1] == 'v'))&&((commandBuffer[2] == 'M') || (commandBuffer[2] == 'm'))&&((commandBuffer[3] == 'X') || (commandBuffer[3] == 'x')))
					{
						if (commandBuffer[4] == '='){
							pSettings->speed_max = atof(commandBuffer+5);
							strcpy(answerString, "VMX=");
						}
						
						if (commandBuffer[4] == '?'){
							char buffer[32];
							double2str(buffer, pSettings->speed_max);
							strcpy(answerString, buffer);
						}
					}

					//VDF Standardgeschwindigkeit
					if (((commandBuffer[1] == 'V') || (commandBuffer[1] == 'v'))&&((commandBuffer[2] == 'D') || (commandBuffer[2] == 'd'))&&((commandBuffer[3] == 'F') || (commandBuffer[3] == 'f')))
					{
						if (commandBuffer[4] == '='){
							pSettings->speed_default = atof(commandBuffer+5);
							strcpy(answerString, "VDF=");
						}
						
						if (commandBuffer[4] == '?'){
							char buffer[32];
							double2str(buffer, pSettings->speed_default);
							strcpy(answerString, buffer);
						}
					}
					
					//PLM Positionsbegrenzung
					if (((commandBuffer[1] == 'P') || (commandBuffer[1] == 'p'))&&((commandBuffer[2] == 'L') || (commandBuffer[2] == 'l'))&&((commandBuffer[3] == 'M') || (commandBuffer[3] == 'm')))
					{
						if (commandBuffer[4] == '=')
						{
							if (commandBuffer[5] == '0')
							{
								pSettings->position_limit_enabled = 0;
								strcpy(answerString, "PLM=");
							}
							else if (commandBuffer[5] == '1')
							{
								pSettings->position_limit_enabled = 1;
								strcpy(answerString, "PLM=");
							}
						}
						if (commandBuffer[4] == '?')
						strcpy(answerString, pSettings->position_limit_enabled == 1 ? "1" : "0");
					}
					
					//PMX Maximale Position
					if (((commandBuffer[1] == 'P') || (commandBuffer[1] == 'p'))&&((commandBuffer[2] == 'M') || (commandBuffer[2] == 'm'))&&((commandBuffer[3] == 'X') || (commandBuffer[3] == 'x')))
					{
						if (commandBuffer[4] == '='){
							pSettings->position_max = atof(commandBuffer+5);
							strcpy(answerString, "PMX=");
						}
						
						if (commandBuffer[4] == '?'){
							char buffer[32];
							double2str(buffer, pSettings->position_max);
							strcpy(answerString, buffer);
						}
					}
					//PMN Minimale Position
					if (((commandBuffer[1] == 'P') || (commandBuffer[1] == 'p'))&&((commandBuffer[2] == 'M') || (commandBuffer[2] == 'm'))&&((commandBuffer[3] == 'N') || (commandBuffer[3] == 'n')))
					{
						if (commandBuffer[4] == '='){
							pSettings->position_min = atof(commandBuffer+5);
							strcpy(answerString, "PMN=");
						}
						
						if (commandBuffer[4] == '?'){
							char buffer[32];
							double2str(buffer, pSettings->position_min);
							strcpy(answerString, buffer);
						}
					}
					//ROF Offset vom Referenzpunkt
					if (((commandBuffer[1] == 'R') || (commandBuffer[1] == 'r'))&&((commandBuffer[2] == 'O') || (commandBuffer[2] == 'o'))&&((commandBuffer[3] == 'F') || (commandBuffer[3] == 'f')))
					{
						if (commandBuffer[4] == '='){
							pSettings->reference_offset = atof(commandBuffer+5);
							strcpy(answerString, "ROF=");
						}
						
						if (commandBuffer[4] == '?'){
							char buffer[32];
							double2str(buffer, pSettings->reference_offset);
							strcpy(answerString, buffer);
						}
					}
					
					//DIN Richtungsumkehr
					if (((commandBuffer[1] == 'D') || (commandBuffer[1] == 'd'))&&((commandBuffer[2] == 'I') || (commandBuffer[2] == 'i'))&&((commandBuffer[3] == 'N') || (commandBuffer[3] == 'n')))
					{
						if (commandBuffer[4] == '=')
						{
							if (commandBuffer[5] == '0')
							{
								pSettings->invert_direction = 0;
								strcpy(answerString, "DIN=");
							}
							else if (commandBuffer[5] == '1')
							{
								pSettings->invert_direction = 1;
								strcpy(answerString, "DIN=");
							}
						}
						if (commandBuffer[4] == '?')
						strcpy(answerString, pSettings->invert_direction == 1 ? "1" : "0");
					}
					//
					
					//RPL Polarität Referenzschalter: 0=OEffner, 1=Schliesser
					if (((commandBuffer[1] == 'R') || (commandBuffer[1] == 'r'))&&((commandBuffer[2] == 'P') || (commandBuffer[2] == 'p'))&&((commandBuffer[3] == 'L') || (commandBuffer[3] == 'l')))
					{
						if (commandBuffer[4] == '=')
						{
							if (commandBuffer[5] == '0')
							{
								pSettings->polarity_reference_switch = 0;
								strcpy(answerString, "RPL=");
							}
							else if (commandBuffer[5] == '1')
							{
								pSettings->polarity_reference_switch = 1;
								strcpy(answerString, "RPL=");
							}
						}
						if (commandBuffer[4] == '?')
						strcpy(answerString, pSettings->polarity_reference_switch == 1 ? "1" : "0");
					}
					//strcpy(answerString, commandBuffer);
					
					eeprom_update_block(pSettings, eeprom_stor_addr, sizeof(stepper_a_settings));
				}

			}
			
			
			//Steuerbefehle
			if(driveState == DRV_STATE_RDY)
			{
				
				//Erstes Zeichen bestimmt den Schrittmotor.
				if ((commandBuffer[0] == 'A') || (commandBuffer[0] == 'a') || (commandBuffer[0] == 'B') || (commandBuffer[0] == 'b')){
					
					void (*pHome)(void);
					void (*pZero)(void);
					void (*pHalt)(void);
					uint8_t (*pSetSetPoint)(double);
					double (*pGetPosition)(void);
					uint8_t (*pSetSpeed)(double);
					double (*pGetSpeed)(void);
					char driveName[2];
					
					if ((commandBuffer[0] == 'A') || (commandBuffer[0] == 'a')){
						
						pHome = &stepper_a_home;
						pZero = &stepper_a_zero;
						pHalt = &stepper_a_halt;

						pSetSetPoint = &stepper_a_setSetPoint;
						pGetPosition = &stepper_a_getPosition;

						pSetSpeed = &stepper_a_setSpeed;
						pGetSpeed = &stepper_a_getSpeed;
						
						strcpy(driveName, "A");
					}
					else{
						pHome = &stepper_b_home;
						pZero = &stepper_b_zero;
						pHalt = &stepper_b_halt;

						pSetSetPoint = &stepper_b_setSetPoint;
						pGetPosition = &stepper_b_getPosition;

						pSetSpeed = &stepper_b_setSpeed;
						pGetSpeed = &stepper_b_getSpeed;
						
						strcpy(driveName, "B");
					}
					

					if ((commandBuffer[1] == 'S') || (commandBuffer[1] == 's'))
					{
						
						if (commandBuffer[2] == '=')
						{
							//Fehler beim Ausführen
							if ((*pSetSpeed)(atof(commandBuffer+3)) == 1)
							{
								strcpy(answerString, driveName);
								strcat(answerString, "S?");
							}
							//Befehl Erkannt
							else
							{
								strcpy(answerString, driveName);
								strcat(answerString, "S=");
							}


						}
						else if (commandBuffer[2] == '?')
						{
							//TS?
							char buffer[32];
							double2str(buffer, (*pGetSpeed)());
							strcpy(answerString, driveName);
							strcat(answerString, "S=");
							strcat(answerString, buffer);
						}
					}
					if ((commandBuffer[1] == 'P') || (commandBuffer[1] == 'p'))
					{
						if (commandBuffer[2] == '=')
						{
							strcpy(answerString, driveName);
							
							if ((*pSetSetPoint)(atof(commandBuffer+3)) == 1)
							{
								strcat(answerString, "P?");
							}
							else
							{
								strcat(answerString, "P=");
							}
						}
						else if (commandBuffer[2] == '?')
						{
							char buffer[32];
							double2str(buffer, (*pGetPosition)());
							strcpy(answerString, driveName);
							strcat(answerString, "P=");
							strcat(answerString, buffer);
						}
					}
					if ((commandBuffer[1] == 'R') || (commandBuffer[1] == 'r'))
					{
						
						(*pHalt)();
						strcpy(answerString, driveName);
						strcat(answerString, "R=");
					}
					if ((commandBuffer[1] == 'H') || (commandBuffer[1] == 'h'))
					{
											
						(*pHome)();
						strcpy(answerString, driveName);
						strcat(answerString, "H=");
					}
					if ((commandBuffer[1] == 'Z') || (commandBuffer[1] == 'z'))
					{
						(*pZero)();
						strcpy(answerString, driveName);
						strcat(answerString, "Z=");
					}
				}
			}
		}
		//Freigabe Not-Halt
		else if(((commandBuffer[0] == 'E') || (commandBuffer[0] == 'e'))&&((commandBuffer[1] == 'S') || (commandBuffer[1] == 's'))&&((commandBuffer[2] == 'R') || (commandBuffer[2] == 'r'))){
			emergencyStopRelease();
			strcpy(answerString, "ESR");
		}
		
		else{
			strcpy(answerString, "EST");
		}

		
		
		//Sending the AnswerString
		CDC_Device_SendString(pInterface, answerString);
		CDC_Device_SendByte(pInterface, 0x0D);
		//CDC_Device_SendByte(pInterface, 0x0A);
		
		//Echo Input to test
		//CDC_Device_SendString(pInterface, commandBuffer);
		


		commandState = CMD_STATE_EXECUTED;
	}


}



/*




Endpoint_SelectEndpoint((*pInterface).Config.DataINEndpoint.Address);

// Check if a packet is already enqueued to the host - if so, we shouldn't try to send more data
// until it completes as there is a chance nothing is listening and a lengthy timeout could occur
if (Endpoint_IsINReady())
{
// Never send more than one bank size less one byte to the host at a time, so that we don't block
// while a Zero Length Packet (ZLP) to terminate the transfer is sent if the host isn't listening
uint8_t BytesToSend = MIN(BufferCount, (CDC_TXRX_EPSIZE - 1));

Read bytes from the USART receive buffer into the USB IN endpoint
while (BytesToSend--)
{
// Try to send the next byte of data to the host, abort if there is an error without dequeuing
if (CDC_Device_SendByte(pInterface,
RingBuffer_Peek(pBuffer)) != ENDPOINT_READYWAIT_NoError)
{
break;
}
// Dequeue the already sent byte from the buffer now we have confirmed that no transmission error occurred
RingBuffer_Remove(pBuffer);
}


if (newCommandAvailable == 0)
{
int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);

if (!(ReceivedByte < 0)){
//fputs(ReceivedByte, &USBSerialStream);
//CDC_Device_SendByte(&VirtualSerial_CDC_Interface, (char)ReceivedByte);

if (((char)ReceivedByte != 0x0D)) // && ((int)commandBuffer - (int)commandBufferPTR < CMD_LEN_MAX)
{
*commandBufferPTR++ = ReceivedByte;
//commandBufferPTR;
}
else
{
*commandBufferPTR++ = 0x00;
commandBufferPTR = commandBuffer;
newCommandAvailable = 0;
fputs("IT WORKS\r\n", &USBSerialStream);
fputs(*commandBufferPTR, &USBSerialStream);
}


}

}
*/
