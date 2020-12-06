/*
** gameconfigfile.cpp
** An .ini parser specifically for zdoom.ini
**
**---------------------------------------------------------------------------
** Copyright 1998-2001 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include <stdio.h>

#include "doomdef.h"
#include "gameconfigfile.h"
#include "c_cvars.h"
#include "c_dispatch.h"
#include "c_bind.h"
#include "gstrings.h"
#include "m_argv.h"
#include "cmdlib.h"
#include "version.h"
#include "m_misc.h"

EXTERN_CVAR (Bool, con_centernotify);
EXTERN_CVAR (Int, msg0color);
EXTERN_CVAR (Color, dimcolor);
EXTERN_CVAR (Color, color);
EXTERN_CVAR (Float, dimamount);

FGameConfigFile::FGameConfigFile ()
{
	char *pathname;
	
	bMigrating = false;
	pathname = GetConfigPath ();
	ChangePathName (pathname);
	delete[] pathname;
	LoadConfigFile (MigrateStub, NULL);

	if (!HaveSections ())
	{ // Config file not found, try the old one
		MigrateOldConfig ();
	}

	// Set default IWAD search paths if none present
	if (!SetSection ("IWADSearch.Directories"))
	{
		SetSection ("IWADSearch.Directories", true);
		SetValueForKey ("Path", ".", true);
		SetValueForKey ("Path", "$DOOMWADDIR", true);
#ifndef unix
		SetValueForKey ("Path", "$HOME", true);
		SetValueForKey ("Path", "$PROGDIR", true);
#else
		SetValueForKey ("Path", "~/.zdoom", true);
		SetValueForKey ("Path", SHARE_DIR, true);
#endif
	}

	// Set default search paths if none present
	if (!SetSection ("FileSearch.Directories"))
	{
		SetSection ("FileSearch.Directories", true);
#ifndef unix
		SetValueForKey ("Path", "$PROGDIR", true);
#else
		SetValueForKey ("Path", SHARE_DIR, true);
#endif
		SetValueForKey ("Path", "$DOOMWADDIR", true);
	}
}

FGameConfigFile::~FGameConfigFile ()
{
}

void FGameConfigFile::WriteCommentHeader (FILE *file) const
{
	fputs ("# This file was automatically generated by ZDOOM " DOTVERSIONSTR "\n\n", file);
}

void FGameConfigFile::MigrateStub (const char *pathname, FConfigFile *config, void *userdata)
{
	static_cast<FGameConfigFile *>(config)->bMigrating = true;
}

void FGameConfigFile::MigrateOldConfig ()
{
	// Set default key bindings. These will be overridden
	// by the bindings in the config file if it exists.
	C_SetDefaultBindings ();

#if 0	// Disabled for now, maybe forever.
	int i;
	char *execcommand;

	i = strlen (GetPathName ()) + 8;
	execcommand = new char[i];
	sprintf (execcommand, "exec \"%s\"", GetPathName ());
	execcommand[i-5] = 'c';
	execcommand[i-4] = 'f';
	execcommand[i-3] = 'g';
	cvar_defflags = CVAR_ARCHIVE;
	C_DoCommand (execcommand);
	cvar_defflags = 0;
	delete[] execcommand;

	FBaseCVar *configver = FindCVar ("configver", NULL);
	if (configver != NULL)
	{
		UCVarValue oldver = configver->GetGenericRep (CVAR_Float);

		if (oldver.Float < 118.f)
		{
			C_DoCommand ("alias idclip noclip");
			C_DoCommand ("alias idspispopd noclip");

			if (oldver.Float < 117.2f)
			{
				dimamount = *dimamount * 0.25f;
				if (oldver.Float <= 113.f)
				{
					C_DoCommand ("bind t messagemode; bind \\ +showscores;"
								 "bind f12 spynext; bind sysrq screenshot");
					if (C_GetBinding (KEY_F5) && !stricmp (C_GetBinding (KEY_F5), "menu_video"))
					{
						C_ChangeBinding ("menu_display", KEY_F5);
					}
				}
			}
		}
		delete configver;
	}
	// Change all impulses to slot commands
	for (i = 0; i < NUM_KEYS; i++)
	{
		char slotcmd[8] = "slot ";
		char *bind, *numpart;

		bind = C_GetBinding (i);
		if (bind != NULL && strnicmp (bind, "impulse ", 8) == 0)
		{
			numpart = strchr (bind, ' ');
			if (numpart != NULL && strlen (numpart) < 4)
			{
				strcpy (slotcmd + 5, numpart);
				C_ChangeBinding (slotcmd, i);
			}
		}
	}

	// Migrate and delete some obsolete cvars
	FBaseCVar *oldvar;
	UCVarValue oldval;

	oldvar = FindCVar ("autoexec", NULL);
	if (oldvar != NULL)
	{
		oldval = oldvar->GetGenericRep (CVAR_String);
		if (oldval.String[0])
		{
			SetSection ("Doom.AutoExec", true);
			SetValueForKey ("Path", oldval.String, true);
		}
		delete oldvar;
	}

	oldvar = FindCVar ("def_patch", NULL);
	if (oldvar != NULL)
	{
		oldval = oldvar->GetGenericRep (CVAR_String);
		if (oldval.String[0])
		{
			SetSection ("Doom.DefaultDehacked", true);
			SetValueForKey ("Path", oldval.String, true);
		}
		delete oldvar;
	}

	oldvar = FindCVar ("vid_noptc", NULL);
	if (oldvar != NULL)
	{
		delete oldvar;
	}
#endif
}

void FGameConfigFile::DoGlobalSetup ()
{
	if (SetSection ("GlobalSettings.Unknown"))
	{
		ReadCVars (CVAR_GLOBALCONFIG);
	}
	if (SetSection ("GlobalSettings"))
	{
		ReadCVars (CVAR_GLOBALCONFIG);
	}
#ifdef _WIN32
	if (SetSection ("LastRun"))
	{
		const char *lastver = GetValueForKey ("Version");
		if (lastver != NULL && atof (lastver) < 123.1f)
		{
			FBaseCVar *noblitter = FindCVar ("vid_noblitter", NULL);
			if (noblitter != NULL)
			{
				noblitter->ResetToDefault ();
			}
		}
	}
#endif
}

void FGameConfigFile::DoGameSetup (const char *gamename)
{
	char section[64];
	char *subsection;
	const char *key;
	const char *value;
	enum { Doom, Heretic, Hexen } game;

	if (strcmp (gamename, "Heretic") == 0)
		game = Heretic;
	else if (strcmp (gamename, "Hexen") == 0)
		game = Hexen;
	else
		game = Doom;

	if (bMigrating)
	{
		MigrateOldConfig ();
	}
	subsection = section + sprintf (section, "%s.", gamename);
	
	strcpy (subsection, "UnknownConsoleVariables");
	if (SetSection (section))
	{
		ReadCVars (0);
	}

	strcpy (subsection, "ConsoleVariables");
	if (!SetSection (section))
	{ // Config file does not contain cvars for the given game
		if (game != Doom)
		{
			SetRavenDefaults (game == Hexen);
		}
	}
	else
	{
		ReadCVars (0);
	}

	strcpy (subsection, "Player");
	if (SetSection (section))
	{
		ReadCVars (0);
	}

	strcpy (subsection, "Bindings");
	if (!SetSection (section))
	{ // Config has no bindings for the given game
		if (!bMigrating)
		{
			C_SetDefaultBindings ();
		}
	}
	else
	{
		C_UnbindAll ();
		while (NextInSection (key, value))
		{
			C_DoBind (key, value, false);
		}
	}

	strcpy (subsection, "DoubleBindings");
	if (SetSection (section))
	{
		while (NextInSection (key, value))
		{
			C_DoBind (key, value, true);
		}
	}

	strcpy (subsection, "ConsoleAliases");
	if (SetSection (section))
	{
		const char *name = NULL;
		while (NextInSection (key, value))
		{
			if (stricmp (key, "Name") == 0)
			{
				name = value;
			}
			else if (stricmp (key, "Command") == 0 && name != NULL)
			{
				C_SetAlias (name, value);
				name = NULL;
			}
		}
	}
}

void FGameConfigFile::ReadCVars (DWORD flags)
{
	const char *key, *value;
	FBaseCVar *cvar;
	UCVarValue val;

	while (NextInSection (key, value))
	{
		cvar = FindCVar (key, NULL);
		if (cvar == NULL)
		{
			cvar = new FStringCVar (key, NULL,
				CVAR_AUTO|CVAR_UNSETTABLE|CVAR_ARCHIVE|flags);
		}
		val.String = const_cast<char *>(value);
		cvar->SetGenericRep (val, CVAR_String);
	}
}

void FGameConfigFile::ArchiveGameData (const char *gamename)
{
	char section[64], *subsection;

	subsection = section + sprintf (section, "%s.", gamename);

	strcpy (subsection, "Player");
	SetSection (section, true);
	ClearCurrentSection ();
	C_ArchiveCVars (this, 4);

	strcpy (subsection, "ConsoleVariables");
	SetSection (section, true);
	ClearCurrentSection ();
	C_ArchiveCVars (this, 0);

	strcpy (subsection, "UnknownConsoleVariables");
	SetSection (section, true);
	ClearCurrentSection ();
	C_ArchiveCVars (this, 2);

	strcpy (subsection, "ConsoleAliases");
	SetSection (section, true);
	ClearCurrentSection ();
	C_ArchiveAliases (this);

	M_SaveCustomKeys (this, section, subsection);

	strcpy (subsection, "Bindings");
	SetSection (section, true);
	ClearCurrentSection ();
	C_ArchiveBindings (this, false);

	strcpy (subsection, "DoubleBindings");
	SetSection (section, true);
	ClearCurrentSection ();
	C_ArchiveBindings (this, true);
}

void FGameConfigFile::ArchiveGlobalData ()
{
	SetSection ("LastRun", true);
	ClearCurrentSection ();
	SetValueForKey ("Version", STRVERSION);

	SetSection ("GlobalSettings", true);
	ClearCurrentSection ();
	C_ArchiveCVars (this, 1);

	SetSection ("GlobalSettings.Unknown", true);
	ClearCurrentSection ();
	C_ArchiveCVars (this, 3);
}

char *FGameConfigFile::GetConfigPath ()
{
	char *path;

	path = Args.CheckValue ("-config");
	if (path)
		return copystring (path);

#ifndef unix
	if (Args.CheckParm ("-cdrom"))
		return copystring ("c:\\zdoomdat\\zdoom.ini");

	path = new char[strlen (progdir) + 10];

	strcpy (path, progdir);
	strcat (path, "zdoom.ini");
	return path;
#else
	return GetUserFile ("zdoom.ini");
#endif
}

void FGameConfigFile::AddAutoexec (DArgs *list, const char *game)
{
	char section[64];
	const char *key;
	const char *value;

	sprintf (section, "%s.AutoExec", game);

	if (bMigrating)
	{
		FBaseCVar *autoexec = FindCVar ("autoexec", NULL);

		if (autoexec != NULL)
		{
			UCVarValue val;
			char *path;

			val = autoexec->GetGenericRep (CVAR_String);
			path = copystring (val.String);
			delete autoexec;
			SetSection (section, true);
			SetValueForKey ("Path", path);
			list->AppendArg (path);
			delete[] path;
		}
	}
	else
	{
		// If <game>.AutoExec section does not exist, create it
		// with a default autoexec.cfg file present.
		if (!SetSection (section))
		{
			char *path;
			
#ifndef unix
			if (Args.CheckParm ("-cdrom"))
			{
				path = copystring ("c:\\zdoomdat\\autoexec.cfg");
			}
			else
			{
				path = new char[strlen (progdir) + 13];
				strcpy (path, progdir);
				strcat (path, "autoexec.cfg");
			}
#else
			path = GetUserFile ("autoexec.cfg");
#endif
			SetSection (section, true);
			SetValueForKey ("Path", path);
			delete[] path;
		}
		// Run any files listed in the <game>.AutoExec section
		if (SetSection (section))
		{
			while (NextInSection (key, value))
			{
				if (stricmp (key, "Path") == 0 && FileExists (value))
				{
					list->AppendArg (value);
				}
			}
		}
	}
}

void FGameConfigFile::SetRavenDefaults (bool isHexen)
{
	UCVarValue val;

	if (bMigrating)
	{
		con_centernotify.ResetToDefault ();
		msg0color.ResetToDefault ();
		dimcolor.ResetToDefault ();
		color.ResetToDefault ();
	}

	val.Bool = true;
	con_centernotify.SetGenericRepDefault (val, CVAR_Bool);
	val.Int = 9;
	msg0color.SetGenericRepDefault (val, CVAR_Int);
	val.Int = 0x0000ff;
	dimcolor.SetGenericRepDefault (val, CVAR_Int);

	if (!isHexen)
	{
		val.Int = 0x3f6040;
		color.SetGenericRepDefault (val, CVAR_Int);
	}
}