#include <psx.h>
#include <string.h>
#include "include/controllers.h"

typedef struct
{
	char* Name;
	unsigned char Data[PAD_RAW_LENGTH];
}RawPadProbe;

RawPadProbe RawPadProbes[PAD_RAW_PROBE_COUNT] = {
	{"POLL42", {1, 0x42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"CFG ON", {1, 0x43, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"CFGOFF", {1, 0x43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"ANALOG", {1, 0x44, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"STAT45", {1, 0x45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"QUERY1", {1, 0x46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"QUERY2", {1, 0x47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"MODE4C", {1, 0x4C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

int RawPadProbeIndex = 0;
int RawPadProbeFire = 0;
int RawPadProbeStream = 0;
int PadDebugMode = PAD_DEBUG_OFF;
PadDebugLogEntry PadDebugLog[PAD_DEBUG_LOG_COUNT];
int PadDebugLogHead = 0;
int PadDebugLogCount = 0;
int PadDebugLogPaused = 0;

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

void SetRawPadProbe(int probe)
{
	while(probe < 0) probe += PAD_RAW_PROBE_COUNT;
	RawPadProbeIndex = probe % PAD_RAW_PROBE_COUNT;
}

char* GetRawPadProbeNameByIndex(int probe)
{
	while(probe < 0) probe += PAD_RAW_PROBE_COUNT;
	return RawPadProbes[probe % PAD_RAW_PROBE_COUNT].Name;
}

int GetRawPadProbe()
{
	return RawPadProbeIndex;
}

char* GetRawPadProbeName()
{
	return GetRawPadProbeNameByIndex(RawPadProbeIndex);
}

void FireRawPadProbe()
{
	RawPadProbeFire = 1;
}

void ToggleRawPadProbeStream()
{
	RawPadProbeStream = !RawPadProbeStream;
}

int IsRawPadProbeStreamEnabled()
{
	return RawPadProbeStream;
}

void SetPadDebugMode(int mode)
{
	if(mode < PAD_DEBUG_OFF || mode > PAD_DEBUG_LOG) mode = PAD_DEBUG_OFF;
	PadDebugMode = mode;
	PadDebugLogPaused = PadDebugMode == PAD_DEBUG_LOG;

	if(PadDebugMode == PAD_DEBUG_OFF)
	{
		RawPadProbeFire = 0;
		RawPadProbeStream = 0;
	}
}

int GetPadDebugMode()
{
	return PadDebugMode;
}

int IsPadDebugRawMode()
{
	return PadDebugMode == PAD_DEBUG_RAW;
}

int IsPadDebugLogMode()
{
	return PadDebugMode == PAD_DEBUG_LOG;
}

void ClearPadDebugLog()
{
	memset(PadDebugLog, 0, sizeof(PadDebugLog));
	PadDebugLogHead = 0;
	PadDebugLogCount = 0;
}

int GetPadDebugLogCount()
{
	return PadDebugLogCount;
}

PadDebugLogEntry* GetPadDebugLogEntry(int offset)
{
	int index = 0;

	if(offset < 0 || offset >= PadDebugLogCount) return 0;

	index = PadDebugLogHead - 1 - offset;
	while(index < 0) index += PAD_DEBUG_LOG_COUNT;

	return &PadDebugLog[index % PAD_DEBUG_LOG_COUNT];
}

void SetPadDebugLogPaused(int paused)
{
	PadDebugLogPaused = paused;
}

void LogPadTransfer(int pad_n, unsigned char *tx, unsigned char *rx, int len)
{
	PadDebugLogEntry* entry = &PadDebugLog[PadDebugLogHead];

	if(PadDebugLogPaused) return;

	memset(entry, 0, sizeof(PadDebugLogEntry));
	entry->Port = pad_n;
	entry->Length = len > PAD_DEBUG_LOG_BYTES ? PAD_DEBUG_LOG_BYTES : len;
	entry->Command = len > 1 ? tx[1] : 0;
	entry->ResponseType = len > 1 ? rx[1] : 0;

	for(int x = 0; x < entry->Length; x++)
	{
		entry->Tx[x] = tx[x];
		entry->Rx[x] = rx[x];
	}

	PadDebugLogHead = (PadDebugLogHead + 1) % PAD_DEBUG_LOG_COUNT;
	if(PadDebugLogCount < PAD_DEBUG_LOG_COUNT) PadDebugLogCount++;
}

void ResetRawPadData(Controller* ctrl)
{
	for(int x = 0; x < PAD_RAW_LENGTH; x++)
	{
		ctrl->RawData[x] = 0;
		ctrl->PrevRawData[x] = 0;
		ctrl->RawChanged[x] = 0;
		ctrl->RawMin[x] = 0xFF;
		ctrl->RawMax[x] = 0;
	}

	ctrl->RawLength = 0;
	ctrl->RawType = PAD_NONE;
	ctrl->RawProbe = RawPadProbeIndex;
	ctrl->RawHoldFrames = 0;
	ctrl->RawIsProbeResponse = 0;
}

int ShouldUseRawPolling(Controller* ctrl, int pad_n)
{
#ifdef RAW_PAD_DEBUG
	if(ctrl->Type != PAD_NONE) return 1;
#endif

	(void)ctrl;

	return pad_n == 0 && IsPadDebugRawMode();
}

void CaptureRawPadData(Controller* ctrl, unsigned char *data, int len)
{
	if(len > PAD_RAW_LENGTH) len = PAD_RAW_LENGTH;

	for(int x = 0; x < PAD_RAW_LENGTH; x++)
	{
		unsigned char value = x < len ? data[x] : 0;

		ctrl->PrevRawData[x] = ctrl->RawData[x];
		ctrl->RawData[x] = value;
		ctrl->RawChanged[x] = ctrl->RawData[x] != ctrl->PrevRawData[x];

		if(x < len)
		{
			if(value < ctrl->RawMin[x]) ctrl->RawMin[x] = value;
			if(value > ctrl->RawMax[x]) ctrl->RawMax[x] = value;
		}
	}

	ctrl->RawLength = len;
	ctrl->RawType = ctrl->RawData[1];
}

int IsFishingPacket(unsigned char *data)
{
	return data[1] == PAD_FISHING && data[2] == 0x5A;
}

int IsFishingMode1Packet(Controller* ctrl, unsigned char *data)
{
	return ctrl->Type == PAD_FISHING &&
		ctrl->FishingMode == PAD_FISHING_MODE1 &&
		(data[1] == PAD_FISHING || data[1] == PAD_DIGITAL) &&
		data[2] == 0x5A;
}

void MapFishingPacket(Controller* ctrl, unsigned char *data, int mode)
{
	ctrl->Type = PAD_FISHING;
	ctrl->FishingMode = mode;
	ctrl->Buttons = ~((data[3] << 8) | data[4]);

	if(mode == PAD_FISHING_MODE2)
	{
		ctrl->FishingActivateFrames = 0;
		ctrl->LeftStickX = data[7] - 128;
		ctrl->LeftStickY = data[8] - 128;
		ctrl->MotionX = data[9] - 128;
		ctrl->MotionY = data[11] - 128;
		ctrl->MotionZ = data[10] - 128;
		ctrl->ReelRate = data[12];
	}
	else
	{
		ctrl->LeftStickX = 0;
		ctrl->LeftStickY = 0;
		ctrl->MotionX = 0;
		ctrl->MotionY = 0;
		ctrl->MotionZ = 0;
		ctrl->ReelRate = 0;
	}
}

void StartFishingMode2Activation(Controller* ctrl, int pad_n)
{
	unsigned char ReceivedData[PAD_RAW_LENGTH];
	unsigned char ConfigStart[PAD_RAW_LENGTH] = {1, 0x43, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	memset(&ReceivedData, 0, sizeof(ReceivedData));
	SendData(pad_n, ConfigStart, ReceivedData, PAD_RAW_LENGTH);
	CaptureRawPadData(ctrl, ReceivedData, PAD_RAW_LENGTH);

	ctrl->FishingActivateFrames = PAD_FISHING_ACTIVATE_FRAMES;
}

void HandleFishingActivationHotkey(Controller* ctrl, int pad_n)
{
	int ComboPressed = (ctrl->Buttons & PAD_START) && (ctrl->Buttons & PAD_SELECT);

	if(ComboPressed && !ctrl->FishingActivateHeld)
	{
		StartFishingMode2Activation(ctrl, pad_n);
	}

	ctrl->FishingActivateHeld = ComboPressed;
}

void ResetPad(Controller* ctrl)
{
	/*Clear controller data*/
	memset(ctrl, 0, sizeof(Controller));
	ResetRawPadData(ctrl);
	
	/*Treat controller as disconnected*/
	ctrl->Type = PAD_NONE;

	/*Reset cursor to center of the screen*/
	ctrl->CursorX = 160;
	ctrl->CursorY = 120;
}

void SendData(int pad_n, unsigned char *in, unsigned char *out, int len)
{
	unsigned char *tx_start = in;
	unsigned char *rx_start = out;

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

	LogPadTransfer(pad_n, tx_start, rx_start, len);
	
	PADSIO_CTRL(0) = 0;
}

void ReadPad(Controller* ctrl, int pad_n)
{
	unsigned char DataToSend[PAD_RAW_LENGTH] =  {1, 0x42, 0, 0, 0, 0, 0, 0, 0};	/*Standard data polling command*/
	unsigned char ReceivedData[16];
	int PollLength = ctrl->Type == PAD_NONE ? PAD_POLL_LENGTH : PAD_DIGITAL_LENGTH;
	int RawPolling = 0;
	int ProbeResponse = 0;
	int ProbeHold = 0;
	int SentProbe = 0;
	int FishingActivationProbe = 0;
	
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
			RawPolling = ShouldUseRawPolling(ctrl, pad_n);

			if(pad_n == 0 && ctrl->RawHoldFrames > 0)
			{
				ctrl->RawHoldFrames--;
				break;
			}

			DataToSend[3] = ctrl->SmallMotor;
			DataToSend[4] = ctrl->BigMotor;

			if(ctrl->Type == PAD_ANALOG || ctrl->Type == PAD_MOUSE)
			{
				PollLength = PAD_POLL_LENGTH;
			}

			if(ctrl->Type == PAD_FISHING)
			{
				if(ctrl->FishingMode == PAD_FISHING_MODE2)
				{
					PollLength = PAD_FISHING_LENGTH;
					memcpy(DataToSend, RawPadProbes[2].Data, PAD_RAW_LENGTH);
				}
			}

			if(ctrl->FishingActivateFrames > 0 && ctrl->FishingMode != PAD_FISHING_MODE2)
			{
				PollLength = PAD_RAW_LENGTH;
				memcpy(DataToSend, RawPadProbes[2].Data, PAD_RAW_LENGTH);
				FishingActivationProbe = 1;
			}

			if(RawPolling)
			{
				PollLength = PAD_RAW_LENGTH;

				if(pad_n == 0 && (RawPadProbeFire || RawPadProbeStream))
				{
					SentProbe = RawPadProbeIndex;
					memcpy(DataToSend, RawPadProbes[RawPadProbeIndex].Data, PAD_RAW_LENGTH);
					ProbeResponse = 1;
					ProbeHold = RawPadProbeFire && !RawPadProbeStream;
					RawPadProbeFire = 0;
				}
				else
				{
					memcpy(DataToSend, RawPadProbes[0].Data, PAD_RAW_LENGTH);
				}
			}
	
			/*Read button status*/
			SendData(pad_n, DataToSend, ReceivedData, PollLength);
			CaptureRawPadData(ctrl, ReceivedData, PollLength);

			ctrl->RawProbe = ProbeResponse ? SentProbe : 0;
			ctrl->RawIsProbeResponse = ProbeResponse;
			ctrl->RawHoldFrames = RawPolling && ProbeResponse && ProbeHold ? PAD_RAW_HOLD_FRAMES : 0;

			if(RawPolling)
			{
				if(ReceivedData[1] != PAD_NONE)
				{
					ctrl->Buttons = ~((ReceivedData[3] << 8) | ReceivedData[4]);
				}
				break;
			}

			if(FishingActivationProbe)
			{
				if(IsFishingPacket(ReceivedData))
				{
					MapFishingPacket(ctrl, ReceivedData, PAD_FISHING_MODE2);
					ctrl->ConfigState = 6;
				}
				else if(ctrl->FishingActivateFrames > 0)
				{
					ctrl->FishingActivateFrames--;
				}

				break;
			}

			/*Check if anything is connected (line not floating high)*/
			if(ReceivedData[1] == PAD_NONE)
			{
				ResetPad(ctrl);
			}
			else if(IsFishingPacket(ReceivedData))
			{
				int FishingMode = ctrl->FishingMode == PAD_FISHING_MODE2 ? PAD_FISHING_MODE2 : PAD_FISHING_MODE1;

				MapFishingPacket(ctrl, ReceivedData, FishingMode);
				ctrl->ConfigState = 6;

				if(ctrl->FishingMode == PAD_FISHING_MODE1)
				{
					HandleFishingActivationHotkey(ctrl, pad_n);
				}
			}
			else if(IsFishingMode1Packet(ctrl, ReceivedData))
			{
				MapFishingPacket(ctrl, ReceivedData, PAD_FISHING_MODE1);
				ctrl->ConfigState = 6;

				HandleFishingActivationHotkey(ctrl, pad_n);
			}
			else
			{
				/*Check if controller type changed from previous reading*/
				if(ctrl->Type != ReceivedData[1])
				{
#ifdef RAW_PAD_DEBUG
					ctrl->ConfigState = 6;
#else
					if(!(pad_n == 0 && IsPadDebugRawMode()) &&
						ReceivedData[1] == PAD_ANALOG)
					{
						ctrl->ConfigState = 0;
					}
					else ctrl->ConfigState = 6;
#endif
				}
			
				/*Store type*/
				ctrl->Type = ReceivedData[1];
				ctrl->FishingMode = 0;
				ctrl->MotionX = 0;
				ctrl->MotionY = 0;
				ctrl->MotionZ = 0;
				ctrl->ReelRate = 0;
		
				/*Get digital buttons*/
				if(PollLength >= PAD_DIGITAL_LENGTH)
				{
					ctrl->Buttons = ~((ReceivedData[3] << 8) | ReceivedData[4]);
				}
				else
				{
					ctrl->Buttons = 0;
				}

				if(ctrl->Type == PAD_DIGITAL)
				{
					HandleFishingActivationHotkey(ctrl, pad_n);
				}
				else
				{
					ctrl->FishingActivateHeld = 0;
				}
				
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

#ifdef RAW_PAD_DEBUG
	if(ctrl->Type != PAD_NONE) ctrl->ConfigState = 6;
#else
	if(ctrl->Type == PAD_ANALOG &&
		!(pad_n == 0 && IsPadDebugRawMode()) && ctrl->ConfigState < 6)
	{
		ctrl->ConfigState++;
	}

	if(ctrl->Type != PAD_ANALOG ||
		(pad_n == 0 && IsPadDebugRawMode()))
	{
		ctrl->ConfigState = 6;
	}
#endif
}
