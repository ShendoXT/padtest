#include <psx.h>
#include "include/text.h"
#include "include/fontspace.h"
#include "include/font.h"

void InitText(){
    GsImage FontImage;

	/*Load a custom font and upload it to VRAM*/
	GsImageFromTim(&FontImage, FontTimData);
	GsUploadImage(&FontImage);
}

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
