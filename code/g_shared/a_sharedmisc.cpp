#include "actor.h"
#include "info.h"
#include "gi.h"
#include "a_sharedglobal.h"

// Default actor for unregistered doomednums -------------------------------

IMPLEMENT_DEF_SERIAL (AUnknown, AActor);
REGISTER_ACTOR (AUnknown, Any);

FState AUnknown::States[] =
{
	S_NORMAL (UNKN, 'A',   -1, NULL 						, NULL)
};

void AUnknown::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->radius = 32 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->flags = MF_NOGRAVITY|MF_NOBLOCKMAP;
}

// Navigation way point ----------------------------------------------------

IMPLEMENT_DEF_SERIAL (AWayPoint, AActor);
REGISTER_ACTOR (AWayPoint, Any);

void AWayPoint::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 9024;
	info->radius = 8 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->mass = 10;
	info->flags = MF_NOBLOCKMAP;
	info->flags2 = MF2_DONTDRAW;
}

// Blood sprite, adjusts itself for each game ------------------------------

IMPLEMENT_DEF_SERIAL (ABlood, AActor);
REGISTER_ACTOR (ABlood, Any);

FState ABlood::States[] =
{
#define S_DBLOOD 0
	S_NORMAL (BLUD, 'C',	8, NULL 						, &States[S_DBLOOD+1]),
	S_NORMAL (BLUD, 'B',	8, NULL 						, &States[S_DBLOOD+2]),
	S_NORMAL (BLUD, 'A',	8, NULL 						, NULL),

#define S_HBLOOD (S_DBLOOD+3)
	S_NORMAL (BLOD, 'C',	8, NULL							, &States[S_HBLOOD+1]),
	S_NORMAL (BLOD, 'B',	8, NULL							, &States[S_HBLOOD+2]),
	S_NORMAL (BLOD, 'A',	8, NULL,						, NULL),
};

void ABlood::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 130;
	if (gameinfo.gametype == GAME_Doom)
		info->spawnstate = &States[S_DBLOOD];
	else
		info->spawnstate = &States[S_HBLOOD];
	info->flags = MF_NOBLOCKMAP;
	info->mass = 5;
}

// Map spot ----------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AMapSpot, AActor);
REGISTER_ACTOR (AMapSpot, Any);

void AMapSpot::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 9001;
	info->flags = MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY;
};

// Map spot with gravity ---------------------------------------------------

IMPLEMENT_DEF_SERIAL (AMapSpotGravity, AMapSpot);
REGISTER_ACTOR (AMapSpotGravity, Any);

void AMapSpotGravity::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 9013;
	info->flags = 0;
	info->flags2 = MF2_DONTDRAW;
}

// Bloody gibs -------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AGibs, AActor);
REGISTER_ACTOR (AGibs, Any);

FState AGibs::States[] =
{
	S_NORMAL (POL5, 'A', -1, NULL, NULL),
	S_NORMAL (GIBS, 'A', -1, NULL, NULL),
};

void AGibs::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->flags = MF_NOBLOCKMAP|MF_DROPOFF|MF_CORPSE;
	info->flags2 = MF2_NOTELEPORT;
	if (gameinfo.gametype == GAME_Doom)
	{
		info->doomednum = 24;
		info->spawnstate = &States[0];
	}
	else if (gameinfo.gametype == GAME_Hexen)
	{
		info->spawnstate = &States[1];
	}
	else
	{
		info->flags2 |= MF2_DONTDRAW;
	}
}
