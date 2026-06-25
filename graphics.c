#include <psx.h>
#include <stdio.h>
#include "include/graphics.h"
#include "include/text.h"

#include "images/buttons.h"
#include "images/mouse.h"

int dbuf = 0;
int VBlank = 0;
int RawPadDebugScroll = 0;
unsigned short PrevRawPadDebugButtons = 0;
int PrevPadDebugModeCombo = 0;
int PadDebugLogScroll = 0;
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
	
	GsPrintString(-1, 4, 128, 128, 128, false, softwareTitle);
	
	FontX = GetPrintedStringWidth(false, "PORT 1");
	GsPrintString(16, 28, 128, 128, 128, false, "PORT 1");
	
	FontX = GetPrintedStringWidth(false, "PORT 2");
	GsPrintString(320 - FontX - 16, 28, 128, 128, 128, false, "PORT 2");
	
	if(copyright)
	{
		GsPrintString(16, 210, 128, 128, 128, false, copyright);
	}
}

void UpdateRawPadDebugControls(Controller* ctrl, Controller* rawCtrl)
{
	unsigned short PressedButtons = 0;
	int ModeCombo = 0;
	int LogCount = 0;

	if(ctrl->Type == PAD_NONE) return;

	PressedButtons = ctrl->Buttons & ~PrevRawPadDebugButtons;
	PrevRawPadDebugButtons = ctrl->Buttons;

	ModeCombo = (ctrl->Buttons & PAD_L1) && (ctrl->Buttons & PAD_R1) && (ctrl->Buttons & PAD_TRIANGLE);
	if(ModeCombo && !PrevPadDebugModeCombo)
	{
		if(GetPadDebugMode() == PAD_DEBUG_OFF)
		{
			SetPadDebugMode(PAD_DEBUG_RAW);
			ResetRawPadData(rawCtrl);
		}
		else
		{
			SetPadDebugMode(PAD_DEBUG_OFF);
		}

		PadDebugLogScroll = 0;
		PrevPadDebugModeCombo = ModeCombo;
		return;
	}
	PrevPadDebugModeCombo = ModeCombo;

	if(GetPadDebugMode() == PAD_DEBUG_OFF) return;

	if((PressedButtons & PAD_TRIANGLE) && !ModeCombo)
	{
		if(IsPadDebugLogMode())
		{
			SetPadDebugMode(PAD_DEBUG_RAW);
		}
		else
		{
			SetPadDebugMode(PAD_DEBUG_LOG);
			PadDebugLogScroll = 0;
		}

		return;
	}

	if(IsPadDebugLogMode())
	{
		LogCount = GetPadDebugLogCount();

		if(PressedButtons & PAD_UP) PadDebugLogScroll--;
		if(PressedButtons & PAD_DOWN) PadDebugLogScroll++;
		if(PressedButtons & PAD_LEFT) PadDebugLogScroll -= 8;
		if(PressedButtons & PAD_RIGHT) PadDebugLogScroll += 8;
		if(PressedButtons & PAD_CROSS) ClearPadDebugLog();

		if(PadDebugLogScroll < 0) PadDebugLogScroll = 0;
		if(LogCount == 0) PadDebugLogScroll = 0;
		else if(PadDebugLogScroll >= LogCount) PadDebugLogScroll = LogCount - 1;

		return;
	}

	if(!IsPadDebugRawMode()) return;

	if(PressedButtons & PAD_UP)
	{
		SetRawPadProbe(PAD_RAW_PROBE_RUM40_01);
		ResetRawPadData(rawCtrl);
		FireRawPadProbe();
	}

	if(PressedButtons & PAD_RIGHT)
	{
		SetRawPadProbe(PAD_RAW_PROBE_RUM40_FF);
		ResetRawPadData(rawCtrl);
		FireRawPadProbe();
	}

	if(PressedButtons & PAD_DOWN)
	{
		SetRawPadProbe(PAD_RAW_PROBE_RUM7F_01);
		ResetRawPadData(rawCtrl);
		FireRawPadProbe();
	}

	if(PressedButtons & PAD_LEFT)
	{
		SetRawPadProbe(PAD_RAW_PROBE_RUM7F_FF);
		ResetRawPadData(rawCtrl);
		FireRawPadProbe();
	}

	if(PressedButtons & PAD_L1)
	{
		SetRawPadProbe(GetRawPadProbe() - 1);
		ResetRawPadData(rawCtrl);
	}

	if(PressedButtons & PAD_R1)
	{
		SetRawPadProbe(GetRawPadProbe() + 1);
		ResetRawPadData(rawCtrl);
	}

	if(PressedButtons & PAD_SELECT)
	{
		ResetRawPadData(rawCtrl);
	}

	if(PressedButtons & PAD_CROSS)
	{
		ResetRawPadData(rawCtrl);
		FireRawPadProbe();
	}

	if(PressedButtons & PAD_CIRCLE)
	{
		ResetRawPadData(rawCtrl);
		ToggleRawPadProbeStream();
	}

	if(RawPadDebugScroll < 0) RawPadDebugScroll = 0;
	if(RawPadDebugScroll > 0) RawPadDebugScroll = 0;
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

char DebugHexChar(unsigned char value)
{
	value &= 0x0F;
	return value < 10 ? '0' + value : 'A' + value - 10;
}

void DebugClearString(char* text)
{
	text[0] = 0;
}

void DebugAppendChar(char* text, char value)
{
	while(*text) text++;
	*text++ = value;
	*text = 0;
}

void DebugAppendString(char* text, char* value)
{
	while(*text) text++;
	while(*value) *text++ = *value++;
	*text = 0;
}

void DebugAppendHexByte(char* text, unsigned char value)
{
	DebugAppendChar(text, DebugHexChar(value >> 4));
	DebugAppendChar(text, DebugHexChar(value));
}

void DebugAppendHexWord(char* text, unsigned short value)
{
	DebugAppendHexByte(text, value >> 8);
	DebugAppendHexByte(text, value & 0xFF);
}

void DebugAppendDec(char* text, int value)
{
	char Scratch[6];
	int pos = 0;

	if(value < 0)
	{
		DebugAppendChar(text, '-');
		value = -value;
	}

	if(value == 0)
	{
		DebugAppendChar(text, '0');
		return;
	}

	while(value > 0 && pos < 5)
	{
		Scratch[pos++] = '0' + (value % 10);
		value /= 10;
	}

	while(pos > 0) DebugAppendChar(text, Scratch[--pos]);
}

void DebugAppendDec2(char* text, int value)
{
	if(value < 0) value = 0;
	if(value > 99)
	{
		DebugAppendDec(text, value);
		return;
	}

	DebugAppendChar(text, '0' + (value / 10));
	DebugAppendChar(text, '0' + (value % 10));
}

void DrawBarRect(int x, int y, int w, int h, int r, int g, int b)
{
	GsRectangle Rect;

	if(w <= 0) return;

	Rect.x = x;
	Rect.y = y;
	Rect.w = w;
	Rect.h = h;
	Rect.r = r;
	Rect.g = g;
	Rect.b = b;
	Rect.attribute = 0;
	GsSortRectangle(&Rect);
}

void DrawFishingStick(int x, int y, int StickX, int StickY, int Pressed)
{
	int CenterX = x + 18;
	int CenterY = y + 18;
	int PressedColor = Pressed ? 109 : 64;

	DrawBarRect(x + 1, y + 1, 34, 34, 24, 24, 24);
	DrawBarRect(CenterX, y + 1, 1, 34, 80, 80, 80);
	DrawBarRect(x + 1, CenterY, 34, 1, 80, 80, 80);
	DrawPlus(CenterX + (StickX / 8), CenterY + (StickY / 8));
	DrawBarRect(CenterX - 2, CenterY - 2, 5, 5, PressedColor, Pressed ? 193 : 64, Pressed ? 99 : 64);
}

void DrawSignedFishingBar(int x, int y, char* label, int value)
{
	char TempString[20];
	int BarX = x + 54;
	int BarW = 62;
	int CenterX = BarX + (BarW / 2);
	int FillW = 0;

	DebugClearString(TempString);
	DebugAppendString(TempString, label);
	DebugAppendChar(TempString, ':');
	DebugAppendDec(TempString, value);
	GsPrintString(x, y, 128, 128, 128, true, TempString);

	DrawBarRect(BarX, y + 2, BarW, 5, 32, 32, 32);
	DrawBarRect(CenterX, y + 1, 1, 7, 80, 80, 80);

	if(value < 0)
	{
		FillW = (-value * (BarW / 2)) / 128;
		DrawBarRect(CenterX - FillW, y + 2, FillW, 5, 109, 193, 99);
	}
	else
	{
		FillW = (value * (BarW / 2)) / 127;
		DrawBarRect(CenterX, y + 2, FillW, 5, 109, 193, 99);
	}
}

void DrawFishingSpeedBar(int x, int y, unsigned char value)
{
	char TempString[26];
	int BarX = x + 52;
	int BarW = 62;
	int FillW = value > 64 ? BarW : (value * BarW) / 64;

	GsPrintString(x, y, 128, 128, 128, false, "Rotation Speed");

	DebugClearString(TempString);
	DebugAppendString(TempString, "VAL:");
	DebugAppendDec(TempString, value);
	GsPrintString(x, y + 9, 128, 128, 128, true, TempString);

	DrawBarRect(BarX, y + 11, BarW, 5, 32, 32, 32);
	DrawBarRect(BarX, y + 11, FillW, 5, 109, 193, 99);
}

void DrawFishingMode2Readout(int x, int y, Controller* ctrl)
{
	DrawSignedFishingBar(x + 4, y, "X", ctrl->MotionX);
	DrawSignedFishingBar(x + 4, y + 12, "Y", ctrl->MotionY);
	DrawSignedFishingBar(x + 4, y + 24, "Z", ctrl->MotionZ);
	DrawFishingSpeedBar(x + 4, y + 36, ctrl->ReelRate);
}

void DrawNegconDigitalValue(int x, int y, char* label, int pressed)
{
	char TempString[20];

	DebugClearString(TempString);
	DebugAppendString(TempString, label);
	DebugAppendChar(TempString, ':');
	DebugAppendChar(TempString, pressed ? '1' : '0');
	GsPrintString(x, y, 128, 128, 128, true, TempString);
}

void DrawNegconAnalogBar(int x, int y, char* label, unsigned char value)
{
	char TempString[20];
	int BarX = x + 46;
	int BarW = 44;
	int FillW = ((int)value * BarW) / 255;

	GsPrintString(x, y, 128, 128, 128, true, label);
	DrawBarRect(BarX, y + 2, BarW, 5, 32, 32, 32);
	DrawBarRect(BarX, y + 2, FillW, 5, 109, 193, 99);

	DebugClearString(TempString);
	DebugAppendDec(TempString, value);
	GsPrintString(x + 96, y, 128, 128, 128, true, TempString);
}

void DrawNegcon(int x, int y, int PadId, Controller* ctrl)
{
	unsigned short buttons = ctrl->Buttons;

	(void)PadId;

	if(ctrl->NegconLayout == PAD_NEGCON_ULTRA_RACER)
	{
		GsPrintString(x + 36, y + 8, 96, 96, 96, true, "Ultra Racer");
	}

	DrawNegconDigitalValue(x + 4, y + 20, "UP", buttons & PAD_UP);
	DrawNegconDigitalValue(x + 4, y + 30, "RIGHT", buttons & PAD_RIGHT);
	DrawNegconDigitalValue(x + 4, y + 40, "DOWN", buttons & PAD_DOWN);
	DrawNegconDigitalValue(x + 4, y + 50, "LEFT", buttons & PAD_LEFT);
	DrawNegconDigitalValue(x + 74, y + 20, "START", buttons & PAD_START);
	DrawNegconDigitalValue(x + 74, y + 30, "A", buttons & PAD_CIRCLE);
	DrawNegconDigitalValue(x + 74, y + 40, "B", buttons & PAD_TRIANGLE);

	DrawNegconDigitalValue(x + 4, y + 64, "L1", buttons & PAD_L1);
	DrawNegconDigitalValue(x + 4, y + 74, "L2", buttons & PAD_L2);
	DrawNegconDigitalValue(x + 74, y + 64, "R1", buttons & PAD_R1);
	DrawNegconDigitalValue(x + 74, y + 74, "R2", buttons & PAD_R2);

	DrawNegconAnalogBar(x + 4, y + 92, "TWIST", ctrl->NegconTwist);
	DrawNegconAnalogBar(x + 4, y + 106, "I", ctrl->NegconI);
	DrawNegconAnalogBar(x + 4, y + 118, "II", ctrl->NegconII);
	DrawNegconAnalogBar(x + 4, y + 130, "L", ctrl->NegconL);
}

void DrawRawPadByte(int x, int y, unsigned char value, unsigned char changed)
{
	char TempString[4];
	int Red = changed ? 109 : 128;
	int Green = changed ? 193 : 128;
	int Blue = changed ? 99 : 128;

	DebugClearString(TempString);
	DebugAppendHexByte(TempString, value);
	GsPrintString(x, y, Red, Green, Blue, true, TempString);
}

void DrawRawPadByteRow(int x, int y, char* label, unsigned char* data, unsigned char* changed, int first, int len)
{
	char TempString[5];

	GsPrintString(x, y, 128, 128, 128, true, label);

	for(int col = 0; col < 4; col++)
	{
		int index = first + col;

		if(index < len)
		{
			DrawRawPadByte(x + 32 + (col * 24), y, data[index], changed ? changed[index] : 0);
		}
		else
		{
			DebugClearString(TempString);
			DebugAppendString(TempString, "--");
			GsPrintString(x + 32 + (col * 24), y, 64, 64, 64, true, TempString);
		}
	}
}

char* GetPadLogCommandName(PadDebugLogEntry* entry)
{
	switch(entry->Command)
	{
		case 0x42:
			return "POLL42";

		case 0x43:
			return entry->Tx[3] ? "CFG ON" : "CFGOFF";

		case 0x44:
			return "ANALOG";

		case 0x45:
			return "STAT45";

		case 0x46:
			return "QUERY1";

		case 0x47:
			return "QUERY2";

		case 0x4C:
			return "MODE4C";

		case 0x4D:
			return "RUMBLE";

	}

	return "UNKNOWN";
}

void DrawPadDebugLog(int x, int y)
{
	char TempString[64];
	int FontX = 0;
	int LogCount = GetPadDebugLogCount();
	PadDebugLogEntry* Entry = GetPadDebugLogEntry(PadDebugLogScroll);

	FontX = GetPrintedStringWidth(false, "Pad bus log");
	GsPrintString(160 - (FontX/2), y, 128, 128, 128, false, "Pad bus log");

	GsPrintString(x, y + 14, 128, 128, 128, true, "U/D STEP L/R PAGE");
	GsPrintString(x, y + 22, 128, 128, 128, true, "X CLR TRI RAW");
	GsPrintString(x, y + 30, 128, 128, 128, true, "L1+R1+TRI EXIT");

	DebugClearString(TempString);
	DebugAppendString(TempString, "ENTRY:");
	DebugAppendDec2(TempString, LogCount ? PadDebugLogScroll + 1 : 0);
	DebugAppendChar(TempString, '/');
	DebugAppendDec(TempString, LogCount);
	GsPrintString(x, y + 44, 128, 128, 128, true, TempString);

	if(!Entry)
	{
		GsPrintString(x, y + 60, 128, 128, 128, true, "No logged transfers");
		return;
	}

	DebugClearString(TempString);
	DebugAppendChar(TempString, 'P');
	DebugAppendDec(TempString, Entry->Port + 1);
	DebugAppendString(TempString, " CMD:");
	DebugAppendHexByte(TempString, Entry->Command);
	DebugAppendChar(TempString, ' ');
	DebugAppendString(TempString, GetPadLogCommandName(Entry));
	GsPrintString(x, y + 56, 128, 128, 128, true, TempString);

	DebugClearString(TempString);
	DebugAppendString(TempString, "LEN:");
	DebugAppendDec2(TempString, Entry->Length);
	DebugAppendString(TempString, " RTYPE:");
	DebugAppendHexByte(TempString, Entry->ResponseType);
	GsPrintString(x, y + 64, 128, 128, 128, true, TempString);

	GsPrintString(x, y + 80, 128, 128, 128, true, "TX");
	for(int row = 0; row < 4; row++)
	{
		DebugClearString(TempString);
		DebugAppendDec2(TempString, row * 4);
		DebugAppendChar(TempString, ':');
		DrawRawPadByteRow(x, y + 90 + (row * 8), TempString, Entry->Tx, 0, row * 4, Entry->Length);
	}

	GsPrintString(x, y + 132, 128, 128, 128, true, "RX");
	for(int row = 0; row < 4; row++)
	{
		DebugClearString(TempString);
		DebugAppendDec2(TempString, row * 4);
		DebugAppendChar(TempString, ':');
		DrawRawPadByteRow(x, y + 142 + (row * 8), TempString, Entry->Rx, 0, row * 4, Entry->Length);
	}

	GsPrintString(x + 164, y + 56, 128, 128, 128, true, "Recent");
	for(int row = 0; row < 12; row++)
	{
		PadDebugLogEntry* Recent = GetPadDebugLogEntry(row);
		if(!Recent) break;

		DebugClearString(TempString);
		DebugAppendDec2(TempString, row);
		DebugAppendString(TempString, " P");
		DebugAppendDec(TempString, Recent->Port + 1);
		DebugAppendChar(TempString, ' ');
		DebugAppendHexByte(TempString, Recent->Command);
		DebugAppendChar(TempString, '>');
		DebugAppendHexByte(TempString, Recent->ResponseType);
		GsPrintString(x + 164, y + 68 + (row * 8), 128, 128, 128, true, TempString);
	}
}

void DrawRawPadDebugHelp(int x, int y)
{
	int FontX = GetPrintedStringWidth(false, "Debug controls");

	GsPrintString(x + 70 - (FontX/2), y, 128, 128, 128, false, "Debug controls");

	GsPrintString(x + 4, y + 18, 128, 128, 128, true, "P2 CONTROLS");
	GsPrintString(x + 4, y + 32, 128, 128, 128, true, "L1/R1 PROBE");
	GsPrintString(x + 4, y + 44, 128, 128, 128, true, "CROSS FIRE ONCE");
	GsPrintString(x + 4, y + 56, 128, 128, 128, true, "CIRCLE STREAM");
	GsPrintString(x + 4, y + 68, 128, 128, 128, true, "SELECT CLR MINMAX");
	GsPrintString(x + 4, y + 80, 128, 128, 128, true, "TRIANGLE RAW/LOG");
	GsPrintString(x + 4, y + 92, 128, 128, 128, true, "L1+R1+TRI EXIT");

	GsPrintString(x + 4, y + 106, 128, 128, 128, true, "D-PAD RUMBLE:");
	GsPrintString(x + 4, y + 118, 128, 128, 128, true, "U 40/01  R 40/FF");
	GsPrintString(x + 4, y + 130, 128, 128, 128, true, "D 7F/01  L 7F/FF");
	GsPrintString(x + 4, y + 154, 128, 128, 128, true, "LOG VIEW");
	GsPrintString(x + 4, y + 168, 128, 128, 128, true, "UP/DOWN STEP");
	GsPrintString(x + 4, y + 180, 128, 128, 128, true, "LEFT/RIGHT PAGE");
	GsPrintString(x + 4, y + 192, 128, 128, 128, true, "CROSS CLEAR LOG");
}

void DrawRawPadDebug(int x, int y, int PadId, Controller* ctrl)
{
	char TempString[64];
	int FontX = 0;
	int ScrollY = PadId == 0 ? RawPadDebugScroll : 0;

	if(PadId != 0)
	{
		DrawRawPadDebugHelp(x, y);
		return;
	}

	FontX = GetPrintedStringWidth(false, "Raw debug");
	GsPrintString(x + 70 - (FontX/2), y, 128, 128, 128, false, "Raw debug");
	y += 10;

	y -= ScrollY;

	DebugClearString(TempString);
	DebugAppendString(TempString, "SEL:");
	DebugAppendDec(TempString, GetRawPadProbe());
	DebugAppendChar(TempString, ' ');
	DebugAppendString(TempString, GetRawPadProbeName());
	GsPrintString(x + 4, y, 128, 128, 128, true, TempString);

	if(ctrl->RawIsProbeResponse)
	{
		if(IsRawPadProbeStreamEnabled())
		{
			DebugClearString(TempString);
			DebugAppendString(TempString, "SRC:STREAM");
		}
		else
		{
			DebugClearString(TempString);
			DebugAppendString(TempString, "SRC:SHOT H:");
			DebugAppendDec2(TempString, ctrl->RawHoldFrames);
		}
	}
	else
	{
		DebugClearString(TempString);
		DebugAppendString(TempString, "SRC:LIVE42");
	}
	GsPrintString(x + 4, y + 10, 128, 128, 128, true, TempString);

	DebugClearString(TempString);
	DebugAppendString(TempString, "TYPE:");
	DebugAppendHexByte(TempString, ctrl->RawType);
	DebugAppendString(TempString, " LEN:");
	DebugAppendDec2(TempString, ctrl->RawLength);
	GsPrintString(x + 4, y + 20, 128, 128, 128, true, TempString);

	DebugClearString(TempString);
	DebugAppendString(TempString, "BTN:");
	DebugAppendHexWord(TempString, ctrl->Buttons);
	GsPrintString(x + 4, y + 30, 128, 128, 128, true, TempString);

	GsPrintString(x + 4, y + 42, 128, 128, 128, true, "RAW");
	for(int row = 0; row < 4; row++)
	{
		DebugClearString(TempString);
		DebugAppendDec2(TempString, row * 4);
		DebugAppendChar(TempString, ':');
		DrawRawPadByteRow(x + 4, y + 50 + (row * 8), TempString, ctrl->RawData, ctrl->RawChanged, row * 4, ctrl->RawLength);
	}

	GsPrintString(x + 4, y + 86, 128, 128, 128, true, "MIN");
	for(int row = 0; row < 4; row++)
	{
		DebugClearString(TempString);
		DebugAppendDec2(TempString, row * 4);
		DebugAppendChar(TempString, ':');
		DrawRawPadByteRow(x + 4, y + 94 + (row * 8), TempString, ctrl->RawMin, 0, row * 4, ctrl->RawLength);
	}

	GsPrintString(x + 4, y + 130, 128, 128, 128, true, "MAX");
	for(int row = 0; row < 4; row++)
	{
		DebugClearString(TempString);
		DebugAppendDec2(TempString, row * 4);
		DebugAppendChar(TempString, ':');
		DrawRawPadByteRow(x + 4, y + 138 + (row * 8), TempString, ctrl->RawMax, 0, row * 4, ctrl->RawLength);
	}
}

/*Draw controller at the specified coordinates*/
void DrawController(int x, int y, int PadId, Controller* ctrl)
{
    GsSprite PadSprite;

	int FontX = 0;
	int PressedOffset = 0;
	int AnalogEnabled = 0;
	int FishingMode2 = 0;
	unsigned short buttons = ctrl->Buttons;
	int StickX[2] = {0, 0};
	int StickY[2] = {0, 0};
	char TempString[50];

#ifdef RAW_PAD_DEBUG
	if(ctrl->Type != PAD_NONE)
	{
		DrawRawPadDebug(x, y, PadId, ctrl);
		return;
	}
#endif

	if(PadId == 0 && IsPadDebugRawMode())
	{
		DrawRawPadDebug(x, y, PadId, ctrl);
		return;
	}
	
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

		case PAD_NEGCON:
			FontX = GetPrintedStringWidth(false, "NeGcon");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "NeGcon");
			DrawNegcon(x, y, PadId, ctrl);
			return;

		case PAD_DIGITAL:
			FontX = GetPrintedStringWidth(false, "ST+SL EXT POLL");
			GsPrintString(x + 70 - (FontX/2), 46, 96, 96, 96, false, "ST+SL EXT POLL");
			FontX = GetPrintedStringWidth(false, "Digital");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Digital");
			break;

		case PAD_FISHING:
			FontX = GetPrintedStringWidth(false, "Fishing");
			GsPrintString(x + 70 - (FontX/2), 56, 128, 128, 128, false, "Fishing");

			if(ctrl->FishingMode == PAD_FISHING_MODE2)
			{
				FishingMode2 = 1;
				StickX[0] = ctrl->LeftStickX;
				StickY[0] = ctrl->LeftStickY;
			}
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
	
	
	if(FishingMode2)
	{
		DrawFishingStick(x, y + 48, StickX[0], StickY[0], buttons & PAD_LANALOGB);

		PadSprite.u = 0;
		PadSprite.v = 48;
		PadSprite.x = x + 20;
		PadSprite.y = y + 58;
	}
	else
	{
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
	}
	
	
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
	
	if(FishingMode2)
	{
		if(buttons & PAD_LANALOGB)
		{
			ctrl->BigMotor = 255;
		}
		else
		{
			ctrl->BigMotor = 0;
		}

		ctrl->SmallMotor = 0;
		DrawFishingMode2Readout(x, y + 92, ctrl);
		return;
	}

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
