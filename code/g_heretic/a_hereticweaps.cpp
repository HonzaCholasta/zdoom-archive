#include "actor.h"
#include "info.h"
#include "s_sound.h"
#include "m_random.h"
#include "a_pickups.h"
#include "a_hereticglobal.h"
#include "d_player.h"
#include "p_pspr.h"
#include "p_local.h"
#include "p_inter.h"
#include "dstrings.h"
#include "p_effect.h"
#include "hstrings.h"
#include "p_enemy.h"

#define FLAME_THROWER_TICS (10*TICRATE)

#define AMMO_GWND_WIMPY 10
#define AMMO_GWND_HEFTY 50
#define AMMO_CBOW_WIMPY 5
#define AMMO_CBOW_HEFTY 20
#define AMMO_BLSR_WIMPY 10
#define AMMO_BLSR_HEFTY 25
#define AMMO_SKRD_WIMPY 20
#define AMMO_SKRD_HEFTY 100
#define AMMO_PHRD_WIMPY 1
#define AMMO_PHRD_HEFTY 10
#define AMMO_MACE_WIMPY 20
#define AMMO_MACE_HEFTY 100

#define USE_GWND_AMMO_1 1
#define USE_GWND_AMMO_2 1
#define USE_CBOW_AMMO_1 1
#define USE_CBOW_AMMO_2 1
#define USE_BLSR_AMMO_1 1
#define USE_BLSR_AMMO_2 5
#define USE_SKRD_AMMO_1 1
#define USE_SKRD_AMMO_2 5
#define USE_PHRD_AMMO_1 1
#define USE_PHRD_AMMO_2 1
#define USE_MACE_AMMO_1 1
#define USE_MACE_AMMO_2 5

//---------------------------------------------------------------------------
//
// FUNC P_AutoUseChaosDevice
//
//---------------------------------------------------------------------------

bool P_AutoUseChaosDevice (player_t *player)
{
	if (player->inventory[arti_teleport])
	{
		P_PlayerUseArtifact (player, arti_teleport);
		player->health = player->mo->health = (player->health+1)/2;
		return true;
	}
	return false;
}

// Base Heretic weapon class ------------------------------------------------

IMPLEMENT_DEF_SERIAL (AHereticWeapon, AWeapon);
REGISTER_ACTOR (AHereticWeapon, Heretic);

void AHereticWeapon::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
}

void AHereticWeapon::PlayPickupSound (AActor *toucher)
{
	S_Sound (toucher, CHAN_VOICE, "*weaponlaugh", 1, ATTN_NORM);
}

// --- Staff ----------------------------------------------------------------

void A_StaffAttackPL1 (player_t *, pspdef_t *);
void A_StaffAttackPL2 (player_t *, pspdef_t *);

// Staff --------------------------------------------------------------------

class AStaff : public AHereticWeapon
{
	DECLARE_ACTOR (AStaff, AHereticWeapon);
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (AStaff, AHereticWeapon);
REGISTER_ACTOR (AStaff, Heretic);

FState AStaff::States[] =
{
#define S_STAFFREADY 0
	S_NORMAL (STFF, 'A',	1, A_WeaponReady				, &States[S_STAFFREADY]),

#define S_STAFFDOWN (S_STAFFREADY+1)
	S_NORMAL (STFF, 'A',	1, A_Lower						, &States[S_STAFFDOWN]),

#define S_STAFFUP (S_STAFFDOWN+1)
	S_NORMAL (STFF, 'A',	1, A_Raise						, &States[S_STAFFUP]),

#define S_STAFFREADY2 (S_STAFFUP+1)
	S_NORMAL (STFF, 'D',	4, A_WeaponReady				, &States[S_STAFFREADY2+1]),
	S_NORMAL (STFF, 'E',	4, A_WeaponReady				, &States[S_STAFFREADY2+2]),
	S_NORMAL (STFF, 'F',	4, A_WeaponReady				, &States[S_STAFFREADY2+0]),

#define S_STAFFDOWN2 (S_STAFFREADY2+3)
	S_NORMAL (STFF, 'D',	1, A_Lower						, &States[S_STAFFDOWN2]),

#define S_STAFFUP2 (S_STAFFDOWN2+1)
	S_NORMAL (STFF, 'D',	1, A_Raise						, &States[S_STAFFUP2]),

#define S_STAFFATK1 (S_STAFFUP2+1)
	S_NORMAL (STFF, 'B',	6, NULL 						, &States[S_STAFFATK1+1]),
	S_NORMAL (STFF, 'C',	8, A_StaffAttackPL1 			, &States[S_STAFFATK1+2]),
	S_NORMAL (STFF, 'B',	8, A_ReFire 					, &States[S_STAFFREADY]),

#define S_STAFFATK2 (S_STAFFATK1+3)
	S_NORMAL (STFF, 'G',	6, NULL 						, &States[S_STAFFATK2+1]),
	S_NORMAL (STFF, 'H',	8, A_StaffAttackPL2 			, &States[S_STAFFATK2+2]),
	S_NORMAL (STFF, 'G',	8, A_ReFire 					, &States[S_STAFFREADY2+0])
};

FWeaponInfo AStaff::WeaponInfo1 =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_STAFFUP],
	&States[S_STAFFDOWN],
	&States[S_STAFFREADY],
	&States[S_STAFFATK1],
	&States[S_STAFFATK1],
	NULL,
	NULL,
	150,
	0,
	NULL,
	NULL
};

FWeaponInfo AStaff::WeaponInfo2 =
{
	WIF_READYSNDHALF,
	am_noammo,
	0,
	0,
	&States[S_STAFFUP2],
	&States[S_STAFFDOWN2],
	&States[S_STAFFREADY2],
	&States[S_STAFFATK2],
	&States[S_STAFFATK2],
	NULL,
	NULL,
	150,
	0,
	NULL,
	"weapons/staffcrackle"
};

void AStaff::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	WeaponSlots[1].AddWeapon (wp_staff, 1);
	wpnlev1info[wp_staff] = &WeaponInfo1;
	wpnlev2info[wp_staff] = &WeaponInfo2;
}

// Staff puff ---------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AStaffPuff, AActor);
REGISTER_ACTOR (AStaffPuff, Heretic);

FState AStaffPuff::States[] =
{
	S_BRIGHT (PUF3, 'A',	4, NULL 						, &States[1]),
	S_NORMAL (PUF3, 'B',	4, NULL 						, &States[2]),
	S_NORMAL (PUF3, 'C',	4, NULL 						, &States[3]),
	S_NORMAL (PUF3, 'D',	4, NULL 						, NULL)
};

void AStaffPuff::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->attacksound = "weapons/staffhit";
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
}

void AStaffPuff::BeginPlay ()
{
	Super::BeginPlay ();
	momz = FRACUNIT;
}

// Staff puff 2 -------------------------------------------------------------

class AStaffPuff2 : public AStaffPuff
{
	DECLARE_ACTOR (AStaffPuff2, AStaffPuff);
public:
	void BeginPlay ();
};

IMPLEMENT_DEF_SERIAL (AStaffPuff2, AStaffPuff);
REGISTER_ACTOR (AStaffPuff2, Heretic);

FState AStaffPuff2::States[] =
{
	S_BRIGHT (PUF4, 'A',	4, NULL 						, &States[1]),
	S_BRIGHT (PUF4, 'B',	4, NULL 						, &States[2]),
	S_BRIGHT (PUF4, 'C',	4, NULL 						, &States[3]),
	S_BRIGHT (PUF4, 'D',	4, NULL 						, &States[4]),
	S_BRIGHT (PUF4, 'E',	4, NULL 						, &States[5]),
	S_BRIGHT (PUF4, 'F',	4, NULL 						, NULL)
};

void AStaffPuff2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->attacksound = "weapons/staffpowerhit";
}

void AStaffPuff2::BeginPlay ()
{
	Super::BeginPlay ();
	momz = 0;
}

//----------------------------------------------------------------------------
//
// PROC A_StaffAttackPL1
//
//----------------------------------------------------------------------------

void A_StaffAttackPL1 (player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;

	damage = 5+(P_Random()&15);
	angle = player->mo->angle;
	angle += PS_Random() << 18;
	slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
	PuffType = RUNTIME_CLASS(AStaffPuff);
	P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);
	if (linetarget)
	{
		//S_StartSound(player->mo, sfx_stfhit);
		// turn to face target
		player->mo->angle = R_PointToAngle2 (player->mo->x,
			player->mo->y, linetarget->x, linetarget->y);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_StaffAttackPL2
//
//----------------------------------------------------------------------------

void A_StaffAttackPL2 (player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;

	// P_inter.c:P_DamageMobj() handles target momentums
	damage = 18+(P_Random()&63);
	angle = player->mo->angle;
	angle += PS_Random() << 18;
	slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
	PuffType = RUNTIME_CLASS(AStaffPuff2);
	P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);
	if (linetarget)
	{
		//S_StartSound(player->mo, sfx_stfpow);
		// turn to face target
		player->mo->angle = R_PointToAngle2 (player->mo->x,
			player->mo->y, linetarget->x, linetarget->y);
	}
}

// --- Gold wand ------------------------------------------------------------

void A_FireGoldWandPL1 (player_t *, pspdef_t *);
void A_FireGoldWandPL2 (player_t *, pspdef_t *);

// Wimpy ammo ---------------------------------------------------------------

class AGoldWandWimpy : public APickup
{
	DECLARE_ACTOR (AGoldWandWimpy, APickup);
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_goldwand, health);
	}
	virtual const char *PickupMessage ()
	{
		return TXT_AMMOGOLDWAND1;
	}
};

IMPLEMENT_DEF_SERIAL (AGoldWandWimpy, APickup);
REGISTER_ACTOR (AGoldWandWimpy, Heretic);

FState AGoldWandWimpy::States[] =
{
	S_NORMAL (AMG1, 'A',   -1, NULL 						, NULL),
};

void AGoldWandWimpy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 10;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_GWND_WIMPY;
	info->flags = MF_SPECIAL;
	AmmoPics[am_goldwand] = "INAMGLD";
}

// Hefty ammo ---------------------------------------------------------------

class AGoldWandHefty : public APickup
{
	DECLARE_ACTOR (AGoldWandHefty, APickup);
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_goldwand, health);
	}
	virtual const char *PickupMessage ()
	{
		return TXT_AMMOGOLDWAND2;
	}
};

IMPLEMENT_DEF_SERIAL (AGoldWandHefty, APickup);
REGISTER_ACTOR (AGoldWandHefty, Heretic);

FState AGoldWandHefty::States[] =
{
	S_NORMAL (AMG2, 'A',	4, NULL 						, &States[1]),
	S_NORMAL (AMG2, 'B',	4, NULL 						, &States[2]),
	S_NORMAL (AMG2, 'C',	4, NULL 						, &States[0])
};

void AGoldWandHefty::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 12;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_GWND_HEFTY;
	info->flags = MF_SPECIAL;
}

// Gold wand ----------------------------------------------------------------

class AGoldWand : public AHereticWeapon
{
	DECLARE_ACTOR (AGoldWand, AHereticWeapon);
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (AGoldWand, AHereticWeapon);
REGISTER_ACTOR (AGoldWand, Heretic);

FState AGoldWand::States[] =
{
#define S_GOLDWANDREADY 0
	S_NORMAL (GWND, 'A',	1, A_WeaponReady				, &States[S_GOLDWANDREADY]),

#define S_GOLDWANDDOWN (S_GOLDWANDREADY+1)
	S_NORMAL (GWND, 'A',	1, A_Lower						, &States[S_GOLDWANDDOWN]),

#define S_GOLDWANDUP (S_GOLDWANDDOWN+1)
	S_NORMAL (GWND, 'A',	1, A_Raise						, &States[S_GOLDWANDUP]),

#define S_GOLDWANDATK1 (S_GOLDWANDUP+1)
	S_NORMAL (GWND, 'B',	3, NULL 						, &States[S_GOLDWANDATK1+1]),
	S_NORMAL (GWND, 'C',	5, A_FireGoldWandPL1			, &States[S_GOLDWANDATK1+2]),
	S_NORMAL (GWND, 'D',	3, NULL 						, &States[S_GOLDWANDATK1+3]),
	S_NORMAL (GWND, 'D',	0, A_ReFire 					, &States[S_GOLDWANDREADY]),

#define S_GOLDWANDATK2 (S_GOLDWANDATK1+4)
	S_NORMAL (GWND, 'B',	3, NULL 						, &States[S_GOLDWANDATK2+1]),
	S_NORMAL (GWND, 'C',	4, A_FireGoldWandPL2			, &States[S_GOLDWANDATK2+2]),
	S_NORMAL (GWND, 'D',	3, NULL 						, &States[S_GOLDWANDATK2+3]),
	S_NORMAL (GWND, 'D',	0, A_ReFire 					, &States[S_GOLDWANDREADY])
};

FWeaponInfo AGoldWand::WeaponInfo1 =
{
	0,
	am_goldwand,
	USE_GWND_AMMO_1,
	25,
	&States[S_GOLDWANDUP],
	&States[S_GOLDWANDDOWN],
	&States[S_GOLDWANDREADY],
	&States[S_GOLDWANDATK1],
	&States[S_GOLDWANDATK1],
	NULL,
	RUNTIME_CLASS(AGoldWandWimpy),
	150,
	5*FRACUNIT,
	NULL,
	NULL
};

FWeaponInfo AGoldWand::WeaponInfo2 =
{
	0,
	am_goldwand,
	USE_GWND_AMMO_2,
	25,
	&States[S_GOLDWANDUP],
	&States[S_GOLDWANDDOWN],
	&States[S_GOLDWANDREADY],
	&States[S_GOLDWANDATK2],
	&States[S_GOLDWANDATK2],
	NULL,
	RUNTIME_CLASS(AGoldWandHefty),
	150,
	5*FRACUNIT,
	NULL,
	NULL
};

void AGoldWand::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	WeaponSlots[2].AddWeapon (wp_goldwand, 3);
	wpnlev1info[wp_goldwand] = &WeaponInfo1;
	wpnlev2info[wp_goldwand] = &WeaponInfo2;
}

// Gold wand FX1 ------------------------------------------------------------

class AGoldWandFX1 : public AActor
{
	DECLARE_ACTOR (AGoldWandFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (AGoldWandFX1, AActor);
REGISTER_ACTOR (AGoldWandFX1, Heretic);

FState AGoldWandFX1::States[] =
{
#define S_GWANDFX1 0
	S_BRIGHT (FX01, 'A',	6, NULL 						, &States[S_GWANDFX1+1]),
	S_BRIGHT (FX01, 'B',	6, NULL 						, &States[S_GWANDFX1+0]),

#define S_GWANDFXI1 (S_GWANDFX1+2)
	S_BRIGHT (FX01, 'E',	3, NULL 						, &States[S_GWANDFXI1+1]),
	S_BRIGHT (FX01, 'F',	3, NULL 						, &States[S_GWANDFXI1+2]),
	S_BRIGHT (FX01, 'G',	3, NULL 						, &States[S_GWANDFXI1+3]),
	S_BRIGHT (FX01, 'H',	3, NULL 						, NULL)
};

void AGoldWandFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_GWANDFX1];
	info->deathstate = &States[S_GWANDFXI1];
	info->deathsound = "weapons/wandhit";
	info->speed = 22 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
}

// Gold wand FX2 ------------------------------------------------------------

class AGoldWandFX2 : public AGoldWandFX1
{
	DECLARE_ACTOR (AGoldWandFX2, AGoldWandFX1);
};

IMPLEMENT_DEF_SERIAL (AGoldWandFX2, AGoldWandFX1);
REGISTER_ACTOR (AGoldWandFX2, Heretic);

FState AGoldWandFX2::States[] =
{
	S_BRIGHT (FX01, 'C',	6, NULL 						, &States[1]),
	S_BRIGHT (FX01, 'D',	6, NULL 						, &States[0])
};

void AGoldWandFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->deathsound = NULL;
	info->speed = 18 * FRACUNIT;
	info->damage = 1;
}

// Gold wand puff 1 ---------------------------------------------------------

class AGoldWandPuff1 : public AActor
{
	DECLARE_ACTOR (AGoldWandPuff1, AActor);
};

IMPLEMENT_DEF_SERIAL (AGoldWandPuff1, AActor);
REGISTER_ACTOR (AGoldWandPuff1, Heretic);

FState AGoldWandPuff1::States[] =
{
	S_BRIGHT (PUF2, 'A',	3, NULL 						, &States[1]),
	S_BRIGHT (PUF2, 'B',	3, NULL 						, &States[2]),
	S_BRIGHT (PUF2, 'C',	3, NULL 						, &States[3]),
	S_BRIGHT (PUF2, 'D',	3, NULL 						, &States[4]),
	S_BRIGHT (PUF2, 'E',	3, NULL 						, NULL)
};

void AGoldWandPuff1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
}

// Gold wand puff 2 ---------------------------------------------------------

class AGoldWandPuff2 : public AGoldWandPuff1
{
	DECLARE_STATELESS_ACTOR (AGoldWandPuff2, AGoldWandPuff1);
};

IMPLEMENT_DEF_SERIAL (AGoldWandPuff2, AGoldWandPuff1);
REGISTER_ACTOR (AGoldWandPuff2, Heretic);

void AGoldWandPuff2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->spawnstate = &AGoldWandFX1::States[S_GWANDFXI1];
}

//----------------------------------------------------------------------------
//
// PROC A_FireGoldWandPL1
//
//----------------------------------------------------------------------------

void A_FireGoldWandPL1 (player_t *player, pspdef_t *psp)
{
	AActor *mo;
	angle_t angle;
	int damage;

	mo = player->mo;
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_goldwand] -= USE_GWND_AMMO_1;
	P_BulletSlope(mo);
	damage = 7+(P_Random()&7);
	angle = mo->angle;
	if (player->refire)
	{
		angle += PS_Random() << 18;
	}
	PuffType = RUNTIME_CLASS(AGoldWandPuff1);
	P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
	S_Sound (player->mo, CHAN_WEAPON, "weapons/wandhit", 1, ATTN_NORM);
}

//----------------------------------------------------------------------------
//
// PROC A_FireGoldWandPL2
//
//----------------------------------------------------------------------------

void A_FireGoldWandPL2 (player_t *player, pspdef_t *psp)
{
	int i;
	AActor *mo;
	angle_t angle;
	int damage;
	fixed_t momz;

	mo = player->mo;
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_goldwand] -=
			deathmatch.value ? USE_GWND_AMMO_1 : USE_GWND_AMMO_2;
	PuffType = RUNTIME_CLASS(AGoldWandPuff2);
	P_BulletSlope (mo);
	momz = FixedMul (RUNTIME_CLASS(AGoldWandFX2)->ActorInfo->speed, bulletslope);
	P_SpawnMissileAngle (mo, RUNTIME_CLASS(AGoldWandFX2), mo->angle-(ANG45/8), momz);
	P_SpawnMissileAngle (mo, RUNTIME_CLASS(AGoldWandFX2), mo->angle+(ANG45/8), momz);
	angle = mo->angle-(ANG45/8);
	for(i = 0; i < 5; i++)
	{
		damage = 1+(P_Random()&7);
		P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
		angle += ((ANG45/8)*2)/4;
	}
	S_Sound (player->mo, CHAN_WEAPON, "weapons/wandhit", 1, ATTN_NORM);
}

// --- Crossbow -------------------------------------------------------------

void A_FireCrossbowPL1 (player_t *, pspdef_t *);
void A_FireCrossbowPL2 (player_t *, pspdef_t *);
void A_BoltSpark (AActor *);

// Wimpy ammo ---------------------------------------------------------------

class ACrossbowWimpy : public APickup
{
	DECLARE_ACTOR (ACrossbowWimpy, APickup);
public:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_crossbow, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOCROSSBOW1;
	}
};

IMPLEMENT_DEF_SERIAL (ACrossbowWimpy, APickup);
REGISTER_ACTOR (ACrossbowWimpy, Heretic);

FState ACrossbowWimpy::States[] =
{
	S_NORMAL (AMC1, 'A',   -1, NULL 						, NULL)
};

void ACrossbowWimpy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 18;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_CBOW_WIMPY;
	info->flags = MF_SPECIAL;
	AmmoPics[am_crossbow] = "INAMBOW";
}

// Hefty ammo ---------------------------------------------------------------

class ACrossbowHefty : public APickup
{
	DECLARE_ACTOR (ACrossbowHefty, APickup);
public:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_crossbow, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOCROSSBOW2;
	}
};

IMPLEMENT_DEF_SERIAL (ACrossbowHefty, APickup);
REGISTER_ACTOR (ACrossbowHefty, Heretic);

FState ACrossbowHefty::States[] =
{
	S_NORMAL (AMC2, 'A',	5, NULL 						, &States[1]),
	S_NORMAL (AMC2, 'B',	5, NULL 						, &States[2]),
	S_NORMAL (AMC2, 'C',	5, NULL 						, &States[0])
};

void ACrossbowHefty::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 19;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_CBOW_HEFTY;
	info->flags = MF_SPECIAL;
}

// Crossbow -----------------------------------------------------------------

class ACrossbow : public AHereticWeapon
{
	DECLARE_ACTOR (ACrossbow, AHereticWeapon);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveWeapon (toucher->player, wp_crossbow, flags & MF_DROPPED);
	}
	const char *PickupMessage ()
	{
		return TXT_WPNCROSSBOW;
	}
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (ACrossbow, AHereticWeapon);
REGISTER_ACTOR (ACrossbow, Heretic);

FState ACrossbow::States[] =
{
#define S_WBOW 0
	S_NORMAL (WBOW, 'A',   -1, NULL 						, NULL),

#define S_CRBOW (S_WBOW+1)
	S_NORMAL (CRBW, 'A',	1, A_WeaponReady				, &States[S_CRBOW+1]),
	S_NORMAL (CRBW, 'A',	1, A_WeaponReady				, &States[S_CRBOW+2]),
	S_NORMAL (CRBW, 'A',	1, A_WeaponReady				, &States[S_CRBOW+3]),
	S_NORMAL (CRBW, 'A',	1, A_WeaponReady				, &States[S_CRBOW+4]),
	S_NORMAL (CRBW, 'A',	1, A_WeaponReady				, &States[S_CRBOW+5]),
	S_NORMAL (CRBW, 'A',	1, A_WeaponReady				, &States[S_CRBOW+6]),
	S_NORMAL (CRBW, 'B',	1, A_WeaponReady				, &States[S_CRBOW+7]),
	S_NORMAL (CRBW, 'B',	1, A_WeaponReady				, &States[S_CRBOW+8]),
	S_NORMAL (CRBW, 'B',	1, A_WeaponReady				, &States[S_CRBOW+9]),
	S_NORMAL (CRBW, 'B',	1, A_WeaponReady				, &States[S_CRBOW+10]),
	S_NORMAL (CRBW, 'B',	1, A_WeaponReady				, &States[S_CRBOW+11]),
	S_NORMAL (CRBW, 'B',	1, A_WeaponReady				, &States[S_CRBOW+12]),
	S_NORMAL (CRBW, 'C',	1, A_WeaponReady				, &States[S_CRBOW+13]),
	S_NORMAL (CRBW, 'C',	1, A_WeaponReady				, &States[S_CRBOW+14]),
	S_NORMAL (CRBW, 'C',	1, A_WeaponReady				, &States[S_CRBOW+15]),
	S_NORMAL (CRBW, 'C',	1, A_WeaponReady				, &States[S_CRBOW+16]),
	S_NORMAL (CRBW, 'C',	1, A_WeaponReady				, &States[S_CRBOW+17]),
	S_NORMAL (CRBW, 'C',	1, A_WeaponReady				, &States[S_CRBOW+0]),

#define S_CRBOWDOWN (S_CRBOW+18)
	S_NORMAL (CRBW, 'A',	1, A_Lower						, &States[S_CRBOWDOWN]),

#define S_CRBOWUP (S_CRBOWDOWN+1)
	S_NORMAL (CRBW, 'A',	1, A_Raise						, &States[S_CRBOWUP]),

#define S_CRBOWATK1 (S_CRBOWUP+1)
	S_NORMAL (CRBW, 'D',	6, A_FireCrossbowPL1			, &States[S_CRBOWATK1+1]),
	S_NORMAL (CRBW, 'E',	3, NULL 						, &States[S_CRBOWATK1+2]),
	S_NORMAL (CRBW, 'F',	3, NULL 						, &States[S_CRBOWATK1+3]),
	S_NORMAL (CRBW, 'G',	3, NULL 						, &States[S_CRBOWATK1+4]),
	S_NORMAL (CRBW, 'H',	3, NULL 						, &States[S_CRBOWATK1+5]),
	S_NORMAL (CRBW, 'A',	4, NULL 						, &States[S_CRBOWATK1+6]),
	S_NORMAL (CRBW, 'B',	4, NULL 						, &States[S_CRBOWATK1+7]),
	S_NORMAL (CRBW, 'C',	5, A_ReFire 					, &States[S_CRBOW+0]),

#define S_CRBOWATK2 (S_CRBOWATK1+8)
	S_NORMAL (CRBW, 'D',	5, A_FireCrossbowPL2			, &States[S_CRBOWATK2+1]),
	S_NORMAL (CRBW, 'E',	3, NULL 						, &States[S_CRBOWATK2+2]),
	S_NORMAL (CRBW, 'F',	2, NULL 						, &States[S_CRBOWATK2+3]),
	S_NORMAL (CRBW, 'G',	3, NULL 						, &States[S_CRBOWATK2+4]),
	S_NORMAL (CRBW, 'H',	2, NULL 						, &States[S_CRBOWATK2+5]),
	S_NORMAL (CRBW, 'A',	3, NULL 						, &States[S_CRBOWATK2+6]),
	S_NORMAL (CRBW, 'B',	3, NULL 						, &States[S_CRBOWATK2+7]),
	S_NORMAL (CRBW, 'C',	4, A_ReFire 					, &States[S_CRBOW+0])
};

FWeaponInfo ACrossbow::WeaponInfo1 =
{
	0,
	am_crossbow,
	USE_CBOW_AMMO_1,
	10,
	&States[S_CRBOWUP],
	&States[S_CRBOWDOWN],
	&States[S_CRBOW],
	&States[S_CRBOWATK1],
	&States[S_CRBOWATK1],
	NULL,
	RUNTIME_CLASS(ACrossbow),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

FWeaponInfo ACrossbow::WeaponInfo2 =
{
	0,
	am_crossbow,
	USE_CBOW_AMMO_2,
	10,
	&States[S_CRBOWUP],
	&States[S_CRBOWDOWN],
	&States[S_CRBOW],
	&States[S_CRBOWATK2],
	&States[S_CRBOWATK2],
	NULL,
	RUNTIME_CLASS(ACrossbow),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

void ACrossbow::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2001;
	info->spawnstate = &States[S_WBOW];
	info->flags = MF_SPECIAL;
	WeaponSlots[3].AddWeapon (wp_crossbow, 4);
	wpnlev1info[wp_crossbow] = &WeaponInfo1;
	wpnlev2info[wp_crossbow] = &WeaponInfo2;
}

// Crossbow FX1 -------------------------------------------------------------

class ACrossbowFX1 : public AActor
{
	DECLARE_ACTOR (ACrossbowFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (ACrossbowFX1, AActor);
REGISTER_ACTOR (ACrossbowFX1, Heretic);

FState ACrossbowFX1::States[] =
{
#define S_CRBOWFX1 0
	S_BRIGHT (FX03, 'B',	1, NULL 						, &States[S_CRBOWFX1]),

#define S_CRBOWFXI1 (S_CRBOWFX1+1)
	S_BRIGHT (FX03, 'H',	8, NULL 						, &States[S_CRBOWFXI1+1]),
	S_BRIGHT (FX03, 'I',	8, NULL 						, &States[S_CRBOWFXI1+2]),
	S_BRIGHT (FX03, 'J',	8, NULL 						, NULL)
};

void ACrossbowFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_CRBOWFX1];
	info->seesound = "weapons/bowshoot";
	info->deathstate = &States[S_CRBOWFXI1];
	info->deathsound = "weapons/bowhit";
	info->speed = 30 * FRACUNIT;
	info->radius = 11 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 10;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

// Crossbow FX2 -------------------------------------------------------------

class ACrossbowFX2 : public ACrossbowFX1
{
	DECLARE_ACTOR (ACrossbowFX2, ACrossbowFX1);
};

IMPLEMENT_DEF_SERIAL (ACrossbowFX2, ACrossbowFX1);
REGISTER_ACTOR (ACrossbowFX2, Heretic);

FState ACrossbowFX2::States[] =
{
#define S_CRBOWFX2 0
	S_BRIGHT (FX03, 'B',	1, A_BoltSpark					, &States[S_CRBOWFX2])
};

void ACrossbowFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_CRBOWFX2];
	info->speed = 32 * FRACUNIT;
	info->damage = 6;
}

// Crossbow FX3 -------------------------------------------------------------

class ACrossbowFX3 : public ACrossbowFX1
{
	DECLARE_ACTOR (ACrossbowFX3, ACrossbowFX1);
};

IMPLEMENT_DEF_SERIAL (ACrossbowFX3, ACrossbowFX1);
REGISTER_ACTOR (ACrossbowFX3, Heretic);

FState ACrossbowFX3::States[] =
{
#define S_CRBOWFX3 0
	S_BRIGHT (FX03, 'A',	1, NULL 						, &States[S_CRBOWFX3]),

#define S_CRBOWFXI3 (S_CRBOWFX3+1)
	S_BRIGHT (FX03, 'C',	8, NULL 						, &States[S_CRBOWFXI3+1]),
	S_BRIGHT (FX03, 'D',	8, NULL 						, &States[S_CRBOWFXI3+2]),
	S_BRIGHT (FX03, 'E',	8, NULL 						, NULL)
};

void ACrossbowFX3::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_CRBOWFX3];
	info->seesound = NULL;
	info->deathstate = &States[S_CRBOWFXI3];
	info->speed = 20 * FRACUNIT;
	info->damage = 2;
	info->flags2 = MF2_WINDTHRUST|MF2_THRUGHOST|MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

// Crossbow FX4 -------------------------------------------------------------

class ACrossbowFX4 : public AActor
{
	DECLARE_ACTOR (ACrossbowFX4, AActor);
};

IMPLEMENT_DEF_SERIAL (ACrossbowFX4, AActor);
REGISTER_ACTOR (ACrossbowFX4, Heretic);

FState ACrossbowFX4::States[] =
{
#define S_CRBOWFX4 0
	S_BRIGHT (FX03, 'F',	8, NULL 						, &States[S_CRBOWFX4+1]),
	S_BRIGHT (FX03, 'G',	8, NULL 						, NULL)
};

void ACrossbowFX4::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_CRBOWFX4];
	info->flags = MF_NOBLOCKMAP;
	info->flags2 = MF2_LOGRAV;
}

//----------------------------------------------------------------------------
//
// PROC A_FireCrossbowPL1
//
//----------------------------------------------------------------------------

void A_FireCrossbowPL1 (player_t *player, pspdef_t *psp)
{
	AActor *pmo;

	pmo = player->mo;
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_crossbow] -= USE_CBOW_AMMO_1;
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX1));
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX3), pmo->angle-(ANG45/10));
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX3), pmo->angle+(ANG45/10));
}

//----------------------------------------------------------------------------
//
// PROC A_FireCrossbowPL2
//
//----------------------------------------------------------------------------

void A_FireCrossbowPL2(player_t *player, pspdef_t *psp)
{
	AActor *pmo;

	pmo = player->mo;
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_crossbow] -=
			deathmatch.value ? USE_CBOW_AMMO_1 : USE_CBOW_AMMO_2;
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX2));
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX2), pmo->angle-(ANG45/10));
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX2), pmo->angle+(ANG45/10));
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX3), pmo->angle-(ANG45/5));
	P_SpawnPlayerMissile (pmo, RUNTIME_CLASS(ACrossbowFX3), pmo->angle+(ANG45/5));
}

//----------------------------------------------------------------------------
//
// PROC A_BoltSpark
//
//----------------------------------------------------------------------------

void A_BoltSpark (AActor *bolt)
{
	AActor *spark;

	if (P_Random() > 50)
	{
		spark = Spawn<ACrossbowFX4> (bolt->x, bolt->y, bolt->z);
		spark->x += PS_Random() << 10;
		spark->y += PS_Random() << 10;
	}
}

// --- Mace -----------------------------------------------------------------

#define MAGIC_JUNK 1234

void A_FireMacePL1B (player_t *, pspdef_t *);
void A_FireMacePL1 (player_t *, pspdef_t *);
void A_MacePL1Check (AActor *);
void A_MaceBallImpact (AActor *);
void A_MaceBallImpact2 (AActor *);
void A_FireMacePL2 (player_t *, pspdef_t *);
void A_DeathBallImpact (AActor *);

// Wimpy ammo ---------------------------------------------------------------

class AMaceWimpy : public APickup
{
	DECLARE_ACTOR (AMaceWimpy, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_mace, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOMACE1;
	}
};

FState AMaceWimpy::States[] =
{
	S_NORMAL (AMM1, 'A', -1, NULL, NULL)
};

IMPLEMENT_DEF_SERIAL (AMaceWimpy, APickup);
REGISTER_ACTOR (AMaceWimpy, Heretic);

void AMaceWimpy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 13;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_MACE_WIMPY;
	info->flags = MF_SPECIAL;
	AmmoPics[am_mace] = "INAMLOB";
}

// Hefty ammo ---------------------------------------------------------------

class AMaceHefty : public APickup
{
	DECLARE_ACTOR (AMaceHefty, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_mace, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOMACE1;
	}
};

IMPLEMENT_DEF_SERIAL (AMaceHefty, APickup);
REGISTER_ACTOR (AMaceHefty, Heretic);

FState AMaceHefty::States[] =
{
	S_NORMAL (AMM2, 'A', -1, NULL, NULL)
};

void AMaceHefty::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 16;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_MACE_HEFTY;
	info->flags = MF_SPECIAL;
}

// The mace itself ----------------------------------------------------------

class AMace : public AHereticWeapon
{
	DECLARE_ACTOR (AMace, AHereticWeapon);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveWeapon (toucher->player, wp_mace, flags & MF_DROPPED);
	}
	const char *PickupMessage ()
	{
		return TXT_WPNMACE;
	}
	bool DoRespawn ();
	int NumMaceSpots;
	AActor *FirstSpot;
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;

	friend void A_SpawnMace (AActor *self);
};

IMPLEMENT_POINTY_SERIAL (AMace, AHereticWeapon)
 DECLARE_POINTER (FirstSpot)
END_POINTERS

REGISTER_ACTOR (AMace, Heretic);

void AMace::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << NumMaceSpots << FirstSpot;
}

FState AMace::States[] =
{
#define S_WMCE 0
	S_NORMAL (WMCE, 'A',   -1, NULL 				, NULL),

#define S_MACEREADY (S_WMCE+1)
	S_NORMAL (MACE, 'A',	1, A_WeaponReady		, &States[S_MACEREADY]),

#define S_MACEDOWN (S_MACEREADY+1)
	S_NORMAL (MACE, 'A',	1, A_Lower				, &States[S_MACEDOWN]),

#define S_MACEUP (S_MACEDOWN+1)
	S_NORMAL (MACE, 'A',	1, A_Raise				, &States[S_MACEUP]),

#define S_MACEATK1 (S_MACEUP+1)
	S_NORMAL (MACE, 'B',	4, NULL 				, &States[S_MACEATK1+1]),
	S_NORMAL (MACE, 'C',	3, A_FireMacePL1		, &States[S_MACEATK1+2]),
	S_NORMAL (MACE, 'D',	3, A_FireMacePL1		, &States[S_MACEATK1+3]),
	S_NORMAL (MACE, 'E',	3, A_FireMacePL1		, &States[S_MACEATK1+4]),
	S_NORMAL (MACE, 'F',	3, A_FireMacePL1		, &States[S_MACEATK1+5]),
	S_NORMAL (MACE, 'C',	4, A_ReFire 			, &States[S_MACEATK1+6]),
	S_NORMAL (MACE, 'D',	4, NULL 				, &States[S_MACEATK1+7]),
	S_NORMAL (MACE, 'E',	4, NULL 				, &States[S_MACEATK1+8]),
	S_NORMAL (MACE, 'F',	4, NULL 				, &States[S_MACEATK1+9]),
	S_NORMAL (MACE, 'B',	4, NULL 				, &States[S_MACEREADY]),

#define S_MACEATK2 (S_MACEATK1+10)
	S_NORMAL (MACE, 'B',	4, NULL 				, &States[S_MACEATK2+1]),
	S_NORMAL (MACE, 'D',	4, A_FireMacePL2		, &States[S_MACEATK2+2]),
	S_NORMAL (MACE, 'B',	4, NULL 				, &States[S_MACEATK2+3]),
	S_NORMAL (MACE, 'A',	8, A_ReFire 			, &States[S_MACEREADY])
};

FWeaponInfo AMace::WeaponInfo1 =
{
	0,
	am_mace,
	USE_MACE_AMMO_1,
	50,
	&States[S_MACEUP],
	&States[S_MACEDOWN],
	&States[S_MACEREADY],
	&States[S_MACEATK1],
	&States[S_MACEATK1+1],
	NULL,
	RUNTIME_CLASS(AMace),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

FWeaponInfo AMace::WeaponInfo2 =
{
	0,
	am_mace,
	USE_MACE_AMMO_2,
	50,
	&States[S_MACEUP],
	&States[S_MACEDOWN],
	&States[S_MACEREADY],
	&States[S_MACEATK2],
	&States[S_MACEATK2],
	NULL,
	RUNTIME_CLASS(AMace),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

void AMace::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL;
	WeaponSlots[7].AddWeapon (wp_mace, 8);
	wpnlev1info[wp_mace] = &WeaponInfo1;
	wpnlev2info[wp_mace] = &WeaponInfo2;
}

// Mace FX1 -----------------------------------------------------------------

class AMaceFX1 : public AActor
{
	DECLARE_ACTOR (AMaceFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (AMaceFX1, AActor);
REGISTER_ACTOR (AMaceFX1, Heretic);

FState AMaceFX1::States[] =
{
#define S_MACEFX1 0
	S_NORMAL (FX02, 'A',	4, A_MacePL1Check		, &States[S_MACEFX1+1]),
	S_NORMAL (FX02, 'B',	4, A_MacePL1Check		, &States[S_MACEFX1+0]),

#define S_MACEFXI1 (S_MACEFX1+2)
	S_BRIGHT (FX02, 'F',	4, A_MaceBallImpact 	, &States[S_MACEFXI1+1]),
	S_BRIGHT (FX02, 'G',	4, NULL 				, &States[S_MACEFXI1+2]),
	S_BRIGHT (FX02, 'H',	4, NULL 				, &States[S_MACEFXI1+3]),
	S_BRIGHT (FX02, 'I',	4, NULL 				, &States[S_MACEFXI1+4]),
	S_BRIGHT (FX02, 'J',	4, NULL 				, NULL)
};

void AMaceFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MACEFX1];
	info->seesound = "weapons/maceshoot";
	info->deathstate = &States[S_MACEFXI1];
	info->speed = 20 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

// Mace FX2 -----------------------------------------------------------------

class AMaceFX2 : public AActor
{
	DECLARE_ACTOR (AMaceFX2, AActor);
};

IMPLEMENT_DEF_SERIAL (AMaceFX2, AActor);
REGISTER_ACTOR (AMaceFX2, Heretic);

FState AMaceFX2::States[] =
{
#define S_MACEFX2 0
	S_NORMAL (FX02, 'C',	4, NULL 				, &States[S_MACEFX2+1]),
	S_NORMAL (FX02, 'D',	4, NULL 				, &States[S_MACEFX2+0]),

#define S_MACEFXI2 (S_MACEFX2+2)
	S_BRIGHT (FX02, 'F',	4, A_MaceBallImpact2	, &AMaceFX1::States[S_MACEFXI1+1])
};

void AMaceFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MACEFX2];
	info->deathstate = &States[S_MACEFXI2];
	info->speed = 10 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 6;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

// Mace FX3 -----------------------------------------------------------------

class AMaceFX3 : public AActor
{
	DECLARE_ACTOR (AMaceFX3, AActor);
};

IMPLEMENT_DEF_SERIAL (AMaceFX3, AActor);
REGISTER_ACTOR (AMaceFX3, Heretic);

FState AMaceFX3::States[] =
{
#define S_MACEFX3 0
	S_NORMAL (FX02, 'A',	4, NULL 				, &States[S_MACEFX3+1]),
	S_NORMAL (FX02, 'B',	4, NULL 				, &States[S_MACEFX3+0])
};

void AMaceFX3::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MACEFX3];
	info->deathstate = &AMaceFX1::States[S_MACEFXI1];
	info->speed = 7 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 4;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

// Mace FX4 -----------------------------------------------------------------

class AMaceFX4 : public AActor
{
	DECLARE_ACTOR (AMaceFX4, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (AMaceFX4, AActor);
REGISTER_ACTOR (AMaceFX4, Heretic);

FState AMaceFX4::States[] =
{
#define S_MACEFX4 0
	S_NORMAL (FX02, 'E',   99, NULL 				, &States[S_MACEFX4+0]),

#define S_MACEFXI4 (S_MACEFX4+1)
	S_BRIGHT (FX02, 'C',	4, A_DeathBallImpact	, &AMaceFX1::States[S_MACEFXI1+1])
};

void AMaceFX4::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MACEFX4];
	info->deathstate = &States[S_MACEFXI4];
	info->speed = 7 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 18;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_TELESTOMP|MF2_PCROSS|MF2_IMPACT;
}

int AMaceFX4::DoSpecialDamage (AActor *target, int damage)
{
	if ((target->flags2 & MF2_BOSS) || (target->flags3 & MF3_DONTSQUASH))
	{ // Don't allow cheap boss kills
		return damage;
	}
	else if (target->player)
	{ // Player specific checks
		if (target->player->powers[pw_invulnerability])
		{ // Can't hurt invulnerable players
			return -1;
		}
		if (P_AutoUseChaosDevice (target->player))
		{ // Player was saved using chaos device
			return -1;
		}
	}
	return 10000; // Something's gonna die
}

// Mace spawn spot ----------------------------------------------------------

void A_SpawnMace (AActor *);

class AMaceSpawner : public AActor
{
	DECLARE_ACTOR (AMaceSpawner, AActor);
public:
	void BeginPlay ();

	AMaceSpawner *NextSpot;
};

IMPLEMENT_POINTY_SERIAL (AMaceSpawner, AActor)
 DECLARE_POINTER (NextSpot)
END_POINTERS

REGISTER_ACTOR (AMaceSpawner, Heretic);

FState AMaceSpawner::States[] =
{
	S_NORMAL (TNT1, 'A', 1, NULL, &States[1]),
	S_NORMAL (TNT1, 'A', -1, A_SpawnMace, NULL)
};

void AMaceSpawner::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2002;
	info->flags = MF_NOSECTOR|MF_NOBLOCKMAP;
	info->spawnstate = &States[0];
}

void AMaceSpawner::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << NextSpot;
}

void AMaceSpawner::BeginPlay ()
{
	Super::BeginPlay ();
	NextSpot = NULL;
}

// Every mace spawn spot will execute this action. The first one
// will build a list of all mace spots in the level and spawn a
// mace. The rest of the spots will do nothing.

void A_SpawnMace (AActor *self)
{
	if (static_cast<AMaceSpawner *>(self)->NextSpot != NULL)
	{ // Another spot already did it
		return;
	}

	TThinkerIterator<AMaceSpawner> iterator;
	AMaceSpawner *spot;
	AMaceSpawner *firstSpot;
	AMace *mace;
	int numspots = 0;

	spot = firstSpot = iterator.Next ();
	while (spot)
	{
		numspots++;
		spot->NextSpot = iterator.Next ();
		if (spot->NextSpot == NULL)
		{
			spot->NextSpot = firstSpot;
			spot = NULL;
		}
		else
		{
			spot = spot->NextSpot;
		}
	}
	if (numspots == 0)
	{
		return;
	}
	if (!deathmatch.value && P_Random() < 64)
	{ // Sometimes doesn't show up if not in deathmatch
		return;
	}
	mace = Spawn<AMace> (self->x, self->y, self->z);
	if (mace)
	{
		mace->FirstSpot = firstSpot;
		mace->NumMaceSpots = numspots;
		mace->DoRespawn ();
	}
}

// AMace::DoRespawn
// Moves the mace to a different spot when it respawns

bool AMace::DoRespawn ()
{
	int spotnum = P_Random () % NumMaceSpots;
	AMaceSpawner *spot = static_cast<AMaceSpawner *>(FirstSpot);

	while (spotnum > 0)
	{
		spot = spot->NextSpot;
		spotnum--;
	}

	SetOrigin (spot->x, spot->y, spot->z);
	z = floorz;
	return true;
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL1B
//
//----------------------------------------------------------------------------

void A_FireMacePL1B (player_t *player, pspdef_t *psp)
{
	AActor *pmo;
	AActor *ball;
	angle_t angle;

	if (player->ammo[am_mace] < USE_MACE_AMMO_1)
	{
		return;
	}
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_mace] -= USE_MACE_AMMO_1;
	pmo = player->mo;
	ball = Spawn<AMaceFX2> (pmo->x, pmo->y, pmo->z + 28*FRACUNIT 
		- pmo->floorclip);
	ball->momz = 2*FRACUNIT+/*((player->lookdir)<<(FRACBITS-5))*/
		finetangent[FINEANGLES/4-(pmo->pitch>>ANGLETOFINESHIFT)];
	angle = pmo->angle;
	ball->target = pmo;
	ball->angle = angle;
	ball->z += 2*finetangent[FINEANGLES/4-(pmo->pitch>>ANGLETOFINESHIFT)];
	angle >>= ANGLETOFINESHIFT;
	ball->momx = (pmo->momx>>1)
		+FixedMul(GetInfo (ball)->speed, finecosine[angle]);
	ball->momy = (pmo->momy>>1)
		+FixedMul(GetInfo (ball)->speed, finesine[angle]);
	S_Sound (ball, CHAN_BODY, "weapons/maceshoot", 1, ATTN_NORM);
	P_CheckMissileSpawn (ball);
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL1
//
//----------------------------------------------------------------------------

void A_FireMacePL1 (player_t *player, pspdef_t *psp)
{
	AActor *ball;

	if (P_Random() < 28)
	{
		A_FireMacePL1B (player, psp);
		return;
	}
	if (player->ammo[am_mace] < USE_MACE_AMMO_1)
	{
		return;
	}
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_mace] -= USE_MACE_AMMO_1;
	psp->sx = ((P_Random()&3)-2)*FRACUNIT;
	psp->sy = WEAPONTOP+(P_Random()&3)*FRACUNIT;
	ball = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AMaceFX1),
		player->mo->angle+(((P_Random()&7)-4)<<24));
	if (ball)
	{
		ball->special1 = 16; // tics till dropoff
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MacePL1Check
//
//----------------------------------------------------------------------------

void A_MacePL1Check (AActor *ball)
{
	angle_t angle;

	if (ball->special1 == 0)
	{
		return;
	}
	ball->special1 -= 4;
	if (ball->special1 > 0)
	{
		return;
	}
	ball->special1 = 0;
	ball->flags2 |= MF2_LOGRAV;
	angle = ball->angle>>ANGLETOFINESHIFT;
	ball->momx = FixedMul(7*FRACUNIT, finecosine[angle]);
	ball->momy = FixedMul(7*FRACUNIT, finesine[angle]);
	ball->momz -= ball->momz>>1;
}

//----------------------------------------------------------------------------
//
// PROC A_MaceBallImpact
//
//----------------------------------------------------------------------------

void A_MaceBallImpact (AActor *ball)
{
	if ((ball->z <= ball->floorz) && P_HitFloor (ball))
	{ // Landed in some sort of liquid
		ball->Destroy ();
		return;
	}
	if ((ball->health != MAGIC_JUNK) && (ball->z <= ball->floorz)
		&& ball->momz)
	{ // Bounce
		ball->health = MAGIC_JUNK;
		ball->momz = (ball->momz*192)>>8;
		ball->flags2 &= ~MF2_FLOORBOUNCE;
		ball->SetState (GetInfo (ball)->spawnstate);
		S_Sound (ball, CHAN_BODY, "weapons/macebounce", 1, ATTN_NORM);
	}
	else
	{ // Explode
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~MF2_LOGRAV;
		S_Sound (ball, CHAN_BODY, "weapons/macehit", 1, ATTN_NORM);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MaceBallImpact2
//
//----------------------------------------------------------------------------

void A_MaceBallImpact2 (AActor *ball)
{
	AActor *tiny;
	angle_t angle;

	if ((ball->z <= ball->floorz) && P_HitFloor(ball))
	{ // Landed in some sort of liquid
		ball->Destroy ();
		return;
	}
	if ((ball->z != ball->floorz) || (ball->momz < 2*FRACUNIT))
	{ // Explode
		ball->momx = ball->momy = ball->momz = 0;
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~(MF2_LOGRAV|MF2_FLOORBOUNCE);
	}
	else
	{ // Bounce
		ball->momz = (ball->momz*192)>>8;
		ball->SetState (GetInfo (ball)->spawnstate);

		tiny = Spawn<AMaceFX3> (ball->x, ball->y, ball->z);
		angle = ball->angle+ANG90;
		tiny->target = ball->target;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = (ball->momx>>1)+FixedMul(ball->momz-FRACUNIT,
			finecosine[angle]);
		tiny->momy = (ball->momy>>1)+FixedMul(ball->momz-FRACUNIT,
			finesine[angle]);
		tiny->momz = ball->momz;
		P_CheckMissileSpawn (tiny);

		tiny = Spawn<AMaceFX3> (ball->x, ball->y, ball->z);
		angle = ball->angle-ANG90;
		tiny->target = ball->target;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = (ball->momx>>1)+FixedMul(ball->momz-FRACUNIT,
			finecosine[angle]);
		tiny->momy = (ball->momy>>1)+FixedMul(ball->momz-FRACUNIT,
			finesine[angle]);
		tiny->momz = ball->momz;
		P_CheckMissileSpawn (tiny);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL2
//
//----------------------------------------------------------------------------

void A_FireMacePL2 (player_t *player, pspdef_t *psp)
{
	AActor *mo;

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_mace] -=
			deathmatch.value ? USE_MACE_AMMO_1 : USE_MACE_AMMO_2;
	mo = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AMaceFX4));
	if (mo)
	{
		mo->momx += player->mo->momx;
		mo->momy += player->mo->momy;
		mo->momz = 2*FRACUNIT+
			finetangent[FINEANGLES/4-(player->mo->pitch>>ANGLETOFINESHIFT)];
		if (linetarget)
		{
			mo->tracer = linetarget;
		}
	}
	S_Sound (player->mo, CHAN_WEAPON, "weapons/maceshoot", 1, ATTN_NORM);
}

//----------------------------------------------------------------------------
//
// PROC A_DeathBallImpact
//
//----------------------------------------------------------------------------

void A_DeathBallImpact (AActor *ball)
{
	int i;
	AActor *target;
	angle_t angle;
	bool newAngle;

	if ((ball->z <= ball->floorz) && P_HitFloor (ball))
	{ // Landed in some sort of liquid
		ball->Destroy ();
		return;
	}
	if ((ball->z <= ball->floorz) && ball->momz)
	{ // Bounce
		newAngle = false;
		target = ball->tracer;
		if (target)
		{
			if (!(target->flags&MF_SHOOTABLE))
			{ // Target died
				ball->tracer = NULL;
			}
			else
			{ // Seek
				angle = R_PointToAngle2(ball->x, ball->y,
					target->x, target->y);
				newAngle = true;
			}
		}
		else
		{ // Find new target
			angle = 0;
			for (i = 0; i < 16; i++)
			{
				P_AimLineAttack (ball, angle, 10*64*FRACUNIT);
				if (linetarget && ball->target != linetarget)
				{
					ball->tracer = linetarget;
					angle = R_PointToAngle2 (ball->x, ball->y,
						linetarget->x, linetarget->y);
					newAngle = true;
					break;
				}
				angle += ANGLE_45/2;
			}
		}
		if (newAngle)
		{
			ball->angle = angle;
			angle >>= ANGLETOFINESHIFT;
			ball->momx = FixedMul (GetInfo (ball)->speed, finecosine[angle]);
			ball->momy = FixedMul (GetInfo (ball)->speed, finesine[angle]);
		}
		ball->SetState (GetInfo (ball)->spawnstate);
		S_Sound (ball, CHAN_BODY, "weapons/macestop", 1, ATTN_NORM);
	}
	else
	{ // Explode
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~MF2_LOGRAV;
		S_Sound (ball, CHAN_BODY, "weapons/maceexplode", 1, ATTN_NORM);
	}
}

// --- Gauntlets ------------------------------------------------------------

void A_GauntletAttack (player_t *, pspdef_t *);

// Gauntlets ----------------------------------------------------------------

class AGauntlets : public AHereticWeapon
{
	DECLARE_ACTOR (AGauntlets, AHereticWeapon);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveWeapon (toucher->player, wp_gauntlets, flags & MF_DROPPED);
	}
	const char *PickupMessage ()
	{
		return TXT_WPNGAUNTLETS;
	}
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (AGauntlets, AHereticWeapon);
REGISTER_ACTOR (AGauntlets, Heretic);

FState AGauntlets::States[] =
{
#define S_WGNT 0
	S_NORMAL (WGNT, 'A',   -1, NULL 					, NULL),

#define S_GAUNTLETREADY (S_WGNT+1)
	S_NORMAL (GAUN, 'A',	1, A_WeaponReady			, &States[S_GAUNTLETREADY]),

#define S_GAUNTLETDOWN (S_GAUNTLETREADY+1)
	S_NORMAL (GAUN, 'A',	1, A_Lower					, &States[S_GAUNTLETDOWN]),

#define S_GAUNTLETUP (S_GAUNTLETDOWN+1)
	S_NORMAL (GAUN, 'A',	1, A_Raise					, &States[S_GAUNTLETUP]),

#define S_GAUNTLETREADY2 (S_GAUNTLETUP+1)
	S_NORMAL (GAUN, 'G',	4, A_WeaponReady			, &States[S_GAUNTLETREADY2+1]),
	S_NORMAL (GAUN, 'H',	4, A_WeaponReady			, &States[S_GAUNTLETREADY2+2]),
	S_NORMAL (GAUN, 'I',	4, A_WeaponReady			, &States[S_GAUNTLETREADY2+0]),

#define S_GAUNTLETDOWN2 (S_GAUNTLETREADY2+3)
	S_NORMAL (GAUN, 'G',	1, A_Lower					, &States[S_GAUNTLETDOWN2]),

#define S_GAUNTLETUP2 (S_GAUNTLETDOWN2+1)
	S_NORMAL (GAUN, 'G',	1, A_Raise					, &States[S_GAUNTLETUP2]),

#define S_GAUNTLETATK1 (S_GAUNTLETUP2+1)
	S_NORMAL (GAUN, 'B',	4, NULL 					, &States[S_GAUNTLETATK1+1]),
	S_NORMAL (GAUN, 'C',	4, NULL 					, &States[S_GAUNTLETATK1+2]),
	S_BRIGHT (GAUN, 'D',	4, A_GauntletAttack 		, &States[S_GAUNTLETATK1+3]),
	S_BRIGHT (GAUN, 'E',	4, A_GauntletAttack 		, &States[S_GAUNTLETATK1+4]),
	S_BRIGHT (GAUN, 'F',	4, A_GauntletAttack 		, &States[S_GAUNTLETATK1+5]),
	S_NORMAL (GAUN, 'C',	4, A_ReFire 				, &States[S_GAUNTLETATK1+6]),
	S_NORMAL (GAUN, 'B',	4, A_Light0 				, &States[S_GAUNTLETREADY]),

#define S_GAUNTLETATK2 (S_GAUNTLETATK1+7)
	S_NORMAL (GAUN, 'J',	4, NULL 					, &States[S_GAUNTLETATK2+1]),
	S_NORMAL (GAUN, 'K',	4, NULL 					, &States[S_GAUNTLETATK2+2]),
	S_BRIGHT (GAUN, 'L',	4, A_GauntletAttack 		, &States[S_GAUNTLETATK2+3]),
	S_BRIGHT (GAUN, 'M',	4, A_GauntletAttack 		, &States[S_GAUNTLETATK2+4]),
	S_BRIGHT (GAUN, 'N',	4, A_GauntletAttack 		, &States[S_GAUNTLETATK2+5]),
	S_NORMAL (GAUN, 'K',	4, A_ReFire 				, &States[S_GAUNTLETATK2+6]),
	S_NORMAL (GAUN, 'J',	4, A_Light0 				, &States[S_GAUNTLETREADY2+0])
};

FWeaponInfo AGauntlets::WeaponInfo1 =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_GAUNTLETUP],
	&States[S_GAUNTLETDOWN],
	&States[S_GAUNTLETREADY],
	&States[S_GAUNTLETATK1],
	&States[S_GAUNTLETATK1+2],
	NULL,
	RUNTIME_CLASS(AGauntlets),
	150,
	15*FRACUNIT,
	"weapons/gauntletsactivate",
	NULL
};

FWeaponInfo AGauntlets::WeaponInfo2 =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_GAUNTLETUP2],
	&States[S_GAUNTLETDOWN2],
	&States[S_GAUNTLETREADY2],
	&States[S_GAUNTLETATK2],
	&States[S_GAUNTLETATK2+2],
	NULL,
	RUNTIME_CLASS(AGauntlets),
	150,
	15*FRACUNIT,
	"weapons/gauntletsactivate",
	NULL
};

void AGauntlets::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2005;
	info->spawnstate = &States[S_WGNT];
	info->flags = MF_SPECIAL;
	WeaponSlots[1].AddWeapon (wp_gauntlets, 2);
	wpnlev1info[wp_gauntlets] = &WeaponInfo1;
	wpnlev2info[wp_gauntlets] = &WeaponInfo2;
}

// Gauntlet puff 1 ----------------------------------------------------------

class AGauntletPuff1 : public AActor
{
	DECLARE_ACTOR (AGauntletPuff1, AActor);
public:
	void BeginPlay ();
};

IMPLEMENT_DEF_SERIAL (AGauntletPuff1, AActor);
REGISTER_ACTOR (AGauntletPuff1, Heretic);

FState AGauntletPuff1::States[] =
{
#define S_GAUNTLETPUFF1 0
	S_BRIGHT (PUF1, 'A',	4, NULL 					, &States[S_GAUNTLETPUFF1+1]),
	S_BRIGHT (PUF1, 'B',	4, NULL 					, &States[S_GAUNTLETPUFF1+2]),
	S_BRIGHT (PUF1, 'C',	4, NULL 					, &States[S_GAUNTLETPUFF1+3]),
	S_BRIGHT (PUF1, 'D',	4, NULL 					, NULL)
};

void AGauntletPuff1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_GAUNTLETPUFF1];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->translucency = HR_SHADOW;
}

void AGauntletPuff1::BeginPlay ()
{
	Super::BeginPlay ();
	momz = FRACUNIT * 8 / 10;
}

// Gauntlett puff 2 ---------------------------------------------------------

class AGauntletPuff2 : public AGauntletPuff1
{
	DECLARE_ACTOR (AGauntletPuff2, AGauntletPuff1);
};

IMPLEMENT_DEF_SERIAL (AGauntletPuff2, AGauntletPuff1);
REGISTER_ACTOR (AGauntletPuff2, Heretic);

FState AGauntletPuff2::States[] =
{
#define S_GAUNTLETPUFF2 0
	S_BRIGHT (PUF1, 'E',	4, NULL 					, &States[S_GAUNTLETPUFF2+1]),
	S_BRIGHT (PUF1, 'F',	4, NULL 					, &States[S_GAUNTLETPUFF2+2]),
	S_BRIGHT (PUF1, 'G',	4, NULL 					, &States[S_GAUNTLETPUFF2+3]),
	S_BRIGHT (PUF1, 'H',	4, NULL 					, NULL)
};

void AGauntletPuff2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_GAUNTLETPUFF2];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->translucency = HR_SHADOW;
}

//---------------------------------------------------------------------------
//
// PROC A_GauntletAttack
//
//---------------------------------------------------------------------------

void A_GauntletAttack (player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	int randVal;
	fixed_t dist;

	psp->sx = ((P_Random()&3)-2) * FRACUNIT;
	psp->sy = WEAPONTOP + (P_Random()&3) * FRACUNIT;
	angle = player->mo->angle;
	if (player->powers[pw_weaponlevel2])
	{
		damage = HITDICE(2);
		dist = 4*MELEERANGE;
		angle += PS_Random() << 17;
		PuffType = RUNTIME_CLASS(AGauntletPuff2);
	}
	else
	{
		damage = HITDICE(2);
		dist = MELEERANGE+1;
		angle += PS_Random() << 18;
		PuffType = RUNTIME_CLASS(AGauntletPuff1);
	}
	slope = P_AimLineAttack (player->mo, angle, dist);
	P_LineAttack (player->mo, angle, dist, slope, damage);
	if (!linetarget)
	{
		if (P_Random() > 64)
		{
			player->extralight = !player->extralight;
		}
		S_Sound (player->mo, CHAN_AUTO, "weapons/gauntletson", 1, ATTN_NORM);
		return;
	}
	randVal = P_Random();
	if (randVal < 64)
	{
		player->extralight = 0;
	}
	else if (randVal < 160)
	{
		player->extralight = 1;
	}
	else
	{
		player->extralight = 2;
	}
	if (player->powers[pw_weaponlevel2])
	{
		P_GiveBody (player, damage>>1);
		S_Sound (player->mo, CHAN_AUTO, "weapons/gauntletspowhit", 1, ATTN_NORM);
	}
	else
	{
		S_Sound (player->mo, CHAN_AUTO, "weapons/gauntletshit", 1, ATTN_NORM);
	}
	// turn to face target
	angle = R_PointToAngle2 (player->mo->x, player->mo->y,
		linetarget->x, linetarget->y);
	if (angle-player->mo->angle > ANG180)
	{
		if (angle-player->mo->angle < -ANG90/20)
			player->mo->angle = angle+ANG90/21;
		else
			player->mo->angle -= ANG90/20;
	}
	else
	{
		if (angle-player->mo->angle > ANG90/20)
			player->mo->angle = angle-ANG90/21;
		else
			player->mo->angle += ANG90/20;
	}
	player->mo->flags |= MF_JUSTATTACKED;
}

// --- Blaster (aka Claw) ---------------------------------------------------

void A_FireBlasterPL1 (player_t *, pspdef_t *);
void A_FireBlasterPL2 (player_t *, pspdef_t *);
void A_SpawnRippers (AActor *);

// Wimpy ammo ---------------------------------------------------------------

class ABlasterWimpy : public APickup
{
	DECLARE_ACTOR (ABlasterWimpy, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_blaster, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOBLASTER1;
	}
};

IMPLEMENT_DEF_SERIAL (ABlasterWimpy, APickup);
REGISTER_ACTOR (ABlasterWimpy, Heretic);

FState ABlasterWimpy::States[] =
{
	S_NORMAL (AMB1, 'A',	4, NULL 					, &States[1]),
	S_NORMAL (AMB1, 'B',	4, NULL 					, &States[2]),
	S_NORMAL (AMB1, 'C',	4, NULL 					, &States[0])
};

void ABlasterWimpy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 54;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_BLSR_WIMPY;
	info->flags = MF_SPECIAL;
	AmmoPics[am_blaster] = "INAMBST";
}

// Hefty ammo ---------------------------------------------------------------

class ABlasterHefty : public APickup
{
	DECLARE_ACTOR (ABlasterHefty, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_blaster, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOBLASTER2;
	}
};

IMPLEMENT_DEF_SERIAL (ABlasterHefty, APickup);
REGISTER_ACTOR (ABlasterHefty, Heretic);

FState ABlasterHefty::States[] =
{
	S_NORMAL (AMB2, 'A',	4, NULL 					, &States[1]),
	S_NORMAL (AMB2, 'B',	4, NULL 					, &States[2]),
	S_NORMAL (AMB2, 'C',	4, NULL 					, &States[0])
};

void ABlasterHefty::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 55;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_BLSR_HEFTY;
	info->flags = MF_SPECIAL;
}

// Blaster ------------------------------------------------------------------

class ABlaster : public AHereticWeapon
{
	DECLARE_ACTOR (ABlaster, AHereticWeapon);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveWeapon (toucher->player, wp_blaster, flags & MF_DROPPED);
	}
	const char *PickupMessage ()
	{
		return TXT_WPNBLASTER;
	}
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (ABlaster, AHereticWeapon);
REGISTER_ACTOR (ABlaster, Heretic);

FState ABlaster::States[] =
{
#define S_BLSR 0
	S_NORMAL (WBLS, 'A',   -1, NULL 						, NULL),

#define S_BLASTERREADY (S_BLSR+1)
	S_NORMAL (BLSR, 'A',	1, A_WeaponReady				, &States[S_BLASTERREADY]),

#define S_BLASTERDOWN (S_BLASTERREADY+1)
	S_NORMAL (BLSR, 'A',	1, A_Lower						, &States[S_BLASTERDOWN]),

#define S_BLASTERUP (S_BLASTERDOWN+1)
	S_NORMAL (BLSR, 'A',	1, A_Raise						, &States[S_BLASTERUP]),

#define S_BLASTERATK1 (S_BLASTERUP+1)
	S_NORMAL (BLSR, 'B',	3, NULL 						, &States[S_BLASTERATK1+1]),
	S_NORMAL (BLSR, 'C',	3, NULL 						, &States[S_BLASTERATK1+2]),
	S_NORMAL (BLSR, 'D',	2, A_FireBlasterPL1 			, &States[S_BLASTERATK1+3]),
	S_NORMAL (BLSR, 'C',	2, NULL 						, &States[S_BLASTERATK1+4]),
	S_NORMAL (BLSR, 'B',	2, NULL 						, &States[S_BLASTERATK1+5]),
	S_NORMAL (BLSR, 'A',	0, A_ReFire 					, &States[S_BLASTERREADY]),

#define S_BLASTERATK2 (S_BLASTERATK1+6)
	S_NORMAL (BLSR, 'B',	0, NULL 						, &States[S_BLASTERATK2+1]),
	S_NORMAL (BLSR, 'C',	0, NULL 						, &States[S_BLASTERATK2+2]),
	S_NORMAL (BLSR, 'D',	3, A_FireBlasterPL2 			, &States[S_BLASTERATK2+3]),
	S_NORMAL (BLSR, 'C',	4, NULL 						, &States[S_BLASTERATK2+4]),
	S_NORMAL (BLSR, 'B',	4, NULL 						, &States[S_BLASTERATK2+5]),
	S_NORMAL (BLSR, 'A',	0, A_ReFire 					, &States[S_BLASTERREADY])
};

FWeaponInfo ABlaster::WeaponInfo1 =
{
	0,
	am_blaster,
	USE_BLSR_AMMO_1,
	30,
	&States[S_BLASTERUP],
	&States[S_BLASTERDOWN],
	&States[S_BLASTERREADY],
	&States[S_BLASTERATK1],
	&States[S_BLASTERATK1+2],
	NULL,
	RUNTIME_CLASS(ABlaster),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

FWeaponInfo ABlaster::WeaponInfo2 =
{
	0,
	am_blaster,
	USE_BLSR_AMMO_2,
	30,
	&States[S_BLASTERUP],
	&States[S_BLASTERDOWN],
	&States[S_BLASTERREADY],
	&States[S_BLASTERATK2],
	&States[S_BLASTERATK2+2],
	NULL,
	RUNTIME_CLASS(ABlaster),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

void ABlaster::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 53;
	info->spawnstate = &States[S_BLSR];
	info->flags = MF_SPECIAL;
	WeaponSlots[4].AddWeapon (wp_blaster, 5);
	wpnlev1info[wp_blaster] = &WeaponInfo1;
	wpnlev2info[wp_blaster] = &WeaponInfo2;
}

// Blaster FX 1 -------------------------------------------------------------

class ABlasterFX1 : public AActor
{
	DECLARE_ACTOR (ABlasterFX1, AActor);
public:
	void RunThink ();
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (ABlasterFX1, AActor);
REGISTER_ACTOR (ABlasterFX1, Heretic);

FState ABlasterFX1::States[] =
{
#define S_BLASTERFX1 0
	S_NORMAL (ACLO, 'E',  200, NULL 					, &States[S_BLASTERFX1+0]),

#define S_BLASTERFXI1 (S_BLASTERFX1+1)
	S_BRIGHT (FX18, 'A',	3, A_SpawnRippers			, &States[S_BLASTERFXI1+1]),
	S_BRIGHT (FX18, 'B',	3, NULL 					, &States[S_BLASTERFXI1+2]),
	S_BRIGHT (FX18, 'C',	4, NULL 					, &States[S_BLASTERFXI1+3]),
	S_BRIGHT (FX18, 'D',	4, NULL 					, &States[S_BLASTERFXI1+4]),
	S_BRIGHT (FX18, 'E',	4, NULL 					, &States[S_BLASTERFXI1+5]),
	S_BRIGHT (FX18, 'F',	4, NULL 					, &States[S_BLASTERFXI1+6]),
	S_BRIGHT (FX18, 'G',	4, NULL 					, NULL)
};

void ABlasterFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BLASTERFX1];
	info->deathstate = &States[S_BLASTERFXI1];
	info->deathsound = "weapons/blasterhit";
	info->speed = 184 * FRACUNIT;
	info->radius = 12 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
	info->flags3 = MF3_VERYFAST;
}

int ABlasterFX1::DoSpecialDamage (AActor *target, int damage)
{
	if (target->IsKindOf (TypeInfo::FindType ("Ironlich")))
	{ // Less damage to Ironlich bosses
		damage = P_Random() & 1;
		if (!damage)
		{
			return -1;
		}
	}
	return damage;
}

// Blaster smoke ------------------------------------------------------------

class ABlasterSmoke : public AActor
{
	DECLARE_ACTOR (ABlasterSmoke, AActor);
};

IMPLEMENT_DEF_SERIAL (ABlasterSmoke, AActor);
REGISTER_ACTOR (ABlasterSmoke, Heretic);

FState ABlasterSmoke::States[] =
{
#define S_BLASTERSMOKE 0
	S_NORMAL (FX18, 'H',	4, NULL 					, &States[S_BLASTERSMOKE+1]),
	S_NORMAL (FX18, 'I',	4, NULL 					, &States[S_BLASTERSMOKE+2]),
	S_NORMAL (FX18, 'J',	4, NULL 					, &States[S_BLASTERSMOKE+3]),
	S_NORMAL (FX18, 'K',	4, NULL 					, &States[S_BLASTERSMOKE+4]),
	S_NORMAL (FX18, 'L',	4, NULL 					, NULL)
};

void ABlasterSmoke::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BLASTERSMOKE];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_CANNOTPUSH;
	info->translucency = HR_SHADOW;
}

// Ripper -------------------------------------------------------------------

class ARipper : public AActor
{
	DECLARE_ACTOR (ARipper, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (ARipper, AActor);
REGISTER_ACTOR (ARipper, Heretic);

FState ARipper::States[] =
{
#define S_RIPPER 0
	S_NORMAL (FX18, 'M',	4, NULL 					, &States[S_RIPPER+1]),
	S_NORMAL (FX18, 'N',	5, NULL 					, &States[S_RIPPER+0]),

#define S_RIPPERX (S_RIPPER+2)
	S_BRIGHT (FX18, 'O',	4, NULL 					, &States[S_RIPPERX+1]),
	S_BRIGHT (FX18, 'P',	4, NULL 					, &States[S_RIPPERX+2]),
	S_BRIGHT (FX18, 'Q',	4, NULL 					, &States[S_RIPPERX+3]),
	S_BRIGHT (FX18, 'R',	4, NULL 					, &States[S_RIPPERX+4]),
	S_BRIGHT (FX18, 'S',	4, NULL 					, NULL)
};

void ARipper::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_RIPPER];
	info->deathsound = "weapons/blasterpowhit";
	info->speed = 14 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 1;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_RIP|MF2_PCROSS|MF2_IMPACT;
}

int ARipper::DoSpecialDamage (AActor *target, int damage)
{
	if (target->IsKindOf (TypeInfo::FindType ("Ironlich")))
	{ // Less damage to Ironlich bosses
		damage = P_Random() & 1;
		if (!damage)
		{
			return -1;
		}
	}
	return damage;
}

// Blaster puff 1 -----------------------------------------------------------

class ABlasterPuff1 : public AActor
{
	DECLARE_ACTOR (ABlasterPuff1, AActor);
};

IMPLEMENT_DEF_SERIAL (ABlasterPuff1, AActor);
REGISTER_ACTOR (ABlasterPuff1, Heretic);

FState ABlasterPuff1::States[] =
{
#define S_BLASTERPUFF1 0
	S_BRIGHT (FX17, 'A',	4, NULL 					, &States[S_BLASTERPUFF1+1]),
	S_BRIGHT (FX17, 'B',	4, NULL 					, &States[S_BLASTERPUFF1+2]),
	S_BRIGHT (FX17, 'C',	4, NULL 					, &States[S_BLASTERPUFF1+3]),
	S_BRIGHT (FX17, 'D',	4, NULL 					, &States[S_BLASTERPUFF1+4]),
	S_BRIGHT (FX17, 'E',	4, NULL 					, NULL),
};

void ABlasterPuff1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BLASTERPUFF1];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
}

// Blaster puff 2 -----------------------------------------------------------

class ABlasterPuff2 : public AActor
{
	DECLARE_ACTOR (ABlasterPuff2, AActor);
};

IMPLEMENT_DEF_SERIAL (ABlasterPuff2, AActor);
REGISTER_ACTOR (ABlasterPuff2, Heretic);

FState ABlasterPuff2::States[] =
{
#define S_BLASTERPUFF2 0
	S_BRIGHT (FX17, 'F',	3, NULL 					, &States[S_BLASTERPUFF2+1]),
	S_BRIGHT (FX17, 'G',	3, NULL 					, &States[S_BLASTERPUFF2+2]),
	S_BRIGHT (FX17, 'H',	4, NULL 					, &States[S_BLASTERPUFF2+3]),
	S_BRIGHT (FX17, 'I',	4, NULL 					, &States[S_BLASTERPUFF2+4]),
	S_BRIGHT (FX17, 'J',	4, NULL 					, &States[S_BLASTERPUFF2+5]),
	S_BRIGHT (FX17, 'K',	4, NULL 					, &States[S_BLASTERPUFF2+6]),
	S_BRIGHT (FX17, 'L',	4, NULL 					, NULL)
};

void ABlasterPuff2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BLASTERPUFF2];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
}

//----------------------------------------------------------------------------
//
// PROC A_FireBlasterPL1
//
//----------------------------------------------------------------------------

void A_FireBlasterPL1 (player_t *player, pspdef_t *psp)
{
	AActor *mo;
	angle_t angle;
	int damage;

	mo = player->mo;
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_blaster] -= USE_BLSR_AMMO_1;
	P_BulletSlope(mo);
	damage = HITDICE(4);
	angle = mo->angle;
	if (player->refire)
	{
		angle += PS_Random() << 18;
	}
	PuffType = RUNTIME_CLASS(ABlasterPuff1);
	HitPuffType = RUNTIME_CLASS(ABlasterPuff2);
	P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
	HitPuffType = NULL;
	S_Sound (mo, CHAN_WEAPON, "weapons/blastershoot", 1, ATTN_NORM);
}

//----------------------------------------------------------------------------
//
// PROC A_FireBlasterPL2
//
//----------------------------------------------------------------------------

void A_FireBlasterPL2 (player_t *player, pspdef_t *psp)
{
	AActor *mo;

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_blaster] -=
			deathmatch.value ? USE_BLSR_AMMO_1 : USE_BLSR_AMMO_2;
	mo = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ABlasterFX1));
	S_Sound (mo, CHAN_WEAPON, "weapons/blastershoot", 1, ATTN_NORM);
}

//----------------------------------------------------------------------------
//
// PROC A_SpawnRippers
//
//----------------------------------------------------------------------------

void A_SpawnRippers (AActor *actor)
{
	int i;
	angle_t angle;
	AActor *ripper;

	for(i = 0; i < 8; i++)
	{
		ripper = Spawn<ARipper> (actor->x, actor->y, actor->z);
		angle = i*ANG45;
		ripper->target = actor->target;
		ripper->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		ripper->momx = FixedMul (GetInfo (ripper)->speed, finecosine[angle]);
		ripper->momy = FixedMul (GetInfo (ripper)->speed, finesine[angle]);
		P_CheckMissileSpawn (ripper);
	}
}

//----------------------------------------------------------------------------
//
// PROC P_BlasterMobjThinker
//
// Thinker for the ultra-fast blaster PL2 ripper-spawning missile.
//
//----------------------------------------------------------------------------

extern void P_ExplodeMissile (AActor *);

void ABlasterFX1::RunThink ()
{
	int i;
	fixed_t xfrac;
	fixed_t yfrac;
	fixed_t zfrac;
	int changexy;

	// Handle movement
	if (momx || momy || (z != floorz) || momz)
	{
		xfrac = momx>>3;
		yfrac = momy>>3;
		zfrac = momz>>3;
		changexy = xfrac | yfrac;
		for (i = 0; i < 8; i++)
		{
			if (changexy)
			{
				if (!P_TryMove (this, x + xfrac, y + yfrac, true))
				{ // Blocked move
					P_ExplodeMissile (this);
					return;
				}
			}
			z += zfrac;
			if (z <= floorz)
			{ // Hit the floor
				z = floorz;
				P_HitFloor (this);
				P_ExplodeMissile (this);
				return;
			}
			if (z + height > ceilingz)
			{ // Hit the ceiling
				z = ceilingz - height;
				P_ExplodeMissile (this);
				return;
			}
			if (changexy && (P_Random() < 64))
			{
				Spawn<ABlasterSmoke> (x, y, MAX (z - 8 * FRACUNIT, floorz));
			}
		}
	}
	// Advance the state
	if (tics != -1)
	{
		tics--;
		while (!tics)
		{
			if (!SetState (state->nextstate))
			{ // mobj was removed
				return;
			}
		}
	}
}

// --- Skull rod ------------------------------------------------------------

void A_FireSkullRodPL1 (player_t *, pspdef_t *);
void A_FireSkullRodPL2 (player_t *, pspdef_t *);
void A_SkullRodPL2Seek (AActor *);
void A_AddPlayerRain (AActor *);
void A_HideInCeiling (AActor *);
void A_SkullRodStorm (AActor *);
void A_RainImpact (AActor *);

// Wimpy ammo ---------------------------------------------------------------

class ASkullRodWimpy : public APickup
{
	DECLARE_ACTOR (ASkullRodWimpy, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_skullrod, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOSKULLROD1;
	}
};

IMPLEMENT_DEF_SERIAL (ASkullRodWimpy, APickup);
REGISTER_ACTOR (ASkullRodWimpy, Heretic);

FState ASkullRodWimpy::States[] =
{
	S_NORMAL (AMS1, 'A',	5, NULL 					, &States[1]),
	S_NORMAL (AMS1, 'B',	5, NULL 					, &States[0])
};

void ASkullRodWimpy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 20;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_SKRD_WIMPY;
	info->flags = MF_SPECIAL;
	AmmoPics[am_skullrod] = "INAMRAM";
}

// Hefty ammo ---------------------------------------------------------------

class ASkullRodHefty : public APickup
{
	DECLARE_ACTOR (ASkullRodHefty, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_skullrod, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOSKULLROD2;
	}
};

IMPLEMENT_DEF_SERIAL (ASkullRodHefty, APickup);
REGISTER_ACTOR (ASkullRodHefty, Heretic);

FState ASkullRodHefty::States[] =
{
	S_NORMAL (AMS2, 'A',	5, NULL 					, &States[1]),
	S_NORMAL (AMS2, 'B',	5, NULL 					, &States[0])
};

void ASkullRodHefty::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 21;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_SKRD_HEFTY;
	info->flags = MF_SPECIAL;
}

// Skull (Horn) Rod ---------------------------------------------------------

class ASkullRod : public AHereticWeapon
{
	DECLARE_ACTOR (ASkullRod, AHereticWeapon);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveWeapon (toucher->player, wp_skullrod, flags & MF_DROPPED);
	}
	const char *PickupMessage ()
	{
		return TXT_WPNSKULLROD;
	}
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (ASkullRod, AHereticWeapon);
REGISTER_ACTOR (ASkullRod, Heretic);

FState ASkullRod::States[] =
{
#define S_WSKL 0
	S_NORMAL (WSKL, 'A',   -1, NULL 						, NULL),

#define S_HORNRODREADY (S_WSKL+1)
	S_NORMAL (HROD, 'A',	1, A_WeaponReady				, &States[S_HORNRODREADY]),

#define S_HORNRODDOWN (S_HORNRODREADY+1)
	S_NORMAL (HROD, 'A',	1, A_Lower						, &States[S_HORNRODDOWN]),

#define S_HORNRODUP (S_HORNRODDOWN+1)
	S_NORMAL (HROD, 'A',	1, A_Raise						, &States[S_HORNRODUP]),

#define S_HORNRODATK1 (S_HORNRODUP+1)
	S_NORMAL (HROD, 'A',	4, A_FireSkullRodPL1			, &States[S_HORNRODATK1+1]),
	S_NORMAL (HROD, 'B',	4, A_FireSkullRodPL1			, &States[S_HORNRODATK1+2]),
	S_NORMAL (HROD, 'B',	0, A_ReFire 					, &States[S_HORNRODREADY]),

#define S_HORNRODATK2 (S_HORNRODATK1+3)
	S_NORMAL (HROD, 'C',	2, NULL 						, &States[S_HORNRODATK2+1]),
	S_NORMAL (HROD, 'D',	3, NULL 						, &States[S_HORNRODATK2+2]),
	S_NORMAL (HROD, 'E',	2, NULL 						, &States[S_HORNRODATK2+3]),
	S_NORMAL (HROD, 'F',	3, NULL 						, &States[S_HORNRODATK2+4]),
	S_NORMAL (HROD, 'G',	4, A_FireSkullRodPL2			, &States[S_HORNRODATK2+5]),
	S_NORMAL (HROD, 'F',	2, NULL 						, &States[S_HORNRODATK2+6]),
	S_NORMAL (HROD, 'E',	3, NULL 						, &States[S_HORNRODATK2+7]),
	S_NORMAL (HROD, 'D',	2, NULL 						, &States[S_HORNRODATK2+8]),
	S_NORMAL (HROD, 'C',	2, A_ReFire 					, &States[S_HORNRODREADY])
};

FWeaponInfo ASkullRod::WeaponInfo1 =
{
	0,
	am_skullrod,
	USE_SKRD_AMMO_1,
	50,
	&States[S_HORNRODUP],
	&States[S_HORNRODDOWN],
	&States[S_HORNRODREADY],
	&States[S_HORNRODATK1],
	&States[S_HORNRODATK1],
	NULL,
	RUNTIME_CLASS(ASkullRod),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

FWeaponInfo ASkullRod::WeaponInfo2 =
{
	0,
	am_skullrod,
	USE_SKRD_AMMO_2,
	50,
	&States[S_HORNRODUP],
	&States[S_HORNRODDOWN],
	&States[S_HORNRODREADY],
	&States[S_HORNRODATK2],
	&States[S_HORNRODATK2],
	NULL,
	RUNTIME_CLASS(ASkullRod),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

void ASkullRod::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2004;
	info->spawnstate = &States[S_WSKL];
	info->flags = MF_SPECIAL;
	WeaponSlots[5].AddWeapon (wp_skullrod, 6);
	wpnlev1info[wp_skullrod] = &WeaponInfo1;
	wpnlev2info[wp_skullrod] = &WeaponInfo2;
}

// Horn Rod FX 1 ------------------------------------------------------------

class AHornRodFX1 : public AActor
{
	DECLARE_ACTOR (AHornRodFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (AHornRodFX1, AActor);
REGISTER_ACTOR (AHornRodFX1, Heretic);

FState AHornRodFX1::States[] =
{
#define S_HRODFX1 0
	S_BRIGHT (FX00, 'A',	6, NULL 					, &States[S_HRODFX1+1]),
	S_BRIGHT (FX00, 'B',	6, NULL 					, &States[S_HRODFX1+0]),

#define S_HRODFXI1 (S_HRODFX1+2)
	S_BRIGHT (FX00, 'H',	5, NULL 					, &States[S_HRODFXI1+1]),
	S_BRIGHT (FX00, 'I',	5, NULL 					, &States[S_HRODFXI1+2]),
	S_BRIGHT (FX00, 'J',	4, NULL 					, &States[S_HRODFXI1+3]),
	S_BRIGHT (FX00, 'K',	4, NULL 					, &States[S_HRODFXI1+4]),
	S_BRIGHT (FX00, 'L',	3, NULL 					, &States[S_HRODFXI1+5]),
	S_BRIGHT (FX00, 'M',	3, NULL 					, NULL)
};

void AHornRodFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_HRODFX1];
	info->seesound = "weapons/hornrodshoot";
	info->deathstate = &States[S_HRODFXI1];
	info->deathsound = "weapons/hornrodhit";
	info->speed = 22 * FRACUNIT;
	info->radius = 12 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 3;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_WINDTHRUST|MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

// Horn Rod FX 2 ------------------------------------------------------------

class AHornRodFX2 : public AActor
{
	DECLARE_ACTOR (AHornRodFX2, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (AHornRodFX2, AActor);
REGISTER_ACTOR (AHornRodFX2, Heretic);

FState AHornRodFX2::States[] =
{
#define S_HRODFX2 0
	S_BRIGHT (FX00, 'C',	3, NULL 					, &States[S_HRODFX2+1]),
	S_BRIGHT (FX00, 'D',	3, A_SkullRodPL2Seek		, &States[S_HRODFX2+2]),
	S_BRIGHT (FX00, 'E',	3, NULL 					, &States[S_HRODFX2+3]),
	S_BRIGHT (FX00, 'F',	3, A_SkullRodPL2Seek		, &States[S_HRODFX2+0]),

#define S_HRODFXI2 (S_HRODFX2+4)
	S_BRIGHT (FX00, 'H',	5, A_AddPlayerRain			, &States[S_HRODFXI2+1]),
	S_BRIGHT (FX00, 'I',	5, NULL 					, &States[S_HRODFXI2+2]),
	S_BRIGHT (FX00, 'J',	4, NULL 					, &States[S_HRODFXI2+3]),
	S_BRIGHT (FX00, 'K',	3, NULL 					, &States[S_HRODFXI2+4]),
	S_BRIGHT (FX00, 'L',	3, NULL 					, &States[S_HRODFXI2+5]),
	S_BRIGHT (FX00, 'M',	3, NULL 					, &States[S_HRODFXI2+6]),
	S_NORMAL (FX00, 'G',	1, A_HideInCeiling			, &States[S_HRODFXI2+7]),
	S_NORMAL (FX00, 'G',	1, A_SkullRodStorm			, &States[S_HRODFXI2+7])
};

void AHornRodFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_HRODFX2];
	info->spawnhealth = 4*35;
	info->deathstate = &States[S_HRODFXI2];
	info->speed = 22 * FRACUNIT;
	info->radius = 12 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 10;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT;
}

int AHornRodFX2::DoSpecialDamage (AActor *target, int damage)
{
	if (target->IsKindOf (RUNTIME_CLASS (ASorcerer2)) && P_Random() < 96)
	{ // D'Sparil teleports away
		P_DSparilTeleport (target);
		return -1;
	}
	return damage;
}

// Rain pillar 1 ------------------------------------------------------------

class ARainPillar : public AActor
{
	DECLARE_ACTOR (ARainPillar, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (ARainPillar, AActor);
REGISTER_ACTOR (ARainPillar, Heretic);

FState ARainPillar::States[] =
{
#define S_RAINPLR 0
	S_BRIGHT (FX22, 'A',   -1, NULL 					, NULL),

#define S_RAINPLRX (S_RAINPLR+1)
	S_BRIGHT (FX22, 'B',	4, A_RainImpact 			, &States[S_RAINPLRX+1]),
	S_BRIGHT (FX22, 'C',	4, NULL 					, &States[S_RAINPLRX+2]),
	S_BRIGHT (FX22, 'D',	4, NULL 					, &States[S_RAINPLRX+3]),
	S_BRIGHT (FX22, 'E',	4, NULL 					, &States[S_RAINPLRX+4]),
	S_BRIGHT (FX22, 'F',	4, NULL 					, NULL),

#define S_RAINAIRXPLR (S_RAINPLRX+5)
	S_BRIGHT (FX22, 'G',	4, NULL 					, &States[S_RAINAIRXPLR+1]),
	S_BRIGHT (FX22, 'H',	4, NULL 					, &States[S_RAINAIRXPLR+2]),
	S_BRIGHT (FX22, 'I',	4, NULL 					, NULL),
};

void ARainPillar::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_RAINPLR];
	info->deathstate = &States[S_RAINPLRX];
	info->speed = 12 * FRACUNIT;
	info->radius = 5 * FRACUNIT;
	info->height = 12 * FRACUNIT;
	info->damage = 5;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
}

int ARainPillar::DoSpecialDamage (AActor *target, int damage)
{
	if (target->flags2 & MF2_BOSS)
	{ // Decrease damage for bosses
		damage = (P_Random() & 7) + 1;
	}
	return damage;
}

//----------------------------------------------------------------------------
//
// PROC A_FireSkullRodPL1
//
//----------------------------------------------------------------------------

void A_FireSkullRodPL1 (player_t *player, pspdef_t *psp)
{
	AActor *mo;

	if (player->ammo[am_skullrod] < USE_SKRD_AMMO_1)
	{
		return;
	}
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_skullrod] -= USE_SKRD_AMMO_1;
	mo = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AHornRodFX1));
	// Randomize the first frame
	if (mo && P_Random() > 128)
	{
		mo->SetState (mo->state->nextstate);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FireSkullRodPL2
//
// The special2 field holds the player number that shot the rain missile.
// The special1 field holds the id of the rain sound.
//
//----------------------------------------------------------------------------

void A_FireSkullRodPL2 (player_t *player, pspdef_t *psp)
{
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_skullrod] -=
			deathmatch.value ? USE_SKRD_AMMO_1 : USE_SKRD_AMMO_2;
	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AHornRodFX2));
	// Use MissileActor instead of the return value from
	// P_SpawnPlayerMissile because we need to give info to the mobj
	// even if it exploded immediately.
	MissileActor->special2 = player - players;
	if (linetarget)
	{
		MissileActor->tracer = linetarget;
	}
	S_Sound (MissileActor, CHAN_BODY, "weapons/hornrodpowshoot", 1, ATTN_NORM);
}

//----------------------------------------------------------------------------
//
// PROC A_SkullRodPL2Seek
//
//----------------------------------------------------------------------------

void A_SkullRodPL2Seek (AActor *actor)
{
	P_SeekerMissile (actor, ANGLE_1*10, ANGLE_1*30);
}

//----------------------------------------------------------------------------
//
// PROC A_AddPlayerRain
//
//----------------------------------------------------------------------------

void A_AddPlayerRain (AActor *actor)
{
	player_t *player;

	if (!playeringame[actor->special2])
	{ // Player left the game
		return;
	}
	player = &players[actor->special2];
	if (player->health <= 0)
	{ // Player is dead
		return;
	}
	if (player->rain1 && player->rain2)
	{ // Terminate an active rain
		if (player->rain1->health < player->rain2->health)
		{
			if (player->rain1->health > 16)
			{
				player->rain1->health = 16;
			}
			player->rain1 = NULL;
		}
		else
		{
			if (player->rain2->health > 16)
			{
				player->rain2->health = 16;
			}
			player->rain2 = NULL;
		}
	}
	// Add rain mobj to list
	if (player->rain1)
	{
		player->rain2 = actor;
	}
	else
	{
		player->rain1 = actor;
	}
	actor->special1 = S_FindSound ("misc/rain");
}

//----------------------------------------------------------------------------
//
// PROC A_SkullRodStorm
//
//----------------------------------------------------------------------------

void A_SkullRodStorm (AActor *actor)
{
	fixed_t x;
	fixed_t y;
	AActor *mo;
	player_t *player;

	if (actor->health-- == 0)
	{
		S_StopSound (actor, CHAN_BODY);
		if (!playeringame[actor->special2])
		{ // Player left the game
			actor->Destroy ();
			return;
		}
		player = &players[actor->special2];
		/*
		if (player->health <= 0)
		{ // Player is dead
			return;
		}
		*/
		if (player->rain1 == actor)
		{
			player->rain1 = NULL;
		}
		else if (player->rain2 == actor)
		{
			player->rain2 = NULL;
		}
		actor->Destroy ();
		return;
	}
	if (P_Random() < 25)
	{ // Fudge rain frequency
		return;
	}
	x = actor->x + ((P_Random()&127) - 64) * FRACUNIT;
	y = actor->y + ((P_Random()&127) - 64) * FRACUNIT;
	mo = Spawn<ARainPillar> (x, y, ONCEILINGZ);
	mo->translation = multiplayer ?
		translationtables + actor->special2*2*256 + 256 : NULL;
	mo->target = actor->target;
	mo->momx = 1; // Force collision detection
	mo->momz = -GetInfo (mo)->speed;
	mo->special2 = actor->special2; // Transfer player number
	P_CheckMissileSpawn (mo);
	if (actor->special1 != -1 && !S_GetSoundPlayingInfo (actor, actor->special1))
	{
		S_LoopedSoundID (actor, CHAN_BODY, actor->special1, 1, ATTN_NORM);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_RainImpact
//
//----------------------------------------------------------------------------

void A_RainImpact (AActor *actor)
{
	if (actor->z > actor->floorz)
	{
		actor->SetState (&ARainPillar::States[S_RAINAIRXPLR]);
	}
	else if (P_Random() < 40)
	{
		P_HitFloor (actor);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_HideInCeiling
//
//----------------------------------------------------------------------------

void A_HideInCeiling (AActor *actor)
{
	actor->z = actor->ceilingz + 4*FRACUNIT;
}

// --- Phoenix Rod ----------------------------------------------------------

void A_FirePhoenixPL1 (player_t *, pspdef_t *);
void A_InitPhoenixPL2 (player_t *, pspdef_t *);
void A_FirePhoenixPL2 (player_t *, pspdef_t *);
void A_ShutdownPhoenixPL2 (player_t *, pspdef_t *);
void A_PhoenixPuff (AActor *);
void A_FlameEnd (AActor *);
void A_FloatPuff (AActor *);

// Wimpy ammo ---------------------------------------------------------------

class APhoenixRodWimpy : public APickup
{
	DECLARE_ACTOR (APhoenixRodWimpy, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_phoenixrod, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOPHOENIXROD1;
	}
};

IMPLEMENT_DEF_SERIAL (APhoenixRodWimpy, APickup);
REGISTER_ACTOR (APhoenixRodWimpy, Heretic);

FState APhoenixRodWimpy::States[] =
{
	S_NORMAL (AMP1, 'A',	4, NULL 					, &States[1]),
	S_NORMAL (AMP1, 'B',	4, NULL 					, &States[2]),
	S_NORMAL (AMP1, 'C',	4, NULL 					, &States[0])
};

void APhoenixRodWimpy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 22;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_PHRD_WIMPY;
	info->flags = MF_SPECIAL;
	AmmoPics[am_phoenixrod] = "INAMPNX";
}

// Hefty ammo ---------------------------------------------------------------

class APhoenixRodHefty : public APickup
{
	DECLARE_ACTOR (APhoenixRodHefty, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_phoenixrod, health);
	}
	const char *PickupMessage ()
	{
		return TXT_AMMOPHOENIXROD2;
	}
};

IMPLEMENT_DEF_SERIAL (APhoenixRodHefty, APickup);
REGISTER_ACTOR (APhoenixRodHefty, Heretic);

FState APhoenixRodHefty::States[] =
{
	S_NORMAL (AMP2, 'A',	4, NULL 					, &States[1]),
	S_NORMAL (AMP2, 'B',	4, NULL 					, &States[2]),
	S_NORMAL (AMP2, 'C',	4, NULL 					, &States[0])
};

void APhoenixRodHefty::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 23;
	info->spawnstate = &States[0];
	info->spawnhealth = AMMO_PHRD_HEFTY;
	info->flags = MF_SPECIAL;
}

// Phoenix Rod --------------------------------------------------------------

class APhoenixRod : public AHereticWeapon
{
	DECLARE_ACTOR (APhoenixRod, AHereticWeapon);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GiveWeapon (toucher->player, wp_phoenixrod, flags & MF_DROPPED);
	}
	const char *PickupMessage ()
	{
		return TXT_WPNPHOENIXROD;
	}
private:
	static FWeaponInfo WeaponInfo1, WeaponInfo2;
};

IMPLEMENT_DEF_SERIAL (APhoenixRod, AHereticWeapon);
REGISTER_ACTOR (APhoenixRod, Heretic);

FState APhoenixRod::States[] =
{
#define S_WPHX 0
	S_NORMAL (WPHX, 'A',   -1, NULL 						, NULL),

#define S_PHOENIXREADY (S_WPHX+1)
	S_NORMAL (PHNX, 'A',	1, A_WeaponReady				, &States[S_PHOENIXREADY]),

#define S_PHOENIXDOWN (S_PHOENIXREADY+1)
	S_NORMAL (PHNX, 'A',	1, A_Lower						, &States[S_PHOENIXDOWN]),

#define S_PHOENIXUP (S_PHOENIXDOWN+1)
	S_NORMAL (PHNX, 'A',	1, A_Raise						, &States[S_PHOENIXUP]),

#define S_PHOENIXATK1 (S_PHOENIXUP+1)
	S_NORMAL (PHNX, 'B',	5, NULL 						, &States[S_PHOENIXATK1+1]),
	S_NORMAL (PHNX, 'C',	7, A_FirePhoenixPL1 			, &States[S_PHOENIXATK1+2]),
	S_NORMAL (PHNX, 'D',	4, NULL 						, &States[S_PHOENIXATK1+3]),
	S_NORMAL (PHNX, 'B',	4, NULL 						, &States[S_PHOENIXATK1+4]),
	S_NORMAL (PHNX, 'B',	0, A_ReFire 					, &States[S_PHOENIXREADY]),

#define S_PHOENIXATK2 (S_PHOENIXATK1+5)
	S_NORMAL (PHNX, 'B',	3, A_InitPhoenixPL2 			, &States[S_PHOENIXATK2+1]),
	S_BRIGHT (PHNX, 'C',	1, A_FirePhoenixPL2 			, &States[S_PHOENIXATK2+2]),
	S_NORMAL (PHNX, 'B',	4, A_ReFire 					, &States[S_PHOENIXATK2+3]),
	S_NORMAL (PHNX, 'B',	4, A_ShutdownPhoenixPL2 		, &States[S_PHOENIXREADY])
};

FWeaponInfo APhoenixRod::WeaponInfo1 =
{
	WIF_NOAUTOFIRE,
	am_phoenixrod,
	USE_PHRD_AMMO_1,
	2,
	&States[S_PHOENIXUP],
	&States[S_PHOENIXDOWN],
	&States[S_PHOENIXREADY],
	&States[S_PHOENIXATK1],
	&States[S_PHOENIXATK1],
	NULL,
	RUNTIME_CLASS(APhoenixRod),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

FWeaponInfo APhoenixRod::WeaponInfo2 =
{
	WIF_NOAUTOFIRE,
	am_phoenixrod,
	USE_PHRD_AMMO_2,
	2,
	&States[S_PHOENIXUP],
	&States[S_PHOENIXDOWN],
	&States[S_PHOENIXREADY],
	&States[S_PHOENIXATK2],
	&States[S_PHOENIXATK2+1],
	NULL,
	RUNTIME_CLASS(APhoenixRod),
	150,
	15*FRACUNIT,
	NULL,
	NULL
};

void APhoenixRod::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2003;
	info->spawnstate = &States[S_WPHX];
	info->flags = MF_SPECIAL;
	WeaponSlots[6].AddWeapon (wp_phoenixrod, 7);
	wpnlev1info[wp_phoenixrod] = &WeaponInfo1;
	wpnlev2info[wp_phoenixrod] = &WeaponInfo2;
}

// Phoenix FX 1 -------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (APhoenixFX1, AActor);
REGISTER_ACTOR (APhoenixFX1, Heretic);

FState APhoenixFX1::States[] =
{
#define S_PHOENIXFX1 0
	S_BRIGHT (FX04, 'A',	4, A_PhoenixPuff			, &States[S_PHOENIXFX1+0]),

#define S_PHOENIXFXI1 (S_PHOENIXFX1+1)
	S_BRIGHT (FX08, 'A',	6, A_Explode				, &States[S_PHOENIXFXI1+1]),
	S_BRIGHT (FX08, 'B',	5, NULL 					, &States[S_PHOENIXFXI1+2]),
	S_BRIGHT (FX08, 'C',	5, NULL 					, &States[S_PHOENIXFXI1+3]),
	S_BRIGHT (FX08, 'D',	4, NULL 					, &States[S_PHOENIXFXI1+4]),
	S_BRIGHT (FX08, 'E',	4, NULL 					, &States[S_PHOENIXFXI1+5]),
	S_BRIGHT (FX08, 'F',	4, NULL 					, &States[S_PHOENIXFXI1+6]),
	S_BRIGHT (FX08, 'G',	4, NULL 					, &States[S_PHOENIXFXI1+7]),
	S_BRIGHT (FX08, 'H',	4, NULL 					, NULL)
};

void APhoenixFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_PHOENIXFX1];
	info->seesound = "weapons/phoenixshoot";
	info->deathstate = &States[S_PHOENIXFXI1];
	info->deathsound = "weapons/phoenixhit";
	info->speed = 20 * FRACUNIT;
	info->radius = 11 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 20;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_THRUGHOST|MF2_NOTELEPORT|MF2_PCROSS|MF2_IMPACT|MF2_FIREDAMAGE;
}

int APhoenixFX1::DoSpecialDamage (AActor *target, int damage)
{
	if (target->IsKindOf (RUNTIME_CLASS (ASorcerer2)) && P_Random() < 96)
	{ // D'Sparil teleports away
		P_DSparilTeleport (target);
		return -1;
	}
	return damage;
}

// Phoenix puff -------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (APhoenixPuff, AActor);
REGISTER_ACTOR (APhoenixPuff, Heretic);

FState APhoenixPuff::States[] =
{
	S_NORMAL (FX04, 'B',	4, NULL 					, &States[1]),
	S_NORMAL (FX04, 'C',	4, NULL 					, &States[2]),
	S_NORMAL (FX04, 'D',	4, NULL 					, &States[3]),
	S_NORMAL (FX04, 'E',	4, NULL 					, &States[4]),
	S_NORMAL (FX04, 'F',	4, NULL 					, NULL),
};

void APhoenixPuff::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_CANNOTPUSH;
	info->translucency = HR_SHADOW;
}

// Phoenix FX 2 -------------------------------------------------------------

class APhoenixFX2 : public AActor
{
	DECLARE_ACTOR (APhoenixFX2, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (APhoenixFX2, AActor);
REGISTER_ACTOR (APhoenixFX2, Heretic);

FState APhoenixFX2::States[] =
{
#define S_PHOENIXFX2 0
	S_BRIGHT (FX09, 'A',	2, NULL 					, &States[S_PHOENIXFX2+1]),
	S_BRIGHT (FX09, 'B',	2, NULL 					, &States[S_PHOENIXFX2+2]),
	S_BRIGHT (FX09, 'A',	2, NULL 					, &States[S_PHOENIXFX2+3]),
	S_BRIGHT (FX09, 'B',	2, NULL 					, &States[S_PHOENIXFX2+4]),
	S_BRIGHT (FX09, 'A',	2, NULL 					, &States[S_PHOENIXFX2+5]),
	S_BRIGHT (FX09, 'B',	2, A_FlameEnd				, &States[S_PHOENIXFX2+6]),
	S_BRIGHT (FX09, 'C',	2, NULL 					, &States[S_PHOENIXFX2+7]),
	S_BRIGHT (FX09, 'D',	2, NULL 					, &States[S_PHOENIXFX2+8]),
	S_BRIGHT (FX09, 'E',	2, NULL 					, &States[S_PHOENIXFX2+9]),
	S_BRIGHT (FX09, 'F',	2, NULL 					, NULL),

#define S_PHOENIXFXI2 (S_PHOENIXFX2+10)
	S_BRIGHT (FX09, 'G',	3, NULL 					, &States[S_PHOENIXFXI2+1]),
	S_BRIGHT (FX09, 'H',	3, A_FloatPuff				, &States[S_PHOENIXFXI2+2]),
	S_BRIGHT (FX09, 'I',	4, NULL 					, &States[S_PHOENIXFXI2+3]),
	S_BRIGHT (FX09, 'J',	5, NULL 					, &States[S_PHOENIXFXI2+4]),
	S_BRIGHT (FX09, 'K',	5, NULL 					, NULL)
};

void APhoenixFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_PHOENIXFX2];
	info->deathstate = &States[S_PHOENIXFXI2];
	info->speed = 10 * FRACUNIT;
	info->radius = 6 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_FIREDAMAGE|MF2_PCROSS|MF2_IMPACT;
}

int APhoenixFX2::DoSpecialDamage (AActor *target, int damage)
{
	if (target->player && P_Random () < 128)
	{ // Freeze player for a bit
		target->reactiontime += 4;
	}
	return damage;
}

//----------------------------------------------------------------------------
//
// PROC A_FirePhoenixPL1
//
//----------------------------------------------------------------------------

void A_FirePhoenixPL1 (player_t *player, pspdef_t *psp)
{
	angle_t angle;

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_1;
	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(APhoenixFX1));
	angle = player->mo->angle + ANG180;
	angle >>= ANGLETOFINESHIFT;
	player->mo->momx += FixedMul (4*FRACUNIT, finecosine[angle]);
	player->mo->momy += FixedMul (4*FRACUNIT, finesine[angle]);
}

//----------------------------------------------------------------------------
//
// PROC A_PhoenixPuff
//
//----------------------------------------------------------------------------

void A_PhoenixPuff (AActor *actor)
{
	AActor *puff;
	angle_t angle;

	//[RH] Heretic never sets the target for seeking
	//P_SeekerMissile (actor, ANGLE_1*5, ANGLE_1*10);
	puff = Spawn<APhoenixPuff> (actor->x, actor->y, actor->z);
	angle = actor->angle + ANG90;
	angle >>= ANGLETOFINESHIFT;
	puff->momx = FixedMul (FRACUNIT*13/10, finecosine[angle]);
	puff->momy = FixedMul (FRACUNIT*13/10, finesine[angle]);
	puff->momz = 0;
	puff = Spawn<APhoenixPuff> (actor->x, actor->y, actor->z);
	angle = actor->angle - ANG90;
	angle >>= ANGLETOFINESHIFT;
	puff->momx = FixedMul (FRACUNIT*13/10, finecosine[angle]);
	puff->momy = FixedMul (FRACUNIT*13/10, finesine[angle]);
	puff->momz = 0;
}

//----------------------------------------------------------------------------
//
// PROC A_InitPhoenixPL2
//
//----------------------------------------------------------------------------

void A_InitPhoenixPL2 (player_t *player, pspdef_t *psp)
{
	player->flamecount = FLAME_THROWER_TICS;
}

//----------------------------------------------------------------------------
//
// PROC A_FirePhoenixPL2
//
// Flame thrower effect.
//
//----------------------------------------------------------------------------

void A_FirePhoenixPL2 (player_t *player, pspdef_t *psp)
{
	AActor *mo;
	AActor *pmo;
	angle_t angle;
	fixed_t x, y, z;
	fixed_t slope;
	int soundid;

	soundid = S_FindSound ("weapons/phoenixpowshoot");

	if (--player->flamecount == 0)
	{ // Out of flame
		P_SetPsprite (player, ps_weapon, &APhoenixFX2::States[S_PHOENIXATK2+3]);
		player->refire = 0;
		S_StopSound (player->mo, CHAN_WEAPON);
		return;
	}
	pmo = player->mo;
	angle = pmo->angle;
	x = pmo->x + (PS_Random() << 9);
	y = pmo->y + (PS_Random() << 9);
	z = pmo->z + 26*FRACUNIT + finetangent[FINEANGLES/4-(pmo->pitch>>ANGLETOFINESHIFT)];
	z -= pmo->floorclip;
	slope = finetangent[FINEANGLES/4-(pmo->pitch>>ANGLETOFINESHIFT)] + (FRACUNIT/10);
	mo = Spawn<APhoenixFX2> (x, y, z);
	mo->target = pmo;
	mo->angle = angle;
	mo->momx = pmo->momx + FixedMul (GetInfo (mo)->speed,
		finecosine[angle>>ANGLETOFINESHIFT]);
	mo->momy = pmo->momy + FixedMul (GetInfo (mo)->speed,
		finesine[angle>>ANGLETOFINESHIFT]);
	mo->momz = FixedMul (GetInfo (mo)->speed, slope);
	if (!player->refire || !S_GetSoundPlayingInfo (pmo, soundid))
	{
		S_LoopedSoundID (pmo, CHAN_WEAPON, soundid, 1, ATTN_NORM);
	}	
	P_CheckMissileSpawn (mo);
}

//----------------------------------------------------------------------------
//
// PROC A_ShutdownPhoenixPL2
//
//----------------------------------------------------------------------------

void A_ShutdownPhoenixPL2 (player_t *player, pspdef_t *psp)
{
	S_StopSound (player->mo, CHAN_WEAPON);
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
}

//----------------------------------------------------------------------------
//
// PROC A_FlameEnd
//
//----------------------------------------------------------------------------

void A_FlameEnd (AActor *actor)
{
	actor->momz += FRACUNIT*3/2;
}

//----------------------------------------------------------------------------
//
// PROC A_FloatPuff
//
//----------------------------------------------------------------------------

void A_FloatPuff (AActor *puff)
{
	puff->momz += FRACUNIT*18/10;
}

// --- Bag of holding -------------------------------------------------------

class ABagOfHolding : public APickup
{
	DECLARE_ACTOR (ABagOfHolding, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		player_t *player = toucher->player;
		int i;

		if (!player->backpack)
		{
			for (i = 0; i < NUMAMMO; i++)
			{
				player->maxammo[i] *= 2;
			}
			player->backpack = true;
		}
		P_GiveAmmo(player, am_goldwand, AMMO_GWND_WIMPY);
		P_GiveAmmo(player, am_blaster, AMMO_BLSR_WIMPY);
		P_GiveAmmo(player, am_crossbow, AMMO_CBOW_WIMPY);
		P_GiveAmmo(player, am_skullrod, AMMO_SKRD_WIMPY);
		P_GiveAmmo(player, am_phoenixrod, AMMO_PHRD_WIMPY);
		return true;
	}
	const char *PickupMessage ()
	{
		return TXT_ITEMBAGOFHOLDING;
	}
};

IMPLEMENT_DEF_SERIAL (ABagOfHolding, APickup);
REGISTER_ACTOR (ABagOfHolding, Heretic);

FState ABagOfHolding::States[] =
{
	S_NORMAL (BAGH, 'A',   -1, NULL, NULL)
};

void ABagOfHolding::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
}
