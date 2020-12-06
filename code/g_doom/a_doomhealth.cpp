#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "dstrings.h"
#include "p_local.h"

// Health bonus -------------------------------------------------------------

class AHealthBonus : public AHealth
{
	DECLARE_ACTOR (AHealthBonus, AHealth);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		player_t *player = toucher->player;
		player->health++;		// can go over 100%
		if (player->health > deh.MaxSoulsphere)
			player->health = deh.MaxSoulsphere;
		player->mo->health = player->health;
		return true;
	}
	virtual const char *PickupMessage ()
	{
		return GOTHTHBONUS;
	}
};

IMPLEMENT_DEF_SERIAL (AHealthBonus, AHealth);
REGISTER_ACTOR (AHealthBonus, Doom);

FState AHealthBonus::States[] =
{
	S_NORMAL (BON1, 'A',	6, NULL 				, &States[1]),
	S_NORMAL (BON1, 'B',	6, NULL 				, &States[2]),
	S_NORMAL (BON1, 'C',	6, NULL 				, &States[3]),
	S_NORMAL (BON1, 'D',	6, NULL 				, &States[4]),
	S_NORMAL (BON1, 'C',	6, NULL 				, &States[5]),
	S_NORMAL (BON1, 'B',	6, NULL 				, &States[0])
};

void AHealthBonus::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2014;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Stimpack -----------------------------------------------------------------

class AStimpack : public AHealth
{
	DECLARE_ACTOR (AStimpack, AHealth);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveBody (toucher->player, 10);
	}
	virtual const char *PickupMessage ()
	{
		return GOTSTIM;
	}
};

IMPLEMENT_DEF_SERIAL (AStimpack, AHealth);
REGISTER_ACTOR (AStimpack, Doom);

FState AStimpack::States[] =
{
	S_NORMAL (STIM, 'A',   -1, NULL 				, NULL)
};

void AStimpack::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2011;
	info->spawnid = 23;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

// Medikit ------------------------------------------------------------------

class AMedikit : public AHealth
{
	DECLARE_ACTOR (AMedikit, AHealth);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		PrevHealth = toucher->player->health;
		return P_GiveBody (toucher->player, 25);
	}
	virtual const char *PickupMessage ()
	{
		return (PrevHealth < 25) ? GOTMEDINEED : GOTMEDIKIT;
	}
	int PrevHealth;
};

IMPLEMENT_DEF_SERIAL (AMedikit, AHealth);
REGISTER_ACTOR (AMedikit, Doom);

FState AMedikit::States[] =
{
	S_NORMAL (MEDI, 'A',   -1, NULL 				, NULL)
};

void AMedikit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2012;
	info->spawnid = 24;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

