#include <psx.h>
#include <string.h>
#include "include/controllers.h"

void InitPad(){
	PADSIO_CTRL(0) = 0x40;
	PADSIO_BAUD(0) = 0x88;
	PADSIO_MODE(0) = 13;
	PADSIO_CTRL(0) = 0;
	BusyLoop(10);
	PADSIO_CTRL(0) = 2;
	BusyLoop(10);
	PADSIO_CTRL(0) = 0x2002;
	BusyLoop(10);
	PADSIO_CTRL(0) = 0;
}

void BusyLoop(int count){
	volatile int cycles = count;
	while (cycles--);
}

void ResetPad(Controller* ctrl)
{
	/*Clear controller data*/
	memset(ctrl, 0, sizeof(Controller));
	
	/*Treat controller as disconnected*/
	ctrl->Type = PAD_NONE;

	/*Reset cursor to center of the screen*/
	ctrl->CursorX = 160;
	ctrl->CursorY = 120;
}

void SendData(int pad_n, unsigned char *in, unsigned char *out, int len)
{

	if (!in || !out)
		return;

	/*This is how the BIOS does it*/
	uint16_t mask = pad_n == 0 ? 0x0000 : 0x2000;

	PADSIO_CTRL(0) = mask | 2;
	PADSIO_DATA(0);
	BusyLoop(40);
	PADSIO_CTRL(0) = mask | 0x1003;

	while (!(PADSIO_STATUS(0) & 1));

	for(int x = 0; x < len; x++)
	{
		/*Wait for TX ready*/
		while((PADSIO_STATUS(0) & 4) < 1);
		
		PADSIO_DATA(0) = *in;
		in++;

		BusyLoop(25);

		/*Read RX status flag*/
		while((PADSIO_STATUS(0) & 2) < 1);
		
		/*Busy loop only after initial byte*/
		if(x == 0) BusyLoop(40);

		*out = PADSIO_DATA(0);
		out++;
	}
	
	PADSIO_CTRL(0) = 0;
}

void ReadPad(Controller* ctrl, int pad_n)
{
	unsigned char DataToSend[] =  {1, 0x42, 0, 0, 0, 0, 0, 0, 0};			/*Standard data polling command*/
	unsigned char ReceivedData[16];
	
	unsigned char ConfigStart[] = {1, 0x43, 0, 1, 0};						/*Config entry command*/
	unsigned char ConfigStop[] = {1, 0x43, 0, 0, 0, 0, 0, 0, 0};			/*Config exit command*/
	unsigned char ConfigAnalog[] = {1, 0x44, 0, 1, 3, 0, 0, 0, 0};			/*Permanent analog on command*/
	unsigned char ConfigRumble[] = {1, 0x4D, 0, 0, 1, 255, 255, 255, 255};	/*Enable rumble motors*/

	/*Remove rumble values*/
	DataToSend[3] = 0;
	DataToSend[4] = 0;

	/*Clear receive buffer*/
	memset(&ReceivedData, 0, sizeof(ReceivedData));

	switch(ctrl->ConfigState)
	{
		default:
			DataToSend[3] = ctrl->SmallMotor;
			DataToSend[4] = ctrl->BigMotor;
	
			/*Read button status*/
			SendData(pad_n, DataToSend, ReceivedData, sizeof(DataToSend));

			/*Check if anything is connected (line not floating high)*/
			if(ReceivedData[1] == PAD_NONE)
			{
				ResetPad(ctrl);
			}
			else
			{
				/*Check if controller type changed from previous reading*/
				if(ctrl->Type != ReceivedData[1]) ctrl->ConfigState = 0;
			
				/*Store type*/
				ctrl->Type = ReceivedData[1];
		
				/*Get digital buttons*/
				ctrl->Buttons = ~((ReceivedData[3] << 8) | ReceivedData[4]);
				
				/*Check if this is analog controller*/
				if(ctrl->Type == PAD_ANALOG)
				{
					/*Get analog sticks*/
					ctrl->LeftStickX = ReceivedData[7] - 128;
					ctrl->LeftStickY = ReceivedData[8] - 128;
					ctrl->RightStickX = ReceivedData[5] - 128;
					ctrl->RightStickY = ReceivedData[6] - 128;
				}

				/*Check if this is a mouse*/
				if(ctrl->Type == PAD_MOUSE){
					ctrl->CursorX += (char)ReceivedData[5];
					ctrl->CursorY += (char)ReceivedData[6];

					/*Clipping*/
					if(ctrl->CursorX < 0) ctrl->CursorX = 0;
					if(ctrl->CursorY < 0) ctrl->CursorY = 0;
					if(ctrl->CursorX > 320) ctrl->CursorX = 320;
					if(ctrl->CursorY > 240) ctrl->CursorY = 240;
				}
			}
			break;
			
		case 1:
			/*Enter configuration mode*/
			SendData(pad_n, ConfigStart, ReceivedData, sizeof(ConfigStart));
			break;
			
		case 2:
			/*Set auto analog mode*/
			SendData(pad_n, ConfigAnalog, ReceivedData, sizeof(ConfigAnalog));
			break;
			
		case 3:
			/*Configure rumble*/
			SendData(pad_n, ConfigRumble, ReceivedData, sizeof(ConfigRumble));
			break;
			
		case 4:
			/*Exit configuration mode*/
			SendData(pad_n, ConfigStop, ReceivedData, sizeof(ConfigStop));
			break;
	}

	if(ctrl->ConfigState < 6) ctrl->ConfigState++;
}