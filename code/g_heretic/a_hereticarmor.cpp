#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "hstrings.h"
#include "p_local.h"

// Silver Shield (Shield1) --------------------------------------------------

class ASilverShield : public AArmor
{
	DECLARE_ACTOR (ASilverShield, AArmor);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveArmor (toucher->player, (armortype_t)1, 100);
	}
	virtual const char *PickupMessage ()
	{
		return TXT_ITEMSHIELD1;
	}
};

IMPLEMENT_DEF_SERIAL (ASilverShield, AArmor);
REGISTER_ACTOR (ASilverShield, Heretic);

FState ASilverShield::States[] =
{
	S_NORMAL (SHLD, 'A', -1, NULL, NULL)
};

void ASilverShield::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 85;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL;
	info->flags2 = MF2_FLOATBOB;
}

// Enchanted shield (Shield2) -----------------------------------------------

class AEnchantedShield : public AArmor
{
	DECLARE_ACTOR (AEnchantedShield, AArmor);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveArmor (toucher->player, (armortype_t)2, 200);
	}
	virtual const char *PickupMessage ()
	{
		return TXT_ITEMSHIELD2;
	}
};

IMPLEMENT_DEF_SERIAL (AEnchantedShield, AArmor);
REGISTER_ACTOR (AEnchantedShield, Heretic);

FState AEnchantedShield::States[] =
{
	S_NORMAL (SHD2, 'A', -1, NULL, NULL)
};

void AEnchantedShield::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 31;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL;
	info->flags2 = MF2_FLOATBOB;
}
