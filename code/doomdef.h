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
//	Internally used data structures for virtually everything,
//	 key definitions, lots of other stuff.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMDEF_H__
#define __DOOMDEF_H__

#include <stdio.h>
#include <string.h>

#include "farchive.h"

//
// Global parameters/defines.
//

// Game mode handling - identify IWAD version
//	to handle IWAD dependend animations etc.
typedef enum
{
	shareware,		// DOOM 1 shareware, E1, M9
	registered,		// DOOM 1 registered, E3, M27
	commercial,		// DOOM 2 retail, E1 M34
	// DOOM 2 german edition not handled
	retail,			// DOOM 1 retail, E4, M36
	undetermined	// Well, no IWAD found.
  
} GameMode_t;


// Mission packs - might be useful for TC stuff?
typedef enum
{
  doom, 		// DOOM 1
  doom2,		// DOOM 2
  pack_tnt, 	// TNT mission pack
  pack_plut,	// Plutonia pack
  none

} GameMission_t;


// If rangecheck is undefined, most parameter validation debugging code
// will not be compiled
#ifndef NORANGECHECKING
#ifndef RANGECHECK
#define RANGECHECK
#endif
#endif

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS		8

// State updates, number of tics / second.
#define TICRATE 		35

// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or a demo. 
typedef enum
{
	GS_LEVEL,
	GS_INTERMISSION,
	GS_FINALE,
	GS_DEMOSCREEN,
	GS_FULLCONSOLE,		// [RH]	Fullscreen console
	GS_HIDECONSOLE,		// [RH] The menu just did something that should hide fs console
	GS_STARTUP,			// [RH] Console is fullscreen, and game is just starting

	GS_FORCEWIPE = -1
} gamestate_t;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define MTF_EASY				1
#define MTF_NORMAL				2
#define MTF_HARD				4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH				8

typedef float skill_t;

enum ESkillLevels
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
};



#define TELEFOGHEIGHT			(gameinfo.telefogheight)

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW			0xcd	// DIK_RIGHT
#define KEY_LEFTARROW			0xcb	// DIK_LEFT
#define KEY_UPARROW 			0xc8	// DIK_UP
#define KEY_DOWNARROW			0xd0	// DIK_DOWN
#define KEY_ESCAPE				0x01	// DIK_ESCAPE
#define KEY_ENTER				0x1c	// DIK_RETURN
#define KEY_SPACE				0x39	// DIK_SPACE
#define KEY_TAB 				0x0f	// DIK_TAB
#define KEY_F1					0x3b	// DIK_F1
#define KEY_F2					0x3c	// DIK_F2
#define KEY_F3					0x3d	// DIK_F3
#define KEY_F4					0x3e	// DIK_F4
#define KEY_F5					0x3f	// DIK_F5
#define KEY_F6					0x40	// DIK_F6
#define KEY_F7					0x41	// DIK_F7
#define KEY_F8					0x42	// DIK_F8
#define KEY_F9					0x43	// DIK_F9
#define KEY_F10 				0x44	// DIK_F10
#define KEY_F11 				0x57	// DIK_F11
#define KEY_F12 				0x58	// DIK_F12

#define KEY_BACKSPACE			0x0e	// DIK_BACK
#define KEY_PAUSE				0xff

#define KEY_EQUALS				0x0d	// DIK_EQUALS
#define KEY_MINUS				0x0c	// DIK_MINUS

#define KEY_LSHIFT				0x2A	// DIK_LSHIFT
#define KEY_LCTRL				0x1d	// DIK_LCONTROL
#define KEY_LALT				0x38	// DIK_LMENU

#define	KEY_RSHIFT				KEY_LSHIFT
#define KEY_RCTRL				KEY_LCTRL
#define KEY_RALT				KEY_LALT

#define KEY_INS 				0xd2	// DIK_INSERT
#define KEY_DEL 				0xd3	// DIK_DELETE
#define KEY_END 				0xcf	// DIK_END
#define KEY_HOME				0xc7	// DIK_HOME
#define KEY_PGUP				0xc9	// DIK_PRIOR
#define KEY_PGDN				0xd1	// DIK_NEXT

#define KEY_MOUSE1				0x100
#define KEY_MOUSE2				0x101
#define KEY_MOUSE3				0x102
#define KEY_MOUSE4				0x103
#define KEY_MWHEELUP			0x104
#define KEY_MWHEELDOWN			0x105

#define KEY_JOY1				0x108
#define KEY_JOY2				0x109
#define KEY_JOY3				0x10a
#define KEY_JOY4				0x10b
#define KEY_JOY5				0x10c
#define KEY_JOY6				0x10d
#define KEY_JOY7				0x10e
#define KEY_JOY8				0x10f
#define KEY_JOY9				0x110
#define KEY_JOY10				0x111
#define KEY_JOY11				0x112
#define KEY_JOY12				0x113
#define KEY_JOY13				0x114
#define KEY_JOY14				0x115
#define KEY_JOY15				0x116
#define KEY_JOY16				0x117
#define KEY_JOY17				0x118
#define KEY_JOY18				0x119
#define KEY_JOY19				0x11a
#define KEY_JOY20				0x11b
#define KEY_JOY21				0x11c
#define KEY_JOY22				0x11d
#define KEY_JOY23				0x11e
#define KEY_JOY24				0x11f
#define KEY_JOY25				0x120
#define KEY_JOY26				0x121
#define KEY_JOY27				0x122
#define KEY_JOY28				0x123
#define KEY_JOY29				0x124
#define KEY_JOY30				0x125
#define KEY_JOY31				0x126
#define KEY_JOY32				0x127

#define NUM_KEYS				0x128

// [RH] dmflags bits (based on Q2's)
#define	DF_NO_HEALTH		1		// Do not spawn health items (DM)
#define	DF_NO_ITEMS			2		// Do not spawn powerups (DM)
#define	DF_WEAPONS_STAY		4		// Leave weapons around after pickup (DM)
#define	DF_YES_FALLING		8		// Falling too far hurts
//#define	DF_INVENTORY_ITEMS	32		// Wait for player to use powerups when picked up
#define	DF_SAME_LEVEL		64		// Stay on the same map when someone exits (DM)
#define	DF_SPAWN_FARTHEST	128		// Spawn players as far as possible from other players (DM)
#define DF_FORCE_RESPAWN	256		// Automatically respawn dead players after respawn_time is up (DM)
#define DF_NO_ARMOR			512		// Do not spawn armor (DM)
#define DF_NO_EXIT			1024	// Kill anyone who tries to exit the level (DM)
#define DF_INFINITE_AMMO	2048	// Don't use up ammo when firing
#define DF_NO_MONSTERS		4096	// Don't spawn monsters (replaces -nomonsters parm)
#define DF_MONSTERS_RESPAWN	8192	// Monsters respawn sometime after their death (replaces -respawn parm)
#define DF_ITEMS_RESPAWN	16384	// Items other than invuln. and invis. respawn
#define DF_FAST_MONSTERS	32768	// Monsters are fast (replaces -fast parm)
#define DF_NO_JUMP			65536	// Don't allow jumping
#define DF_NO_FREELOOK		131072	// Don't allow freelook
#define DF_RESPAWN_SUPER	262144	// Respawn invulnerability and invisibility

// [BC] More dmflags. w00p!
//#define	DF2_YES_IMPALING		1		// Player gets implaed on MF2_IMPALE items
#define	DF2_YES_WEAPONDROP		2		// Drop current weapon upon death
//#define DF2_NO_RUNES			4		// Don't spawn runes
//#define DF2_YES_IRETURN			8		// Instantly return skull when player carrying it dies
//#define DF2_YES_RETURN			16		// Return dropped skulls after 30 seconds
//#define DF2_YES_TEAMCOLORS		32		// Don't use player's base color in teamgames
//#define DF2_NO_SWITCH			64		// Player is not allowed to switch teams
//#define DF2_FORCE_RANDOM		128		// Player put on random team
//#define DF2_YES_RUNEDROP		256		// Drop current rune upon death
//#define DF2_YES_200MAX			512		// Don't all max. health/armor items to bring
//										// health or armor over 200%
//#define DF2_YES_DOUBLEAMMO		1024	// Doubles ammo like skill 1 and 5 do
//#define DF2_NO_CLEARFRAGS		2048	// Don't clear frags after each level
//#define DF2_FORCE_NORESPAWN		4096	// Player cannot respawn
//#define DF2_YES_DEGENERATION	8192	// Quake-style degeneration
//#define DF2_YES_LOSEFRAG		16384	// Lose a frag when killed. More incentive to try to
//										// not get yerself killed
#define	DF2_NO_FREEAIMBFG		32768	// Don't allow BFG to be aimed at the ground
										// or ceiling. Prevents cheap BFG frags
#define	DF2_BARRELS_RESPAWN		65536	// Barrels respawn (duh)
#define	DF2_YES_INVUL			131072	// Player is temporarily invulnerable when respawned

// phares 3/20/98:
//
// Player friction is variable, based on controlling
// linedefs. More friction can create mud, sludge,
// magnetized floors, etc. Less friction can create ice.

#define MORE_FRICTION_MOMENTUM	15000	// mud factor based on momentum
#define ORIG_FRICTION			0xE800	// original value
#define ORIG_FRICTION_FACTOR	2048	// original value
#define FRICTION_LOW			0xf900
#define FRICTION_FLY			0xeb00

// [RH] Means of death flags (based on Quake2's)
#define MOD_UNKNOWN			0
#define MOD_FIST			1
#define MOD_PISTOL			2
#define MOD_SHOTGUN			3
#define MOD_CHAINGUN		4
#define MOD_ROCKET			5
#define MOD_R_SPLASH		6
#define MOD_PLASMARIFLE		7
#define MOD_BFG_BOOM		8
#define MOD_BFG_SPLASH		9
#define MOD_CHAINSAW		10
#define MOD_SSHOTGUN		11
#define MOD_WATER			12
#define MOD_SLIME			13
#define MOD_LAVA			14
#define MOD_CRUSH			15
#define MOD_TELEFRAG		16
#define MOD_FALLING			17
#define MOD_SUICIDE			18
#define MOD_BARREL			19
#define MOD_EXIT			20
#define MOD_SPLASH			21
#define MOD_HIT				22
#define MOD_RAILGUN			23
#define MOD_ICE				24
#define MOD_FRIENDLY_FIRE	0x80000000

// Most damage defined using HITDICE
#define HITDICE(a) ((1+(P_Random()&7))*a)

#define BLINKTHRESHOLD (4*32)

#ifndef __BIG_ENDIAN__
#define MAKE_ID(a,b,c,d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))
#else
#define MAKE_ID(a,b,c,d)	((d)|((c)<<8)|((b)<<16)|((a)<<24))
#endif

#endif	// __DOOMDEF_H__
