#ifndef CONTROLLERS_H
#define CONTROLLERS_H

/*Copied from pad.c*/
#define PADSIO_DATA(x)      *((unsigned char*)(0x1f801040 + (x<<4)))
#define PADSIO_STATUS(x)    *((unsigned short*)(0x1f801044 + (x<<4)))
#define PADSIO_MODE(x)      *((unsigned short*)(0x1f801048 + (x<<4)))
#define PADSIO_CTRL(x)      *((unsigned short*)(0x1f80104a + (x<<4)))
#define PADSIO_BAUD(x)      *((unsigned short*)(0x1f80104e + (x<<4)))

/*Uncomment to show raw packet debug data for all controllers*/
/*#define RAW_PAD_DEBUG*/

#define PAD_DIGITAL_LENGTH	5
#define PAD_POLL_LENGTH		9
#define PAD_RAW_LENGTH		16
#define PAD_RAW_PROBE_COUNT	12
#define PAD_RAW_PROBE_RUM40_01	8
#define PAD_RAW_PROBE_RUM40_FF	9
#define PAD_RAW_PROBE_RUM7F_01	10
#define PAD_RAW_PROBE_RUM7F_FF	11
#define PAD_RAW_HOLD_FRAMES	60
#define PAD_DEBUG_LOG_COUNT	128
#define PAD_DEBUG_LOG_BYTES	16
#define PAD_FISHING_LENGTH	13

/*Runtime debug modes*/
#define PAD_DEBUG_OFF		0
#define PAD_DEBUG_RAW		1
#define PAD_DEBUG_LOG		2

/*Bass Landing/Fishing controller modes*/
#define PAD_FISHING_MODE1	1
#define PAD_FISHING_MODE2	2
#define PAD_FISHING_ACTIVATE_FRAMES	30

/*NeGcon-compatible controller layouts*/
#define PAD_NEGCON_STANDARD		0
#define PAD_NEGCON_ULTRA_RACER	1

/*Types of controllers*/
#define PAD_NONE			0xFF
#define PAD_DIGITAL         0x41
#define PAD_NEGCON          0x23
#define PAD_ANALOG          0x73
#define PAD_FLIGHT			0x53
#define PAD_MOUSE			0x12
#define PAD_FISHING			0xE5

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
	unsigned char NegconTwist;
	unsigned char NegconI;
	unsigned char NegconII;
	unsigned char NegconL;
	unsigned char NegconLayout;
	char MotionX;
	char MotionY;
	char MotionZ;
	unsigned char ReelRate;
	unsigned char FishingMode;
	unsigned char FishingActivateHeld;
	unsigned char FishingActivateFrames;
	unsigned char RawData[PAD_RAW_LENGTH];
	unsigned char PrevRawData[PAD_RAW_LENGTH];
	unsigned char RawChanged[PAD_RAW_LENGTH];
	unsigned char RawMin[PAD_RAW_LENGTH];
	unsigned char RawMax[PAD_RAW_LENGTH];
	unsigned char RawLength;
	unsigned char RawType;
	unsigned char RawProbe;
	unsigned char RawHoldFrames;
	unsigned char RawIsProbeResponse;
	int CursorX;
	int CursorY;
}Controller;

/*Logged controller bus transaction*/
typedef struct
{
	unsigned char Port;
	unsigned char Length;
	unsigned char Command;
	unsigned char ResponseType;
	unsigned char Tx[PAD_DEBUG_LOG_BYTES];
	unsigned char Rx[PAD_DEBUG_LOG_BYTES];
}PadDebugLogEntry;

/*Setup SIO port for controllers*/
void InitPad();

/*Send data to PAD_SIO*/
void SendData(int pad_n, unsigned char *in, unsigned char *out, int len);

/*Clear logged controller bus transactions*/
void ClearPadDebugLog();

/*Get number of logged controller bus transactions*/
int GetPadDebugLogCount();

/*Get a logged transaction by newest-first offset*/
PadDebugLogEntry* GetPadDebugLogEntry(int offset);

/*Pause or resume controller transaction logging*/
void SetPadDebugLogPaused(int paused);

/*Set the runtime debug mode*/
void SetPadDebugMode(int mode);

/*Get the runtime debug mode*/
int GetPadDebugMode();

/*Check whether raw debug polling is active for port 1*/
int IsPadDebugRawMode();

/*Check whether the transaction log view is active*/
int IsPadDebugLogMode();

/*Reset controller data to default values*/
void ResetPad(Controller* ctrl);

/*Read controller data from a single port*/
void ReadPad(Controller* ctrl, int pad_n);

/*Change the command template used for port 1 raw probing*/
void SetRawPadProbe(int probe);

/*Get the current port 1 raw probe command template*/
int GetRawPadProbe();

/*Get a short name for a raw probe command template*/
char* GetRawPadProbeName();

/*Get a short name for a raw probe command template by index*/
char* GetRawPadProbeNameByIndex(int probe);

/*Capture the selected raw probe command once*/
void FireRawPadProbe();

/*Toggle streaming of the selected raw probe command*/
void ToggleRawPadProbeStream();

/*Check whether selected raw probe streaming is active*/
int IsRawPadProbeStreamEnabled();

/*Reset raw min/max/change history for a controller*/
void ResetRawPadData(Controller* ctrl);

/*
* Critical timing loop for the controllers
* BIOS actually does it this way
* Thanks to OpenBIOS for the info
*/
void BusyLoop(int count);

#endif
