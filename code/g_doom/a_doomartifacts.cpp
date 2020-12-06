#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "p_local.h"
#include "gi.h"
#include "dstrings.h"
#include "s_sound.h"
#include "m_random.h"
#include "p_local.h"
#include "p_spec.h"
#include "p_lnspec.h"
#include "p_enemy.h"
#include "p_effect.h"

// Invulnerability Sphere ---------------------------------------------------

class AInvulnerabilitySphere : public APowerup
{
	DECLARE_ACTOR (AInvulnerabilitySphere, APowerup);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
public:
	bool ShouldRespawn ();
};

IMPLEMENT_DEF_SERIAL (AInvulnerabilitySphere, APowerup);
REGISTER_ACTOR (AInvulnerabilitySphere, Doom);

FState AInvulnerabilitySphere::States[] =
{
	S_BRIGHT (PINV, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (PINV, 'B',	6, NULL 				, &States[2]),
	S_BRIGHT (PINV, 'C',	6, NULL 				, &States[3]),
	S_BRIGHT (PINV, 'D',	6, NULL 				, &States[0])
};

void AInvulnerabilitySphere::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2022;
	info->spawnid = 133;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

bool AInvulnerabilitySphere::TryPickup (AActor *toucher)
{
	return P_GivePower (toucher->player, pw_invulnerability);
}

const char *AInvulnerabilitySphere::PickupMessage ()
{
	return GOTINVUL;
}

bool AInvulnerabilitySphere::ShouldRespawn ()
{
	return Super::ShouldRespawn () && (dmflags & DF_RESPAWN_SUPER);
}

// Soulsphere --------------------------------------------------------------

class ASoulsphere : public APowerup
{
	DECLARE_ACTOR (ASoulsphere, APowerup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		player_t *player = toucher->player;
		player->health += deh.SoulsphereHealth;
		if (player->health > deh.MaxSoulsphere)
			player->health = deh.MaxSoulsphere;
		player->mo->health = player->health;
		return true;
	}
	virtual const char *PickupMessage ()
	{
		return GOTSUPER;
	}
};

IMPLEMENT_DEF_SERIAL (ASoulsphere, APowerup);
REGISTER_ACTOR (ASoulsphere, Doom);

FState ASoulsphere::States[] =
{
	S_BRIGHT (SOUL, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (SOUL, 'B',	6, NULL 				, &States[2]),
	S_BRIGHT (SOUL, 'C',	6, NULL 				, &States[3]),
	S_BRIGHT (SOUL, 'D',	6, NULL 				, &States[4]),
	S_BRIGHT (SOUL, 'C',	6, NULL 				, &States[5]),
	S_BRIGHT (SOUL, 'B',	6, NULL 				, &States[0])
};

void ASoulsphere::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2013;
	info->spawnid = 25;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Mega sphere --------------------------------------------------------------

class AMegasphere : public APowerup
{
	DECLARE_ACTOR (AMegasphere, APowerup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		player_t *player = toucher->player;
		player->health = deh.MegasphereHealth;
		player->mo->health = player->health;
		return P_GiveArmor (player, (armortype_t)deh.BlueAC, 100*deh.BlueAC);
	}
	virtual const char *PickupMessage ()
	{
		return GOTMSPHERE;
	}
};

IMPLEMENT_DEF_SERIAL (AMegasphere, APowerup);
REGISTER_ACTOR (AMegasphere, Doom);

FState AMegasphere::States[] =
{
	S_BRIGHT (MEGA, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (MEGA, 'B',	6, NULL 				, &States[2]),
	S_BRIGHT (MEGA, 'C',	6, NULL 				, &States[3]),
	S_BRIGHT (MEGA, 'D',	6, NULL 				, &States[0])
};

void AMegasphere::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 83;
	info->spawnid = 132;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Berserk ------------------------------------------------------------------

class ABerserk : public APowerup
{
	DECLARE_ACTOR (ABerserk, APowerup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		if (P_GivePower (toucher->player, pw_strength))
		{
			if (toucher->player->readyweapon != wp_fist)
				toucher->player->pendingweapon = wp_fist;
			return true;
		}
		return false;
	}
	virtual const char *PickupMessage ()
	{
		return GOTBERSERK;
	}
};

IMPLEMENT_DEF_SERIAL (ABerserk, APowerup);
REGISTER_ACTOR (ABerserk, Doom);

FState ABerserk::States[] =
{
	S_BRIGHT (PSTR, 'A',   -1, NULL 				, NULL)
};

void ABerserk::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2023;
	info->spawnid = 134;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Invisibility -------------------------------------------------------------

class ABlurSphere : public APowerup
{
	DECLARE_ACTOR (ABlurSphere, APowerup);
public:
	virtual void PostBeginPlay ()
	{
		Super::PostBeginPlay ();
		effects |= FX_VISIBILITYPULSE;
		special2 = -1;
	}
	virtual bool ShouldRespawn ()
	{
		return Super::ShouldRespawn () && (dmflags & DF_RESPAWN_SUPER);
	}
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GivePower (toucher->player, pw_invisibility);
	}
	virtual const char *PickupMessage ()
	{
		return GOTINVIS;
	}
};

IMPLEMENT_DEF_SERIAL (ABlurSphere, APowerup);
REGISTER_ACTOR (ABlurSphere, Doom);

FState ABlurSphere::States[] =
{
	S_BRIGHT (PINS, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (PINS, 'B',	6, NULL 				, &States[2]),
	S_BRIGHT (PINS, 'C',	6, NULL 				, &States[3]),
	S_BRIGHT (PINS, 'D',	6, NULL 				, &States[0])
};

void ABlurSphere::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2024;
	info->spawnid = 135;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Radiation suit (aka iron feet) -------------------------------------------

class ARadSuit : public APowerup
{
	DECLARE_ACTOR (ARadSuit, APowerup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GivePower (toucher->player, pw_ironfeet);
	}
	virtual const char *PickupMessage ()
	{
		return GOTSUIT;
	}
};

IMPLEMENT_DEF_SERIAL (ARadSuit, APowerup);
REGISTER_ACTOR (ARadSuit, Doom);

FState ARadSuit::States[] =
{
	S_BRIGHT (SUIT, 'A',   -1, NULL 				, NULL)
};

void ARadSuit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2025;
	info->spawnid = 136;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 46 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

// infrared -----------------------------------------------------------------

class AInfrared : public APowerup
{
	DECLARE_ACTOR (AInfrared, APowerup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GivePower (toucher->player, pw_infrared);
	}
	virtual const char *PickupMessage ()
	{
		return GOTVISOR;
	}
};

IMPLEMENT_DEF_SERIAL (AInfrared, APowerup);
REGISTER_ACTOR (AInfrared, Doom);

FState AInfrared::States[] =
{
	S_BRIGHT (PVIS, 'A',	6, NULL 				, &States[1]),
	S_NORMAL (PVIS, 'B',	6, NULL 				, &States[0])
};

void AInfrared::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2045;
	info->spawnid = 138;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Allmap -------------------------------------------------------------------

// Note that the allmap is a subclass of pickup, not powerup, because we
// always want it to be activated immediately on pickup.

class AAllmap : public APickup
{
	DECLARE_ACTOR (AAllmap, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GivePower (toucher->player, pw_allmap);
	}
	virtual const char *PickupMessage ()
	{
		return GOTMAP;
	}
};

IMPLEMENT_DEF_SERIAL (AAllmap, APickup);
REGISTER_ACTOR (AAllmap, Doom);

FState AAllmap::States[] =
{
	S_BRIGHT (PMAP, 'A',	6, NULL 				, &States[1]),
	S_BRIGHT (PMAP, 'B',	6, NULL 				, &States[2]),
	S_BRIGHT (PMAP, 'C',	6, NULL 				, &States[3]),
	S_BRIGHT (PMAP, 'D',	6, NULL 				, &States[4]),
	S_BRIGHT (PMAP, 'C',	6, NULL 				, &States[5]),
	S_BRIGHT (PMAP, 'B',	6, NULL 				, &States[0])
};

void AAllmap::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2026;
	info->spawnid = 137;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
}

// Backpack -----------------------------------------------------------------

// The backpack is alsa pickup, because there's not much point to carrying
// a backpack around unused.

class ABackpack : public APickup
{
	DECLARE_ACTOR (ABackpack, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		player_t *player = toucher->player;
		int i;

		if (!player->backpack)
		{
			for (i = 0; i < NUMAMMO; i++)
				player->maxammo[i] *= 2;
			player->backpack = true;
		}
		for (i = 0; i < NUMAMMO; i++)
			P_GiveAmmo (player, (ammotype_t)i, 1);
		return true;
	}
	virtual const char *PickupMessage ()
	{
		return GOTBACKPACK;
	}
};

IMPLEMENT_DEF_SERIAL (ABackpack, APickup);
REGISTER_ACTOR (ABackpack, Doom);

FState ABackpack::States[] =
{
	S_NORMAL (BPAK, 'A',   -1, NULL 						, NULL)
};

void ABackpack::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8;
	info->spawnid = 144;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 26 * FRACUNIT;
	info->flags = MF_SPECIAL;
}
