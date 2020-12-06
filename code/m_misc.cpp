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
//
// $Log:$
//
// DESCRIPTION:
//		Main loop menu stuff.
//		Default Config File.
//		PCX Screenshots.
//
//-----------------------------------------------------------------------------


#include "doomtype.h"
#include "version.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#include <ctype.h>

#include "m_alloc.h"

#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

#include "w_wad.h"

#include "c_cvars.h"
#include "c_dispatch.h"
#include "c_bind.h"

#include "i_system.h"
#include "i_video.h"
#include "v_video.h"

#include "hu_stuff.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

#include "cmdlib.h"

#include "g_game.h"
#include "gi.h"

//
// M_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

BOOL M_WriteFile (char const *name, void *source, int length)
{
	int handle;
	int count;

	handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

	if (handle == -1)
		return false;

	count = write (handle, source, length);
	close (handle);

	if (count < length)
		return false;

	return true;
}


//
// M_ReadFile
//
int M_ReadFile (char const *name, byte **buffer)
{
	int handle, count, length;
	struct stat fileinfo;
	byte *buf;

	handle = open (name, O_RDONLY | O_BINARY, 0666);
	if (handle == -1)
		I_Error ("Couldn't read file %s", name);
	if (fstat (handle,&fileinfo) == -1)
		I_Error ("Couldn't read file %s", name);
	length = fileinfo.st_size;
	buf = (byte *)Z_Malloc (length, PU_STATIC, NULL);
	count = read (handle, buf, length);
	close (handle);

	if (count < length)
		I_Error ("Couldn't read file %s", name);

	*buffer = buf;
	return length;
}

//---------------------------------------------------------------------------
//
// PROC M_FindResponseFile
//
//---------------------------------------------------------------------------

#define MAXARGVS 100

void M_FindResponseFile (void)
{
	int i;

	for (i = 1; i < Args.NumArgs(); i++)
	{
		if (Args.GetArg(i)[0] == '@')
		{
			char	**argv;
			FILE	*handle;
			int 	size;
			int 	k;
			int 	index;
			int 	indexinfile;
			char	*infile;
			char	*file;

			// READ THE RESPONSE FILE INTO MEMORY
			handle = fopen (Args.GetArg(i) + 1,"rb");
			if (!handle)
				I_FatalError ("\nNo such response file!");

			Printf (PRINT_HIGH, "Found response file %s!\n", Args.GetArg(i) + 1);
			fseek (handle, 0, SEEK_END);
			size = ftell (handle);
			fseek (handle, 0, SEEK_SET);
			file = new char[size];
			fread (file, size, 1, handle);
			fclose (handle);

			argv = new char *[MAXARGVS];
			for (index = 0; index < i; index++)
				argv[index] = Args.GetArg (index);

			infile = file;
			k = 0;
			indexinfile = index;
			do
			{
				argv[indexinfile++] = infile+k;
				while(k < size &&
					  ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
					k++;
				*(infile+k) = 0;
				while(k < size &&
					  ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
					k++;
			} while(k < size);

			for (index = i + 1; index < Args.NumArgs (); index++)
				argv[indexinfile++] = Args.GetArg (index);

			DArgs newargs (indexinfile, argv);
			Args = newargs;

			delete[] file;
		
			// DISPLAY ARGS
			Printf (PRINT_HIGH, "%d command-line args:\n", Args.NumArgs ());
			for (k = 1; k < Args.NumArgs (); k++)
				Printf (PRINT_HIGH, "%s\n", Args.GetArg (k));

			break;
		}
	}
}


//
// DEFAULTS
//
// [RH] Handled by console code now.

#ifdef UNIX
char *GetUserFile (const char *file)
{
	char *home = getenv ("HOME");
	if (home == NULL || *home == '\0')
		I_FatalError ("Please set your HOME variable");

	char *path = new char[strlen (home) + 9 + strlen (file)];
	strcpy (path, home);
	if (home[strlen (home)-1] != '/')
		strcat (path, "/.zdoom");
	else
		strcat (path, ".zdoom");

	struct stat info;
	if (stat (path, &info) == -1)
	{
		if (mkdir (path, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
		{
			I_FatalError ("Failed to create %s directory:\n%s",
						  path, strerror (errno));
		}
	}
	else
	{
		if (!S_ISDIR(info.st_mode))
		{
			I_FatalError ("%s must be a directory", path);
		}
	}
	strcat (path, "/");
	strcat (path, file);
	return path;
}
#endif

// [RH] Get configfile path.
// This file contains commands to set all
// archived cvars, bind commands to keys,
// and set other general game information.
char *GetConfigPath (void)
{
	static const char *confignames[3][2] =
	{
		{ "zdoom.cfg", "c:\\zdoomdot\\zdoom.cfg" },
		{ "zheretic.cfg", "c:\\zdoomdot\\zheretic.cfg" },
		{ "zhexen.cfg", "c:\\zdoomdot\\zhexen.cfg" },
	};

	int configtype;
	char *path;

	path = Args.CheckValue ("-config");
	if (path)
		return copystring (path);

	configtype = (gameinfo.gametype == GAME_Doom) ? 0 :
		(gameinfo.gametype == GAME_Heretic) ? 1 : 2;
#ifndef UNIX
	if (Args.CheckParm ("-cdrom"))
		return copystring (confignames[configtype][1]);

	path = new char[strlen (progdir) + strlen(confignames[configtype][0]) + 1];

	strcpy (path, progdir);
	strcat (path, confignames[configtype][0]);
	return path;
#else
	return GetUserFile (confignames[configtype][0]);
#endif
}

static cvar_t autovar ("autoexec", "", CVAR_ARCHIVE);

char *GetAutoexecPath (void)
{
#ifndef UNIX
	if (Args.CheckParm ("-cdrom"))
	{
		return copystring ("c:\\zdoomdat\\autoexec.cfg");
	}
	else
	{
		char *path = new char[strlen (progdir) + 13];

		strcpy (path, progdir);
		strcat (path, "autoexec.cfg");

		autovar.SetDefault (path);
		delete[] path;

		return copystring (autovar.string);
	}
#else
	return GetUserFile ("autoexec.cfg");
#endif
}

//
// M_SaveDefaults
//

// [RH] Don't write a config file if M_LoadDefaults hasn't been called.
static BOOL DefaultsLoaded;

void STACK_ARGS M_SaveDefaults (void)
{
	FILE *f;
	char *configfile;

	if (!DefaultsLoaded)
		return;

	configfile = GetConfigPath ();

	// Make sure the user hasn't changed configver
	cvar_set ("configver", CONFIGVERSIONSTR);

	if ( (f = fopen (configfile, "w")) )
	{
		fprintf (f, "// Generated by ZDOOM " DOTVERSIONSTR " - don't hurt anything\n");

		// Archive all cvars marked as CVAR_ARCHIVE
		C_ArchiveCVars (f);

		// Archive all active key bindings
		C_ArchiveBindings (f);

		// Archive all aliases
		C_ArchiveAliases (f);

		fclose (f);
	}
	delete[] configfile;
}


//
// M_LoadDefaults
//
extern byte scantokey[128];
extern int cvar_defflags;
EXTERN_CVAR (dimamount)

// Used to identify the version of the game that saved
// a config file to compensate for new features that get
// put into newer configfiles.
static CVAR (configver, CONFIGVERSIONSTR, CVAR_ARCHIVE)

void M_LoadDefaults ()
{
	extern char DefBindings[];
	char *configfile;
	char *execcommand;

	// Set default key bindings. These will be overridden
	// by the bindings in the config file if it exists.
	AddCommandString ("binddefaults");

	if (gameinfo.gametype & GAME_Raven)
	{
		EXTERN_CVAR (con_centernotify);
		EXTERN_CVAR (msg0color);
		EXTERN_CVAR (dimcolor);
		EXTERN_CVAR (color);
		EXTERN_CVAR (splashfactor);

		con_centernotify.SetDefault ("1");
		msg0color.SetDefault ("9");
		dimcolor.SetDefault ("0 0 f");

		if (gameinfo.gametype == GAME_Heretic)
		{
			color.SetDefault ("3f 60 40");
			Strings[333].builtin = "zticsv";	// 333 = SAVEGAMENAME
		}
		else	// Hexen
		{
			Strings[333].builtin = "zhexsv";
		}
	}

	configfile = GetConfigPath ();
	execcommand = new char[strlen (configfile) + 8];
	sprintf (execcommand, "exec \"%s\"", configfile);
	delete[] configfile;
	cvar_defflags = CVAR_ARCHIVE;
	AddCommandString (execcommand);
	cvar_defflags = 0;
	delete[] execcommand;

	configfile = GetAutoexecPath ();
	execcommand = new char[strlen (configfile) + 8];
	sprintf (execcommand, "exec \"%s\"", configfile);
	delete[] configfile;
	AddCommandString (execcommand);
	delete[] execcommand;

	if (configver.value < 118.0f)
	{
		AddCommandString ("alias idclip noclip");
		AddCommandString ("alias idspispopd noclip");

		if (configver.value < 117.2f)
		{
			dimamount.Set (dimamount.value / 4);
			if (configver.value <= 113.0f)
			{
				AddCommandString ("bind t messagemode; bind \\ +showscores;"
								  "bind f12 spynext; bind sysrq screenshot");
				if (C_GetBinding (KEY_F5) && !stricmp (C_GetBinding (KEY_F5), "menu_video"))
					AddCommandString ("bind f5 menu_display");
			}
		}
	}

	DefaultsLoaded = true;
	atterm (M_SaveDefaults);
}


//
// SCREEN SHOTS
//


typedef struct
{
	char				manufacturer;
	char				version;
	char				encoding;
	char				bits_per_pixel;

	unsigned short		xmin;
	unsigned short		ymin;
	unsigned short		xmax;
	unsigned short		ymax;
	
	unsigned short		hdpi;
	unsigned short		vdpi;

	unsigned char		palette[48];
	
	char				reserved;
	char				color_planes;
	unsigned short		bytes_per_line;
	unsigned short		palette_type;
	
	char				filler[58];
	unsigned char		data;			// unbounded
} pcx_t;


//
// WritePCXfile
//
void WritePCXfile (char *filename, byte *data, int width, int height, DWORD *palette)
{
	int x, y;
	int runlen;
	BYTE color;
	pcx_t pcx;
	FILE *file;

	pcx.manufacturer = 10;				// PCX id
	pcx.version = 5;					// 256 color
	pcx.encoding = 1;
	pcx.bits_per_pixel = 8;				// 256 color
	pcx.xmin = 0;
	pcx.ymin = 0;
	pcx.xmax = width-1;
	pcx.ymax = height-1;
	pcx.hdpi = 75;
	pcx.vdpi = 75;
	memset (pcx.palette, 0, sizeof(pcx.palette));
	pcx.reserved = 0;
	pcx.color_planes = 1;				// chunky image
	pcx.bytes_per_line = width + (width & 1);
	pcx.palette_type = 1;				// not a grey scale
	memset (pcx.filler, 0, sizeof(pcx.filler));

	file = fopen (filename, "wb");
	if (file == NULL)
	{
		fprintf (stderr, "Could not open %s for writing\n", filename);
		return;
	}

	fwrite (&pcx, sizeof(pcx), 1, file);

	// pack the image
	for (y = height; y > 0; y--)
	{
		color = *data++;
		runlen = 1;

		for (x = width - 1; x > 0; x--)
		{
			if (*data == color)
			{
				runlen++;
			}
			else
			{
				if (runlen > 1 || color >= 0xc0)
				{
					while (runlen > 63)
					{
						putc (0xff, file);
						putc (color, file);
						runlen -= 63;
					}
					if (runlen > 0)
					{
						putc (0xc0 + runlen, file);
					}
				}
				if (runlen > 0)
				{
					putc (color, file);
				}
				runlen = 1;
				color = *data;
			}
			data++;
		}

		if (runlen > 1 || color >= 0xc0)
		{
			while (runlen > 63)
			{
				putc (0xff, file);
				putc (color, file);
				runlen -= 63;
			}
			if (runlen > 0)
			{
				putc (0xc0 + runlen, file);
			}
		}
		if (runlen > 0)
		{
			putc (color, file);
		}

		if (width & 1)
			putc (0, file);
	}

	// write the palette
	putc (12, file);		// palette ID byte
	for (x = 0; x < 256; x++, palette++)
	{
		putc (RPART(*palette), file);
		putc (GPART(*palette), file);
		putc (BPART(*palette), file);
	}

	fclose (file);
}


//
// M_ScreenShot
//
static BOOL FindFreeName (char *lbmname, const char *extension)
{
	int i;

	for (i=0 ; i<=9999 ; i++)
	{
		sprintf (lbmname, "DOOM%04d.%s", i, extension);
		if (!FileExists (lbmname))
			break;		// file doesn't exist
	}
	if (i==10000)
		return false;
	else
		return true;
}

extern DWORD IndexedPalette[256];

void M_ScreenShot (char *filename)
{
	byte *linear;
	char  autoname[32];
	char *lbmname;

	// find a file name to save it to
	if (!filename)
	{
#ifndef UNIX
		if (Args.CheckParm ("-cdrom"))
		{
			strcpy (autoname, "C:\\ZDOOMDAT\\");
			lbmname = autoname + 12;
		}
		else
#endif
		{
			lbmname = autoname;
		}
		if (!FindFreeName (lbmname, "tga\0pcx" + (screen->is8bit << 2)))
		{
			Printf (PRINT_HIGH, "M_ScreenShot: Delete some screenshots\n");
			return;
		}
		filename = autoname;
	}

	if (screen->is8bit)
	{
		// munge planar buffer to linear
		linear = new byte[screen->width * screen->height];
		I_ReadScreen (linear);
		
		// save the pcx file
		WritePCXfile (filename, linear,
					  screen->width, screen->height,
					  IndexedPalette);

		delete[] linear;
	}
	else
	{
		// save the tga file
		//I_WriteTGAfile (filename, &screen);
	}
	Printf (PRINT_HIGH, "screen shot\n");
}

BEGIN_COMMAND (screenshot)
{
	if (argc == 1)
		G_ScreenShot (NULL);
	else
		G_ScreenShot (argv[1]);
}
END_COMMAND (screenshot)
