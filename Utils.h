//////////////////////////////////////////////////////////////////////////
// Utils.h - declaration of Utils

#ifndef __UTILS_H
#define __UTILS_H

/* Coefficients for BPM tempo sync of delays and LFOs
How to use:
Sec = (60 / BPM) * (4 / msCoeff)
ms = Sec / 1000
Hz = 1000 / ms
*/
struct TEMPOSYNC
{
	const char		*Label;
	const float		msCoeff;
};
static const TEMPOSYNC TempoDivisions[] = 
{
	{ "1 Bar", 		1.f		},
	{ "Half Bar", 	2.f		},
	{ "1/4", 		4.f		},
	{ "1/8", 		8.f		},
	{ "1/16", 		16.f	},
	{ "1/32", 		32.f	},
	{ "1/1T", 		1.5f	},
	{ "1/2T", 		3.f		},
	{ "1/4T", 		6.f		},
	{ "1/8T", 		12.f	},
	{ "1/16T", 		24.f	},
	{ "1/32T", 		48.f	},
	{ "1/1D", 		0.666f	},
	{ "1/2D", 		1.333f	},
	{ "1/4D", 		2.666f	},
	{ "1/8D", 		5.333f	},
	{ "1/16D", 		10.666f	},
	{ "1/32D", 		21.333f	},
};


// Channel Message Staus
#define MIDI_STATUS_NOTEOFF				0x80	// b2=note, b3=velocity
#define MIDI_STATUS_NOTEON				0x90	// b2=note, b3=velocity
#define MIDI_STATUS_AFTERTOUCH			0xA0	// b2=note, b3=pressure
#define MIDI_STATUS_CONTROLCHANGE		0xB0	// b2=controller, b3=value
#define MIDI_STATUS_PROGRAMCHANGE		0xC0	// b2=program number
#define MIDI_STATUS_CHANNELPRESSURE		0xD0	// b2=pressure
#define MIDI_STATUS_PITCHBEND			0xE0	// pitch (b3 & 0x7f) << 7 | (b2 & 0x7f) and center=0x2000

// System messages
#define MIDI_STATUS_SYSEX				0xF0
#define MIDI_STATUS_SYSEX_CONT			0xF7
#define MIDI_STATUS_META				0xFF
#define MIDI_STATUS_ACTIVESENSE			0xFE

// Common controller numbers
#define	MIDI_CONTROLLER_BANKSELECT_MSB		0
#define MIDI_CONTROLLER_BANKSELECT_LSB		32
#define	MIDI_CONTROLLER_DAMPER				64
#define	MIDI_CONTROLLER_PORTAMENTO			65
#define	MIDI_CONTROLLER_SOSTENUTO			66
#define	MIDI_CONTROLLER_SOFTPEDAL			67
#define	MIDI_CONTROLLER_ALLSOUNDOFF			120
#define	MIDI_CONTROLLER_RESETALLCONTROLLERS	121
#define	MIDI_CONTROLLER_LOCALCONTROL		122
#define	MIDI_CONTROLLER_ALLNOTESOFF			123
#define	MIDI_CONTROLLER_OMNIMODEOFF			124
#define	MIDI_CONTROLLER_OMNIMODEON			125
#define	MIDI_CONTROLLER_POLYMODE			126
#define	MIDI_CONTROLLER_POLYMODEON			127

#endif	// __UTILS_H

