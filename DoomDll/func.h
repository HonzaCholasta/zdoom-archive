#ifndef __FUNC_H__
#define __FUNC_H__

#include "doomdll.h"

//
// Defaults
//

typedef unsigned char		BYTE;
typedef signed char			SBYTE;

typedef unsigned short		WORD;
typedef signed short		SWORD;

typedef unsigned long		DWORD;
typedef signed long			SDWORD;

typedef unsigned __int64	QWORD;
typedef signed __int64		SQWORD;

#define FRACBITS			16
#define FRACUNIT			(1<<FRACBITS)

typedef SDWORD				fixed_t;
typedef DWORD				dsfixed_t;		// fixedpt used by span drawer

// game print flags
#define	PRINT_LOW			0		// pickup messages
#define	PRINT_MEDIUM		1		// death messages
#define	PRINT_HIGH			2		// critical messages
#define	PRINT_CHAT			3		// chat messages
#define PRINT_TEAMCHAT		4		// chat messages from a teammate

#define ANG45			0x20000000
#define ANG90			0x40000000
#define ANG180			0x80000000
#define ANG270			0xc0000000

#define ANGLE_45		0x20000000
#define ANGLE_90		0x40000000
#define ANGLE_180		0x80000000
#define ANGLE_MAX		0xffffffff
#define ANGLE_1			(ANGLE_45/45)
#define ANGLE_60		(ANGLE_180/3)


#define SLOPERANGE		2048
#define SLOPEBITS		11
#define DBITS			(FRACBITS-SLOPEBITS)

typedef DWORD			angle_t;

typedef char AActor;	// Heh

//
// Console
//

enum
{
	CVAR_ARCHIVE		= 1,	// set to cause it to be saved to config
	CVAR_USERINFO		= 2,	// added to userinfo  when changed
	CVAR_SERVERINFO		= 4,	// added to serverinfo when changed
	CVAR_NOSET			= 8,	// don't allow change from console at all,
								// but can be set from the command line
	CVAR_LATCH			= 16,	// save changes until server restart
	CVAR_UNSETTABLE		= 32,	// can unset this var from console
	CVAR_DEMOSAVE		= 64,	// save the value of this cvar in a demo
	CVAR_ISDEFAULT		= 128,	// is cvar unchanged since creation?
	CVAR_AUTO			= 256,	// allocated; needs to be freed when destroyed
	CVAR_NOINITCALL		= 512,	// don't call callback at game start
	CVAR_GLOBALCONFIG	= 1024,	// cvar is saved to global config section
	CVAR_VIDEOCONFIG	= 2048, // cvar is saved to video config section
};

// Class that can parse command lines
class FCommandLine
{
public:
	FCommandLine (const char *commandline);
	~FCommandLine ();
	int argc ();
	char *operator[] (int i);
	const char *args () { return cmd; }

private:
	const char *cmd;
	int _argc;
	char **_argv;
	long argsize;
};

typedef void (*CCmdRun) (FCommandLine &argv, AActor *instigator, int key);

//
// Zone memory
//

//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC				1		// static entire execution time
#define PU_SOUND				2		// static while playing
#define PU_MUSIC				3		// static while playing
#define PU_DAVE 				4		// anything else Dave wants static
#define PU_DEHACKED				5		// static while processing a patch
#define PU_SOUNDCHANNELS		6		// sound channels in s_sound.cpp
#define PU_LEVEL				50		// static until level exited
#define PU_LEVSPEC				51		// a special thinker in a level
#define PU_LEVACS				52		// [RH] An ACS script in a level
// Tags >= 100 are purgable whenever needed.
#define PU_PURGELEVEL			100
#define PU_CACHE				101

#endif	// __FUNC_H__
