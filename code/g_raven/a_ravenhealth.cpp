#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "hstrings.h"
#include "p_local.h"

// Crystal vial -------------------------------------------------------------

class ACrystalVial : public AHealth
{
	DECLARE_ACTOR (ACrystalVial, AHealth);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveBody (toucher->player, 10);
	}
	virtual const char *PickupMessage ()
	{
		return TXT_ITEMHEALTH;
	}
};

IMPLEMENT_DEF_SERIAL (ACrystalVial, AHealth);
REGISTER_ACTOR (ACrystalVial, Raven);

FState ACrystalVial::States[] =
{
	S_NORMAL (PTN1, 'A',	3, NULL 				, &States[1]),
	S_NORMAL (PTN1, 'B',	3, NULL 				, &States[2]),
	S_NORMAL (PTN1, 'C',	3, NULL 				, &States[0])
};

void ACrystalVial::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 81;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL;
	info->flags2 = MF2_FLOATBOB;
}

