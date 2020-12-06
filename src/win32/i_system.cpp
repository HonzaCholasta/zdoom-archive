// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <string.h>
#include <process.h>

#include <stdarg.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "hardware.h"
#include "errors.h"
#include <math.h>

#include "doomtype.h"
#include "version.h"
#include "doomdef.h"
#include "cmdlib.h"
#include "m_argv.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "i_music.h"
#include "resource.h"

#include "d_main.h"
#include "d_net.h"
#include "g_game.h"
#include "i_input.h"
#include "i_system.h"
#include "c_dispatch.h"

#include "stats.h"

EXTERN_CVAR (String, language)

#ifdef USEASM
extern "C" BOOL STACK_ARGS CheckMMX (char *vendorid);
#endif

extern "C"
{
	BOOL		HaveRDTSC = 0;
	BOOL		HaveCMOV = 0;
	double		SecondsPerCycle = 1e-8;
	double		CyclesPerSecond = 1e8;		// 100 MHz
	byte		CPUFamily, CPUModel, CPUStepping;
}

static cycle_t ClockCalibration;

extern HWND Window;
extern HINSTANCE hInstance;

BOOL UseMMX;
UINT TimerPeriod;
UINT TimerEventID;
HANDLE NewTicArrived;
DWORD LanguageIDs[4];
void CalculateCPUSpeed ();

float mb_used = 8.0;

int (*I_GetTime) (void);
int (*I_WaitForTic) (int);

os_t OSPlatform;

void I_Tactile (int on, int off, int total)
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t emptycmd;
ticcmd_t *I_BaseTiccmd(void)
{
	return &emptycmd;
}

int I_GetHeapSize (void)
{
	return (int)(mb_used*1024*1024);
}

byte *I_ZoneBase (size_t *size)
{
	char *p;
	void *zone;

	p = Args.CheckValue ("-heapsize");
	if (p)
		mb_used = (float)atof (p);
	*size = (size_t)(mb_used*1024*1024);

	while (NULL == (zone = malloc (*size)) && *size >= 2*1024*1024)
		*size -= 1024*1024;

	return (byte *)zone;
}	

byte *I_AllocLow(int length)
{
	byte *mem;

	mem = (byte *)malloc (length);
	if (mem) {
		memset (mem,0,length);
	}
	return mem;
}


static DWORD basetime = 0;

// [RH] Returns time in milliseconds
unsigned int I_MSTime (void)
{
	DWORD tm;

	tm = timeGetTime();
	if (!basetime)
		basetime = tm;

	return tm - basetime;
}

//
// I_GetTime
// returns time in 1/35th second tics
//
int I_GetTimePolled (void)
{
	DWORD tm;

	tm = timeGetTime();
	if (!basetime)
		basetime = tm;

	return ((tm-basetime)*TICRATE)/1000;
}

int I_WaitForTicPolled (int prevtic)
{
	int time;

	while ((time = I_GetTimePolled()) <= prevtic)
		;

	return time;
}


static int tics;

int I_GetTimeEventDriven (void)
{
	return tics;
}

int I_WaitForTicEvent (int prevtic)
{
	while (prevtic >= tics)
	{
		WaitForSingleObject (NewTicArrived, 1000/TICRATE);
	}

	return tics;
}

void CALLBACK TimerTicked (UINT id, UINT msg, DWORD user, DWORD dw1, DWORD dw2)
{
	tics++;
	SetEvent (NewTicArrived);
}

void I_WaitVBL (int count)
{
	// I_WaitVBL is never used to actually synchronize to the
	// vertical blank. Instead, it's used for delay purposes.
	Sleep (1000 * count / 70);
}

// [RH] Detect the OS the game is running under
void I_DetectOS (void)
{
	OSVERSIONINFO info;
	const char *osname;

	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&info);

	switch (info.dwPlatformId) {
		case VER_PLATFORM_WIN32s:
			OSPlatform = os_Win32s;
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			OSPlatform = os_Win95;
			osname = info.dwMinorVersion >= 10 ? "Windows 98" : "Windows 95";
			break;
		case VER_PLATFORM_WIN32_NT:
			OSPlatform = info.dwMajorVersion < 5 ? os_WinNT : os_Win2k;
			osname = "Windows NT";
			break;
		default:
			OSPlatform = os_unknown;
			osname = "Unknown OS";
			break;
	}

	Printf ("OS: %s %u.%u (build %u)\n",
			osname,
			info.dwMajorVersion, info.dwMinorVersion,
			OSPlatform == os_Win95 ? info.dwBuildNumber & 0xffff : info.dwBuildNumber,
			info.szCSDVersion);
	if (info.szCSDVersion[0])
		Printf ("  %s\n", info.szCSDVersion);

	if (OSPlatform == os_Win32s) {
		I_FatalError ("Sorry, Win32s is not supported.\n"
					  "Upgrade to a newer version of Windows.");
	} else if (OSPlatform == os_unknown) {
		Printf ("(Assuming Windows 95)\n");
		OSPlatform = os_Win95;
	}
}

//
// SubsetLanguageIDs
//
static void SubsetLanguageIDs (LCID id, LCTYPE type, int idx)
{
	char buf[8];
	LCID langid;
	char *idp;

	if (!GetLocaleInfo (id, type, buf, 8))
		return;
	langid = MAKELCID (strtoul(buf, NULL, 16), SORT_DEFAULT);
	if (!GetLocaleInfo (langid, LOCALE_SABBREVLANGNAME, buf, 8))
		return;
	idp = (char *)(&LanguageIDs[idx]);
	memset (idp, 0, 4);
	idp[0] = tolower(buf[0]);
	idp[1] = tolower(buf[1]);
	idp[2] = tolower(buf[2]);
	idp[3] = 0;
}

//
// SetLanguageIDs
//
void SetLanguageIDs ()
{
	int langlen = strlen (language);

	if (langlen < 2 || langlen > 3)
	{
		memset (LanguageIDs, 0, sizeof(LanguageIDs));
		SubsetLanguageIDs (LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, 0);
		SubsetLanguageIDs (LOCALE_USER_DEFAULT, LOCALE_IDEFAULTLANGUAGE, 1);
		SubsetLanguageIDs (LOCALE_SYSTEM_DEFAULT, LOCALE_ILANGUAGE, 2);
		SubsetLanguageIDs (LOCALE_SYSTEM_DEFAULT, LOCALE_IDEFAULTLANGUAGE, 3);
	}
	else
	{
		DWORD lang = 0;

		((BYTE *)&lang)[0] = (language)[0];
		((BYTE *)&lang)[1] = (language)[1];
		((BYTE *)&lang)[2] = (language)[2];
		LanguageIDs[0] = lang;
		LanguageIDs[1] = lang;
		LanguageIDs[2] = lang;
		LanguageIDs[3] = lang;
	}
}

//
// I_Init
//
void I_Init (void)
{
#ifndef USEASM
	UseMMX = 0;
#else
	char vendorid[13];

	vendorid[0] = vendorid[12] = 0;
	UseMMX = CheckMMX (vendorid);
	if (Args.CheckParm ("-nommx"))
		UseMMX = 0;

	if (vendorid[0])
		Printf ("CPUID: %s  (", vendorid);

	if (UseMMX)
		Printf ("using MMX)\n");
	else
		Printf ("not using MMX)\n");

	Printf ("       family %d, model %d, stepping %d\n", CPUFamily, CPUModel, CPUStepping);
	CalculateCPUSpeed ();
#endif

	// Use a timer event if possible
	NewTicArrived = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (NewTicArrived)
	{
		UINT delay;
		char *cmdDelay;

		cmdDelay = Args.CheckValue ("-timerdelay");
		delay = 0;
		if (cmdDelay != 0)
		{
			delay = atoi (cmdDelay);
		}
		if (delay == 0)
		{
			delay = 1000/TICRATE;
		}
		TimerEventID = timeSetEvent
			(
				delay,
				0,
				TimerTicked,
				0,
				TIME_PERIODIC
			);
	}
	if (TimerEventID != 0)
	{
		I_GetTime = I_GetTimeEventDriven;
		I_WaitForTic = I_WaitForTicEvent;
	}
	else
	{	// If no timer event, busy-loop with timeGetTime
		I_GetTime = I_GetTimePolled;
		I_WaitForTic = I_WaitForTicPolled;
	}

	I_InitSound ();
	I_InitInput (Window);
	I_InitHardware ();
}

void CalculateCPUSpeed ()
{
	LARGE_INTEGER freq;

	QueryPerformanceFrequency (&freq);

	if (freq.QuadPart != 0 && HaveRDTSC)
	{
		LARGE_INTEGER count1, count2;
		DWORD minDiff;

		// Count cycles for at least 55 milliseconds.
		// The performance counter is very low resolution compared to CPU
		// speeds today, so the longer we count, the more accurate our estimate.
		// On the other hand, we don't want to count too long, because we don't
		// want the user to notice us spend time here, since most users will
		// probably never use the performance statistics.
		minDiff = freq.LowPart * 11 / 200;

		// Minimize the chance of task switching during the testing by going very
		// high priority. This is another reason to avoid timing for too long.
		SetPriorityClass (GetCurrentProcess (), REALTIME_PRIORITY_CLASS);
		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
		ClockCalibration = 0;
		clock (ClockCalibration);
		QueryPerformanceCounter (&count1);
		do
		{
			QueryPerformanceCounter (&count2);
		} while ((DWORD)((unsigned __int64)count2.QuadPart - (unsigned __int64)count1.QuadPart) < minDiff);
		unclock (ClockCalibration);
		QueryPerformanceCounter (&count2);
		SetPriorityClass (GetCurrentProcess (), NORMAL_PRIORITY_CLASS);
		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_NORMAL);

		CyclesPerSecond = (double)ClockCalibration *
			(double)freq.QuadPart /
			(double)((unsigned __int64)count2.QuadPart - (unsigned __int64)count1.QuadPart);
		SecondsPerCycle = 1.0 / CyclesPerSecond;
	}
	else
	{
		Printf ("Can't determine CPU speed, so pretending.\n");
	}

	Printf ("CPU Speed: %f MHz\n", CyclesPerSecond / 1e6);
}

//
// I_Quit
//
static int has_exited;

void STACK_ARGS I_Quit (void)
{
	has_exited = 1;		/* Prevent infinitely recursive exits -- killough */

	if (TimerEventID)
		timeKillEvent (TimerEventID);
	if (NewTicArrived)
		CloseHandle (NewTicArrived);

	timeEndPeriod (TimerPeriod);

	if (demorecording)
		G_CheckDemoStatus();
	G_ClearSnapshots ();
}


//
// I_Error
//
extern FILE *Logfile;
BOOL gameisdead;

extern "C" {

void STACK_ARGS I_FatalError (const char *error, ...)
{
	static BOOL alreadyThrown = false;
	gameisdead = true;

	if (!alreadyThrown)		// ignore all but the first message -- killough
	{
		char errortext[MAX_ERRORTEXT];
		int index;
		va_list argptr;
		va_start (argptr, error);
		index = vsprintf (errortext, error, argptr);
// GetLastError() is usually useless because we don't do a lot of Win32 stuff
//		sprintf (errortext + index, "\nGetLastError = %ld", GetLastError());
		va_end (argptr);

		// Record error to log (if logging)
		if (Logfile)
			fprintf (Logfile, "\n**** DIED WITH FATAL ERROR:\n%s\n", errortext);

		throw CFatalError (errortext);
	}

	if (!has_exited)	// If it hasn't exited yet, exit now -- killough
	{
		has_exited = 1;	// Prevent infinitely recursive exits -- killough
		exit(-1);
	}
}

void STACK_ARGS I_Error (const char *error, ...)
{
	va_list argptr;
	char errortext[MAX_ERRORTEXT];

	va_start (argptr, error);
	vsprintf (errortext, error, argptr);
	va_end (argptr);

	throw CRecoverableError (errortext);
}

}	// extern "C"

char DoomStartupTitle[256] = { 0 };

void I_SetTitleString (const char *title)
{
	int i;

	for (i = 0; title[i]; i++)
		DoomStartupTitle[i] = title[i];
}

void I_PrintStr (int xp, const char *cp, int count, BOOL scroll)
{
	// used in the DOS version
}

EXTERN_CVAR (Bool, queryiwad);
static WadStuff *WadList;
static int NumWads;

static void SetQueryIWad (HWND dialog)
{
	HWND checkbox = GetDlgItem (dialog, IDC_DONTASKIWAD);
	int state = SendMessage (checkbox, BM_GETCHECK, 0, 0);

	queryiwad = (state != BST_CHECKED);
}

BOOL CALLBACK IWADBoxCallback (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND list;
	int i;

	switch (message)
	{
	case WM_INITDIALOG:
		list = GetDlgItem (hDlg, IDC_IWADLIST);
		for (i = 0; i < NumWads; i++)
		{
			char work[256];
			char *filepart = strrchr (WadList[i].Path, '/');
			if (filepart == NULL)
				filepart = WadList[i].Path;
			else
				filepart++;
			sprintf (work, "%s (%s)", IWADTypeNames[WadList[i].Type], filepart);
			SendMessage (list, LB_ADDSTRING, 0, (LPARAM)work);
			SendMessage (list, LB_SETITEMDATA, i, (LPARAM)i);
		}
		SendMessage (list, LB_SETCURSEL, 0, 0);
		SetFocus (list);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog (hDlg, -1);
		}
		else if (LOWORD(wParam) == IDOK ||
			(LOWORD(wParam) == IDC_IWADLIST && HIWORD(wParam) == LBN_DBLCLK))
		{
			SetQueryIWad (hDlg);
			list = GetDlgItem (hDlg, IDC_IWADLIST);
			EndDialog (hDlg, SendMessage (list, LB_GETCURSEL, 0, 0));
		}
		break;
	}
	return FALSE;
}

int I_PickIWad (WadStuff *wads, int numwads)
{
	WadList = wads;
	NumWads = numwads;

	return DialogBox (hInstance, MAKEINTRESOURCE(IDD_IWADDIALOG),
		(HWND)Window, (DLGPROC)IWADBoxCallback);
}

long I_FindFirst (char *filespec, findstate_t *fileinfo)
{
	return (long)FindFirstFileA (filespec, (LPWIN32_FIND_DATAA)fileinfo);
}
int I_FindNext (long handle, findstate_t *fileinfo)
{
	return !FindNextFileA ((HANDLE)handle, (LPWIN32_FIND_DATAA)fileinfo);
}

int I_FindClose (long handle)
{
	return FindClose ((HANDLE)handle);
}
