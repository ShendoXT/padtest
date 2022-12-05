#include <psx.h>
#include <stdio.h>
#include <string.h>

#include "include/text.h"
#include "include/controllers.h"
#include "include/graphics.h"

#define SOFTWARE_TITLE		"PadTest 1.1\n2022-12-05"
#define SOFTWARE_COPYRIGHT	"Authors: Shendo, ggrtk.\nPSXSDK by Tails92."

//#define DEBUG

/*Controller for each port*/
Controller Controllers[2];

int main()
{
	InitGraphics();
	InitPad();
	
	/*Set default values for both controllers*/
	ResetPad(&Controllers[0]);
	ResetPad(&Controllers[1]);

	/*Main loop of the application*/
	while(1)
	{
		/*Flip main and back buffer*/
		FlipBuffer();
	
		GsSortCls(0,0,0);
		
		DrawTitle(SOFTWARE_TITLE, SOFTWARE_COPYRIGHT);
		ReadPad(&Controllers[0], 0);
		ReadPad(&Controllers[1], 1);

		/*Draw controllers on the screen*/
		DrawController(10, 65, 0, &Controllers[0]);
		DrawController(170, 65, 1, &Controllers[1]);
		
		/*Draw primitives from the list*/
		GsDrawList();
		
		/*Wait for GPU to finish drawing*/
		while(GsIsDrawing());
		
		/*Wait for vertical sync*/
		VSync();

#ifdef DEBUG
		/*Return to loader (unirom) if 'x' is sent trough serial port*/
		if(SIOCheckInBuffer()){
			if(SIOReadByte() == 'x') __asm__("j 0x801B0000");
		}
#endif
	}
	
	return 0;
}
