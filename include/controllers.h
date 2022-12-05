#ifndef CONTROLLERS_H
#define CONTROLLERS_H

/*Copied from pad.c*/
#define PADSIO_DATA(x)      *((unsigned char*)(0x1f801040 + (x<<4)))
#define PADSIO_STATUS(x)    *((unsigned short*)(0x1f801044 + (x<<4)))
#define PADSIO_MODE(x)      *((unsigned short*)(0x1f801048 + (x<<4)))
#define PADSIO_CTRL(x)      *((unsigned short*)(0x1f80104a + (x<<4)))
#define PADSIO_BAUD(x)      *((unsigned short*)(0x1f80104e + (x<<4)))

/*Types of controllers*/
#define PAD_NONE			0xFF
#define PAD_DIGITAL         0x41
#define PAD_ANALOG          0x73
#define PAD_FLIGHT			0x53
#define PAD_MOUSE			0x12

/*Buttons*/
#define MOUSE_RB			0x4
#define MOUSE_LB			0x8

/*All properties of a controller*/
typedef struct
{	unsigned char Type;
	unsigned char ConfigState;
	unsigned char SmallMotor;
	unsigned char BigMotor;
	unsigned short Buttons;
	char LeftStickX;
	char LeftStickY;
	char RightStickX;
	char RightStickY;
	int CursorX;
	int CursorY;
}Controller;

/*Setup SIO port for controllers*/
void InitPad();

/*Send data to PAD_SIO*/
void SendData(int pad_n, unsigned char *in, unsigned char *out, int len);

/*Reset controller data to default values*/
void ResetPad(Controller* ctrl);

/*Read controller data from a single port*/
void ReadPad(Controller* ctrl, int pad_n);

/*
* Critical timing loop for the controllers
* BIOS actually does it this way
* Thanks to OpenBIOS for the info
*/
void BusyLoop(int count);

#endif