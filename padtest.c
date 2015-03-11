/*
 * PS1 controller test application
 * Shendo 2014, PSXSDK by Tails92
 */

#include <psx.h>
#include <stdio.h>

/*Font related*/
#include "include/fontspace.h"
#include "include/font.h"

/*Images*/
#include "images/buttons.h"

/*Defines*/
#define IMASK					*((unsigned int*)0x1F801074)
#define IPENDING				*((unsigned int*)0x1F801070)

/*Copied from pad.c*/
#define PADSIO_DATA(x)	*((unsigned char*)(0x1f801040 + (x<<4)))
#define PADSIO_STATUS(x)	*((unsigned short*)(0x1f801044 + (x<<4)))
#define PADSIO_MODE(x)	*((unsigned short*)(0x1f801048 + (x<<4)))
#define PADSIO_CTRL(x)	*((unsigned short*)(0x1f80104a + (x<<4)))
#define PADSIO_BAUD(x)	*((unsigned short*)(0x1f80104e + (x<<4)))


#define PAD_NONE			0xFF
#define PAD_DIGITAL		0x41
#define PAD_ANALOG		0x73
#define PAD_FLIGHT			0x53
#define PAD_MOUSE			0x12

unsigned int PrimList[0x8000];

int dbuf = 0;
int VBlank = 0;
int i, j, k;

/*All properties of a controller*/
typedef struct
{
	unsigned char Type;
	unsigned char ConfigState;
	unsigned char SmallMotor;
	unsigned char BigMotor;
	unsigned short Buttons;
	char LeftStickX;
	char LeftStickY;
	char RightStickX;
	char RightStickY;
}Controller;

Controller Controllers[2];

/*Global variables*/
GsImage TempImage;
GsSprite PadSprite;
GsRectangle TopRect;

/*Function prototypes*/
void VBlankHandler();
void VSync();
void FlipBuffer();
void SendData(int pad_n, unsigned char *in, unsigned char *out, int len);
void ResetPad(int PadSlot);
void ReadPads();
int GetPrintedStringWidth(char monospace, char *string);
void GsPrintString(int x, int y, char Red, char Green, char Blue, char monospace, char *string);
void DrawPlus(int x, int y);
void DrawController(int x, int y, int PadId);
void DrawTitle();

/*This function gets called each VBlank*/
void VBlankHandler()
{
	VBlank = 1;
	IPENDING &= 0xFFFE;		/*Acknowledge VBlank in status register*/
}

/*Wait for vertical sync*/
void VSync()
{
	VBlank = 0;
	while(VBlank == 0);
}

/*Flip main and back-buffer*/
void FlipBuffer()
{
	dbuf=!dbuf;
	GsSetDispEnvSimple(0, dbuf ? 0 : 256);
	GsSetDrawEnvSimple(0, dbuf ? 256 : 0, 320, 240);
}

/*Reset controller data to default values*/
void ResetPad(int PadSlot)
{
	/*Clear controller data*/
	memset(&Controllers[PadSlot], 0, sizeof(Controller));
	
	/*Treat controller as disconnected*/
	Controllers[PadSlot].Type = PAD_NONE;
}

/*Send data to PAD_SIO*/
void SendData(int pad_n, unsigned char *in, unsigned char *out, int len)
{
	int x;
	int y;
	int i;
	
	PADSIO_MODE(0) = 0x0D;
	PADSIO_BAUD(0) = 0x88;
	
	if(pad_n == 1) PADSIO_CTRL(0) = 0x3003; else PADSIO_CTRL(0) = 0x1003;
		
	for(y=0;y<400;y++);	/*Slight delay before first transmission*/

	for(x = 0; x < len; x++)
	{
		/*Wait for TX ready*/
		while((PADSIO_STATUS(0) & 4) < 1);
		
		PADSIO_DATA(0) = *in;
		in++;

		/*Read RX status flag*/
		while((PADSIO_STATUS(0) & 2) < 1);
		
		*out = PADSIO_DATA(0);
		out++;
	}
	
	PADSIO_CTRL(0) = 0;
}

/*Read controller data from both ports*/
void ReadPads()
{
	unsigned char DataToSend[] =  {1, 0x42, 0, 0, 0, 0, 0, 0, 0};					/*Standard data polling command*/
	unsigned char ReceivedData[16];
	
	unsigned char ConfigStart[] = {1, 0x43, 0, 1, 0};										/*Config entry command*/
	unsigned char ConfigStop[] = {1, 0x43, 0, 0, 0};										/*Config exit command*/
	unsigned char ConfigAnalog[] = {1, 0x44, 0, 1, 3, 0, 0, 0, 0};					/*Permanent analog on command*/
	unsigned char ConfigRumble[] = {1, 0x4D, 0, 0, 1, 255, 255, 255, 255};	/*Enable rumble motors*/
	
	for(i = 0; i < 2; i++)
	{
		/*Remove rumble values*/
		DataToSend[3] = 0;
		DataToSend[4] = 0;
	
		switch(Controllers[i].ConfigState)
		{
			default:
				DataToSend[3] = Controllers[i].SmallMotor;
				DataToSend[4] = Controllers[i].BigMotor;
		
				/*Read button status*/
				SendData(i, DataToSend, ReceivedData, sizeof(DataToSend));

				/*Check if anything is connected (line not floating high)*/
				if(ReceivedData[1] == PAD_NONE)
				{
					ResetPad(i);
				}
				else
				{
					/*Check if controller type changed from previous reading*/
					if(Controllers[i].Type != ReceivedData[1]) Controllers[i].ConfigState = 0;
				
					/*Store type*/
					Controllers[i].Type = ReceivedData[1];
			
					/*Get digital buttons*/
					Controllers[i].Buttons = ~((ReceivedData[3] << 8) | ReceivedData[4]);
					
					/*Check if this is analog controller*/
					if(Controllers[i].Type == PAD_ANALOG)
					{
						/*Get analog sticks*/
						Controllers[i].LeftStickX = ReceivedData[7] - 128;
						Controllers[i].LeftStickY = ReceivedData[8] - 128;
						Controllers[i].RightStickX = ReceivedData[5] - 128;
						Controllers[i].RightStickY = ReceivedData[6] - 128;
					}
				}
				break;
				
			case 1:
				/*Enter configuration mode*/
				SendData(i, ConfigStart, NULL, sizeof(ConfigStart));
				break;
				
			case 2:
				/*Set auto analog mode*/
				SendData(i, ConfigAnalog, NULL, sizeof(ConfigAnalog));
				break;
				
			case 3:
				/*Configure rumble*/
				SendData(i, ConfigRumble, NULL, sizeof(ConfigRumble));
				break;
				
			case 4:
				/*Exit configuration mode*/
				SendData(i, ConfigStop, NULL, sizeof(ConfigStop));
				break;
		}
	
		if(Controllers[i].ConfigState < 6)Controllers[i].ConfigState++;
	}
}

/*Return a length in pixels for the given string*/
int GetPrintedStringWidth(char monospace, char *string)
{
	int StringWidth = 0;
	char CharOffset;
		
	while(*string)
	{
		/*Check if this is a printable character*/
		if(*string >= 0x20 && *string <= 0x7F)
		{
			/*Get char offset*/
			CharOffset = *string - 0x20;
			
			if(monospace) StringWidth += 8;
			else StringWidth += (FontSpace[CharOffset] + 1);
		}

		/*Check if this is a newline character*/
		if(*string == '\n') return StringWidth;
		
		/*Point to the next character*/
		string++;
	}
	
	return StringWidth;
}

/*
 * Print a string at the specified coordinates in color
 * Supports \n - newline. If monospace is true each character is spaced 8px from the previous one.
 */
void GsPrintString(int x, int y, char Red, char Green, char Blue, char monospace, char *string)
{
	GsSprite CharSprite;
	char CharOffset;
	
	/*Set up character sprite*/
	if(x < 0)CharSprite.x = 160 - (GetPrintedStringWidth(monospace, string)/2);
	else CharSprite.x = x;
	
	CharSprite.y = y;
	CharSprite.w = 8;
	CharSprite.h = 8;
	CharSprite.r = Red;
	CharSprite.g = Green;
	CharSprite.b = Blue;
	CharSprite.cx = 320;
	CharSprite.cy = 240;
	CharSprite.tpage = 5;
	CharSprite.attribute = COLORMODE(COLORMODE_8BPP);

	while(*string)
	{
		/*Check if this is a printable character*/
		if(*string >= 0x20 && *string <= 0x7F)
		{
			/*Get char offset*/
			CharOffset = *string - 0x20;

			CharSprite.u = (CharOffset%0x20) * 8;
			CharSprite.v = (CharOffset/0x20) * 8;

			/*Place sprite in the drawing list*/
			GsSortSimpleSprite(&CharSprite);
			
			/*Increase X offset*/
			if(monospace) CharSprite.x += 8;
			else CharSprite.x += (FontSpace[CharOffset] + 1);
		}
		
		/*Check if this is a newline character*/
		if(*string == '\n')
		{
			string++;
		
			if(x < 0)CharSprite.x = 160 - (GetPrintedStringWidth(monospace, string)/2);
			else CharSprite.x = x;
			
			CharSprite.y += 10;
		}
		else string++;
	}
}

/*Draw green plus (used for analog sticks)*/
void DrawPlus(int x, int y)
{
	GsLine PlusLine;
	
	PlusLine.x[0] = x - 2;
	PlusLine.x[1] = x + 2;
	PlusLine.y[0] = y;
	PlusLine.y[1] = y;
	PlusLine.r = 109;
	PlusLine.g = 193;
	PlusLine.b = 99;
	PlusLine.attribute = 0;
	
	GsSortLine(&PlusLine);
	
	PlusLine.x[0] = x;
	PlusLine.x[1] = x;
	PlusLine.y[0] = y - 2;
	PlusLine.y[1] = y + 2;
	
	GsSortLine(&PlusLine);
}

/*Draw controller at the specified coordinates*/
void DrawController(int x, int y, int PadId)
{
	int FontX = 0;
	int PressedOffset = 0;
	int AnalogEnabled = 0;
	unsigned short buttons = Controllers[PadId].Buttons;
	int StickX[2] = {0, 0};
	int StickY[2] = {0, 0};
	char TempString[50];
	
	/*Check what kind of controller is connected to the port*/
	switch(Controllers[PadId].Type)
	{
		default:
			FontX = GetPrintedStringWidth(false, "Not supported");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Not supported");
			return;
			
		case PAD_NONE:
			FontX = GetPrintedStringWidth(false, "Not connected");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Not connected");
			return;
			
		case PAD_DIGITAL:
			FontX = GetPrintedStringWidth(false, "Digital");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Digital");
			break;
			
		case PAD_ANALOG:
			AnalogEnabled = 1;
			FontX = GetPrintedStringWidth(false, "Analog");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Analog");
			StickX[0] = Controllers[PadId].LeftStickX;
			StickY[0] = Controllers[PadId].LeftStickY;
			StickX[1] = Controllers[PadId].RightStickX;
			StickY[1] = Controllers[PadId].RightStickY;
			break;
	}
	
	PadSprite.x = x + 10;
	PadSprite.y = y;
	PadSprite.w = 16;
	PadSprite.h = 16;
	PadSprite.u = 32;
	PadSprite.v = 16;
	PadSprite.r = PadSprite.g = PadSprite.b = 128;
	PadSprite.cx = 320;
	PadSprite.cy = 241;
	PadSprite.tpage = 7;
	PadSprite.attribute = COLORMODE(COLORMODE_8BPP);
	
	/*L1*/
	if(buttons & PAD_L1)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*L2*/
	PadSprite.v -= 16;
	PadSprite.y += 16;
	
	if(buttons & PAD_L2)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*UP*/
	PadSprite.u -= 32;
	PadSprite.y += 32;
	
	if(buttons & PAD_UP)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*LEFT*/
	PadSprite.v += 32;
	PadSprite.x -= 10;
	PadSprite.y += 10;
	
	if(buttons & PAD_LEFT)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	/*DOWN*/
	PadSprite.v -= 16;
	PadSprite.x += 10;
	PadSprite.y += 10;
	
	if(buttons & PAD_DOWN)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*RIGHT*/
	PadSprite.v += 32;
	PadSprite.x +=10;
	PadSprite.y -= 10;
	
	if(buttons & PAD_RIGHT)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*SELECT*/
	PadSprite.u += 32;
	PadSprite.v -= 16;
	PadSprite.x += 26;
	
	if(buttons & PAD_SELECT)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*START*/
	PadSprite.v += 16;
	PadSprite.x += 26;
	
	if(buttons & PAD_START)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*SQUARE*/
	PadSprite.u -= 32;	
	PadSprite.v += 64;
	PadSprite.x += 26;
	
	if(buttons & PAD_SQUARE)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	/*CROSS*/
	PadSprite.v -= 32;
	PadSprite.x += 13;
	PadSprite.y += 13;	
	
	if(buttons & PAD_CROSS)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*CIRCLE*/
	PadSprite.v -= 16;
	PadSprite.x += 13;
	PadSprite.y -= 13;	
	
	if(buttons & PAD_CIRCLE)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*TRIANGLE*/
	PadSprite.v += 32;
	PadSprite.x -= 13;
	PadSprite.y -= 13;	
	
	if(buttons & PAD_TRIANGLE)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*R2*/
	PadSprite.u += 32;
	PadSprite.v -= 96;
	PadSprite.y -= 29;
	
	if(buttons & PAD_R2)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	
	/*R1*/
	PadSprite.v += 16;
	PadSprite.y -= 16;
	
	if(buttons & PAD_R1)
	{
		PadSprite.u += 16;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.u -= 16;
	}
	else GsSortSimpleSprite(&PadSprite);
	
	/*Return if this is not analog controller*/
	if(AnalogEnabled == 0) return;
	
	PadSprite.x = x + 26;
	PadSprite.y = y + 80;
	PadSprite.w = 32;
	PadSprite.h = 32;
	PadSprite.u = 32;
	PadSprite.v = 64;
	PadSprite.r = PadSprite.g = PadSprite.b = 128;
	PadSprite.cx = 320;
	PadSprite.cy = 241;
	PadSprite.tpage = 7;
	PadSprite.attribute = COLORMODE(COLORMODE_8BPP);
	
	/*Left analog stick*/
	if(buttons & PAD_LANALOGB)
	{
		PadSprite.v += 32;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.v -= 32;
		
		/*Rumble big motor*/
		Controllers[PadId].BigMotor = 255;
	}
	else 
	{
		GsSortSimpleSprite(&PadSprite);
		Controllers[PadId].BigMotor = 0;
	}
	
	/*Left stick position*/
	DrawPlus(x + 42 + (StickX[0]/8), y + 96 + (StickY[0]/8));
	sprintf(TempString, "X: %d\nY: %d", StickX[0], StickY[0]);
	GsPrintString(x + 26, y + 116, 128, 128, 128, false, TempString);
	
	/*Right analog stick*/
	PadSprite.x += 52;
	if(buttons & PAD_RANALOGB)
	{
		PadSprite.v += 32;
		GsSortSimpleSprite(&PadSprite);
		PadSprite.v -= 32;
		
		/*Rumble small motor*/
		Controllers[PadId].SmallMotor = 255;
	}
	else
	{
		GsSortSimpleSprite(&PadSprite);
		Controllers[PadId].SmallMotor = 0;
	}
	
	DrawPlus(x + 94 + (StickX[1]/8), y + 96 + (StickY[1]/8));
	sprintf(TempString, "X: %d\nY: %d", StickX[1], StickY[1]);
	GsPrintString(x + 78, y + 116, 128, 128, 128, false, TempString);
}

/*Draw title bar*/
void DrawTitle()
{
	int FontX = 0;
	
	/*Draw top rectangle*/
	TopRect.x = 0;
	TopRect.y = 0;
	TopRect.w = 320;
	TopRect.h = 40;
	TopRect.r = 0;
	TopRect.g = 76;
	TopRect.b = 163;
	TopRect.attribute = 0;
	
	GsSortRectangle(&TopRect);
	
	GsPrintString(16, 16, 128, 128, 128, false, "PadTest 1.0");
	
	FontX = GetPrintedStringWidth(false, "PORT 1");
	GsPrintString(80 - (FontX/2), 46, 128, 128, 128, false, "PORT 1");
	
	FontX = GetPrintedStringWidth(false, "PORT 2");
	GsPrintString(240 - (FontX/2), 46, 128, 128, 128, false, "PORT 2");
	
	GsPrintString(16, 210, 128, 128, 128, false, "Coded by Shendo, 2014.\nPSXSDK by Tails92.");
}

int main()
{
	GsInit();					/*Init GPU*/
	GsSetList(PrimList);
	GsClearMem();
	
	/*Set video mode based on the console's region*/
	if(*(char *)0xbfc7ff52 == 'E')	GsSetVideoMode(320, 240, VMODE_PAL);
	else GsSetVideoMode(320, 240, VMODE_NTSC);

	/*Load a custom font and upload it to VRAM*/
	GsImageFromTim(&TempImage, FontTimData);
	GsUploadImage(&TempImage);
	
	/*Load controller buttons image*/
	GsImageFromTim(&TempImage, ButtonsTimData);
	GsUploadImage(&TempImage);
	
	SetVBlankHandler(VBlankHandler);
	
	/*Set default values for both controllers*/
	ResetPad(0);
	ResetPad(1);
	
	/*Main loop of the application*/
	while(1)
	{
		/*Flip main and back buffer*/
		FlipBuffer();
	
		GsSortCls(0,0,0);
		
		DrawTitle();
		ReadPads();

		/*Draw controllers on the screen*/
		DrawController(10, 65, 0);
		DrawController(170, 65, 1);
		
		/*Draw primitives from the list*/
		GsDrawList();
		
		/*Wait for GPU to finish drawing*/
		while(GsIsDrawing());
		
		/*Wait for vertical sync*/
		VSync();
	}
	
	return 0;
}
