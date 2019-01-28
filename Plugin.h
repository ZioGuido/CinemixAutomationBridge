/////////////////////////////////////////////////////////////////////////////
/*
	D&R Cinemix Automation Bridge V.1.01
	by Guido Scognamiglio - www.GenuineSoundware.com
	Programming started: July 2012
	Last Update: Feb 2017

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation files
	(the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	Any person wishing to distribute modifications to the Software is
	asked to send the modifications to the original developer so that
	they can be incorporated into the canonical version.  This is,
	however, not a binding provision of this license.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
	ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __Plugin_H
#define __Plugin_H

// Useful definitions
#define ONE127TH		0.00787401574803f
#define ONE255TH		0.00392156862745f
#define ONE16384		0.00006103515625f
#define PI 				3.14159265358f
#define TWO_PI 			6.28318530718f
#define RAND_MAX_RECIP	0.00003051757f

#include "Utils.h"
#include "resource.h"
#include "stdlib.h"
#include "math.h"
#include <ctime>

#ifdef RTMIDI
#include "RtMidi/RtMidi.h"
#endif

// CPlugin
class CPlugin : public AudioEffectX
{
public:
	// This variable activates the TEST MODE
	bool TestMode;

	void TestModeON()
	{
		ResetAllMixer();
		SetAllChannelsMode(1);

		// Calculate frame rates for faders and mutes animations separately
		// This is rather slow, but avoids flooding the Cinemix MIDI buffers
		FaderSpeed = int(sampleRate / 25);
		MuteSpeed = int(sampleRate / 10);
		count_samples1 = count_samples2 = 0;

		float scaleP = 2.f / NUMBER_OF_FADERS;
		// Starting point of all faders
		for (int i=0; i<NUMBER_OF_FADERS; i++) Anim_ramp[i] = -2.f + scaleP * i;

		TestMode = true;
	}

	void TestModeOFF()
	{
		TestMode = false;
		ResetAllMixer();
		SetAllChannelsMode(3);
	}

/* ////////////////////////////////////////////////////////////////////////////////////////////////
This plugin can use RtMidi, a free open source multiplatform library for accessing MIDI functions.
In order to compile this in Visual Studio, you have to define __WINDOWS_MM__ and include winmm.lib
*/
#ifdef RTMIDI
	// RtMidi port objects
	RtMidiIn	*MidiIn1, *MidiIn2;
	RtMidiOut	*MidiOut1, *MidiOut2;

	int MidiInPort1, MidiInPort2, MidiOutPort1, MidiOutPort2;
	int MidiInPorts, MidiOutPorts;
	std::vector<string> MidiInPortName;
	std::vector<string> MidiOutPortName;

	// Send a complete 3-byte MIDI event
	void RtMidiSendEvent(unsigned char Byte1, unsigned char Byte2, unsigned char Byte3, unsigned char Port = 0)
	{
		if (!RtMidiPortsAreOpen) return;

		std::vector<unsigned char> MidiMessage(3, 0); // 3 unsigned chars with value 0
		MidiMessage[0] = Byte1;
		MidiMessage[1] = Byte2;
		MidiMessage[2] = Byte3;
		//try { MidiOut1->sendMessage(&MidiMessage); } catch(RtMidiError &rtErr) { }
		//try { MidiOut2->sendMessage(&MidiMessage); } catch(RtMidiError &rtErr) { }
		if (!Port || Port == 1) MidiOut1->sendMessage(&MidiMessage);
		if (!Port || Port == 2) MidiOut2->sendMessage(&MidiMessage);
	}
	// Send a single byte, used to send FF (System Reset) to deactivate the console
	void RtMidiSendByte(unsigned char Byte)
	{
		if (!RtMidiPortsAreOpen) return;

		std::vector<unsigned char> MidiMessage(1, 0); // 1 unsigned char with value 0
		MidiMessage[0] = Byte;
		try { MidiOut1->sendMessage(&MidiMessage); } catch(RtMidiError &rtErr) { }
		try { MidiOut2->sendMessage(&MidiMessage); } catch(RtMidiError &rtErr) { }
	}

	// This function must be static, otherwise it can't be referenced, but we use the *userData pointer to point to a non-static function
	static void ProcessMidiInput(double deltatime, std::vector<unsigned char> *message, void *userData)
	{
		vector<unsigned char>&midiData = *message;	// Deference

		// If a complete 3-byte MIDI message is received
		if (midiData.size() == 3)
		{
			unsigned char bStatus	= midiData[0] & 0xf0;
			unsigned char bChannel	= midiData[0] & 0x0f; bChannel += 1;
			unsigned char bByte1	= midiData[1] & 0x7f;
			unsigned char bByte2	= midiData[2] & 0x7f;

			if (bStatus == MIDI_STATUS_CONTROLCHANGE)
				((CPlugin*)userData)->ProcessMIDIControlChange(bChannel, bByte1, bByte2);
		}
	}

	// Try to open MIDI ports
	bool OpenMidiInPort1()
	{
		MidiIn1->closePort();

		char ermsg[256];
		try { MidiIn1->openPort(MidiInPort1); }
		catch ( RtMidiError &rtErr ) {
			sprintf(ermsg, "Unable to open MIDI Input port: %s", MidiInPortName[MidiInPort1].c_str());
			alert(ermsg);
			return false;
		}

		// Set CallBack function
		MidiIn1->setCallback(&CPlugin::ProcessMidiInput, this);

		return true;
	}
	bool OpenMidiInPort2()
	{
		MidiIn2->closePort();

		char ermsg[256];
		try { MidiIn2->openPort(MidiInPort2); }
		catch ( RtMidiError &rtErr ) {
			sprintf(ermsg, "Unable to open MIDI Input port: %s", MidiInPortName[MidiInPort2].c_str());
			alert(ermsg);
			return false;
		}

		// Set CallBack function
		MidiIn2->setCallback(&CPlugin::ProcessMidiInput, this);

		return true;
	}
	bool OpenMidiOutPort1()
	{
		MidiOut1->closePort();

		char ermsg[256];
		try { MidiOut1->openPort(MidiOutPort1); }
		catch ( RtMidiError &rtErr ) {
			sprintf(ermsg, "Unable to open MIDI Output port: %s", MidiOutPortName[MidiOutPort1].c_str());
			alert(ermsg);
			return false;
		}
		return true;
	}
	bool OpenMidiOutPort2()
	{
		MidiOut2->closePort();

		char ermsg[256];
		try { MidiOut2->openPort(MidiOutPort2); }
		catch ( RtMidiError &rtErr ) {
			sprintf(ermsg, "Unable to open MIDI Output port: %s", MidiOutPortName[MidiOutPort2].c_str());
			alert(ermsg);
			return false;
		}
		return true;
	}

	bool RtMidiPortsAreOpen;

	// Close MIDI Ports
	void CloseAllMidiPorts()
	{
		// No MIDI ports found? Nothing to close!
		if (MidiInPorts + MidiOutPorts < 2) return;

		MidiIn1->closePort();
		MidiIn2->closePort();
		MidiOut1->closePort();
		MidiOut2->closePort();
		RtMidiPortsAreOpen = false;
	}

	// Open MIDI Ports
	void OpenAllMidiPorts()
	{
		int test(0);
		CloseAllMidiPorts();
		if (OpenMidiInPort1()) test++;
		if (OpenMidiInPort2()) test++;
		if (OpenMidiOutPort1()) test++;
		if (OpenMidiOutPort2()) test++;
		if (test == 4) RtMidiPortsAreOpen = true;
	}
#endif	// RTMIDI

// Construction
			CPlugin(audioMasterCallback audioMaster);
	virtual ~CPlugin();

// Overrides from AudioEffectX -------------------------------------------------------------------
	virtual VstInt32 getProgram();
	virtual void setProgram(VstInt32 program);
	virtual void setProgramName(char* name);
	virtual void getProgramName(char* name);
	virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);

	virtual void setParameter(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char* label);
	virtual void getParameterDisplay(VstInt32 index, char* text);
	virtual void getParameterName(VstInt32 index, char* text);

	bool isOpen;
	virtual void open()		{ resume();			}
	virtual void close()	{ isOpen = false;	}
	virtual void suspend()	{ isOpen = false;	}
	virtual void resume();	// see Plugin.cpp

	virtual bool getEffectName(char* name);
	virtual bool getVendorString(char* text);
	virtual bool getProductString(char* text);
	virtual VstInt32 getVendorVersion();
	virtual VstPlugCategory getPlugCategory();
	virtual VstInt32 canDo(char* text);

	virtual void setSampleRate(float sampleRate);
	virtual void setBlockSize(VstInt32 blockSize);
	virtual bool getInputProperties(VstInt32 index, VstPinProperties* properties);
	virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	virtual VstInt32 getNumMidiInputChannels();
	virtual VstInt32 getNumMidiOutputChannels();

	virtual VstInt32 processEvents(VstEvents* events);
	virtual void process(float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);
///////////////////////////////////////////////////////////////////////////////////////////////////////////

	void OnAllNotesOff();

	void SendMidiEvent(unsigned char byte1, unsigned char byte2, unsigned char byte3)
	{
		VstEvents eventsToHost;
		VstMidiEvent MidiEvent;
		MidiEvent.type = kVstMidiType;
		MidiEvent.byteSize = sizeof(VstMidiEvent);
		MidiEvent.deltaFrames = 1;
		MidiEvent.flags = kVstMidiEventIsRealtime;
		MidiEvent.midiData[0] = byte1;
		MidiEvent.midiData[1] = byte2;
		MidiEvent.midiData[2] = byte3;
		eventsToHost.numEvents = 1;
		eventsToHost.events[0] = reinterpret_cast<VstEvent*>(&MidiEvent);
		sendVstEventsToHost(&eventsToHost);
	}

	void SendMidiCC(unsigned char Channel, unsigned char CCnum, unsigned char Value, unsigned char Port = 0)
	{
#ifdef RTMIDI
		RtMidiSendEvent(176 + Channel - 1, CCnum, Value, Port);
#else
		SendMidiEvent(0xB0 + Channel - 1, CCnum, Value);
#endif
	}

	void SetAllChannelsMode(int mode)
	{
		for (int i=0; i<48; i++)
		{
			SendMidiCC(3, 64 + i, mode);
			SendMidiCC(4, 64 + i, mode);
			FaderModeLeft[i] = mode;
			FaderModeRight[i] = mode;
		}
	}

	void ActivateMixer()
	{
#ifdef RTMIDI
		OpenAllMidiPorts();
#endif
		SendMidiCC(5, 127, 127);

		// Set all channels to Write Mode
		SetAllChannelsMode(2);

		// Set Master Section to Write Mode (Master fader and Joysticks)
		SendMidiCC(5, 64, 2);
		SendMidiCC(4, 88, 2);
		SendMidiCC(4, 90, 2);
		FaderModeMaster = 2;

		// Now send a snapshot of all controls
		SendSnapshot();

		// Set all channels to RW
		SetAllChannelsMode(3);

		// Set Master Section to RW (Master fader and Joysticks)
		SendMidiCC(5, 64, 3);
		SendMidiCC(4, 88, 3);
		SendMidiCC(4, 90, 3);
		FaderModeMaster = 3;
	}

	void DeactivateMixer()
	{
		SendMidiCC(5, 127, 0);

		// Reset all channels
		SetAllChannelsMode(0);

		// Master Section (Master fader and Joysticks)
		SendMidiCC(5, 64, 0);
		SendMidiCC(4, 88, 0);
		SendMidiCC(4, 90, 0);

		// Send a single MIDI Byte value FF for System Reset
#ifdef RTMIDI
		RtMidiSendByte(255);
		//CloseAllMidiPorts();
#else
		SendMidiEvent(255, NULL, NULL);
#endif
	}

	void SendSnapshot()
	{
		for (int index=0; index < NUMBER_OF_PARAMETERS; index++)
		{
			prev_CC_Val[index] = -1;
			setParameterAutomated(index, getParameter(index));
		}
	}

	void ResetAllMixer()
	{
		// Set all to AUTO mode
		for (int i=0; i<48; i++)
		{
			SendMidiCC(3, 64 + i, 3);
			SendMidiCC(4, 64 + i, 3);
			FaderModeLeft[i] = 3;
			FaderModeRight[i] = 3;
		}

		// Master Section (Master fader and Joysticks) to AUTO
		SendMidiCC(5, 64, 3);
		SendMidiCC(4, 88, 3);
		SendMidiCC(4, 90, 3);

		// All parameters to zero...
		for (int c = 0; c < NUMBER_OF_PARAMETERS; c++)
			m_Programs[curProgram].m_prmValue[c] = 0;

		// ...except the Master Fader
		m_Programs[curProgram].m_prmValue[160] = 1.f;

		SendSnapshot();
		AllMutesStatus = false;
	}

	bool AllMutesStatus;
	void ToggleAllMutes()
	{
		AllMutesStatus = !AllMutesStatus;

		for (int i = NUMBER_OF_FADERS; i < NUMBER_OF_FADERS * 2; i++)
			setParameterAutomated(i, AllMutesStatus ? 1.f : 0);

		// The AUX Mutes
		for (int i = 0; i < 10; i++)
			setParameterAutomated(144 + i, AllMutesStatus ? 1.f : 0);

		// Don't forget the Joysticks!
		setParameterAutomated(156, AllMutesStatus ? 1.f : 0);
		setParameterAutomated(159, AllMutesStatus ? 1.f : 0);
	}

#ifdef RTMIDI
	bool FileExists(const char* FileName)
	{
		FILE* fp = NULL;
		fp = fopen(FileName, "rb");
		if (fp != NULL) { fclose(fp); return true; }
		return false;
	}

	void SaveConfig()
	{
		// No MIDI ports found? Don't save!
		if (MidiInPorts + MidiOutPorts < 2) return;

		char filename[256] = {};
		strcpy(filename, GetConfigFilePath());

		FILE *fp = NULL;
		fp = fopen(filename, "w");
		if (fp != NULL)
		{
			fprintf(fp, "[MIDI PORTS]\n");
			fprintf(fp, "MIDIIN1=%s\n", MidiInPortName[MidiInPort1].c_str());
			fprintf(fp, "MIDIIN2=%s\n", MidiInPortName[MidiInPort2].c_str());
			fprintf(fp, "MIDIOUT1=%s\n", MidiOutPortName[MidiOutPort1].c_str());
			fprintf(fp, "MIDIOUT2=%s\n", MidiOutPortName[MidiOutPort2].c_str());
			fclose(fp);
		}
	}

	void LoadConfig()
	{
		char filename[256]; strcpy(filename, GetConfigFilePath());

		ifstream fh; string line;
		fh.open (filename);
		if (!fh) return;
		if (!fh.eof())
		{
			// Skip header
			getline (fh, line);

			// Read variables...
			getline (fh, line);
			if (line.substr(0, 8)	== "MIDIIN1=")
				for (int i=0; i<MidiInPorts; i++)
					if (line.substr(8, line.length()) == MidiInPortName[i])
						MidiInPort1 = i;

			getline (fh, line);
			if (line.substr(0, 8)	== "MIDIIN2=")
				for (int i=0; i<MidiInPorts; i++)
					if (line.substr(8, line.length()) == MidiInPortName[i])
						MidiInPort2 = i;

			getline (fh, line);
			if (line.substr(0, 9)	== "MIDIOUT1=")
				for (int i=0; i<MidiOutPorts; i++)
					if (line.substr(9, line.length()) == MidiOutPortName[i])
						MidiOutPort1 = i;

			getline (fh, line);
			if (line.substr(0, 9)	== "MIDIOUT2=")
				for (int i=0; i<MidiOutPorts; i++)
					if (line.substr(9, line.length()) == MidiOutPortName[i])
						MidiOutPort2 = i;
		}
		fh.close();
	}

	char *GetConfigFilePath()
	{
		char ConfigFilePath[MAX_PATH] = {};
#if 0
		MEMORY_BASIC_INFORMATION mbi;
		static int dllpath_i;
		VirtualQuery( &dllpath_i, &mbi, sizeof(mbi) );
		DWORD hMod = (DWORD)mbi.AllocationBase;
		char dllfullpath[MAX_PATH];
		GetModuleFileName( (HMODULE)hMod, dllfullpath, sizeof(dllfullpath) );
		strncpy(ConfigFilePath, dllfullpath, strlen(dllfullpath)-4);
		strncat(ConfigFilePath, ".ini", 4);
#endif

#if WINDOWS
	sprintf(ConfigFilePath, "%s/GSi/%s/%s%s", getenv("APPDATA"), _Plugin_Product_Name_, _Plugin_Product_Name_, ".ini");

	// If no .ini file, create the Home Directory for storing the Plugin's support files
	if (!FileExists(ConfigFilePath))
	{
		char base_path[260];
		sprintf(base_path, "%s/GSi/", getenv("APPDATA")); mkdir(base_path);
		sprintf(base_path, "%s/GSi/%s/", getenv("APPDATA"), _Plugin_Product_Name_); mkdir(base_path);
	}
#elif MAC
		sprintf(ConfigFilePath, "%s/Library/Audio/Presets/GSi/%s/%s.ini", getenv("HOME"), _Plugin_Product_Name_, _Plugin_Product_Name_);
		if (!FileExists(ConfigFilePath)) {
			system("mkdir ~/Library/Audio/Presets");
			system("mkdir ~/Library/Audio/Presets/GSi");
			system("mkdir ~/Library/Audio/Presets/GSi/CinemixAutomationBridge");
		}
#endif
		return ConfigFilePath;
	}
#endif // RTMIDI


// Implementation
private:

	bool FullInit;
	int MidiController[NUMBER_OF_PARAMETERS];
	int MidiController2[NUMBER_OF_FADERS];	// Faders use 2 controllers each
	int FaderModeLeft[48];
	int FaderModeRight[48];
	int FaderModeMaster;
	int MidiChannel[NUMBER_OF_PARAMETERS];
	int prev_CC_Val[NUMBER_OF_PARAMETERS];

	int FaderSpeed, MuteSpeed, count_samples1, count_samples2;
	float Anim_ramp[NUMBER_OF_FADERS];


// Operations
	void OnNoteOn(unsigned char bChannel, unsigned char bNote, unsigned char bVelocity, VstInt32 deltaFrames);
	void OnNoteOff(unsigned char bChannel, unsigned char bNote, unsigned char bVelocity, VstInt32 deltaFrames);
	void OnDamper(bool PedalDown);
	bool PedalDown;

#if WIN32
	inline float RandGen() { return (float)rand() * RAND_MAX_RECIP; }
#elif MAC
	inline float RandGen() { return (float)(rand() & 0x7FFF) * RAND_MAX_RECIP; }
#endif

	class CParamSmooth
	{
	public:
		CParamSmooth() { a = 0.99f; b = 1.f - a; z = 0; };
		~CParamSmooth() { };
		inline float Process(float in) { z += 1e-6f; z = (in * b) + (z * a); return z; }
		float z;
	private:
		float a, b;
	};

	// CProgram - information about a single progam
	class CProgram
	{
	public:
	// Construction
		CProgram() {};
		~CProgram() {};

	// Attributes
		char	m_szName[24];
		float	m_prmValue[NUMBER_OF_PARAMETERS];	// Set according to the number of parameters needed
	};

	char	ParameterName[NUMBER_OF_PARAMETERS][24];

	CProgram	m_Programs[NUMBER_OF_PROGRAMS];		// Change this to set the number of programs


	// This function is called by setParameter to translate parameter values to MIDI data
	void SendMidiData(VstInt32 index, float value)
	{
		int chn = MidiChannel[index];
		int ctl = MidiController[index];
		int val = int(value * 127);

		// Avoid sending duplicated values for the same parameter
		if (val != prev_CC_Val[index])
		{
			// Channel Strips
			if (index < NUMBER_OF_FADERS*2)
			{
				// Faders
				if (index < NUMBER_OF_FADERS)
					SendMidiCC(chn, ctl, val, index < 48 ? 1 : 2);

				// Mutes
				else
					SendMidiCC(chn, ctl, value ? 3 : 2, index < 120 ? 1 : 2);

			// Master Section and other
			} else {
				switch (index)
				{
				case 144:	// AUX 1
					SendMidiCC(chn, ctl, value ? 3 : 2, 2);
					break;
				case 145:	// AUX 2
					SendMidiCC(chn, ctl, value ? 5 : 4, 2);
					break;
				case 146:	// AUX 3
					SendMidiCC(chn, ctl, value ? 7 : 6, 2);
					break;
				case 147:	// AUX 4
					SendMidiCC(chn, ctl, value ? 9 : 8, 2);
					break;
				case 148:	// AUX 5
					SendMidiCC(chn, ctl, value ? 11 : 10, 2);
					break;
				case 149:	// AUX 6
					SendMidiCC(chn, ctl, value ? 13 : 12, 2);
					break;
				case 150:	// AUX 7
					SendMidiCC(chn, ctl, value ? 15 : 14, 2);
					break;
				case 151:	// AUX 8
					SendMidiCC(chn, ctl, value ? 17 : 16, 2);
					break;
				case 152:	// AUX 9
					SendMidiCC(chn, ctl, value ? 19 : 18, 2);
					break;
				case 153:	// AUX 10
					SendMidiCC(chn, ctl, value ? 21 : 20, 2);
					break;

				case 156:	// Joy 1 Mute
					SendMidiCC(chn, ctl, value ? 3 : 2, 2);
					break;
				case 159:	// Joy 2 Mute
					SendMidiCC(chn, ctl, value ? 3 : 2, 2);
					break;

				default:
					SendMidiCC(chn, ctl, val, 2);
					break;
				}
			}
			prev_CC_Val[index] = val;
		}
	}


	// This function is called to process the incoming MIDI CC events
	void ProcessMIDIControlChange(unsigned char bChannel, unsigned char bByte1, unsigned char bByte2)
	{
		// Get Touch sensors and set the channels in WRITE mode automatically
		if (bChannel == 3 && bByte1 > 63 && bByte1 < 112)	// Left part
		{
			// When SEL button is depressed...
			if (bByte2 == 1)
			{
				if (++FaderModeLeft[bByte1 - 64] > 3) FaderModeLeft[bByte1 - 64] = 0; // Rotate between modes: 0=ISO, 1=READ, 2=WRITE, 3=AUTO
				SendMidiCC(bChannel, bByte1, FaderModeLeft[bByte1 - 64]);
			}
			// In AUTO mode...
			if (FaderModeLeft[bByte1 - 64] == 3)
			{
				     if (bByte2 == 6) SendMidiCC(bChannel, bByte1, 2);		// Touch, set to W
				else if (bByte2 == 5) SendMidiCC(bChannel, bByte1, 3);		// Release, set to RW
			}
		}
		if (bChannel == 4 && bByte1 > 63 && bByte1 < 112)	// Right part
		{
			if (bByte2 == 1)
			{
				if (++FaderModeRight[bByte1 - 64] > 3) FaderModeRight[bByte1 - 64] = 0; // Rotate between modes: 0=ISO, 1=READ, 2=WRITE, 3=AUTO
				SendMidiCC(bChannel, bByte1, FaderModeRight[bByte1 - 64]);
			}
			// In AUTO mode...
			if (FaderModeRight[bByte1 - 64] == 3)
			{
					 if (bByte2 == 6) SendMidiCC(bChannel, bByte1, 2);		// Touch, set to W
				else if (bByte2 == 5) SendMidiCC(bChannel, bByte1, 3);		// Release, set to RW
			}
		}
		// Master SEL R/W Button selects status for all channels
		if (bChannel == 5 && bByte1 == 64 && bByte2 == 1)
		{
			if (++FaderModeMaster > 3) FaderModeMaster = 0;
			SetAllChannelsMode(FaderModeMaster);
		}

		// Special check for AUX Mutes (could have found a more elegant way, but this one works!)
		if (bChannel == 5 && bByte1 == 96)
		{
			// AUX 1
			if (bByte2 ==  2) setParameterAutomated(144, 0);
			if (bByte2 ==  3) setParameterAutomated(144, 1.f);
			// AUX 2
			if (bByte2 ==  4) setParameterAutomated(145, 0);
			if (bByte2 ==  5) setParameterAutomated(145, 1.f);
			// AUX 3
			if (bByte2 ==  6) setParameterAutomated(146, 0);
			if (bByte2 ==  7) setParameterAutomated(146, 1.f);
			// AUX 4
			if (bByte2 ==  8) setParameterAutomated(147, 0);
			if (bByte2 ==  9) setParameterAutomated(147, 1.f);
			// AUX 5
			if (bByte2 == 10) setParameterAutomated(148, 0);
			if (bByte2 == 11) setParameterAutomated(148, 1.f);
			// AUX 6
			if (bByte2 == 12) setParameterAutomated(149, 0);
			if (bByte2 == 13) setParameterAutomated(149, 1.f);
			// AUX 7
			if (bByte2 == 14) setParameterAutomated(150, 0);
			if (bByte2 == 15) setParameterAutomated(150, 1.f);
			// AUX 8
			if (bByte2 == 16) setParameterAutomated(151, 0);
			if (bByte2 == 17) setParameterAutomated(151, 1.f);
			// AUX 9
			if (bByte2 == 18) setParameterAutomated(152, 0);
			if (bByte2 == 19) setParameterAutomated(152, 1.f);
			// AUX 10
			if (bByte2 == 20) setParameterAutomated(153, 0);
			if (bByte2 == 21) setParameterAutomated(153, 1.f);
		}

		// Move screen controls and automation parameters
		for (int p = 0; p < NUMBER_OF_PARAMETERS; p++)
		{
			if (MidiChannel[p] == bChannel && (MidiController[p] == bByte1 || MidiController2[p] == bByte1))
			{
				// Channel Strips
				if (p < NUMBER_OF_FADERS*2)
				{
					// Faders
					if (p < NUMBER_OF_FADERS)
						setParameterAutomated(p, bByte2 * ONE127TH);
					// Mutes
					else
						setParameterAutomated(p, bByte2 == 3 ? 1.f : 0);

				// Master Section and other
				} else {
					switch (p)
					{
					case 156:	// Joy 1 Mute
						setParameterAutomated(p, bByte2 == 3 ? 1.f : 0);
						break;
					case 159:	// Joy 2 Mute
						setParameterAutomated(p, bByte2 == 3 ? 1.f : 0);
						break;

					// Joysticks 1&2 XY and Master fader
					case 154: case 155: case 157: case 158: case 160:
						setParameterAutomated(p, bByte2 * ONE127TH);
						break;
					}
				}
				break;	// stop for cycle
			} // if
		} // for

		// Special: second CC for Master fader
		if (bChannel == 5 && bByte1 == 1) setParameterAutomated(160, bByte2 * ONE127TH);
	}
};

#endif		// __Plugin_H
