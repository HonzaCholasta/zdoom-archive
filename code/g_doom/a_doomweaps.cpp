#include "actor.h"
#include "info.h"
#include "s_sound.h"
#include "m_random.h"
#include "a_pickups.h"
#include "a_doomglobal.h"
#include "d_player.h"
#include "p_pspr.h"
#include "p_local.h"
#include "p_inter.h"
#include "dstrings.h"
#include "p_effect.h"

/* ammo ********************************************************************/

// a big item has five clip loads.
int clipammo[NUMAMMO] =
{
	10,		// bullets
	4,		// shells
	20,		// cells
	1		// rockets
};

// Clip --------------------------------------------------------------------

class AClip : public APickup
{
	DECLARE_ACTOR (AClip, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		if (flags & MF_DROPPED)
			return P_GiveAmmo (toucher->player, am_clip, clipammo[am_clip]/2);
		else
			return P_GiveAmmo (toucher->player, am_clip, clipammo[am_clip]);
	}
	virtual const char *PickupMessage ()
	{
		return GOTCLIP;
	}
};

IMPLEMENT_DEF_SERIAL (AClip, APickup);
REGISTER_ACTOR (AClip, Doom);

FState AClip::States[] =
{
	S_NORMAL (CLIP, 'A',   -1, NULL 				, NULL)
};

void AClip::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2007;
	info->spawnid = 11;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	AmmoPics[am_clip] = "CLIPA0";
}

// Clip box ----------------------------------------------------------------

class AClipBox : public APickup
{
	DECLARE_ACTOR (AClipBox, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_clip, clipammo[am_clip]*5);
	}
	virtual const char *PickupMessage ()
	{
		return GOTCLIPBOX;
	}
};

IMPLEMENT_DEF_SERIAL (AClipBox, APickup);
REGISTER_ACTOR (AClipBox, Doom);

FState AClipBox::States[] =
{
	S_NORMAL (AMMO, 'A',   -1, NULL 				, NULL)
};

void AClipBox::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2048;
	info->spawnid = 139;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

// Rocket ------------------------------------------------------------------

class ARocketAmmo : public APickup
{
	DECLARE_ACTOR (ARocketAmmo, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_misl, clipammo[am_misl]);
	}
	virtual const char *PickupMessage ()
	{
		return GOTROCKET;
	}
};

IMPLEMENT_DEF_SERIAL (ARocketAmmo, APickup);
REGISTER_ACTOR (ARocketAmmo, Doom);

FState ARocketAmmo::States[] =
{
	S_NORMAL (ROCK, 'A',   -1, NULL 				, NULL)
};

void ARocketAmmo::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2010;
	info->spawnid = 140;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 26 * FRACUNIT;
	info->flags = MF_SPECIAL;
	AmmoPics[am_misl] = "ROCKA0";
}

// Rocket box --------------------------------------------------------------

class ARocketBox : public APickup
{
	DECLARE_ACTOR (ARocketBox, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_misl, clipammo[am_misl]*5);
	}
	virtual const char *PickupMessage ()
	{
		return GOTROCKBOX;
	}
};

IMPLEMENT_DEF_SERIAL (ARocketBox, APickup);
REGISTER_ACTOR (ARocketBox, Doom);

FState ARocketBox::States[] =
{
	S_NORMAL (BROK, 'A',   -1, NULL 				, NULL)
};

void ARocketBox::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2046;
	info->spawnid = 141;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

// Cell --------------------------------------------------------------------

class ACell : public APickup
{
	DECLARE_ACTOR (ACell, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_cell, clipammo[am_cell]);
	}
	virtual const char *PickupMessage ()
	{
		return GOTCELL;
	}
};

IMPLEMENT_DEF_SERIAL (ACell, APickup);
REGISTER_ACTOR (ACell, Doom);

FState ACell::States[] =
{
	S_NORMAL (CELL, 'A',   -1, NULL 				, NULL)
};

void ACell::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2047;
	info->spawnid = 75;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 10 * FRACUNIT;
	info->flags = MF_SPECIAL;
	AmmoPics[am_cell] = "CELLA0";
}

// Cell pack ---------------------------------------------------------------

class ACellPack : public APickup
{
	DECLARE_ACTOR (ACellPack, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_cell, clipammo[am_cell]*5);
	}
	virtual const char *PickupMessage ()
	{
		return GOTCELLBOX;
	}
};

IMPLEMENT_DEF_SERIAL (ACellPack, APickup);
REGISTER_ACTOR (ACellPack, Doom);

FState ACellPack::States[] =
{
	S_NORMAL (CELP, 'A',   -1, NULL 				, NULL)
};

void ACellPack::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 17;
	info->spawnid = 142;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 18 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

// Shells ------------------------------------------------------------------

class AShell : public APickup
{
	DECLARE_ACTOR (AShell, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_shell, clipammo[am_shell]);
	}
	virtual const char *PickupMessage ()
	{
		return GOTSHELLS;
	}
};

IMPLEMENT_DEF_SERIAL (AShell, APickup);
REGISTER_ACTOR (AShell, Doom);

FState AShell::States[] =
{
	S_NORMAL (SHEL, 'A',   -1, NULL 				, NULL)
};

void AShell::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2008;
	info->spawnid = 12;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->flags = MF_SPECIAL;
	AmmoPics[am_shell] = "SHELA0";
}

// Shell box ---------------------------------------------------------------

class AShellBox : public APickup
{
	DECLARE_ACTOR (AShellBox, APickup);
protected:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_shell, clipammo[am_shell]*5);
	}
	virtual const char *PickupMessage ()
	{
		return GOTSHELLBOX;
	}
};

IMPLEMENT_DEF_SERIAL (AShellBox, APickup);
REGISTER_ACTOR (AShellBox, Doom);

FState AShellBox::States[] =
{
	S_NORMAL (SBOX, 'A',   -1, NULL 				, NULL)
};

void AShellBox::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2049;
	info->spawnid = 143;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 10 * FRACUNIT;
	info->flags = MF_SPECIAL;
}

/* the weapons that use the ammo above *************************************/

// Fist ---------------------------------------------------------------------

void A_Punch (player_t *, pspdef_t *);

class AFist : public AWeapon
{
	DECLARE_ACTOR (AFist, AWeapon);
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (AFist, AWeapon);
REGISTER_ACTOR (AFist, Doom);

FState AFist::States[] =
{
#define S_PUNCH 0
	S_NORMAL (PUNG, 'A',	1, A_WeaponReady		, &States[S_PUNCH]),

#define S_PUNCHDOWN (S_PUNCH+1)
	S_NORMAL (PUNG, 'A',	1, A_Lower				, &States[S_PUNCHDOWN]),

#define S_PUNCHUP (S_PUNCHDOWN+1)
	S_NORMAL (PUNG, 'A',	1, A_Raise				, &States[S_PUNCHUP]),

#define S_PUNCH1 (S_PUNCHUP+1)
	S_NORMAL (PUNG, 'B',	4, NULL 				, &States[S_PUNCH1+1]),
	S_NORMAL (PUNG, 'C',	4, A_Punch				, &States[S_PUNCH1+2]),
	S_NORMAL (PUNG, 'D',	5, NULL 				, &States[S_PUNCH1+3]),
	S_NORMAL (PUNG, 'C',	4, NULL 				, &States[S_PUNCH1+4]),
	S_NORMAL (PUNG, 'B',	5, A_ReFire 			, &States[S_PUNCH])
};

FWeaponInfo AFist::WeaponInfo =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_PUNCHUP],
	&States[S_PUNCHDOWN],
	&States[S_PUNCH],
	&States[S_PUNCH1],
	&States[S_PUNCH1],
	NULL,
	NULL,
	100,
	0,
	NULL,
	NULL
};

void AFist::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	wpnlev1info[wp_fist] = wpnlev2info[wp_fist] = &WeaponInfo;
	WeaponSlots[1].AddWeapon (wp_fist, 1);
};

//
// A_Punch
//
void A_Punch (player_t *player, pspdef_t *psp)
{
	angle_t 	angle;
	int 		damage;
	int 		slope;
		
	damage = (P_Random (pr_punch)%10+1)<<1;

	if (player->powers[pw_strength])	
		damage *= 10;

	angle = player->mo->angle;

	angle += PS_Random (pr_punch) << 18;
	slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
	P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);

	// turn to face target
	if (linetarget)
	{
		S_Sound (player->mo, CHAN_WEAPON, "*fist", 1, ATTN_NORM);
		player->mo->angle = R_PointToAngle2 (player->mo->x,
											 player->mo->y,
											 linetarget->x,
											 linetarget->y);
	}
}

// Pistol -------------------------------------------------------------------

void A_FirePistol (player_t *, pspdef_t *);

class APistol : public AWeapon
{
	DECLARE_ACTOR (APistol, AWeapon);
private:
	static FWeaponInfo WeaponInfo;
};

FState APistol::States[] =
{
#define S_PISTOL 0
	S_NORMAL (PISG, 'A',	1, A_WeaponReady		, &States[S_PISTOL]),

#define S_PISTOLDOWN (S_PISTOL+1)
	S_NORMAL (PISG, 'A',	1, A_Lower				, &States[S_PISTOLDOWN]),

#define S_PISTOLUP (S_PISTOLDOWN+1)
	S_NORMAL (PISG, 'A',	1, A_Raise				, &States[S_PISTOLUP]),

#define S_PISTOL1 (S_PISTOLUP+1)
	S_NORMAL (PISG, 'A',	4, NULL 				, &States[S_PISTOL1+1]),
	S_NORMAL (PISG, 'B',	6, A_FirePistol 		, &States[S_PISTOL1+2]),
	S_NORMAL (PISG, 'C',	4, NULL 				, &States[S_PISTOL1+3]),
	S_NORMAL (PISG, 'B',	5, A_ReFire 			, &States[S_PISTOL]),

#define S_PISTOLFLASH (S_PISTOL1+4)
	S_BRIGHT (PISF, 'A',	7, A_Light1 			, &AWeapon::States[S_LIGHTDONE])
};

IMPLEMENT_DEF_SERIAL (APistol, AWeapon);
REGISTER_ACTOR (APistol, Doom);

FWeaponInfo APistol::WeaponInfo =
{
	0,
	am_clip,
	1,
	20,
	&States[S_PISTOLUP],
	&States[S_PISTOLDOWN],
	&States[S_PISTOL],
	&States[S_PISTOL1],
	&States[S_PISTOL1],
	&States[S_PISTOLFLASH],
	RUNTIME_CLASS(AClip),
	100,
	0,
	NULL,
	NULL
};

void APistol::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	wpnlev1info[wp_pistol] = wpnlev2info[wp_pistol] = &WeaponInfo;
	WeaponSlots[2].AddWeapon (wp_pistol, 3);
}

//
// A_FirePistol
//
void A_FirePistol (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/pistol", 1, ATTN_NORM);

	player->mo->PlayAttacking2 ();
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo]--;

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);
	P_GunShot (player->mo, !player->refire);
}

// Chainsaw -----------------------------------------------------------------

void A_Saw (player_t *, pspdef_t *);

class AChainsaw : public AWeapon
{
	DECLARE_ACTOR (AChainsaw, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (AChainsaw, AWeapon);
REGISTER_ACTOR (AChainsaw, Doom);

FState AChainsaw::States[] =
{
#define S_SAW 0
	S_NORMAL (SAWG, 'C',	4, A_WeaponReady		, &States[S_SAW+1]),
	S_NORMAL (SAWG, 'D',	4, A_WeaponReady		, &States[S_SAW+0]),

#define S_SAWDOWN (S_SAW+2)
	S_NORMAL (SAWG, 'C',	1, A_Lower				, &States[S_SAWDOWN]),

#define S_SAWUP (S_SAWDOWN+1)
	S_NORMAL (SAWG, 'C',	1, A_Raise				, &States[S_SAWUP]),

#define S_SAW1 (S_SAWUP+1)
	S_NORMAL (SAWG, 'A',	4, A_Saw				, &States[S_SAW1+1]),
	S_NORMAL (SAWG, 'B',	4, A_Saw				, &States[S_SAW1+2]),
	S_NORMAL (SAWG, 'B',	0, A_ReFire 			, &States[S_SAW]),

#define S_CSAW (S_SAW1+3)
	S_NORMAL (CSAW, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo AChainsaw::WeaponInfo =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_SAWUP],
	&States[S_SAWDOWN],
	&States[S_SAW],
	&States[S_SAW1],
	&States[S_SAW1],
	NULL,
	RUNTIME_CLASS(AChainsaw),
	0,
	0,
	"weapons/sawup",
	"weapons/sawidle"
};

void AChainsaw::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2005;
	info->spawnid = 32;
	info->spawnstate = &States[S_CSAW];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_chainsaw] = wpnlev2info[wp_chainsaw] = &WeaponInfo;
	WeaponSlots[1].AddWeapon (wp_chainsaw, 2);
}

bool AChainsaw::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_chainsaw, flags & MF_DROPPED);
}

const char *AChainsaw::PickupMessage ()
{
	return GOTCHAINSAW;
}

//
// A_Saw
//
void A_Saw (player_t *player, pspdef_t *psp)
{
	angle_t 	angle;
	int 		damage;

	damage = 2 * (P_Random (pr_saw)%10+1);
	angle = player->mo->angle;
	angle += PS_Random (pr_saw) << 18;
	
	// use meleerange + 1 so the puff doesn't skip the flash
	P_LineAttack (player->mo, angle, MELEERANGE+1,
				  P_AimLineAttack (player->mo, angle, MELEERANGE+1), damage);

	if (!linetarget)
	{
		S_Sound (player->mo, CHAN_WEAPON, "weapons/sawfull", 1, ATTN_NORM);
		return;
	}
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sawhit", 1, ATTN_NORM);
		
	// turn to face target
	angle = R_PointToAngle2 (player->mo->x, player->mo->y,
							 linetarget->x, linetarget->y);
	if (angle - player->mo->angle > ANG180)
	{
		if (angle - player->mo->angle < (angle_t)(-ANG90/20))
			player->mo->angle = angle + ANG90/21;
		else
			player->mo->angle -= ANG90/20;
	}
	else
	{
		if (angle - player->mo->angle > ANG90/20)
			player->mo->angle = angle - ANG90/21;
		else
			player->mo->angle += ANG90/20;
	}
	player->mo->flags |= MF_JUSTATTACKED;
}

// Shotgun ------------------------------------------------------------------

void A_FireShotgun (player_t *, pspdef_t *);

class AShotgun : public AWeapon
{
	DECLARE_ACTOR (AShotgun, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (AShotgun, AWeapon);
REGISTER_ACTOR (AShotgun, Doom);

FState AShotgun::States[] =
{
#define S_SGUN 0
	S_NORMAL (SHTG, 'A',	1, A_WeaponReady		, &States[S_SGUN]),

#define S_SGUNDOWN (S_SGUN+1)
	S_NORMAL (SHTG, 'A',	1, A_Lower				, &States[S_SGUNDOWN]),

#define S_SGUNUP (S_SGUNDOWN+1)
	S_NORMAL (SHTG, 'A',	1, A_Raise				, &States[S_SGUNUP]),

#define S_SGUN1 (S_SGUNUP+1)
	S_NORMAL (SHTG, 'A',	3, NULL 				, &States[S_SGUN1+1]),
	S_NORMAL (SHTG, 'A',	7, A_FireShotgun		, &States[S_SGUN1+2]),
	S_NORMAL (SHTG, 'B',	5, NULL 				, &States[S_SGUN1+3]),
	S_NORMAL (SHTG, 'C',	5, NULL 				, &States[S_SGUN1+4]),
	S_NORMAL (SHTG, 'D',	4, NULL 				, &States[S_SGUN1+5]),
	S_NORMAL (SHTG, 'C',	5, NULL 				, &States[S_SGUN1+6]),
	S_NORMAL (SHTG, 'B',	5, NULL 				, &States[S_SGUN1+7]),
	S_NORMAL (SHTG, 'A',	3, NULL 				, &States[S_SGUN1+8]),
	S_NORMAL (SHTG, 'A',	7, A_ReFire 			, &States[S_SGUN]),

#define S_SGUNFLASH (S_SGUN1+9)
	S_BRIGHT (SHTF, 'A',	4, A_Light1 			, &States[S_SGUNFLASH+1]),
	S_BRIGHT (SHTF, 'B',	3, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

#define S_SHOT (S_SGUNFLASH+2)
	S_NORMAL (SHOT, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo AShotgun::WeaponInfo =
{
	0,
	am_shell,
	1,
	8,
	&States[S_SGUNUP],
	&States[S_SGUNDOWN],
	&States[S_SGUN],
	&States[S_SGUN1],
	&States[S_SGUN1],
	&States[S_SGUNFLASH],
	RUNTIME_CLASS(AShotgun),
	100,
	0,
	NULL,
	NULL
};

void AShotgun::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2001;
	info->spawnid = 27;
	info->spawnstate = &States[S_SHOT];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_shotgun] = wpnlev2info[wp_shotgun] = &WeaponInfo;
	WeaponSlots[3].AddWeapon (wp_shotgun, 4);
}

bool AShotgun::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_shotgun, flags & MF_DROPPED);
}

const char *AShotgun::PickupMessage ()
{
	return GOTSHOTGUN;
}

//
// A_FireShotgun
//
void A_FireShotgun (player_t *player, pspdef_t *psp)
{
	int i;
		
	S_Sound (player->mo, CHAN_WEAPON,  "weapons/shotgf", 1, ATTN_NORM);
	player->mo->PlayAttacking2 ();

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo]--;

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);
		
	for (i=0 ; i<7 ; i++)
		P_GunShot (player->mo, false);
}

// Super Shotgun ------------------------------------------------------------

void A_FireShotgun2 (player_t *, pspdef_t *);
void A_CheckReload (player_t *, pspdef_t *);
void A_OpenShotgun2 (player_t *, pspdef_t *);
void A_LoadShotgun2 (player_t *, pspdef_t *);
void A_CloseShotgun2 (player_t *, pspdef_t *);

class ASuperShotgun : public AWeapon
{
	DECLARE_ACTOR (ASuperShotgun, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (ASuperShotgun, AWeapon);
REGISTER_ACTOR (ASuperShotgun, Doom);

FState ASuperShotgun::States[] =
{
#define S_DSGUN 0
	S_NORMAL (SHT2, 'A',	1, A_WeaponReady		, &States[S_DSGUN]),

#define S_DSGUNDOWN (S_DSGUN+1)
	S_NORMAL (SHT2, 'A',	1, A_Lower				, &States[S_DSGUNDOWN]),

#define S_DSGUNUP (S_DSGUNDOWN+1)
	S_NORMAL (SHT2, 'A',	1, A_Raise				, &States[S_DSGUNUP]),

#define S_DSGUN1 (S_DSGUNUP+1)
	S_NORMAL (SHT2, 'A',	3, NULL 				, &States[S_DSGUN1+1]),
	S_NORMAL (SHT2, 'A',	7, A_FireShotgun2		, &States[S_DSGUN1+2]),
	S_NORMAL (SHT2, 'B',	7, NULL 				, &States[S_DSGUN1+3]),
	S_NORMAL (SHT2, 'C',	7, A_CheckReload		, &States[S_DSGUN1+4]),
	S_NORMAL (SHT2, 'D',	7, A_OpenShotgun2		, &States[S_DSGUN1+5]),
	S_NORMAL (SHT2, 'E',	7, NULL 				, &States[S_DSGUN1+6]),
	S_NORMAL (SHT2, 'F',	7, A_LoadShotgun2		, &States[S_DSGUN1+7]),
	S_NORMAL (SHT2, 'G',	6, NULL 				, &States[S_DSGUN1+8]),
	S_NORMAL (SHT2, 'H',	6, A_CloseShotgun2		, &States[S_DSGUN1+9]),
	S_NORMAL (SHT2, 'A',	5, A_ReFire 			, &States[S_DSGUN]),

#define S_DSNR (S_DSGUN1+10)
	S_NORMAL (SHT2, 'B',	7, NULL 				, &States[S_DSNR+1]),
	S_NORMAL (SHT2, 'A',	3, NULL 				, &States[S_DSGUNDOWN]),

#define S_DSGUNFLASH (S_DSNR+2)
	S_BRIGHT (SHT2, 'I',	5, A_Light1 			, &States[S_DSGUNFLASH+1]),
	S_BRIGHT (SHT2, 'J',	4, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

#define S_SHOT2 (S_DSGUNFLASH+2)
	S_NORMAL (SGN2, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ASuperShotgun::WeaponInfo =
{
	0,
	am_shell,
	2,
	8,
	&States[S_DSGUNUP],
	&States[S_DSGUNDOWN],
	&States[S_DSGUN],
	&States[S_DSGUN1],
	&States[S_DSGUN1],
	&States[S_DSGUNFLASH],
	RUNTIME_CLASS(ASuperShotgun),
	100,
	0,
	NULL,
	NULL
};

void ASuperShotgun::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 82;
	info->spawnid = 33;
	info->spawnstate = &States[S_SHOT2];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_supershotgun] = wpnlev2info[wp_supershotgun] = &WeaponInfo;
	WeaponSlots[3].AddWeapon (wp_supershotgun, 5);
}

bool ASuperShotgun::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_supershotgun, flags & MF_DROPPED);
}

const char *ASuperShotgun::PickupMessage ()
{
	return GOTSHOTGUN2;
}

//
// A_FireShotgun2
//
void A_FireShotgun2 (player_t *player, pspdef_t *psp)
{
	int 		i;
	angle_t 	angle;
	int 		damage;
				
		
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sshotf", 1, ATTN_NORM);
	player->mo->PlayAttacking2 ();

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo]-=2;

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);
		
	for (i=0 ; i<20 ; i++)
	{
		damage = 5*(P_Random (pr_fireshotgun2)%3+1);
		angle = player->mo->angle;
		angle += PS_Random (pr_fireshotgun2) << 19;
		P_LineAttack (player->mo,
					  angle,
					  MISSILERANGE,
					  bulletslope + (PS_Random (pr_fireshotgun2) << 5), damage);
	}
}

void A_CheckReload (player_t *player, pspdef_t *psp)
{
	P_CheckAmmo (player);
}

void A_OpenShotgun2 (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sshoto", 1, ATTN_NORM);
}

void A_LoadShotgun2 (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sshotl", 1, ATTN_NORM);
}

void A_CloseShotgun2 (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sshotc", 1, ATTN_NORM);
	A_ReFire(player,psp);
}

// Chaingun -----------------------------------------------------------------

void A_FireCGun (player_t *, pspdef_t *);

class AChaingun : public AWeapon
{
	DECLARE_ACTOR (AChaingun, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (AChaingun, AWeapon);
REGISTER_ACTOR (AChaingun, Doom);

FState AChaingun::States[] =
{
#define S_CHAIN 0
	S_NORMAL (CHGG, 'A',	1, A_WeaponReady		, &States[S_CHAIN]),

#define S_CHAINDOWN (S_CHAIN+1)
	S_NORMAL (CHGG, 'A',	1, A_Lower				, &States[S_CHAINDOWN]),

#define S_CHAINUP (S_CHAINDOWN+1)
	S_NORMAL (CHGG, 'A',	1, A_Raise				, &States[S_CHAINUP]),

#define S_CHAIN1 (S_CHAINUP+1)
	S_NORMAL (CHGG, 'A',	4, A_FireCGun			, &States[S_CHAIN1+1]),
	S_NORMAL (CHGG, 'B',	4, A_FireCGun			, &States[S_CHAIN1+2]),
	S_NORMAL (CHGG, 'B',	0, A_ReFire 			, &States[S_CHAIN]),

#define S_CHAINFLASH (S_CHAIN1+3)
	S_BRIGHT (CHGF, 'A',	5, A_Light1 			, &AWeapon::States[S_LIGHTDONE]),
	S_BRIGHT (CHGF, 'B',	5, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

#define S_MGUN (S_CHAINFLASH+2)
	S_NORMAL (MGUN, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo AChaingun::WeaponInfo =
{
	0,
	am_clip,
	1,
	20,
	&States[S_CHAINUP],
	&States[S_CHAINDOWN],
	&States[S_CHAIN],
	&States[S_CHAIN1],
	&States[S_CHAIN1],
	&States[S_CHAINFLASH],
	RUNTIME_CLASS(AChaingun),
	100,
	0,
	NULL,
	NULL
};

void AChaingun::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2002;
	info->spawnid = 28;
	info->spawnstate = &States[S_MGUN];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_chaingun] = wpnlev2info[wp_chaingun] = &WeaponInfo;
	WeaponSlots[4].AddWeapon (wp_chaingun, 6);
}

bool AChaingun::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_chaingun, flags & MF_DROPPED);
}

const char *AChaingun::PickupMessage ()
{
	return GOTCHAINGUN;
}

//
// A_FireCGun
//
void A_FireCGun (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/chngun", 1, ATTN_NORM);

	if (!player->ammo[wpnlev1info[player->readyweapon]->ammo])
		return;
				
	player->mo->PlayAttacking2 ();
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo]--;

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate
				  + (psp->state - wpnlev1info[player->readyweapon]->atkstate));

	P_BulletSlope (player->mo);
		
	P_GunShot (player->mo, !player->refire);
}

// Rocket launcher ---------------------------------------------------------

void A_FireMissile (player_t *, pspdef_t *);
void A_Explode (AActor *);

class ARocketLauncher : public AWeapon
{
	DECLARE_ACTOR (ARocketLauncher, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (ARocketLauncher, AWeapon);
REGISTER_ACTOR (ARocketLauncher, Doom);

FState ARocketLauncher::States[] =
{
#define S_MISSILE 0
	S_NORMAL (MISG, 'A',	1, A_WeaponReady		, &States[S_MISSILE]),

#define S_MISSILEDOWN (S_MISSILE+1)
	S_NORMAL (MISG, 'A',	1, A_Lower				, &States[S_MISSILEDOWN]),

#define S_MISSILEUP (S_MISSILEDOWN+1)
	S_NORMAL (MISG, 'A',	1, A_Raise				, &States[S_MISSILEUP]),

#define S_MISSILE1 (S_MISSILEUP+1)
	S_NORMAL (MISG, 'B',	8, A_GunFlash			, &States[S_MISSILE1+1]),
	S_NORMAL (MISG, 'B',   12, A_FireMissile		, &States[S_MISSILE1+2]),
	S_NORMAL (MISG, 'B',	0, A_ReFire 			, &States[S_MISSILE]),

#define S_MISSILEFLASH (S_MISSILE1+3)
	S_BRIGHT (MISF, 'A',	3, A_Light1 			, &States[S_MISSILEFLASH+1]),
	S_BRIGHT (MISF, 'B',	4, NULL 				, &States[S_MISSILEFLASH+2]),
	S_BRIGHT (MISF, 'C',	4, A_Light2 			, &States[S_MISSILEFLASH+3]),
	S_BRIGHT (MISF, 'D',	4, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

#define S_LAUN (S_MISSILEFLASH+4)
	S_NORMAL (LAUN, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ARocketLauncher::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_misl,
	1,
	2,
	&States[S_MISSILEUP],
	&States[S_MISSILEDOWN],
	&States[S_MISSILE],
	&States[S_MISSILE1],
	&States[S_MISSILE1],
	&States[S_MISSILEFLASH],
	RUNTIME_CLASS(ARocketLauncher),
	100,
	0,
	NULL,
	NULL
};

void ARocketLauncher::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2003;
	info->spawnid = 29;
	info->spawnstate = &States[S_LAUN];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_missile] = wpnlev2info[wp_missile] = &WeaponInfo;
	WeaponSlots[5].AddWeapon (wp_missile, 7);
}

bool ARocketLauncher::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_missile, flags & MF_DROPPED);
}

const char *ARocketLauncher::PickupMessage ()
{
	return GOTLAUNCHER;
}

IMPLEMENT_DEF_SERIAL (ARocket, AActor);
REGISTER_ACTOR (ARocket, Doom);

FState ARocket::States[] =
{
#define S_ROCKET 0
	S_BRIGHT (MISL, 'A',	1, NULL 						, &States[S_ROCKET]),

#define S_EXPLODE (S_ROCKET+1)
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[S_EXPLODE+1]),
	S_BRIGHT (MISL, 'C',	6, NULL 						, &States[S_EXPLODE+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

void ARocket::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 127;
	info->spawnstate = &States[S_ROCKET];
	info->seesound = "weapons/rocklf";
	info->deathstate = &States[S_EXPLODE];
	info->deathsound = "weapons/rocklx";
	info->speed = 20 * FRACUNIT;
	info->radius = 11 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 20;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
}

void ARocket::BeginPlay ()
{
	Super::BeginPlay ();
	effects |= FX_ROCKET;
}

//
// A_FireMissile
//
void A_FireMissile (player_t *player, pspdef_t *psp)
{
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo]--;
	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ARocket));
}

// Plasma rifle ------------------------------------------------------------

void A_FirePlasma (player_t *, pspdef_t *);

class APlasmaRifle : public AWeapon
{
	DECLARE_ACTOR (APlasmaRifle, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

IMPLEMENT_DEF_SERIAL (APlasmaRifle, AWeapon);
REGISTER_ACTOR (APlasmaRifle, Doom);

FState APlasmaRifle::States[] =
{
#define S_PLASMA 0
	S_NORMAL (PLSG, 'A',	1, A_WeaponReady		, &States[S_PLASMA]),

#define S_PLASMADOWN (S_PLASMA+1)
	S_NORMAL (PLSG, 'A',	1, A_Lower				, &States[S_PLASMADOWN]),

#define S_PLASMAUP (S_PLASMADOWN+1)
	S_NORMAL (PLSG, 'A',	1, A_Raise				, &States[S_PLASMAUP]),

#define S_PLASMA1 (S_PLASMAUP+1)
	S_NORMAL (PLSG, 'A',	3, A_FirePlasma 		, &States[S_PLASMA1+1]),
	S_NORMAL (PLSG, 'B',   20, A_ReFire 			, &States[S_PLASMA]),

#define S_PLASMAFLASH (S_PLASMA1+2)
	S_BRIGHT (PLSF, 'A',	4, A_Light1 			, &AWeapon::States[S_LIGHTDONE]),
	S_BRIGHT (PLSF, 'B',	4, A_Light1 			, &AWeapon::States[S_LIGHTDONE]),

#define S_PLAS (S_PLASMAFLASH+2)
	S_NORMAL (PLAS, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo APlasmaRifle::WeaponInfo =
{
	0,
	am_cell,
	1,
	40,
	&States[S_PLASMAUP],
	&States[S_PLASMADOWN],
	&States[S_PLASMA],
	&States[S_PLASMA1],
	&States[S_PLASMA1],
	&States[S_PLASMAFLASH],
	RUNTIME_CLASS(APlasmaRifle),
	100,
	0,
	NULL,
	NULL
};

void APlasmaRifle::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2004;
	info->spawnid = 30;
	info->spawnstate = &States[S_PLAS];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_plasma] = wpnlev2info[wp_plasma] = &WeaponInfo;
	WeaponSlots[6].AddWeapon (wp_plasma, 8);
}

bool APlasmaRifle::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_plasma, flags & MF_DROPPED);
}

const char *APlasmaRifle::PickupMessage ()
{
	return GOTPLASMA;
}

IMPLEMENT_DEF_SERIAL (APlasmaBall, AActor);
REGISTER_ACTOR (APlasmaBall, Doom);

FState APlasmaBall::States[] =
{
#define S_PLASBALL 0
	S_BRIGHT (PLSS, 'A',	6, NULL 						, &States[S_PLASBALL+1]),
	S_BRIGHT (PLSS, 'B',	6, NULL 						, &States[S_PLASBALL]),

#define S_PLASEXP (S_PLASBALL+2)
	S_BRIGHT (PLSE, 'A',	4, NULL 						, &States[S_PLASEXP+1]),
	S_BRIGHT (PLSE, 'B',	4, NULL 						, &States[S_PLASEXP+2]),
	S_BRIGHT (PLSE, 'C',	4, NULL 						, &States[S_PLASEXP+3]),
	S_BRIGHT (PLSE, 'D',	4, NULL 						, &States[S_PLASEXP+4]),
	S_BRIGHT (PLSE, 'E',	4, NULL 						, NULL)
};

void APlasmaBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 51;
	info->spawnstate = &States[S_PLASBALL];
	info->seesound = "weapons/plasmaf";
	info->deathstate = &States[S_PLASEXP];
	info->deathsound = "weapons/plasmax";
	info->speed = 25 * FRACUNIT;
	info->radius = 13 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 5;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC66;
}
//
// A_FirePlasma
//
void A_FirePlasma (player_t *player, pspdef_t *psp)
{
	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo]--;

	if (wpnlev1info[player->readyweapon]->flashstate)
	{
		P_SetPsprite (player,
					  ps_flash,
					  wpnlev1info[player->readyweapon]->flashstate
					   + (P_Random (pr_fireplasma)&1));
	}

	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(APlasmaBall));
}

//
// [RH] A_FireRailgun
//
static int RailOffset;

void A_FireRailgun (player_t *player, pspdef_t *psp)
{
	int damage;

	if (player->ammo[wpnlev1info[player->readyweapon]->ammo] < 10)
	{
		int ammo = player->ammo[wpnlev1info[player->readyweapon]->ammo];
		player->ammo[wpnlev1info[player->readyweapon]->ammo] = 0;
		P_CheckAmmo (player);
		player->ammo[wpnlev1info[player->readyweapon]->ammo] = ammo;
		return;
	}

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo] -= 10;

	if (wpnlev1info[player->readyweapon]->flashstate)
	{
		P_SetPsprite (player,
					  ps_flash,
					  wpnlev1info[player->readyweapon]->flashstate
					   + (P_Random (pr_fireplasma)&1));
	}

	if (deathmatch.value)
		damage = 100;
	else
		damage = 150;

	P_RailAttack (player->mo, damage, RailOffset);
	RailOffset = 0;
}

void A_FireRailgunRight (player_t *player, pspdef_t *psp)
{
	RailOffset = 10;
	A_FireRailgun (player, psp);
}

void A_FireRailgunLeft (player_t *player, pspdef_t *psp)
{
	RailOffset = -10;
	A_FireRailgun (player, psp);
}

void A_RailWait (player_t *player, pspdef_t *psp)
{
	// Okay, this was stupid. Just use a NULL function instead of this.
}

// BFG 9000 -----------------------------------------------------------------

void A_FireBFG (player_t *, pspdef_t *);
void A_BFGSpray (AActor *);
void A_BFGsound (player_t *, pspdef_t *);

class ABFG9000 : public AWeapon
{
	DECLARE_ACTOR (ABFG9000, AWeapon);
protected:
	bool TryPickup (AActor *toucher);
	const char *PickupMessage ();
private:
	static FWeaponInfo WeaponInfo;
};

class ABFGBall : public AActor
{
	DECLARE_ACTOR (ABFGBall, AActor);
public:
	int GetMOD () { return MOD_BFG_BOOM; }
};

class ABFGExtra : public AActor
{
	DECLARE_ACTOR (ABFGExtra, AActor);
};

IMPLEMENT_DEF_SERIAL (ABFG9000, AWeapon);
REGISTER_ACTOR (ABFG9000, Doom);

FState ABFG9000::States[] =
{
#define S_BFG 0
	S_NORMAL (BFGG, 'A',	1, A_WeaponReady		, &States[S_BFG]),

#define S_BFGDOWN (S_BFG+1)
	S_NORMAL (BFGG, 'A',	1, A_Lower				, &States[S_BFGDOWN]),

#define S_BFGUP (S_BFGDOWN+1)
	S_NORMAL (BFGG, 'A',	1, A_Raise				, &States[S_BFGUP]),

#define S_BFG1 (S_BFGUP+1)
	S_NORMAL (BFGG, 'A',   20, A_BFGsound			, &States[S_BFG1+1]),
	S_NORMAL (BFGG, 'B',   10, A_GunFlash			, &States[S_BFG1+2]),
	S_NORMAL (BFGG, 'B',   10, A_FireBFG			, &States[S_BFG1+3]),
	S_NORMAL (BFGG, 'B',   20, A_ReFire 			, &States[S_BFG]),

#define S_BFGFLASH (S_BFG1+4)
	S_BRIGHT (BFGF, 'A',   11, A_Light1 			, &States[S_BFGFLASH+1]),
	S_BRIGHT (BFGF, 'B',	6, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

#define S_BFUG (S_BFGFLASH+2)
	S_NORMAL (BFUG, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ABFG9000::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_cell,
	40,
	40,
	&States[S_BFGUP],
	&States[S_BFGDOWN],
	&States[S_BFG],
	&States[S_BFG1],
	&States[S_BFG1],
	&States[S_BFGFLASH],
	RUNTIME_CLASS(ABFG9000),
	100,
	0,
	NULL,
	NULL
};

void ABFG9000::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2006;
	info->spawnid = 31;
	info->spawnstate = &States[S_BFUG];
	info->radius = 20 * FRACUNIT;
	info->height = 20 * FRACUNIT;
	info->flags = MF_SPECIAL;
	wpnlev1info[wp_bfg] = wpnlev2info[wp_bfg] = &WeaponInfo;
	WeaponSlots[7].AddWeapon (wp_bfg, 9);
}

bool ABFG9000::TryPickup (AActor *toucher)
{
	return P_GiveWeapon (toucher->player, wp_bfg, flags & MF_DROPPED);
}

const char *ABFG9000::PickupMessage ()
{
	return GOTBFG9000;
}

IMPLEMENT_DEF_SERIAL (ABFGBall, AActor);
REGISTER_ACTOR (ABFGBall, Doom);

FState ABFGBall::States[] =
{
#define S_BFGSHOT 0
	S_BRIGHT (BFS1, 'A',	4, NULL 						, &States[S_BFGSHOT+1]),
	S_BRIGHT (BFS1, 'B',	4, NULL 						, &States[S_BFGSHOT]),

#define S_BFGLAND (S_BFGSHOT+2)
	S_BRIGHT (BFE1, 'A',	8, NULL 						, &States[S_BFGLAND+1]),
	S_BRIGHT (BFE1, 'B',	8, NULL 						, &States[S_BFGLAND+2]),
	S_BRIGHT (BFE1, 'C',	8, A_BFGSpray					, &States[S_BFGLAND+3]),
	S_BRIGHT (BFE1, 'D',	8, NULL 						, &States[S_BFGLAND+4]),
	S_BRIGHT (BFE1, 'E',	8, NULL 						, &States[S_BFGLAND+5]),
	S_BRIGHT (BFE1, 'F',	8, NULL 						, NULL)
};

void ABFGBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 128;
	info->spawnstate = &States[S_BFGSHOT];
	info->deathstate = &States[S_BFGLAND];
	info->deathsound = "weapons/bfgx";
	info->speed = 25 * FRACUNIT;
	info->radius = 13 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 100;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC66;
}

IMPLEMENT_DEF_SERIAL (ABFGExtra, AActor);
REGISTER_ACTOR (ABFGExtra, Doom);

FState ABFGExtra::States[] =
{
	S_BRIGHT (BFE2, 'A',	8, NULL 				, &States[1]),
	S_BRIGHT (BFE2, 'B',	8, NULL 				, &States[2]),
	S_BRIGHT (BFE2, 'C',	8, NULL 				, &States[3]),
	S_BRIGHT (BFE2, 'D',	8, NULL 				, NULL)
};

void ABFGExtra::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->translucency = TRANSLUC66;
}

//
// A_FireBFG
//
CVAR (nobfgaim, "0", CVAR_SERVERINFO)

void A_FireBFG (player_t *player, pspdef_t *psp)
{
	// [RH] bfg can be forced to not use freeaim
	angle_t storedpitch = player->mo->pitch;
	int storedaimdist = player->userinfo.aimdist;

	if (!(dmflags & DF_INFINITE_AMMO))
		player->ammo[wpnlev1info[player->readyweapon]->ammo] -=
			wpnlev1info[player->readyweapon]->ammouse;

	if (nobfgaim.value)
	{
		player->mo->pitch = 0;
		player->userinfo.aimdist = 81920000;
	}
	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ABFGBall));
	player->mo->pitch = storedpitch;
	player->userinfo.aimdist = storedaimdist;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray (AActor *mo) 
{
	int 				i;
	int 				j;
	int 				damage;
	angle_t 			an;

	// [RH] Don't crash if no target
	if (!mo->target)
		return;

	// offset angles from its attack angle
	for (i = 0; i < 40; i++)
	{
		an = mo->angle - ANG90/2 + ANG90/40*i;

		// mo->target is the originator (player)
		//	of the missile
		P_AimLineAttack (mo->target, an, 16*64*FRACUNIT);

		if (!linetarget)
			continue;

		Spawn<ABFGExtra> (linetarget->x, linetarget->y,
			linetarget->z + (linetarget->height>>2));
		
		damage = 0;
		for (j=0;j<15;j++)
			damage += (P_Random (pr_bfgspray) & 7) + 1;

		P_DamageMobj (linetarget, mo->target, mo->target, damage, MOD_BFG_SPLASH);
	}
}

//
// A_BFGsound
//
void A_BFGsound (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/bfgf", 1, ATTN_NORM);
}

