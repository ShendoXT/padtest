#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "controllers.h"

/*Set everything up*/
void InitGraphics();

/*This function gets called each VBlank*/
void VBlankHandler();

/*Wait for vertical sync*/
void VSync();

/*Flip main and back-buffer*/
void FlipBuffer();

/*Draw green plus (used for analog sticks)*/
void DrawPlus(int x, int y);

/*Draw title bar*/
void DrawTitle(char* softwareTitle, char* copyright);

/*Draw controller on screen with all the properties*/
void DrawController(int x, int y, int PadId, Controller* ctrl);

/*As the name implies draw mouse with all it's properties*/
void DrawMouse(int x, int y, int PadId, Controller* ctrl);

#endif