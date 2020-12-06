/*
** p_things.cpp
** ACS-accessible thing utilities
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

#include "doomtype.h"
#include "p_local.h"
#include "p_effect.h"
#include "info.h"
#include "s_sound.h"
#include "tables.h"
#include "doomstat.h"
#include "m_random.h"
#include "c_console.h"
#include "c_dispatch.h"
#include "a_sharedglobal.h"

// List of spawnable things for the Thing_Spawn and Thing_Projectile specials.
const TypeInfo *SpawnableThings[MAX_SPAWNABLES];

bool P_Thing_Spawn (int tid, int type, angle_t angle, bool fog, int newtid)
{
	int rtn = 0;
	const TypeInfo *kind;
	AActor *spot, *mobj;
	FActorIterator iterator (tid);

	if (type >= MAX_SPAWNABLES)
		return false;

	if ( (kind = SpawnableThings[type]) == NULL)
		return false;

	if ((GetDefaultByType (kind)->flags & MF_COUNTKILL) && (dmflags & DF_NO_MONSTERS))
		return false;

	while ( (spot = iterator.Next ()) )
	{
		mobj = Spawn (kind, spot->x, spot->y, spot->z);

		if (mobj != NULL)
		{
			if (P_TestMobjLocation (mobj))
			{
				rtn++;
				mobj->angle = (angle != ANGLE_MAX ? angle : spot->angle);
				if (fog)
				{
					Spawn<ATeleportFog> (spot->x, spot->y, spot->z);
				}
				if (mobj->flags & MF_SPECIAL)
					mobj->flags |= MF_DROPPED;	// Don't respawn
				mobj->tid = newtid;
				mobj->AddToHash ();
			}
			else
			{
				mobj->Destroy ();
				rtn = false;
			}
		}
	}

	return rtn != 0;
}

// [BC] Added
// [RH] Fixed

bool P_Thing_Move (int tid, int mapspot)
{
	FActorIterator iterator1 (tid);
	FActorIterator iterator2 (mapspot);
	AActor *source, *target;

	source = iterator1.Next ();
	target = iterator2.Next ();

	if (source != NULL && target != NULL)
	{
		fixed_t oldx, oldy, oldz;

		oldx = source->x;
		oldy = source->y;
		oldz = source->z;

		source->SetOrigin (target->x, target->y, target->z);
		if (P_TestMobjLocation (source))
		{
			Spawn<ATeleportFog> (target->x, target->y, target->z);
			Spawn<ATeleportFog> (oldx, oldy, oldz);
			return true;
		}
		else
		{
			source->SetOrigin (oldx, oldy, oldz);
			return false;
		}
	}
	return false;
}

bool P_Thing_Projectile (int tid, int type, angle_t angle,
	fixed_t speed, fixed_t vspeed, bool gravity)
{
	int rtn = 0;
	const TypeInfo *kind;
	AActor *spot, *mobj;
	TActorIterator<AMapSpot> iterator (tid);

	if (type >= MAX_SPAWNABLES)
		return false;

	if ((kind = SpawnableThings[type]) == NULL)
		return false;

	if ((GetDefaultByType (kind)->flags & MF_COUNTKILL) && (dmflags & DF_NO_MONSTERS))
		return false;

	while ( (spot = iterator.Next ()) )
	{
		mobj = Spawn (kind, spot->x, spot->y, spot->z);

		if (mobj)
		{
			if (mobj->SeeSound)
			{
				S_SoundID (mobj, CHAN_VOICE, mobj->SeeSound, 1, ATTN_NORM);
			}
			if (gravity)
			{
				mobj->flags &= ~MF_NOGRAVITY;
				if (!(mobj->flags & MF_COUNTKILL))
					mobj->flags2 |= MF2_LOGRAV;
			}
			else
			{
				mobj->flags |= MF_NOGRAVITY;
			}
			mobj->target = spot;
			mobj->angle = angle;
			mobj->momx = FixedMul (speed, finecosine[angle>>ANGLETOFINESHIFT]);
			mobj->momy = FixedMul (speed, finesine[angle>>ANGLETOFINESHIFT]);
			mobj->momz = vspeed;
			if (mobj->flags & MF_SPECIAL)
				mobj->flags |= MF_DROPPED;
			if (mobj->flags & MF_MISSILE)
				rtn = P_CheckMissileSpawn (mobj);
			else if (!P_TestMobjLocation (mobj))
				mobj->Destroy ();
		} 
	}

	return rtn != 0;
}

CCMD (dumpspawnables)
{
	int i;

	for (i = 0; i < MAX_SPAWNABLES; i++)
	{
		if (SpawnableThings[i] != NULL)
		{
			Printf ("%d %s\n", i, SpawnableThings[i]->Name + 1);
		}
	}
}
