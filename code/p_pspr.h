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
//	Sprite animation.
//
//-----------------------------------------------------------------------------


#ifndef __P_PSPR_H__
#define __P_PSPR_H__

// Basic data types.
// Needs fixed point, and BAM angles.
#include "m_fixed.h"
#include "tables.h"


//
// Needs to include the precompiled
//	sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
#include "info.h"

//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//
#define FF_FULLBRIGHT	0x8000	// flag in thing->frame
#define FF_FRAMEMASK	0x7fff


#define WEAPONBOTTOM			128*FRACUNIT

// [RH] +0x6000 helps it meet the screen bottom
//		at higher resolutions while still being in
//		the right spot at 320x200.
#define WEAPONTOP				(32*FRACUNIT+0x6000)


//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
	ps_weapon,
	ps_flash,
	NUMPSPRITES

} psprnum_t;

inline FArchive &operator<< (FArchive &arc, psprnum_t &i)
{
	BYTE val = (BYTE)i;
	arc << val;
	i = (psprnum_t)val;
	return arc;
}

typedef struct pspdef_s
{
	FState*		state;	// a NULL state means not active
	int 		tics;
	fixed_t 	sx;
	fixed_t 	sy;

} pspdef_t;

FArchive &operator<< (FArchive &, pspdef_t &);

class player_s;

void P_SetPsprite (player_s *player, int position, FState *state);
void P_SetPspriteNF (player_s *player, int position, FState *state);
void P_CalcSwing (player_s *player);
void P_ActivateMorphWeapon (player_s *player);
void P_PostMorphWeapon (player_s *player, weapontype_t weapon);
void P_BringUpWeapon (player_s *player);
bool P_CheckAmmo (player_s *player);
void P_FireWeapon (player_s *player);
void P_DropWeapon (player_s *player);
void P_BulletSlope (AActor *mo);
void P_GunShot (AActor *mo, BOOL accurate);

void A_WeaponReady (player_s *player, pspdef_t *psp);
void A_ReFire (player_s *player, pspdef_t *psp);
void A_Lower (player_s *player, pspdef_t *psp);
void A_Raise (player_s *player, pspdef_t *psp);
void A_GunFlash (player_s *player, pspdef_t *psp);
void A_Light0 (player_s *player, pspdef_t *psp);
void A_Light1 (player_s *player, pspdef_t *psp);
void A_Light2 (player_s *player, pspdef_t *psp);

extern fixed_t bulletslope;

#endif	// __P_PSPR_H__
