#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "dstrings.h"
#include "p_local.h"

// Blue key card ------------------------------------------------------------

class ABlueCard : public AKey
{
	DECLARE_ACTOR (ABlueCard, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return it_bluecard;
	}
	virtual const char *PickupMessage ()
	{
		return GOTBLUECARD;
	}
};

IMPLEMENT_DEF_SERIAL (ABlueCard, AKey);
REGISTER_ACTOR (ABlueCard, Doom);

FState ABlueCard::States[] =
{
	S_NORMAL (BKEY, 'A',   10, NULL 				, &States[1]),
	S_BRIGHT (BKEY, 'B',   10, NULL 				, &States[0])
};

void ABlueCard::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 5;
	info->spawnid = 85;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Yellow key card ----------------------------------------------------------

class AYellowCard : public AKey
{
	DECLARE_ACTOR (AYellowCard, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return it_yellowcard;
	}
	virtual const char *PickupMessage ()
	{
		return GOTYELWCARD;
	}
};

IMPLEMENT_DEF_SERIAL (AYellowCard, AKey);
REGISTER_ACTOR (AYellowCard, Doom);

FState AYellowCard::States[] =
{
	S_NORMAL (YKEY, 'A',   10, NULL 				, &States[1]),
	S_BRIGHT (YKEY, 'B',   10, NULL 				, &States[0])
};

void AYellowCard::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 6;
	info->spawnid = 87;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Red key card -------------------------------------------------------------

class ARedCard : public AKey
{
	DECLARE_ACTOR (ARedCard, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return it_redcard;
	}
	virtual const char *PickupMessage ()
	{
		return GOTREDCARD;
	}
};

IMPLEMENT_DEF_SERIAL (ARedCard, AKey);
REGISTER_ACTOR (ARedCard, Doom);

FState ARedCard::States[] =
{
	S_NORMAL (RKEY, 'A',   10, NULL 				, &States[1]),
	S_BRIGHT (RKEY, 'B',   10, NULL 				, &States[0])
};

void ARedCard::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 13;
	info->spawnid = 86;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Blue skull key -----------------------------------------------------------

class ABlueSkull : public AKey
{
	DECLARE_ACTOR (ABlueSkull, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return it_blueskull;
	}
	virtual const char *PickupMessage ()
	{
		return GOTBLUESKUL;
	}
};

IMPLEMENT_DEF_SERIAL (ABlueSkull, AKey);
REGISTER_ACTOR (ABlueSkull, Doom);

FState ABlueSkull::States[] =
{
	S_NORMAL (BSKU, 'A',   10, NULL 				, &States[1]),
	S_BRIGHT (BSKU, 'B',   10, NULL 				, &States[0])
};

void ABlueSkull::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 40;
	info->spawnid = 90;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Yellow skull key ---------------------------------------------------------

class AYellowSkull : public AKey
{
	DECLARE_ACTOR (AYellowSkull, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return it_yellowskull;
	}
	virtual const char *PickupMessage ()
	{
		return GOTYELWSKUL;
	}
};

IMPLEMENT_DEF_SERIAL (AYellowSkull, AKey);
REGISTER_ACTOR (AYellowSkull, Doom);

FState AYellowSkull::States[] =
{
	S_NORMAL (YSKU, 'A',   10, NULL 				, &States[1]),
	S_BRIGHT (YSKU, 'B',   10, NULL 				, &States[0])
};

void AYellowSkull::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 39;
	info->spawnid = 88;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Red skull key ------------------------------------------------------------

class ARedSkull : public AKey
{
	DECLARE_ACTOR (ARedSkull, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return it_redskull;
	}
	virtual const char *PickupMessage ()
	{
		return GOTREDSKULL;
	}
};

IMPLEMENT_DEF_SERIAL (ARedSkull, AKey);
REGISTER_ACTOR (ARedSkull, Doom);

FState ARedSkull::States[] =
{
	S_NORMAL (RSKU, 'A',   10, NULL 				, &States[1]),
	S_BRIGHT (RSKU, 'B',   10, NULL 				, &States[0])
};

void ARedSkull::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 38;
	info->spawnid = 89;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}
