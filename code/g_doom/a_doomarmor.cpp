#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "dstrings.h"
#include "p_local.h"

// Armor bonus --------------------------------------------------------------

class AArmorBonus : public AArmor
{
	DECLARE_ACTOR (AArmorBonus, AArmor);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		player_t *player = toucher->player;
		player->armorpoints[0]++;		// can go over 100%
		if (player->armorpoints[0] > deh.MaxArmor)
			player->armorpoints[0] = deh.MaxArmor;
		if (!player->armortype)
			player->armortype = deh.GreenAC;
		return true;
	}
	virtual const char *PickupMessage ()
	{
		return GOTARMBONUS;
	}
};

IMPLEMENT_DEF_SERIAL (AArmorBonus, AArmor);
REGISTER_ACTOR (AArmorBonus, Doom);

FState AArmorBonus::States[] =
{
	S_NORMAL (BON2, 'A',	6, NULL 				, &States[1]),
	S_NORMAL (BON2, 'B',	6, NULL 				, &States[2]),
	S_NORMAL (BON2, 'C',	6, NULL 				, &States[3]),
	S_NORMAL (BON2, 'D',	6, NULL 				, &States[4]),
	S_NORMAL (BON2, 'C',	6, NULL 				, &States[5]),
	S_NORMAL (BON2, 'B',	6, NULL 				, &States[0])
};

void AArmorBonus::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2015;
	info->spawnid = 22;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Green armor --------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AArmor, APickup);

class AGreenArmor : public AArmor
{
	DECLARE_ACTOR (AGreenArmor, AArmor);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveArmor (toucher->player, (armortype_t)deh.GreenAC, 100*deh.GreenAC);
	}
	virtual const char *PickupMesage ()
	{
		return GOTARMOR;
	}
};

IMPLEMENT_DEF_SERIAL (AGreenArmor, AArmor);
REGISTER_ACTOR (AGreenArmor, Doom);

FState AGreenArmor::States[] =
{
	S_NORMAL (ARM1, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (ARM1, 'B',	7, NULL 				, &States[0])
};

void AGreenArmor::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2018;
	info->spawnid = 68;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	ArmorPics[0] = "ARM1A0";
}

// Blue armor ---------------------------------------------------------------

class ABlueArmor : public AArmor
{
	DECLARE_ACTOR (ABlueArmor, AArmor);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveArmor (toucher->player, (armortype_t)deh.BlueAC, 100*deh.BlueAC);
	}
	virtual const char *PickupMessage ()
	{
		return GOTMEGA;
	}
};

IMPLEMENT_DEF_SERIAL (ABlueArmor, AArmor);
REGISTER_ACTOR (ABlueArmor, Doom);

FState ABlueArmor::States[] =
{
	S_NORMAL (ARM2, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (ARM2, 'B',	6, NULL 				, &States[0])
};

void ABlueArmor::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2019;
	info->spawnid = 69;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	ArmorPics[1] = "ARM2A0";
}
