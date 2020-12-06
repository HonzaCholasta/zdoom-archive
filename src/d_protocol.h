/*
** d_protocol.h
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

#ifndef __D_PROTOCOL_H__
#define __D_PROTOCOL_H__

#include "doomstat.h"
#include "doomtype.h"
#include "doomdef.h"
#include "m_fixed.h"
#include "farchive.h"

#define FORM_ID		(('F'<<24)|('O'<<16)|('R'<<8)|('M'))
#define ZDEM_ID		(('Z'<<24)|('D'<<16)|('E'<<8)|('M'))
#define ZDHD_ID		(('Z'<<24)|('D'<<16)|('H'<<8)|('D'))
#define VARS_ID		(('V'<<24)|('A'<<16)|('R'<<8)|('S'))
#define UINF_ID		(('U'<<24)|('I'<<16)|('N'<<8)|('F'))
#define BODY_ID		(('B'<<24)|('O'<<16)|('D'<<8)|('Y'))

#define	ANGLE2SHORT(x)	((((x)/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*360)


struct zdemoheader_s {
	byte	demovermajor;
	byte	demoverminor;
	byte	minvermajor;
	byte	minverminor;
	byte	map[8];
	unsigned int rngseed;
	byte	consoleplayer;
};

struct usercmd_s
{
	byte	buttons;
	byte	pad;
	short	pitch;			// up/down
	short	yaw;			// left/right	// If you haven't guessed, I just
	short	roll;			// tilt			// ripped these from Quake2's usercmd.
	short	forwardmove;
	short	sidemove;
	short	upmove;
};
typedef struct usercmd_s usercmd_t;

FArchive &operator<< (FArchive &arc, usercmd_t &cmd);

// When transmitted, the above message is preceded by a byte
// indicating which fields are actually present in the message.
enum
{
	UCMDF_BUTTONS		= 0x01,
	UCMDF_PITCH			= 0x02,
	UCMDF_YAW			= 0x04,
	UCMDF_FORWARDMOVE	= 0x08,
	UCMDF_SIDEMOVE		= 0x10,
	UCMDF_UPMOVE		= 0x20,
	UCMDF_ROLL			= 0x40
};

// When changing the following enum, be sure to update Net_SkipCommand()
// and Net_DoCommand() in d_net.cpp.
enum EDemoCommand
{
	DEM_BAD,			// Bad command
	DEM_USERCMD,		// Player movement
	DEM_UNDONE1,
	DEM_UNDONE2,
	DEM_MUSICCHANGE,	// Followed by name of new music
	DEM_PRINT,			// Print string to console
	DEM_CENTERPRINT,	// Print string to middle of screen
	DEM_STOP,			// End of demo
	DEM_UINFCHANGED,	// User info changed
	DEM_SINFCHANGED,	// Server/Host info changed
	DEM_GENERICCHEAT,	// Next byte is cheat to apply (see next enum)
	DEM_GIVECHEAT,		// String: item to give, Byte: quantity
	DEM_SAY,			// Byte: who to talk to, String: message to display
	DEM_DROPPLAYER,		// Not implemented, takes a byte
	DEM_CHANGEMAP,		// Name of map to change to
	DEM_SUICIDE,		// Player wants to die
	DEM_ADDBOT,			// Byte: player#, String: userinfo for bot
	DEM_KILLBOTS,		// Remove all bots from the world
	DEM_INVSEL,			// Byte: inventory item to select, for status bar
	DEM_INVUSE,			// Byte: inventory item to use
	DEM_PAUSE,			// Pause game
	DEM_SAVEGAME,		// String: Filename, String: Description
	DEM_WEAPSEL,		// Byte: weapontype_t to change to
	DEM_WEAPSLOT,		// Byte: Weapon slot to pick a weapon from
	DEM_WEAPNEXT,		// Select next weapon
	DEM_WEAPPREV,		// Select previous weapon
	DEM_SUMMON,			// String: Thing to fabricate
	DEM_FOV,			// Byte: New FOV for all players
	DEM_MYFOV,			// Byte: New FOV for this player
};

// The following are implemented by cht_DoCheat in m_cheat.cpp
enum ECheatCommand
{
	CHT_GOD,
	CHT_NOCLIP,
	CHT_NOTARGET,
	CHT_CHAINSAW,
	CHT_IDKFA,
	CHT_IDFA,
	CHT_BEHOLDV,
	CHT_BEHOLDS,
	CHT_BEHOLDI,
	CHT_BEHOLDR,
	CHT_BEHOLDA,
	CHT_BEHOLDL,
	CHT_IDDQD,			// Same as CHT_GOD, but sets health
	CHT_MASSACRE,
	CHT_CHASECAM,
	CHT_FLY,
	CHT_MORPH,
	CHT_POWER,
	CHT_HEALTH,
	CHT_KEYS,
	CHT_TAKEWEAPS,
	CHT_NOWUDIE,
	CHT_ALLARTI,
	CHT_PUZZLE,
	CHT_MDK,			// Kill actor player is aiming at
	CHT_ANUBIS,
};

void StartChunk (int id, byte **stream);
void FinishChunk (byte **stream);
void SkipChunk (byte **stream);

int UnpackUserCmd (usercmd_t *ucmd, const usercmd_t *basis, byte **stream);
int PackUserCmd (const usercmd_t *ucmd, const usercmd_t *basis, byte **stream);
int WriteUserCmdMessage (usercmd_t *ucmd, const usercmd_t *basis, byte **stream);

struct ticcmd_t;

int SkipTicCmd (byte **stream, int count);
void ReadTicCmd (byte **stream, int player, int tic);
void RunNetSpecs (int player, int buf);

int ReadByte (byte **stream);
int ReadWord (byte **stream);
int ReadLong (byte **stream);
float ReadFloat (byte **stream);
char *ReadString (byte **stream);
void WriteByte (byte val, byte **stream);
void WriteWord (short val, byte **stream);
void WriteLong (int val, byte **stream);
void WriteFloat (float val, byte **stream);
void WriteString (const char *string, byte **stream);

#endif //__D_PROTOCOL_H__