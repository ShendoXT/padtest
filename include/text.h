#ifndef TEXT_H
#define TEXT_H

#include <string.h>

/*Upload text to VRAM*/
void InitText();

/*Return a length in pixels for the given string*/
int GetPrintedStringWidth(char monospace, char *string);

/*
 * Print a string at the specified coordinates in color
 * Supports \n - newline. If monospace is true each character is spaced 8px from the previous one.
 */
void GsPrintString(int x, int y, char Red, char Green, char Blue, char monospace, char *string);

#endif