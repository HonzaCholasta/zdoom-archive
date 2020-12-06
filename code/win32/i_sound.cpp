/* i_sound.cpp: System interface for sound, uses fmod.dll
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "resource.h"
extern HWND Window;
extern HINSTANCE hInstance;

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "doomtype.h"
#include "m_alloc.h"
#include <math.h>


#include <fmod.h>

#include "m_swap.h"
#include "z_zone.h"

#include "c_cvars.h"
#include "c_dispatch.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_music.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "i_video.h"
#include "s_sound.h"

#include "doomdef.h"

static const char *OutputNames[] =
{
	"No sound",
	"Windows Multimedia",
	"DirectSound",
	"A3D"
};
static const char *MixerNames[] =
{
	"Auto",
	"Non-MMX blendmode",
	"Pentium MMX",
	"PPro MMX",
	"Quality auto",
	"Quality FPU",
	"Quality Pentium MMX",
	"Quality PPro MMX"
};

EXTERN_CVAR (snd_sfxvolume)
CVAR (snd_samplerate, "44100", CVAR_ARCHIVE)
CVAR (snd_buffersize, "0", CVAR_ARCHIVE)
CVAR (snd_driver, "0", CVAR_ARCHIVE)
CVAR (snd_output, "default", CVAR_ARCHIVE)

// killough 2/21/98: optionally use varying pitched sounds
CVAR (snd_pitched, "0", CVAR_ARCHIVE)
#define PITCH(f,x) (snd_pitched.value ? ((f)*(x))/128 : (f))

// Maps sfx channels onto FMOD channels
static struct ChanMap
{
	int soundID;		// sfx playing on this channel
	long channelID;
	BOOL bIsLooping;
} *ChannelMap;

int _nosound = 0;
static int numChannels;

static const char *FmodErrors[] =
{
	"No errors",
	"Cannot call this command after FSOUND_Init.  Call FSOUND_Close first.",
	"This command failed because FSOUND_Init was not called",
	"Error initializing output device.",
	"Error initializing output device, but more specifically, the output device is already in use and cannot be reused.",
	"Playing the sound failed.",
	"Soundcard does not support the features needed for this soundsystem (16bit stereo output)",
	"Error setting cooperative level for hardware.",
	"Error creating hardware sound buffer.",
	"File not found",
	"Unknown file format",
	"Error loading file",
	"Not enough memory ",
	"The version number of this file format is not supported",
	"Incorrect mixer selected",
	"An invalid parameter was passed to this function",
	"Tried to use a3d and not an a3d hardware card, or dll didnt exist, try another output type.",
	"Tried to use an EAX command on a non EAX enabled channel or output.",
	"Failed to allocate a new channel"
};

// Simple file loader for FSOUND using an already-opened
// file handle. Also supports loading from a WAD opened
// in w_wad.cpp.

static unsigned long STACK_ARGS FIO_Open (char *name)
{
	return (unsigned long)name;
}

static void STACK_ARGS FIO_Close (unsigned long handle)
{
	if (handle)
	{
		delete (FileHandle *)handle;
	}
}

static long STACK_ARGS FIO_Read (void *buffer, long size, unsigned long handle)
{
	if (handle)
	{
		FileHandle *file = (FileHandle *)handle;
		if (size + file->pos > file->len)
			size = file->len - file->pos;
		if (size < 0)
		{
			size = 0;
		}
		else if (size > 0)
		{
			if (lseek (file->handle, file->base + file->pos, SEEK_SET) == -1)
				return 0;
			size = read (file->handle, buffer, size);
			file->pos += size;
		}
		return size;
	}
	return 0;
}

static void STACK_ARGS FIO_Seek (unsigned long handle, long pos, signed char mode)
{
	if (handle)
	{
		FileHandle *file = (FileHandle *)handle;

		switch (mode)
		{
		case SEEK_SET:
			file->pos = pos;
			break;
		case SEEK_CUR:
			file->pos += pos;
			break;
		case SEEK_END:
			file->pos = file->len + pos;
			break;
		}
		if (file->pos < 0)
			file->pos = 0;
		if (file->pos > file->len)
			file->pos = file->len;
	}
}

static long STACK_ARGS FIO_Tell (unsigned long handle)
{
	return handle ? ((FileHandle *)handle)->pos : 0;
}

void Enable_FSOUND_IO_Loader ()
{
	typedef unsigned long (*OpenCallback)(char *name);
	typedef void          (*CloseCallback)(unsigned long handle);
    typedef long          (*ReadCallback)(void *buffer, long size, unsigned long handle);
    typedef void		  (*SeekCallback)(unsigned long handle, long pos, signed char mode);
    typedef long          (*TellCallback)(unsigned long handle);
	
	FSOUND_File_SetCallbacks	
		((OpenCallback)FIO_Open,
		 (CloseCallback)FIO_Close,
		 (ReadCallback)FIO_Read,
		 (SeekCallback)FIO_Seek,
		 (TellCallback)FIO_Tell);
}

void Disable_FSOUND_IO_Loader ()
{
	FSOUND_File_SetCallbacks (NULL, NULL, NULL, NULL, NULL);
}

static void DoLoad (void **slot, sfxinfo_t *sfx)
{
	int size;
	int errcount;

	errcount = 0;
	while (errcount < 2)
	{
		if (errcount)
			sfx->lumpnum = W_GetNumForName ("dsempty");

		size = W_LumpLength (sfx->lumpnum);
		if (size == 0)
		{
			errcount++;
			continue;
		}

		// Assume WAVE data. If that fails, load it as DMX.
		*slot = FSOUND_Sample_LoadWav (FSOUND_UNMANAGED, (char *)new FileHandle (sfx->lumpnum), FSOUND_2D);
		if (*slot == NULL)
		{ // DMX sound (presumably)
			byte *sfxdata = (byte *)W_CacheLumpNum (sfx->lumpnum, PU_CACHE);
			SDWORD len = ((SDWORD *)sfxdata)[1];
			FSOUND_SAMPLE *sample;

			if (len > size - 8)
			{
				Printf (PRINT_HIGH, "%s is missing %d bytes\n", sfx->name, sfx->length - size + 8);
				len = size - 8;
			}

			sample = FSOUND_Sample_Alloc (FSOUND_UNMANAGED, len,
				FSOUND_2D|FSOUND_LOOP_OFF|FSOUND_8BITS|FSOUND_MONO|FSOUND_UNSIGNED,
				sfx->frequency, 255, FSOUND_STEREOPAN, 255);

			if (sample == NULL)
			{
				DPrintf ("Failed to allocate sample: %d\n", FSOUND_GetError ());
				errcount++;
				continue;
			}

			sfx->frequency = ((WORD *)sfxdata)[1];
			if (sfx->frequency == 0)
				sfx->frequency = 11025;
			sfx->ms = sfx->length = len;

			if (!FSOUND_Sample_Upload (sample, sfxdata+8, FSOUND_8BITS|FSOUND_MONO|FSOUND_UNSIGNED))
			{
				DPrintf ("Failed to upload sample: %d\n", FSOUND_GetError ());
				FSOUND_Sample_Free (sample);
				sample = NULL;
				errcount++;
				continue;
			}
			*slot = sample;
		}
		else
		{
			long probe;

			FSOUND_Sample_GetDefaults ((FSOUND_SAMPLE *)sfx->data, &probe,
				NULL, NULL, NULL);

			sfx->frequency = probe;
			sfx->ms = FSOUND_Sample_GetLength ((FSOUND_SAMPLE *)sfx->data);
			sfx->length = sfx->ms;
		}
		break;
	}

	sfx->ms = (sfx->ms * 1000) / (sfx->frequency);
	DPrintf ("sound loaded: %d Hz %d samples\n", sfx->frequency, sfx->length);
}

static void getsfx (sfxinfo_t *sfx)
{
	char sndtemp[128];
	unsigned int i;

	// Get the sound data from the WAD and register it with sound library

	// If the sound doesn't exist, try a generic male sound (if
	// this is a player sound) or the empty sound.
	if (sfx->lumpnum == -1)
	{
		char *basename;
		int sfx_id;

		if (!strnicmp (sfx->name, "player/", 7) &&
			 (basename = strchr (sfx->name + 7, '/')))
		{
			sprintf (sndtemp, "player/male/%s", basename+1);
			sfx_id = S_FindSound (sndtemp);
			if (sfx_id != -1)
				sfx->lumpnum = S_sfx[sfx_id].lumpnum;
		}

		if (sfx->lumpnum == -1)
			sfx->lumpnum = W_GetNumForName ("dsempty");
	}
	
	// See if there is another sound already initialized with this lump. If so,
	// then set this one up as a link, and don't load the sound again.
	for (i = 0; i < S_sfx.Size (); i++)
	{
		if (S_sfx[i].data && S_sfx[i].link == -1 && S_sfx[i].lumpnum == sfx->lumpnum)
		{
			DPrintf ("Linked to %s (%d)\n", S_sfx[i].name, i);
			sfx->link = i;
			sfx->ms = S_sfx[i].ms;
			sfx->data = S_sfx[i].data;
			sfx->altdata = S_sfx[i].altdata;
			return;
		}
	}

	sfx->bHaveLoop = false;
	sfx->normal = 0;
	sfx->looping = 0;
	sfx->altdata = NULL;
	DoLoad (&sfx->data, sfx);
}


// Right now, FMOD's biggest shortcoming compared to MIDAS is that it does not
// support multiple samples with the same sample data. Thus, if we want to
// play a looped and non-looped version of the same sound, we need to create
// two copies of it. Fortunately, most sounds will either be played looped or
// not, but not both at the same time, so this really isn't too much of a
// problem. This function juggles the sample between looping and non-looping,
// creating a copy if necessary. It also increments the appropriate use
// counter.
static FSOUND_SAMPLE *CheckLooping (sfxinfo_t *sfx, BOOL looped)
{
	if (looped)
	{
		sfx->looping++;
		if (sfx->bHaveLoop)
		{
			return (FSOUND_SAMPLE *)(sfx->altdata ? sfx->altdata : sfx->data);
		}
		else
		{
			if (sfx->normal == 0)
			{
				sfx->bHaveLoop = true;
				FSOUND_Sample_SetLoopMode ((FSOUND_SAMPLE *)sfx->data,
					FSOUND_LOOP_NORMAL);
				return (FSOUND_SAMPLE *)sfx->data;
			}
		}
	}
	else
	{
		sfx->normal++;
		if (sfx->altdata || !sfx->bHaveLoop)
		{
			return (FSOUND_SAMPLE *)sfx->data;
		}
		else
		{
			if (sfx->looping == 0)
			{
				sfx->bHaveLoop = false;
				FSOUND_Sample_SetLoopMode ((FSOUND_SAMPLE *)sfx->data,
					FSOUND_LOOP_OFF);
				return (FSOUND_SAMPLE *)sfx->data;
			}
		}
	}

	// If we get here, we need to create an alternate version of the sample.
	FSOUND_Sample_SetLoopMode ((FSOUND_SAMPLE *)sfx->data,
		FSOUND_LOOP_OFF);
	DoLoad (&sfx->altdata, sfx);
	FSOUND_Sample_SetLoopMode ((FSOUND_SAMPLE *)sfx->altdata,
		FSOUND_LOOP_NORMAL);
	sfx->bHaveLoop = true;
	return (FSOUND_SAMPLE *)(looped ? sfx->altdata : sfx->data);
}

void UncheckSound (sfxinfo_t *sfx, BOOL looped)
{
	if (looped)
	{
		if (sfx->looping > 0)
			sfx->looping--;
	}
	else
	{
		if (sfx->normal > 0)
			sfx->normal--;
	}
}

//
// SFX API
//
 
void I_SetSfxVolume (int volume)
{
	// volume range is 0-15, but FMOD wants 0-255
	FSOUND_SetSFXMasterVolume ((volume << 4) | volume);
}


//
// Starting a sound means adding it
//		to the current list of active sounds
//		in the internal channels.
// As the SFX info struct contains
//		e.g. a pointer to the raw data,
//		it is ignored.
//
// vol range is 0-255
// sep range is 0-255, -1 for surround, -2 for full vol middle
int I_StartSound (sfxinfo_t *sfx, int vol, int sep, int pitch, int channel, BOOL looping)
{
	if (_nosound)
		return 0;

	int id = sfx - &S_sfx[0];
	long volume;
	long pan;
	long freq;
	long chan;

	freq = PITCH(sfx->frequency,pitch);
	volume = vol;

	if (sep < 0)
	{
		pan = 128; // FSOUND_STEREOPAN is too loud relative to everything else
	}
	else
	{
		pan = sep;
	}

	chan = FSOUND_PlaySoundAttrib (FSOUND_FREE/*1+channel*/, CheckLooping (sfx, looping), freq, vol, pan);
	
	if (chan != -1)
	{
		FSOUND_SetReserved (chan, TRUE);
		FSOUND_SetSurround (chan, sep == -1 ? TRUE : FALSE);
		ChannelMap[channel].channelID = chan;
		ChannelMap[channel].soundID = id;
		ChannelMap[channel].bIsLooping = looping;
		return channel + 1;
	}
	else
	{
		DPrintf ("Sound failed to play: %d\n", FSOUND_GetError ());
	}

	return 0;
}



void I_StopSound (int handle)
{
	if (_nosound || !handle)
		return;

	handle--;
	if (ChannelMap[handle].soundID != -1)
	{
		FSOUND_StopSound (ChannelMap[handle].channelID);
		FSOUND_SetReserved (ChannelMap[handle].channelID, FALSE);
		UncheckSound (&S_sfx[ChannelMap[handle].soundID], ChannelMap[handle].bIsLooping);
		ChannelMap[handle].soundID = -1;
	}
}


int I_SoundIsPlaying (int handle)
{
	if (_nosound || !handle)
		return 0;

	handle--;
	if (ChannelMap[handle].soundID == -1)
	{
		return 0;
	}
	else
	{
		int is = FSOUND_IsPlaying (ChannelMap[handle].channelID);
		if (!is)
		{
			FSOUND_SetReserved (ChannelMap[handle].channelID, FALSE);
			UncheckSound (&S_sfx[ChannelMap[handle].soundID], ChannelMap[handle].bIsLooping);
			ChannelMap[handle].soundID = -1;
		}
		return is;
	}
}


void I_UpdateSoundParams (int handle, int vol, int sep, int pitch)
{
	if (_nosound || !handle)
		return;

	handle--;
	if (ChannelMap[handle].soundID == -1)
		return;

	long volume;
	long pan;
	long freq;

	freq = PITCH(S_sfx[ChannelMap[handle].soundID].frequency,pitch);
	volume = vol;

	if (sep < 0)
	{
		pan = 128; //FSOUND_STEREOPAN
	}
	else
	{
		pan = sep;
	}

	FSOUND_SetSurround (ChannelMap[handle].channelID, sep == -1 ? TRUE : FALSE);
	FSOUND_SetPan (ChannelMap[handle].channelID, pan);
	FSOUND_SetVolume (ChannelMap[handle].channelID, volume);
	FSOUND_SetFrequency (ChannelMap[handle].channelID, freq);
}


void I_LoadSound (struct sfxinfo_struct *sfx)
{
	if (_nosound)
		return;

	if (!sfx->data)
	{
		int i = sfx - &S_sfx[0];
		DPrintf ("loading sound \"%s\" (%d)\n", sfx->name, i);
		getsfx (sfx);
	}
}

// [RH] Dialog procedure for the error dialog that appears if FMOD
//		could not be initialized for some reason.
BOOL CALLBACK InitBoxCallback (HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		if (wParam == IDOK ||
			wParam == IDC_NOSOUND ||
			wParam == IDCANCEL)
		{
			EndDialog (hwndDlg, wParam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static char FModLog (char success)
{
	if (success)
	{
		Printf (PRINT_HIGH, " succeeded\n");
	}
	else
	{
		Printf (PRINT_HIGH, " failed (error %d)\n", FSOUND_GetError());
	}
	return success;
}

void I_InitSound ()
{
	static const FSOUND_OUTPUTTYPES outtypes[3] =
		{ FSOUND_OUTPUT_DSOUND, FSOUND_OUTPUT_WINMM, FSOUND_OUTPUT_DSOUND };

	/* Get command line options: */
	_nosound = !!Args.CheckParm ("-nosfx") || !!Args.CheckParm ("-nosound");

	if (_nosound)
	{
		return;
	}

	int outindex;
	int trynum;

	if (stricmp (snd_output.string, "dsound") == 0)
	{
		outindex = 0;
	}
	else if (stricmp (snd_output.string, "winmm") == 0)
	{
		outindex = 1;
	}
	else
	{
		outindex = (OSPlatform == os_WinNT) ? 1 : 0;
	}

	Printf (PRINT_HIGH, "I_InitSound: Initializing FMOD\n");
	FSOUND_SetHWND (Window);

	while (!_nosound)
	{
		trynum = 0;
		while (trynum < 2)
		{
			Printf (PRINT_HIGH, "  Setting %s output",
				OutputNames[outtypes[outindex+trynum]]);
			FModLog (FSOUND_SetOutput (outtypes[outindex+trynum]));
			if (FSOUND_GetOutput() != outtypes[outindex+trynum])
			{
				Printf (PRINT_HIGH, "  Got %s output instead.\n",
					OutputNames[FSOUND_GetOutput()]);
				trynum++;
				continue;
			}
			Printf (PRINT_HIGH, "  Setting driver %d",
				(int)snd_driver.value);
			FModLog (FSOUND_SetDriver ((int)snd_driver.value));
			if (FSOUND_GetOutput() != outtypes[outindex+trynum])
			{
				Printf (PRINT_HIGH, "   Output changed to %s\n   Trying driver 0",
					OutputNames[FSOUND_GetOutput()]);
				FSOUND_SetOutput (outtypes[outindex+trynum]);
				FModLog (FSOUND_SetDriver (0));
			}
			if (snd_buffersize.value)
			{
				Printf (PRINT_HIGH, "  Setting buffer size %d",
					(int)snd_buffersize.value);
				FModLog (FSOUND_SetBufferSize ((int)snd_buffersize.value));
			}
			Printf (PRINT_HIGH, "  Initialization");
			if (!FModLog (FSOUND_Init ((int)snd_samplerate.value, 64, 0)))
			{
				trynum++;
			}
			else
			{
				break;
			}
		}
		if (trynum < 2)
		{ // Initialized successfully
			break;
		}
		// If sound cannot be initialized, give the user some options.
		switch (DialogBox (hInstance,
						   MAKEINTRESOURCE(IDD_FMODINITFAILED),
						   (HWND)Window,
						   (DLGPROC)InitBoxCallback))
		{
		case IDC_NOSOUND:
			_nosound = true;
			break;

		case IDCANCEL:
			exit (0);
			break;
		}
	}

	if (!_nosound)
	{
		Enable_FSOUND_IO_Loader ();
		static bool didthis = false;
		if (!didthis)
		{
			didthis = true;
			atterm (I_ShutdownSound);
		}
	}

	I_InitMusic ();

	snd_sfxvolume.Callback ();
}

void I_SetChannels (int numchannels)
{
	int i;

	if (_nosound)
		return;

	ChannelMap = new ChanMap[numchannels];
	for (i = 0; i < numchannels; i++)
	{
		/*
		if (!FSOUND_SetReserved (i, TRUE))
		{
			_nosound = true;
		}
		*/
		ChannelMap[i].soundID = -1;
	}

	numChannels = numchannels;
}

void STACK_ARGS I_ShutdownSound (void)
{
	if (!_nosound)
	{
		unsigned int i, c = 0;
		size_t len = 0;

		FSOUND_StopAllChannels ();

		if (ChannelMap)
		{
			delete[] ChannelMap;
			ChannelMap = NULL;
		}

		// Free all loaded samples
		for (i = 0; i < S_sfx.Size (); i++)
		{
			if (S_sfx[i].link == -1 && S_sfx[i].data)
			{
				if (S_sfx[i].data)
				{
					FSOUND_Sample_Free ((FSOUND_SAMPLE *)S_sfx[i].data);
					len += S_sfx[i].length;
					c++;
				}
				if (S_sfx[i].altdata)
				{
					FSOUND_Sample_Free ((FSOUND_SAMPLE *)S_sfx[i].altdata);
					S_sfx[i].altdata = NULL;
				}
				S_sfx[i].bHaveLoop = false;
			}
			S_sfx[i].data = NULL;
		}

		FSOUND_Close ();
	}
}

BEGIN_COMMAND (snd_status)
{
	if (_nosound)
	{
		Printf (PRINT_HIGH, "sound is not active\n");
		return;
	}

	long output = FSOUND_GetOutput ();
	long driver = FSOUND_GetDriver ();
	long mixer = FSOUND_GetMixer ();
	unsigned long caps;

	Printf (PRINT_HIGH, "Output: %s\n", OutputNames[output]);
	Printf (PRINT_HIGH, "Driver: %d (%s)\n", driver,
		FSOUND_GetDriverName (driver));
	Printf (PRINT_HIGH, "Mixer: %s\n", MixerNames[mixer]);
	if (FSOUND_GetDriverCaps (driver, &caps))
	{
		Printf (PRINT_HIGH, "Driver caps: 0x%x\n", caps);
	}
}
END_COMMAND (snd_status)

BEGIN_COMMAND (snd_reset)
{
	I_ShutdownMusic ();
	I_ShutdownSound ();
	I_InitSound ();
	I_SetChannels (numChannels);
	S_RestartMusic ();
}
END_COMMAND (snd_reset)

BEGIN_COMMAND (snd_listdrivers)
{
	long numdrivers = FSOUND_GetNumDrivers ();
	long i;

	for (i = 0; i < numdrivers; i++)
	{
		Printf (PRINT_HIGH, "%ld. %s\n", i, FSOUND_GetDriverName (i));
	}
}
END_COMMAND (snd_listdrivers)
