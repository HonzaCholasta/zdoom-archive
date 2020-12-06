/*
** c_cmds.cpp
** Miscellaneous console commands.
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
** It might be a good idea to move these into files that they are more
** closely related to, but right now, I am too lazy to do that.
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "version.h"
#include "c_console.h"
#include "c_dispatch.h"

#include "i_system.h"

#include "doomstat.h"
#include "gstrings.h"
#include "s_sound.h"
#include "g_game.h"
#include "p_inter.h"
#include "z_zone.h"
#include "w_wad.h"
#include "g_level.h"
#include "gi.h"
#include "r_defs.h"
#include "d_player.h"
#include "r_main.h"
#include "templates.h"

extern FILE *Logfile;

CVAR (Bool, sv_cheats, false, CVAR_SERVERINFO | CVAR_LATCH)

CCMD (toggleconsole)
{
	C_ToggleConsole();
}

BOOL CheckCheatmode ()
{
	if (((gameskill == sk_nightmare) || netgame || deathmatch) && (!sv_cheats))
	{
		Printf ("sv_cheats must be true to enable this command.\n");
		return true;
	}
	else
	{
		return false;
	}
}

CCMD (quit)
{
	exit (0);
}

CCMD (exit)
{
	exit (0);
}

/*
==================
Cmd_God

Sets client to godmode

argv(0) god
==================
*/
CCMD (god)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_GOD);
}

CCMD (iddqd)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_IDDQD);
}

CCMD (notarget)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_NOTARGET);
}

CCMD (fly)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_FLY);
}

/*
==================
Cmd_Noclip

argv(0) noclip
==================
*/
CCMD (noclip)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_NOCLIP);
}

CCMD (powerup)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_POWER);
}

CCMD (morphme)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_MORPH);
}

CCMD (anubis)
{
	if (CheckCheatmode ())
		return;

	Net_WriteByte (DEM_GENERICCHEAT);
	Net_WriteByte (CHT_ANUBIS);
}

EXTERN_CVAR (Bool, chasedemo)

CCMD (chase)
{
	if (demoplayback)
	{
		int i;

		if (chasedemo)
		{
			chasedemo = false;
			for (i = 0; i < MAXPLAYERS; i++)
				players[i].cheats &= ~CF_CHASECAM;
		}
		else
		{
			chasedemo = true;
			for (i = 0; i < MAXPLAYERS; i++)
				players[i].cheats |= CF_CHASECAM;
		}
	}
	else
	{
		if (deathmatch && CheckCheatmode ())
			return;

		Net_WriteByte (DEM_GENERICCHEAT);
		Net_WriteByte (CHT_CHASECAM);
	}
}

CCMD (idclev)
{
	if (CheckCheatmode ())
		return;

	if ((argv.argc() > 1) && (*(argv[1] + 2) == 0) && *(argv[1] + 1) && *argv[1])
	{
		int epsd, map;
		char buf[2];
		char *mapname;

		buf[0] = argv[1][0] - '0';
		buf[1] = argv[1][1] - '0';

		if (gameinfo.flags & GI_MAPxx)
		{
			epsd = 1;
			map = buf[0]*10 + buf[1];
		}
		else
		{
			epsd = buf[0];
			map = buf[1];
		}

		// Catch invalid maps.
		mapname = CalcMapName (epsd, map);
		if (W_CheckNumForName (mapname) == -1)
			return;

		// So be it.
		Printf ("%s\n", GStrings(STSTR_CLEV));
      	G_DeferedInitNew (mapname);
	}
}

CCMD (changemap)
{
	if (m_Instigator == NULL)
	{
		Printf ("Use the map command when not in a game.\n");
		return;
	}

	if (m_Instigator->player - players != Net_Arbitrator && multiplayer)
	{
		Printf ("Only player %d can change the map.\n", Net_Arbitrator+1);
		return;
	}

	if (argv.argc() > 1)
	{
		if (W_CheckNumForName (argv[1]) == -1)
		{
			Printf ("No map %s\n", argv[1]);
		}
		else
		{
			Net_WriteByte (DEM_CHANGEMAP);
			Net_WriteString (argv[1]);
		}
	}
}

CCMD (give)
{
	if (CheckCheatmode () || argv.argc() < 2)
		return;

	Net_WriteByte (DEM_GIVECHEAT);
	Net_WriteString (argv[1]);
	if (argv.argc() > 2)
		Net_WriteByte (clamp (atoi (argv[2]), 1, 255));
	else
		Net_WriteByte (0);
}

CCMD (gameversion)
{
	Printf ("%d.%d : " __DATE__ "\n", VERSION / 100, VERSION % 100);
}

CCMD (print)
{
	if (argv.argc() != 2)
	{
		Printf ("print <name>: Print a string from the string table\n");
		return;
	}
	GStrings.LoadNames ();
	int strnum = GStrings.FindString (argv[1]);
	if (strnum < 0)
	{
		Printf ("%s unknown\n", argv[1]);
	}
	else
	{
		Printf ("%s\n", GStrings(strnum));
	}
	GStrings.FlushNames ();
}

CCMD (exec)
{
	if (argv.argc() < 2)
		return;

	for (int i = 1; i < argv.argc(); ++i)
	{
		switch (C_ExecFile (argv[i], gamestate == GS_STARTUP))
		{
		case 1: Printf ("Could not open \"%s\"\n", argv[1]); break;
		case 2: Printf ("Error parsing \"%s\"\n", argv[1]); break;
		default: break;
		}
	}
}

CCMD (dumpheap)
{
	int lo = PU_STATIC, hi = PU_CACHE;

	if (argv.argc() >= 2)
	{
		lo = atoi (argv[1]);
		if (argv.argc() >= 3)
		{
			hi = atoi (argv[2]);
		}
	}

	Z_DumpHeap (lo, hi);
}

CCMD (logfile)
{
	time_t clock;
	char *timestr;

	time (&clock);
	timestr = asctime (localtime (&clock));

	if (Logfile)
	{
		Printf ("Log stopped: %s\n", timestr);
		fclose (Logfile);
		Logfile = NULL;
	}

	if (argv.argc() >= 2)
	{
		if ( (Logfile = fopen (argv[1], "w")) )
		{
			Printf ("Log started: %s\n", timestr);
		}
		else
		{
			Printf ("Could not start log\n");
		}
	}
}

bool P_StartScript (AActor *who, line_t *where, int script, char *map, int lineSide,
					int arg0, int arg1, int arg2, int always);

CCMD (puke)
{
	int argc = argv.argc();

	if (argc < 2 || argc > 5)
	{
		Printf (" puke <script> [arg1] [arg2] [arg3]\n");
	}
	else
	{
		int script = atoi (argv[1]);
		int arg0=0, arg1=0, arg2=0;

		if (argc > 2)
		{
			arg0 = atoi (argv[2]);
			if (argc > 3)
			{
				arg1 = atoi (argv[3]);
				if (argc > 4)
				{
					arg2 = atoi (argv[4]);
				}
			}
		}
		P_StartScript (m_Instigator, NULL, script, level.mapname, 0, arg0, arg1, arg2, false);
	}
}

CCMD (error)
{
	char *textcopy = copystring (argv[1]);
	I_Error (textcopy);
}

CCMD (dir)
{
	char dir[256], curdir[256];
	char *match;
	findstate_t c_file;
	long file;

	if (!getcwd (curdir, 256))
	{
		Printf ("Current path too long\n");
		return;
	}

	if (argv.argc() == 1 || chdir (argv[1]))
	{
		match = argv.argc() == 1 ? (char *)"./*" : argv[1];

		ExtractFilePath (match, dir);
		if (dir[0])
		{
			match += strlen (dir);
		}
		else
		{
			dir[0] = '.';
			dir[1] = '/';
			dir[2] = '\0';
		}
		if (!match[0])
			match = "*";

		if (chdir (dir))
		{
			Printf ("%s not found\n", dir);
			return;
		}
	}
	else
	{
		match = "*";
		strcpy (dir, argv[1]);
		if (dir[strlen(dir) - 1] != '/')
			strcat (dir, "/");
	}

	if ( (file = I_FindFirst (match, &c_file)) == -1)
		Printf ("Nothing matching %s%s\n", dir, match);
	else
	{
		Printf ("Listing of %s%s:\n", dir, match);
		do
		{
			if (I_FindAttr (&c_file) & FA_DIREC)
				Printf_Bold ("%s <dir>\n", I_FindName (&c_file));
			else
				Printf ("%s\n", I_FindName (&c_file));
		} while (I_FindNext (file, &c_file) == 0);
		I_FindClose (file);
	}

	chdir (curdir);
}

CCMD (fov)
{
	player_t *player = m_Instigator ? m_Instigator->player
		: &players[consoleplayer];

	if (argv.argc() != 2)
	{
		Printf ("fov is %g\n", player->DesiredFOV);
		return;
	}
	else if (dmflags & DF_NO_FOV)
	{
		if (consoleplayer == Net_Arbitrator)
		{
			Net_WriteByte (DEM_FOV);
		}
		else
		{
			Printf ("The arbitrator has disabled FOV changes.\n");
			return;
		}
	}
	else
	{
		Net_WriteByte (DEM_MYFOV);
	}
	Net_WriteByte (clamp (atoi (argv[1]), 5, 179));
}

//==========================================================================
//
// CCMD r_visibility
//
// Controls how quickly light ramps across a 1/z range. Set this, and it
// sets all the r_*Visibility variables (except r_SkyVisibilily, which is
// currently unused).
//
//==========================================================================

CCMD (r_visibility)
{
	if (argv.argc() < 2)
	{
		Printf ("Visibility is %g\n", R_GetVisibility());
	}
	else if (!netgame)
	{
		R_SetVisibility (atof (argv[1]));
	}
	else
	{
		Printf ("Visibility cannot be changed in net games.\n");
	}
}