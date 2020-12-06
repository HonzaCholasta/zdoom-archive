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
// DESCRIPTION:
//	 All the global variables that store the internal state.
//	 Theoretically speaking, the internal state of the engine
//	  should be found by looking at the variables collected
//	  here, and every relevant module will have to include
//	  this header file.
//	 In practice, things are a bit messy.
//
//-----------------------------------------------------------------------------


#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//	for defining the global state variables.
// We need the player data structure as well.
//#include "d_player.h"

#include "doomdata.h"
#include "d_net.h"
#include "g_level.h"

// We also need the definition of a cvar
#include "c_cvars.h"

// -----------------------
// Game speed.
//
enum EGameSpeed
{
	SPEED_Normal,
	SPEED_Fast,
};
extern EGameSpeed GameSpeed;


// ------------------------
// Command line parameters.
//
extern	BOOL			devparm;		// DEBUG: launched with -devparm



// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t		gamemode;
extern GameMission_t	gamemission;

// -------------------------------------------
// Selected skill type, map etc.
//

extern	char			startmap[8];		// [RH] Actual map name now

extern	BOOL 			autostart;

// Selected by user. 
EXTERN_CVAR (Int, gameskill);
extern	int				NextSkill;			// [RH] Skill to use at next level load

// Nightmare mode flag, single player.
extern	BOOL 			respawnmonsters;

// Netgame? Only true if >1 player.
extern	BOOL			netgame;

// Bot game? Like netgame, but doesn't involve network communication.
extern	BOOL			multiplayer;

// Flag: true only if started as net deathmatch.
EXTERN_CVAR (Int, deathmatch)

// [RH] Pretend as deathmatch for purposes of dmflags
EXTERN_CVAR (Bool, alwaysapplydmflags)

// [RH] Teamplay mode
EXTERN_CVAR (Bool, teamplay)

// [RH] Friendly fire amount
EXTERN_CVAR (Float, teamdamage)
		
// -------------------------
// Internal parameters for sound rendering.

EXTERN_CVAR (Float, snd_sfxvolume)		// maximum volume for sound
EXTERN_CVAR (Float, snd_musicvolume)	// maximum volume for music


// -------------------------
// Status flags for refresh.
//

extern	bool			automapactive;	// In AutoMap mode?
extern	bool			menuactive; 	// Menu overlayed?
extern	int				paused; 		// Game Pause?


extern	bool			viewactive;

extern	BOOL	 		nodrawers;
extern	BOOL	 		noblit;

extern	int 			viewwindowx;
extern	int 			viewwindowy;
extern	"C" int 		viewheight;
extern	"C" int 		viewwidth;
extern	"C"	int			halfviewwidth;		// [RH] Half view width, for plane drawing
extern	"C" int			realviewwidth;		// [RH] Physical width of view window
extern	"C" int			realviewheight;		// [RH] Physical height of view window
extern	"C" int			detailxshift;		// [RH] X shift for horizontal detail level
extern	"C" int			detailyshift;		// [RH] Y shift for vertical detail level





// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern	int				viewangleoffset;

// Player taking events, and displaying.
extern	int				consoleplayer;	
extern	int				displayplayer;


extern level_locals_t level;


// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern	bool			usergame;

extern	BOOL			demoplayback;
extern	BOOL			demorecording;
extern	int				demover;

// Quit after playing a demo from cmdline.
extern	BOOL			singledemo; 	




extern	gamestate_t 	gamestate;






//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//	according to user inputs. Partly load from
//	WAD, partly set at startup time.



extern	int 			gametic;


// Alive? Disconnected?
extern	bool	 		playeringame[MAXPLAYERS];


// Player spawn spots for deathmatch.
extern TArray<mapthing2_t> deathmatchstarts;

// Player spawn spots.
extern	mapthing2_t		playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern	struct wbstartstruct_s wminfo; 







//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern	FILE*			debugfile;

// if true, load all graphics at level load
extern	BOOL	 		precache;


//-------
//REFRESH
//-------

// wipegamestate can be set to -1
//	to force a wipe on the next draw
extern gamestate_t wipegamestate;
extern bool setsizeneeded;
extern BOOL setmodeneeded;

extern int BorderNeedRefresh;
extern int BorderTopRefresh;


EXTERN_CVAR (Float, mouse_sensitivity)
//?
// debug flag to cancel adaptiveness
extern	BOOL	 		singletics; 	

extern	int 			bodyqueslot;



// Needed to store the number of the dummy sky flat.
// Used for rendering,
//	as well as tracking projectiles etc.
extern int				skyflatnum;



// Netgame stuff (buffers and pointers, i.e. indices).

// This is ???
extern	doomcom_t*		doomcom;

// This points inside doomcom.
extern	doomdata_t* 	netbuffer;		


extern	struct ticcmd_t		localcmds[BACKUPTICS];

extern	int 			maketic;
extern	int 			nettics[MAXNETNODES];

extern	ticcmd_t		netcmds[MAXPLAYERS][BACKUPTICS];
extern	int 			ticdup;


// ---- [RH] ----
EXTERN_CVAR (Bool, developer)

extern int Net_Arbitrator;

// Use MMX routines? (Only if USEASM is defined)
extern	BOOL			UseMMX;

// Have conditional move instructions? (x86 only)
extern "C" BOOL			HaveCMOV;

#ifdef USEASM
extern "C" void EndMMX (void);

#ifdef _MSC_VER
#define ENDMMX if (UseMMX) __asm emms;
#else
#define ENDMMX if (UseMMX) EndMMX();
#endif

#endif

EXTERN_CVAR (Bool, var_friction)
EXTERN_CVAR (Bool, var_pushers)


// [RH] Miscellaneous info for DeHackEd support
struct DehInfo
{
	int StartHealth;
	int StartBullets;
	int MaxHealth;
	int MaxArmor;
	int GreenAC;
	int BlueAC;
	int MaxSoulsphere;
	int SoulsphereHealth;
	int MegasphereHealth;
	int GodHealth;
	int FAArmor;
	int FAAC;
	int KFAArmor;
	int KFAAC;
	int Infight;
	char PlayerSprite[5];
	BYTE ExplosionStyle;
	fixed_t ExplosionAlpha;
};
extern DehInfo deh;

// [RH] Deathmatch flags

EXTERN_CVAR (Int, dmflags);
EXTERN_CVAR (Int, dmflags2);	// [BC]

#endif