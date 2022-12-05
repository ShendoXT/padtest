#include <psx.h>
#include <stdio.h>
#include "include/graphics.h"
#include "include/text.h"

#include "images/buttons.h"
#include "images/mouse.h"

int dbuf = 0;
int VBlank = 0;
unsigned int PrimList[0x8000];

void InitGraphics(){
    GsImage PadImage;
    GsImage MouseImage;

	GsInit();					/*Init GPU*/
	GsSetList(PrimList);
	GsClearMem();
	
	/*Set video mode based on the console's region*/
	if(*(char *)0xbfc7ff52 == 'E')	GsSetVideoMode(320, 240, VMODE_PAL);
	else GsSetVideoMode(320, 240, VMODE_NTSC);

	/*Load font to VRAM*/
	InitText();
	
	/*Load controller buttons image*/
	GsImageFromTim(&PadImage, (void*)Buttons_tim);
	GsUploadImage(&PadImage);
	
	/*Load mouse image*/
	GsImageFromTim(&MouseImage, (void*)Mouse_tim);
	GsUploadImage(&MouseImage);

	SetVBlankHandler(VBlankHandler);
}

void VBlankHandler()
{
	VBlank = 1;
	IPENDING &= 0xFFFE;		/*Acknowledge VBlank in status register*/
}

void VSync()
{
	VBlank = 0;
	while(VBlank == 0);
}

void FlipBuffer()
{
	dbuf=!dbuf;
	GsSetDispEnvSimple(0, dbuf ? 0 : 256);
	GsSetDrawEnvSimple(0, dbuf ? 256 : 0, 320, 240);
}

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

void DrawTitle(char* softwareTitle, char* copyright)
{
	int FontX = 0;
	GsRectangle TopRect;

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
	
	GsPrintString(16, 16, 128, 128, 128, false, softwareTitle);
	
	FontX = GetPrintedStringWidth(false, "PORT 1");
	GsPrintString(80 - (FontX/2), 46, 128, 128, 128, false, "PORT 1");
	
	FontX = GetPrintedStringWidth(false, "PORT 2");
	GsPrintString(240 - (FontX/2), 46, 128, 128, 128, false, "PORT 2");
	
	GsPrintString(16, 210, 128, 128, 128, false, copyright);
}

void DrawMouse(int x, int y, int PadId, Controller* ctrl){
	GsSprite MouseSprite;
	char test[25];


	MouseSprite.x = x + 26;
	MouseSprite.y = y + 10;
	MouseSprite.w = 86;
	MouseSprite.h = 128;
	MouseSprite.u = 64;
	MouseSprite.v = 0;
	MouseSprite.r = MouseSprite.g = MouseSprite.b = 128;
	MouseSprite.cx = 320;
	MouseSprite.cy = 242;
	MouseSprite.tpage = 7;
	MouseSprite.attribute = COLORMODE(COLORMODE_8BPP);

	GsSortSimpleSprite(&MouseSprite);

	MouseSprite.v = 128;
	MouseSprite.h = 43;

	/*Left mouse button*/
	if(ctrl->Buttons & MOUSE_LB){
		MouseSprite.w = 45;
		GsSortSimpleSprite(&MouseSprite);
	}

	/*Right mouse button*/
	if(ctrl->Buttons & MOUSE_RB){
		MouseSprite.x = x + 26 + 45;
		MouseSprite.u = 64 + 45;
		MouseSprite.w = 41;
		GsSortSimpleSprite(&MouseSprite);
	}

	/*Draw cursor*/
	DrawPlus(ctrl->CursorX, ctrl->CursorY);
}

/*Draw controller at the specified coordinates*/
void DrawController(int x, int y, int PadId, Controller* ctrl)
{
    GsSprite PadSprite;

	int FontX = 0;
	int PressedOffset = 0;
	int AnalogEnabled = 0;
	unsigned short buttons = ctrl->Buttons;
	int StickX[2] = {0, 0};
	int StickY[2] = {0, 0};
	char TempString[50];
	
	/*Check what kind of controller is connected to the port*/
	switch(ctrl->Type)
	{
		default:
			FontX = GetPrintedStringWidth(false, "Not supported");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Not supported");
			return;
			
		case PAD_NONE:
			FontX = GetPrintedStringWidth(false, "Not connected");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Not connected");
			return;
			
        case PAD_MOUSE:
			FontX = GetPrintedStringWidth(false, "Mouse");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Mouse");
			DrawMouse(x, y, PadId, ctrl);
            return;

		case PAD_DIGITAL:
			FontX = GetPrintedStringWidth(false, "Digital");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Digital");
			break;
			
		case PAD_ANALOG:
			AnalogEnabled = 1;
			FontX = GetPrintedStringWidth(false, "Analog");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Analog");
			StickX[0] = ctrl->LeftStickX;
			StickY[0] = ctrl->LeftStickY;
			StickX[1] = ctrl->RightStickX;
			StickY[1] = ctrl->RightStickY;
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
		ctrl->BigMotor = 255;
	}
	else 
	{
		GsSortSimpleSprite(&PadSprite);
		ctrl->BigMotor = 0;
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
		ctrl->SmallMotor = 255;
	}
	else
	{
		GsSortSimpleSprite(&PadSprite);
		ctrl->SmallMotor = 0;
	}
	
	DrawPlus(x + 94 + (StickX[1]/8), y + 96 + (StickY[1]/8));
	sprintf(TempString, "X: %d\nY: %d", StickX[1], StickY[1]);
	GsPrintString(x + 78, y + 116, 128, 128, 128, false, TempString);
}