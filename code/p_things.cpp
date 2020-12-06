// [RH] new file to deal with general things

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

BOOL P_Thing_Spawn (int tid, int type, angle_t angle, BOOL fog)
{
	fixed_t z;
	int rtn = 0;
	const TypeInfo *kind;
	AActor *spot, *mobj;
	FActorIterator iterator (tid);

	if (type >= MAX_SPAWNABLES)
		return false;

	if ( (kind = SpawnableThings[type]) == NULL)
		return false;

	if ((kind->ActorInfo->flags & MF_COUNTKILL) && (dmflags & DF_NO_MONSTERS))
		return false;

	while ( (spot = iterator.Next ()) )
	{
		if (kind->ActorInfo->flags2 & MF2_FLOATBOB)
			z = spot->z - spot->floorz;
		else
			z = spot->z;

		mobj = Spawn (kind, spot->x, spot->y, z);

		if (mobj)
		{
			if (P_TestMobjLocation (mobj))
			{
				rtn++;
				mobj->angle = angle;
				if (fog)
				{
					Spawn<ATeleportFog> (spot->x, spot->y, spot->z);
				}
				mobj->flags |= MF_DROPPED;	// Don't respawn
				if (mobj->flags2 & MF2_FLOATBOB)
				{
					mobj->special1 = mobj->z - mobj->floorz;
				}
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

BOOL P_Thing_Projectile (int tid, int type, angle_t angle,
						 fixed_t speed, fixed_t vspeed, BOOL gravity)
{
	int rtn = 0;
	const TypeInfo *kind;
	AActor *spot, *mobj;
	FActorIterator iterator (tid);

	if (type >= MAX_SPAWNABLES)
		return false;

	if ((kind = SpawnableThings[type]) == NULL)
		return false;

	if ((kind->ActorInfo->flags & MF_COUNTKILL) && (dmflags & DF_NO_MONSTERS))
		return false;

	while ( (spot = iterator.Next ()) )
	{
		if (!spot->IsKindOf (RUNTIME_CLASS(AMapSpot)))
			continue;

		mobj = Spawn (kind, spot->x, spot->y, spot->z);

		if (mobj)
		{
			if (GetInfo (mobj)->seesound)
				S_Sound (mobj, CHAN_VOICE, GetInfo (mobj)->seesound, 1, ATTN_NORM);
			if (gravity)
			{
				mobj->flags &= ~MF_NOGRAVITY;
				if (!(mobj->flags & MF_COUNTKILL))
					mobj->flags2 |= MF2_LOGRAV;
			}
			else
				mobj->flags |= MF_NOGRAVITY;
			mobj->target = spot;
			mobj->angle = angle;
			mobj->momx = FixedMul (speed, finecosine[angle>>ANGLETOFINESHIFT]);
			mobj->momy = FixedMul (speed, finesine[angle>>ANGLETOFINESHIFT]);
			mobj->momz = vspeed;
			mobj->flags |= MF_DROPPED;
			if (mobj->flags & MF_MISSILE)
				rtn = P_CheckMissileSpawn (mobj);
			else if (!P_TestMobjLocation (mobj))
				mobj->Destroy ();
		} 
	}

	return rtn;
}

BEGIN_COMMAND (dumpspawnables)
{
	int i;

	for (i = 0; i < MAX_SPAWNABLES; i++)
	{
		if (SpawnableThings[i] != NULL)
		{
			Printf (PRINT_HIGH, "%d %s\n", i, SpawnableThings[i]->Name + 1);
		}
	}
}
END_COMMAND (dumpspawnables)
