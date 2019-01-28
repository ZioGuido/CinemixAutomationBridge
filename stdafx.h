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

#ifndef _STDAFX_H
#define _STDAFX_H

// Global variables and definitions
const char _Plugin_Product_Name_[] = "CinemixAutomationBridge";
const long _Plugin_Unique_ID_ = 'DRcm';
const long _Plugin_Version_ = 1010;

// Use RtMidi library?
#define RTMIDI	1

#define NUMBER_OF_PROGRAMS			1
#define NUMBER_OF_PARAMETERS		161		// 144 params for faders and mutes, 10 for aux mutes, 7 for the master section
#define NUMBER_OF_CHANNELSTRIPS		36
#define NUMBER_OF_FADERS			72





#pragma warning(disable:4996)

// Windows Header Files:
//#define WIN32_LEAN_AND_MEAN	1	// Exclude rarely-used stuff from Windows headers
#if WIN32
#include <windows.h>
#include <malloc.h>
#elif MAC
#define MAX_PATH 260
#include <sys/malloc.h>
#endif

// Other common headers
#include <stdio.h>
#include <assert.h>
#include <direct.h>

/*********** for fstream (ifstream, ofstream) ********/
#include <iostream>
#include <sstream>  // Required for stringstreams
#include <fstream>
using namespace std;


// VST SDK headers
#if WIN32
#include "c:/vst/vstsdk2.4/public.sdk/source/vst2.x/AudioEffectX.h"
#include "c:/vst/vstsdk2.4/public.sdk/source/vst2.x/AEffEditor.h"
#include "c:/vst/vstsdk2.4/vstgui.sf/vstgui/vstgui.h"
#include "c:/vst/vstsdk2.4/vstgui.sf/vstgui/AEffGUIEditor.h"
#else
#include "AudioEffectX.h"
#include "AEffEditor.h"
#include "AEffGUIEditor.h"
#include "vstgui.h"
#endif

// Useful macros
#define ASSERT assert
#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global scope functions
static void alert(char *msg)
{
#if WIN32
	MessageBox(GetForegroundWindow(), msg, _Plugin_Product_Name_, MB_ICONINFORMATION);
#elif MAC
	char* title = "CinemixAutomationBridge";
	char* message = msg;
	CFStringRef theTitle = CFStringCreateWithCString (kCFAllocatorDefault, title, kCFStringEncodingMacRoman);
	CFStringRef theMessage = CFStringCreateWithCString (kCFAllocatorDefault, message, kCFStringEncodingMacRoman);
	// Flags can be: kCFUserNotificationStopAlertLevel or kCFUserNotificationPlainAlertLevel;
	CFOptionFlags flags = kCFUserNotificationStopAlertLevel;

	CFOptionFlags retFlag = 0;
	CFUserNotificationDisplayAlert(0, flags, NULL, NULL, NULL, theTitle, theMessage, NULL, NULL, NULL, &retFlag);

	CFRelease(theTitle);
	CFRelease(theMessage);
#endif
}

static bool confirm(char *msg)
{
#if WIN32
	if (MessageBox(GetForegroundWindow(), msg, _Plugin_Product_Name_, MB_YESNO | MB_ICONQUESTION) == IDNO) return false;
	return true;
#elif MAC
	char* title = "CinemixAutomationBridge";
	char* message = msg;
	CFStringRef theTitle = CFStringCreateWithCString (kCFAllocatorDefault, title, kCFStringEncodingMacRoman);
	CFStringRef theMessage = CFStringCreateWithCString (kCFAllocatorDefault, message, kCFStringEncodingMacRoman);
	CFOptionFlags flags = kCFUserNotificationPlainAlertLevel;

	CFOptionFlags retFlag = 0;
	CFUserNotificationDisplayAlert(0, flags, NULL, NULL, NULL, theTitle, theMessage, CFSTR("No"), CFSTR("Yes"), NULL, &retFlag);

	CFRelease(theTitle);
	CFRelease(theMessage);

	return retFlag == 0 ? false : true;
#endif
}



#endif _STDAFX_H
