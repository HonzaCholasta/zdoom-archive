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
#include "gstrings.h"
#include "p_effect.h"
#include "gi.h"

#include "p_trace.h"

#include "p_grubber.h" // [GRB]

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

class AClip : public AAmmo
{
	DECLARE_ACTOR (AClip, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		if (flags & MF_DROPPED)
			return P_GiveAmmo (toucher->player, am_clip, clipammo[am_clip]/2);
		else
			return P_GiveAmmo (toucher->player, am_clip, clipammo[am_clip]);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTCLIP);
	}
};

FState AClip::States[] =
{
	S_NORMAL (CLIP, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (AClip, Doom, 2007, 11)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

AT_GAME_SET (Clip)
{
	AmmoPics[am_clip] = "CLIPA0";
}

// Clip box ----------------------------------------------------------------

class AClipBox : public AAmmo
{
	DECLARE_ACTOR (AClipBox, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_clip, clipammo[am_clip]*5);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTCLIPBOX);
	}
};

FState AClipBox::States[] =
{
	S_NORMAL (AMMO, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (AClipBox, Doom, 2048, 139)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

// Rocket ------------------------------------------------------------------

class ARocketAmmo : public AAmmo
{
	DECLARE_ACTOR (ARocketAmmo, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_misl, clipammo[am_misl]);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTROCKET);
	}
};

FState ARocketAmmo::States[] =
{
	S_NORMAL (ROCK, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (ARocketAmmo, Doom, 2010, 140)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (26)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

AT_GAME_SET (RocketAmmo)
{
	AmmoPics[am_misl] = "ROCKA0";
}

// Rocket box --------------------------------------------------------------

class ARocketBox : public AAmmo
{
	DECLARE_ACTOR (ARocketBox, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_misl, clipammo[am_misl]*5);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTROCKBOX);
	}
};

FState ARocketBox::States[] =
{
	S_NORMAL (BROK, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (ARocketBox, Doom, 2046, 141)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

// Cell --------------------------------------------------------------------

class ACell : public AAmmo
{
	DECLARE_ACTOR (ACell, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_cell, clipammo[am_cell]);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTCELL);
	}
};

FState ACell::States[] =
{
	S_NORMAL (CELL, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (ACell, Doom, 2047, 75)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (10)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

AT_GAME_SET (Cell)
{
	AmmoPics[am_cell] = "CELLA0";
}

// Cell pack ---------------------------------------------------------------

class ACellPack : public AAmmo
{
	DECLARE_ACTOR (ACellPack, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_cell, clipammo[am_cell]*5);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTCELLBOX);
	}
};

FState ACellPack::States[] =
{
	S_NORMAL (CELP, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (ACellPack, Doom, 17, 142)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (18)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

// Shells ------------------------------------------------------------------

class AShell : public AAmmo
{
	DECLARE_ACTOR (AShell, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_shell, clipammo[am_shell]);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTSHELLS);
	}
};

FState AShell::States[] =
{
	S_NORMAL (SHEL, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (AShell, Doom, 2008, 12)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (8)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

AT_GAME_SET (Shell)
{
	AmmoPics[am_shell] = "SHELA0";
}

// Shell box ---------------------------------------------------------------

class AShellBox : public AAmmo
{
	DECLARE_ACTOR (AShellBox, AAmmo)
public:
	virtual bool TryPickup (AActor *toucher)
	{
		return P_GiveAmmo (toucher->player, am_shell, clipammo[am_shell]*5);
	}
protected:
	virtual const char *PickupMessage ()
	{
		return GStrings(GOTSHELLBOX);
	}
};

FState AShellBox::States[] =
{
	S_NORMAL (SBOX, 'A',   -1, NULL 				, NULL)
};

IMPLEMENT_ACTOR (AShellBox, Doom, 2049, 143)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (10)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (0)
END_DEFAULTS

/* the weapons that use the ammo above *************************************/

// Fist ---------------------------------------------------------------------

void A_Punch (player_t *, pspdef_t *);

class AFist : public AWeapon
{
	DECLARE_ACTOR (AFist, AWeapon)
	AT_GAME_SET_FRIEND (Fist)
protected:
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	NULL,
	RUNTIME_CLASS(AFist),
	-1
};

IMPLEMENT_ACTOR (AFist, Doom, -1, 0)
END_DEFAULTS

AT_GAME_SET (Fist)
{
	wpnlev1info[wp_fist] = wpnlev2info[wp_fist] = &AFist::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[1].AddWeapon (wp_fist, 8);
	}
}

weapontype_t AFist::OldStyleID () const
{
	return wp_fist;
}

//
// A_Punch
//
void A_Punch (player_t *player, pspdef_t *psp)
{
	angle_t 	angle;
	int 		damage;
	int 		pitch;

	player->UseAmmo ();

	damage = (P_Random (pr_punch)%10+1)<<1;

	if (player->powers[pw_strength])	
		damage *= 10;

	angle = player->mo->angle;

	angle += PS_Random (pr_punch) << 18;
	pitch = P_AimLineAttack (player->mo, angle, MELEERANGE);
	P_LineAttack (player->mo, angle, MELEERANGE, pitch, damage);

	// turn to face target
	if (linetarget)
	{
		S_Sound (player->mo, CHAN_WEAPON, "*fist", 1, ATTN_NORM);
		player->mo->angle = R_PointToAngle2 (player->mo->x,
											 player->mo->y,
											 linetarget->x,
											 linetarget->y);
		// [GRB]
		if (cl_wpn_fist_thrust)
		{
			linetarget->momz = damage*FRACUNIT/2;
			linetarget->momx = FixedMul (damage*FRACUNIT, finecosine[player->mo->angle>>ANGLETOFINESHIFT]);
			linetarget->momy = FixedMul (damage*FRACUNIT, finesine[player->mo->angle>>ANGLETOFINESHIFT]);
		}
	}
}

// Pistol -------------------------------------------------------------------

void A_FirePistol (player_t *, pspdef_t *);
void A_FirePistol2 (player_t *, pspdef_t *);	// [GRB]

class APistol : public AWeapon
{
	DECLARE_ACTOR (APistol, AWeapon)
	AT_GAME_SET_FRIEND (Pistol)
protected:
	weapontype_t OldStyleID () const;
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
	S_NORMAL (PISG, 'A',	4, NULL			 		, &States[S_PISTOL1+1]),
	S_NORMAL (PISG, 'B',	2, A_FirePistol 		, &States[S_PISTOL1+2]),
	S_NORMAL (PISG, 'B',	2, A_FirePistol2 		, &States[S_PISTOL1+3]),
	S_NORMAL (PISG, 'B',	2, A_FirePistol2 		, &States[S_PISTOL1+4]),
	S_NORMAL (PISG, 'C',	4, NULL					, &States[S_PISTOL1+5]),
	S_NORMAL (PISG, 'B',	5, A_ReFire 			, &States[S_PISTOL]),

#define S_PISTOLFLASH (S_PISTOL1+6)
	S_BRIGHT (PISF, 'A',	7, A_Light1 			, &AWeapon::States[S_LIGHTDONE])
};

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
	NULL,
	RUNTIME_CLASS(APistol),
	-1
};

IMPLEMENT_ACTOR (APistol, Doom, -1, 0)
END_DEFAULTS

AT_GAME_SET (Pistol)
{
	wpnlev1info[wp_pistol] = wpnlev2info[wp_pistol] = &APistol::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[2].AddWeapon (wp_pistol, 3);
	}
}

weapontype_t APistol::OldStyleID () const
{
	return wp_pistol;
}

//
// A_FirePistol
//
void A_FirePistol (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/pistol", 1, ATTN_NORM);

	player->mo->PlayAttacking2 ();
	player->UseAmmo ();

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);
	//P_GunShot (player->mo, !player->refire);
	P_GunShot2 (player->mo, cl_wpn_pistol_spread);	// [GRB]
	if (cl_wpn_pistol_expuffs)	// [GRB]
		P_SpawnExplosion (player->mo);

	P_EjectShell (player->mo, 1, 5);	// [GRB]
}

//
// A_FirePistol [GRB]
//
void A_FirePistol2 (player_t *player, pspdef_t *psp)
{
	if (cl_wpn_pistol_burst && player->ammo[am_clip] > 0)
		A_FirePistol (player, psp);
}

// Chainsaw -----------------------------------------------------------------

void A_Saw (player_t *, pspdef_t *);
// [GRB]
void A_Explode (AActor *);
void A_SawDie (AActor *);

class AChainsaw : public AWeapon
{
	DECLARE_ACTOR (AChainsaw, AWeapon)
	AT_GAME_SET_FRIEND (Chainsaw)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	"weapons/sawidle",
	RUNTIME_CLASS(AChainsaw),
	-1
};

IMPLEMENT_ACTOR (AChainsaw, Doom, 2005, 32)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_CSAW)
END_DEFAULTS

AT_GAME_SET (Chainsaw)
{
	wpnlev1info[wp_chainsaw] = wpnlev2info[wp_chainsaw] = &AChainsaw::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[1].AddWeapon (wp_chainsaw, 7);
	}
}

weapontype_t AChainsaw::OldStyleID () const
{
	return wp_chainsaw;
}

const char *AChainsaw::PickupMessage ()
{
	return GStrings(GOTCHAINSAW);
}

// [GRB]
class AThrowSaw : public AActor
{
	DECLARE_ACTOR (AThrowSaw, AActor)
public:
	void Die (AActor *source, AActor *inflictor);
};

FState AThrowSaw::States[] =
{
	S_NORMAL (CSAW,	'A',	1, NULL						, &States[0]),
	S_NORMAL (CSAW,	'A',	1, A_SawDie					, NULL)
};

IMPLEMENT_ACTOR (AThrowSaw, Doom, -1, 0)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (25)
	PROP_Damage (50)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (0)
	PROP_DeathState (1)

	PROP_SeeSound ("weapons/sawfull")
	PROP_DeathSound ("weapons/sawhit")
END_DEFAULTS

void AThrowSaw::Die (AActor *source, AActor *inflictor)
{
	RenderStyle = STYLE_Add;
}

//
// A_Saw
//
void A_Saw (player_t *player, pspdef_t *psp)
{
	angle_t 	angle;
	int 		damage;

	player->UseAmmo ();

	if (cl_wpn_chainsaw_throw && player->weaponowned[wp_chainsaw])
	{
		P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AThrowSaw));
		player->weaponowned[wp_chainsaw] = false;
		player->pendingweapon = wp_fist;
		return;
	}

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

// [GRB]
void A_SawDie (AActor *self)
{
	AActor *saw = Spawn<AChainsaw> (self->x, self->y, self->z);
}

// Shotgun ------------------------------------------------------------------

void A_FireShotgun (player_t *, pspdef_t *);
void A_AutoReFire (player_t *, pspdef_t *);

class AShotgun : public AWeapon
{
	DECLARE_ACTOR (AShotgun, AWeapon)
	AT_GAME_SET_FRIEND (Shotgun)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	S_NORMAL (SHTG, 'B',	5, A_AutoReFire			, &States[S_SGUN1+3]),
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
	NULL,
	RUNTIME_CLASS(AShotgun),
	-1
};

IMPLEMENT_ACTOR (AShotgun, Doom, 2001, 27)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_SHOT)
END_DEFAULTS

AT_GAME_SET (Shotgun)
{
	wpnlev1info[wp_shotgun] = wpnlev2info[wp_shotgun] = &AShotgun::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[3].AddWeapon (wp_shotgun, 4);
	}
}

weapontype_t AShotgun::OldStyleID () const
{
	return wp_shotgun;
}

const char *AShotgun::PickupMessage ()
{
	return GStrings(GOTSHOTGUN);
}

//
// A_FireShotgun
//
void A_FireShotgun (player_t *player, pspdef_t *psp)
{
	int i;
		
	S_Sound (player->mo, CHAN_WEAPON,  "weapons/shotgf", 1, ATTN_NORM);
	player->mo->PlayAttacking2 ();
	player->UseAmmo ();

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);

	for (i=0 ; i<7 ; i++)
	{
		//P_GunShot (player->mo, false);
		P_GunShot2 (player->mo, cl_wpn_shotgun_spread);	// [GRB]
		if (cl_wpn_shotgun_expuffs)	// [GRB]
			P_SpawnExplosion (player->mo);
	}

	P_EjectShell (player->mo, 0, 5);	// [GRB]
}

// [GRB]
void A_AutoReFire (player_t *player, pspdef_t *psp)
{
	if ((player->readyweapon == wp_shotgun && cl_wpn_shotgun_auto) ||
		(player->readyweapon == wp_supershotgun && cl_wpn_sshotgun_auto))
		A_ReFire (player, psp);
}

// Super Shotgun ------------------------------------------------------------

void A_FireShotgun2 (player_t *, pspdef_t *);
void A_CheckReload (player_t *, pspdef_t *);
void A_OpenShotgun2 (player_t *, pspdef_t *);
void A_LoadShotgun2 (player_t *, pspdef_t *);
void A_CloseShotgun2 (player_t *, pspdef_t *);

class ASuperShotgun : public AWeapon
{
	DECLARE_ACTOR (ASuperShotgun, AWeapon)
	AT_GAME_SET_FRIEND (SuperShotgun)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	S_NORMAL (SHT2, 'B',	7, A_AutoReFire			, &States[S_DSGUN1+3]),
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
	S_BRIGHT (SHT2, 'I',	4, A_Light1 			, &States[S_DSGUNFLASH+1]),
	S_BRIGHT (SHT2, 'J',	3, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

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
	NULL,
	RUNTIME_CLASS(ASuperShotgun),
	-1
};

IMPLEMENT_ACTOR (ASuperShotgun, Doom, 82, 33)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_SHOT2)
END_DEFAULTS

AT_GAME_SET (SuperShotgun)
{
	wpnlev1info[wp_supershotgun] = wpnlev2info[wp_supershotgun] = &ASuperShotgun::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[3].AddWeapon (wp_supershotgun, 5);
	}
}

weapontype_t ASuperShotgun::OldStyleID () const
{
	return wp_supershotgun;
}

const char *ASuperShotgun::PickupMessage ()
{
	return GStrings(GOTSHOTGUN2);
}

//
// A_FireShotgun2
//
void A_FireShotgun2 (player_t *player, pspdef_t *psp)
{
	int 		i, k;
	//angle_t 	angle;
	//int 		damage;
				
		
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sshotf", 1, ATTN_NORM);
	player->mo->PlayAttacking2 ();
	if (cl_wpn_sshotgun_fireone == 1)
	{
		wpnlev1info[wp_supershotgun]->ammouse = 1;
		k = 10;
	}
	else
	{
		wpnlev1info[wp_supershotgun]->ammouse = 2;
		k = 20;
	}
	player->UseAmmo ();

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);
		
	for (i=0 ; i<k ; i++)
	{
		/*
		damage = 5*(P_Random (pr_fireshotgun2)%3+1);
		angle = player->mo->angle;
		angle += PS_Random (pr_fireshotgun2) << 19;
		P_LineAttack (player->mo,
					  angle,
					  MISSILERANGE,
					  bulletpitch + (PS_Random (pr_fireshotgun2) << 18), damage);
		*/
		P_GunShot2 (player->mo, cl_wpn_sshotgun_spread);	// [GRB]
		if (cl_wpn_sshotgun_expuffs)	// [GRB]
			P_SpawnExplosion (player->mo);
	}
}

void A_CheckReload (player_t *player, pspdef_t *psp)
{
	P_CheckAmmo (player);
}

void A_OpenShotgun2 (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/sshoto", 1, ATTN_NORM);

	// [GRB]
	P_EjectShell (player->mo, 0, 5);
	if (!cl_wpn_sshotgun_fireone)
		P_EjectShell (player->mo, 0, 10);
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
	DECLARE_ACTOR (AChaingun, AWeapon)
	AT_GAME_SET_FRIEND (Chaingun)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	NULL,
	RUNTIME_CLASS(AChaingun),
	-1
};

IMPLEMENT_ACTOR (AChaingun, Doom, 2002, 28)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_MGUN)
END_DEFAULTS

AT_GAME_SET (Chaingun)
{
	wpnlev1info[wp_chaingun] = wpnlev2info[wp_chaingun] = &AChaingun::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[4].AddWeapon (wp_chaingun, 6);
	}
}

weapontype_t AChaingun::OldStyleID () const
{
	return wp_chaingun;
}

const char *AChaingun::PickupMessage ()
{
	return GStrings(GOTCHAINGUN);
}

//
// A_FireCGun
//
void A_FireCGun (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/chngun", 1, ATTN_NORM);

	if (wpnlev1info[player->readyweapon]->ammo < NUMAMMO &&
		!player->ammo[wpnlev1info[player->readyweapon]->ammo])
		return;
				
	// [GRB]
	if (cl_wpn_chaingun_shell == 1)
		wpnlev1info[wp_chaingun]->ammo = am_shell;
	else
		wpnlev1info[wp_chaingun]->ammo = am_clip;

	player->mo->PlayAttacking2 ();
	player->UseAmmo ();

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate
				  + (psp->state - wpnlev1info[player->readyweapon]->atkstate));

	P_BulletSlope (player->mo);

	// [GRB]
	//P_GunShot (player->mo, !player->refire);
	if (cl_wpn_chaingun_shell && player->ammo[am_shell] > 0)
	{
		for (int i=0; i<10; i++)
		{
			P_GunShot2 (player->mo, cl_wpn_chaingun_spread);
			if (cl_wpn_chaingun_expuffs)
				P_SpawnExplosion (player->mo);
		}
		P_EjectShell (player->mo, 0, 5);
	}
	else
	{
		P_GunShot2 (player->mo, cl_wpn_chaingun_spread);
		if (cl_wpn_chaingun_expuffs)
			P_SpawnExplosion (player->mo);
		P_EjectShell (player->mo, 1, 5);
	}
}

// Rocket launcher ---------------------------------------------------------

void A_FireMissile (player_t *, pspdef_t *);
void A_Explode (AActor *);

class ARocketLauncher : public AWeapon
{
	DECLARE_ACTOR (ARocketLauncher, AWeapon)
	AT_GAME_SET_FRIEND (RocketLauncher)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	NULL,
	RUNTIME_CLASS(ARocketLauncher),
	-1
};

IMPLEMENT_ACTOR (ARocketLauncher, Doom, 2003, 29)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_LAUN)
END_DEFAULTS

AT_GAME_SET (RocketLauncher)
{
	wpnlev1info[wp_missile] = wpnlev2info[wp_missile] = &ARocketLauncher::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[5].AddWeapon (wp_missile, 7);
	}
}

weapontype_t ARocketLauncher::OldStyleID () const
{
	return wp_missile;
}

const char *ARocketLauncher::PickupMessage ()
{
	return GStrings(GOTLAUNCHER);
}

FState ARocket::States[] =
{
#define S_ROCKET 0
	S_BRIGHT (MISL, 'A',	1, A_Guide 						, &States[S_ROCKET]),

#define S_EXPLODE (S_ROCKET+1)
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[S_EXPLODE+1]),
	S_BRIGHT (MISL, 'C',	6, NULL 						, &States[S_EXPLODE+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ARocket, Doom, -1, 127)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (20)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_ROCKET)
	PROP_DeathState (S_EXPLODE)

	PROP_SeeSound ("weapons/rocklf")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

void ARocket::BeginPlay ()
{
	Super::BeginPlay ();
	effects |= FX_ROCKET;
}

// [GRB]
void A_NapalmExplode (AActor *);

class AFlame : public AActor
{
	DECLARE_ACTOR (AFlame, AActor);
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState AFlame::States[] =
{
	S_BRIGHT (MISL, 'B',	8, NULL							, &States[1]),
	S_BRIGHT (MISL, 'C',	6, A_NapalmExplode 				, &States[2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (AFlame, Doom, -1, 0)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (20)
	PROP_Damage (15)
	PROP_RenderStyle (STYLE_Add)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (0)
	PROP_DeathState (0)

	PROP_SeeSound ("vile/firestrt")
	PROP_DeathSound ("vile/firecrkl")
END_DEFAULTS

void AFlame::BeginPlay ()
{
	momz = 4*FRACUNIT;
}

void A_Napalm (AActor *);

class ANapalm : public AActor
{
	DECLARE_ACTOR (ANapalm, AActor);
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState ANapalm::States[] =
{
	S_BRIGHT (MISL, 'A',	5, A_Napalm 					, &States[0]),
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[2]),
	S_BRIGHT (MISL, 'C',	6, NULL 						, &States[3]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ANapalm, Doom, -1, 0)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (20)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (0)
	PROP_DeathState (1)

	PROP_SeeSound ("weapons/rocklf")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

void ANapalm::BeginPlay ()
{
	Super::BeginPlay ();
	effects |= FX_ROCKET;
}

void A_NapalmExplode (AActor *thing)
{
	int damage = 32;
	int distance = 64;
	bool hurtSource = true;

	if (thing->momz < 0)
	{
		thing->PreExplode ();
		thing->GetExplodeParms (damage, distance, hurtSource);
		P_RadiusAttack (thing, thing->target, damage, distance, hurtSource, thing->GetMOD ());
		if (thing->z <= thing->floorz + (distance<<FRACBITS))
		{
			P_HitFloor (thing);
		}
	}
}

void A_Napalm (AActor *self)
{
	AActor *missile = Spawn<AFlame> (self->x, self->y, self->z);

	missile->momz = -missile->momz;
}

void A_Mushroom (AActor *);

class ANuclearRocket : public AActor
{
	DECLARE_ACTOR (ANuclearRocket, AActor);
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState ANuclearRocket::States[] =
{
#define S_NROCKET 0
	S_BRIGHT (MISL, 'A',	1, A_Guide 						, &States[S_ROCKET]),

#define S_NEXPLODE (S_NROCKET+1)
	S_BRIGHT (MISL, 'B',	8, A_Mushroom					, &States[S_EXPLODE+1]),
	S_BRIGHT (MISL, 'C',	6, NULL 						, &States[S_EXPLODE+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ANuclearRocket, Doom, -1, 127)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (20)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_NROCKET)
	PROP_DeathState (S_NEXPLODE)

	PROP_SeeSound ("weapons/rocklf")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

void ANuclearRocket::BeginPlay ()
{
	Super::BeginPlay ();
	effects |= FX_ROCKET;
}

//
// A_FireMissile
//
void A_FireMissile (player_t *player, pspdef_t *psp)
{
	AActor *missile;	// [GRB]

	for (int i = 0; i < (cl_wpn_rocket_spreadfire ? 8 : 1); i++)
	{
		if (cl_wpn_rocket_mode == 1)
		{
			wpnlev1info[wp_missile]->ammouse = 1;
			ARocketLauncher::States[S_MISSILE1+1].Tics = 2;
			missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AFlame), (cl_wpn_rocket_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
		}
		else if (cl_wpn_rocket_mode == 2)
		{
			wpnlev1info[wp_missile]->ammouse = 10;
			ARocketLauncher::States[S_MISSILE1+1].Tics = 18;
			missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ANapalm), (cl_wpn_rocket_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
		}
		else if (cl_wpn_rocket_mode == 3)
		{
			wpnlev1info[wp_missile]->ammouse = 10;
			ARocketLauncher::States[S_MISSILE1+1].Tics = 18;
			missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ANuclearRocket), (cl_wpn_rocket_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
		}
		else
		{
			wpnlev1info[wp_missile]->ammouse = 1;
			ARocketLauncher::States[S_MISSILE1+1].Tics = 12;
			missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ARocket), (cl_wpn_rocket_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
		}

		if (!missile)
			continue;

		if (player->ammo[am_misl] < wpnlev1info[wp_missile]->ammouse)
			return;

		player->UseAmmo ();

		// [GRB]
		if (cl_wpn_rocket_spreadfire)
		{
			missile->momz += ((M_Random () - 128) * FRACUNIT / 50);
		}
		if (cl_wpn_rocket_bounce)
		{
			missile->flags &= ~MF_NOGRAVITY;
			missile->flags2 |= MF2_FLOORBOUNCE;
		}
		if (cl_wpn_rocket_killable)
		{
			missile->flags |= MF_SHOOTABLE|MF_SOLID;
			missile->health = 1;
		}
		if (cl_wpn_rocket_bobbing)
		{
			missile->flags2 |= MF2_FLOATBOB;
		}
	}
}

// Plasma rifle ------------------------------------------------------------

void A_FirePlasma (player_t *, pspdef_t *);
// [GRB]
void A_FireRailgun (player_t *, pspdef_t *);
void A_Mirv (AActor *);

class APlasmaRifle : public AWeapon
{
	DECLARE_ACTOR (APlasmaRifle, AWeapon)
	AT_GAME_SET_FRIEND (PlasmaRifle)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

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
	NULL,
	RUNTIME_CLASS(APlasmaRifle),
	-1
};

IMPLEMENT_ACTOR (APlasmaRifle, Doom, 2004, 30)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_PLAS)
END_DEFAULTS

AT_GAME_SET (PlasmaRifle)
{
	wpnlev1info[wp_plasma] = wpnlev2info[wp_plasma] = &APlasmaRifle::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[6].AddWeapon (wp_plasma, 8);
	}
}

weapontype_t APlasmaRifle::OldStyleID () const
{
	return wp_plasma;
}

const char *APlasmaRifle::PickupMessage ()
{
	return GStrings(GOTPLASMA);
}

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

IMPLEMENT_ACTOR (APlasmaBall, Doom, -1, 51)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (25)
	PROP_Damage (5)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)
	PROP_RenderStyle (STYLE_Add)
	PROP_Alpha (TRANSLUC75)

	PROP_SpawnState (S_PLASBALL)
	PROP_DeathState (S_PLASEXP)

	PROP_SeeSound ("weapons/plasmaf")
	PROP_DeathSound ("weapons/plasmax")
END_DEFAULTS

// [GRB]
class AGreenPlasma : public APlasmaBall
{
	DECLARE_ACTOR (AGreenPlasma, APlasmaBall)
};

FState AGreenPlasma::States[] =
{
#define S_GREEN_PLAZ 0
	S_BRIGHT (APLS, 'A',	5, NULL							, &States[S_GREEN_PLAZ+1]),
	S_BRIGHT (APLS, 'B',	5, NULL							, &States[S_GREEN_PLAZ]),

#define S_GREEN_PLEX (S_GREEN_PLAZ+2)
	S_BRIGHT (APBX, 'A',	5, NULL							, &States[S_GREEN_PLEX+1]),
	S_BRIGHT (APBX, 'B',	5, NULL 						, &States[S_GREEN_PLEX+2]),
	S_BRIGHT (APBX, 'C',	5, NULL 						, &States[S_GREEN_PLEX+3]),
	S_BRIGHT (APBX, 'D',	5, NULL 						, &States[S_GREEN_PLEX+4]),
	S_BRIGHT (APBX, 'E',	5, NULL		 					, NULL)
};

IMPLEMENT_ACTOR (AGreenPlasma, Doom, -1, 0)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (30)
	PROP_Damage (4)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)
	PROP_RenderStyle (STYLE_Add)

	PROP_SpawnState (S_GREEN_PLAZ)
	PROP_DeathState (S_GREEN_PLEX)

	PROP_SeeSound ("baby/attack")
	PROP_DeathSound ("baby/shotx")
END_DEFAULTS

class APlasmaMirv : public APlasmaBall
{
	DECLARE_ACTOR (APlasmaMirv, APlasmaBall)
};

FState APlasmaMirv::States[] =
{
#define S_MIRVBALL 0
	S_BRIGHT (PLSS, 'A',	6, NULL							, &States[S_MIRVBALL+1]),
	S_BRIGHT (PLSS, 'B',	6, NULL 						, &States[S_MIRVBALL]),

#define S_MIRVEXP (S_MIRVBALL+2)
	S_BRIGHT (PLSE, 'A',	4, A_Mirv						, &States[S_MIRVEXP+1]),
	S_BRIGHT (PLSE, 'B',	4, NULL 						, &States[S_MIRVEXP+2]),
	S_BRIGHT (PLSE, 'C',	4, NULL 						, &States[S_MIRVEXP+3]),
	S_BRIGHT (PLSE, 'D',	4, NULL							, &States[S_MIRVEXP+4]),
	S_BRIGHT (PLSE, 'E',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (APlasmaMirv, Doom, -1, 0)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (25)
	PROP_Damage (5)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)
	PROP_RenderStyle (STYLE_Add)
	PROP_Alpha (TRANSLUC75)

	PROP_SpawnState (S_PLASBALL)
	PROP_DeathState (S_PLASEXP)

	PROP_SeeSound ("weapons/plasmaf")
	PROP_DeathSound ("weapons/plasmax")
END_DEFAULTS

class AGreenPlasmaMirv : public APlasmaBall
{
	DECLARE_ACTOR (AGreenPlasmaMirv, APlasmaBall)
};

FState AGreenPlasmaMirv::States[] =
{
#define S_GREENMIRV_PLAZ 0
	S_BRIGHT (APLS, 'A',	5, NULL							, &States[S_GREENMIRV_PLAZ+1]),
	S_BRIGHT (APLS, 'B',	5, NULL							, &States[S_GREENMIRV_PLAZ]),

#define S_GREENMIRV_PLEX (S_GREENMIRV_PLAZ+2)
	S_BRIGHT (APBX, 'A',	5, A_Mirv						, &States[S_GREENMIRV_PLEX+1]),
	S_BRIGHT (APBX, 'B',	5, NULL 						, &States[S_GREENMIRV_PLEX+2]),
	S_BRIGHT (APBX, 'C',	5, NULL 						, &States[S_GREENMIRV_PLEX+3]),
	S_BRIGHT (APBX, 'D',	5, NULL 						, &States[S_GREENMIRV_PLEX+4]),
	S_BRIGHT (APBX, 'E',	5, NULL		 					, NULL)
};

IMPLEMENT_ACTOR (AGreenPlasmaMirv, Doom, -1, 0)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (30)
	PROP_Damage (4)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)
	PROP_RenderStyle (STYLE_Add)

	PROP_SpawnState (S_GREENMIRV_PLAZ)
	PROP_DeathState (S_GREENMIRV_PLEX)

	PROP_SeeSound ("baby/attack")
	PROP_DeathSound ("baby/shotx")
END_DEFAULTS

class AGreenPlasmaLaser : public APlasmaBall
{
	DECLARE_ACTOR (AGreenPlasmaLaser, APlasmaBall)
};

FState AGreenPlasmaLaser::States[] =
{
#define S_GREENLASR_PLAZ 0
	S_BRIGHT (APLS, 'A',	5, NULL							, &States[S_GREENLASR_PLAZ+1]),
	S_BRIGHT (APLS, 'B',	5, NULL							, NULL),

#define S_GREENLASR_PLEX (S_GREENLASR_PLAZ+2)
	S_BRIGHT (APBX, 'A',	5, A_Mirv						, &States[S_GREENLASR_PLEX+1]),
	S_BRIGHT (APBX, 'B',	5, NULL 						, &States[S_GREENLASR_PLEX+2]),
	S_BRIGHT (APBX, 'C',	5, NULL 						, &States[S_GREENLASR_PLEX+3]),
	S_BRIGHT (APBX, 'D',	5, NULL 						, &States[S_GREENLASR_PLEX+4]),
	S_BRIGHT (APBX, 'E',	5, NULL		 					, NULL)
};

IMPLEMENT_ACTOR (AGreenPlasmaLaser, Doom, -1, 0)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_Damage (4)
	PROP_SpeedFixed (0)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)
	PROP_RenderStyle (STYLE_Add)

	PROP_SpawnState (S_GREENLASR_PLAZ)
	PROP_DeathState (S_GREENLASR_PLEX)

	PROP_SeeSound ("baby/attack")
	PROP_DeathSound ("baby/shotx")
END_DEFAULTS

//
// A_FirePlasma
//
void A_FirePlasma (player_t *player, pspdef_t *psp)
{
	AActor *missile;	// [GRB]

	if (cl_wpn_plasma_mode == 2)
	{
		wpnlev1info[wp_plasma]->ammouse = 20;
		APlasmaRifle::States[S_PLASMA1].Tics = 48;
		A_FireRailgun (player, psp);
		return;
	}
	else
	{
		if (cl_wpn_plasma_mode == 1)
			wpnlev1info[wp_plasma]->ammouse = 10;
		else
			wpnlev1info[wp_plasma]->ammouse = 1;
		APlasmaRifle::States[S_PLASMA1].Tics = 3;
	}

	for (int i = 0; i < (cl_wpn_plasma_spreadfire ? 8 : 1); i++)
	{
		if (player->ammo[am_cell] < wpnlev1info[wp_plasma]->ammouse)
			return;

		player->UseAmmo ();
	}

	if (wpnlev1info[player->readyweapon]->flashstate)
	{
		P_SetPsprite (player,
					  ps_flash,
					  wpnlev1info[player->readyweapon]->flashstate
					   + (P_Random (pr_fireplasma)&1));
	}

	for (i = 0; i < (cl_wpn_plasma_spreadfire ? 8 : 1); i++)
	{
		if (cl_wpn_plasma_green)
			if (cl_wpn_plasma_mode == 1)
				missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AGreenPlasmaMirv), (cl_wpn_plasma_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
			else
				missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(AGreenPlasma), (cl_wpn_plasma_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
		else
			if (cl_wpn_plasma_mode == 1)
				missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(APlasmaMirv), (cl_wpn_plasma_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));
			else
				missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(APlasmaBall), (cl_wpn_plasma_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));

		if (!missile)
			continue;

		// [GRB]
		if (cl_wpn_plasma_spreadfire)
		{
			missile->momz += ((M_Random () - 128) * FRACUNIT / 50);
		}
		if (cl_wpn_plasma_bounce)
		{
			missile->flags &= ~MF_NOGRAVITY;
			missile->flags2 |= MF2_FLOORBOUNCE;
		}
		if (cl_wpn_plasma_killable)
		{
			missile->flags |= MF_SHOOTABLE|MF_SOLID;
		}
		if (cl_wpn_plasma_bobbing)
		{
			missile->flags2 |= MF2_FLOATBOB;
		}
	}
}

// [GRB]
void A_Mirv (AActor *self)
{
	int i;
	angle_t angle;
	AActor *plasma;

	for(i = 0; i < 8; i++)
	{
		if (cl_wpn_plasma_green)
			plasma = Spawn<AGreenPlasma> (self->x, self->y, self->z);
		else
			plasma = Spawn<APlasmaBall> (self->x, self->y, self->z);
		plasma->flags &= ~MF_NOGRAVITY;
		angle = i*ANG45;
		plasma->target = self->target;
		plasma->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		plasma->momx = FixedMul (plasma->Speed/2, finecosine[angle]);
		plasma->momy = FixedMul (plasma->Speed/2, finesine[angle]);
		P_CheckMissileSpawn (plasma);
	}
}

//
// [RH] A_FireRailgun
//
static int RailOffset;

void A_FireRailgun (player_t *player, pspdef_t *psp)
{
	int damage;

	player->UseAmmo ();

	if (wpnlev1info[player->readyweapon]->flashstate)
	{
		P_SetPsprite (player,
					  ps_flash,
					  wpnlev1info[player->readyweapon]->flashstate
					   + (P_Random (pr_fireplasma)&1));
	}

	damage = deathmatch ? 100 : 150;

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
	DECLARE_ACTOR (ABFG9000, AWeapon)
	AT_GAME_SET_FRIEND (BFG9000)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

// [GRB] Made global
//class ABFGBall : public AActor
//{
//	DECLARE_ACTOR (ABFGBall, AActor)
//public:
//	int GetMOD () { return MOD_BFG_BOOM; }
//};

class ABFGExtra : public AActor
{
	DECLARE_ACTOR (ABFGExtra, AActor)
};

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
	NULL,
	RUNTIME_CLASS(ABFG9000),
	-1
};

IMPLEMENT_ACTOR (ABFG9000, Doom, 2006, 31)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (20)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_BFUG)
END_DEFAULTS

AT_GAME_SET (BFG9000)
{
	wpnlev1info[wp_bfg] = wpnlev2info[wp_bfg] = &ABFG9000::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[7].AddWeapon (wp_bfg, 9);
	}
}

weapontype_t ABFG9000::OldStyleID () const
{
	return wp_bfg;
}

const char *ABFG9000::PickupMessage ()
{
	return GStrings(GOTBFG9000);
}

FState ABFGBall::States[] =
{
#define S_BFGSHOT 0
	S_BRIGHT (BFS1, 'A',	4, NULL 						, &States[S_BFGSHOT+1]),
	S_BRIGHT (BFS1, 'B',	4, NULL 						, &States[S_BFGSHOT]),

#define S_BFGMINI (S_BFGSHOT+2)
	S_BRIGHT (APLS, 'A',	5, NULL							, &States[S_BFGMINI+1]),
	S_BRIGHT (APLS, 'B',	5, NULL 						, &States[S_BFGMINI]),

#define S_BFGLAND (S_BFGMINI+2)
	S_BRIGHT (BFE1, 'A',	8, NULL 						, &States[S_BFGLAND+1]),
	S_BRIGHT (BFE1, 'B',	8, NULL 						, &States[S_BFGLAND+2]),
	S_BRIGHT (BFE1, 'C',	8, A_BFGSpray					, &States[S_BFGLAND+3]),
	S_BRIGHT (BFE1, 'D',	8, NULL 						, &States[S_BFGLAND+4]),
	S_BRIGHT (BFE1, 'E',	8, NULL 						, &States[S_BFGLAND+5]),
	S_BRIGHT (BFE1, 'F',	8, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ABFGBall, Doom, -1, 128)
	PROP_RadiusFixed (13)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (25)
	PROP_Damage (100)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)
	PROP_RenderStyle (STYLE_Add)
	PROP_Alpha (TRANSLUC75)

	PROP_SpawnState (S_BFGSHOT)
	PROP_DeathState (S_BFGLAND)

	PROP_DeathSound ("weapons/bfgx")
END_DEFAULTS

FState ABFGExtra::States[] =
{
	S_BRIGHT (BFE2, 'A',	8, NULL 				, &States[1]),
	S_BRIGHT (BFE2, 'B',	8, NULL 				, &States[2]),
	S_BRIGHT (BFE2, 'C',	8, NULL 				, &States[3]),
	S_BRIGHT (BFE2, 'D',	8, NULL 				, NULL)
};

IMPLEMENT_ACTOR (ABFGExtra, Doom, -1, 0)
	PROP_Flags (MF_NOBLOCKMAP|MF_NOGRAVITY)
	PROP_RenderStyle (STYLE_Add)
	PROP_Alpha (TRANSLUC75)

	PROP_SpawnState (0)
END_DEFAULTS

//
// A_FireBFG
//

void A_FireBFG (player_t *player, pspdef_t *psp)
{
	AActor *missile;	// [GRB]

	// [RH] bfg can be forced to not use freeaim
	angle_t storedpitch = player->mo->pitch;
	int storedaimdist = player->userinfo.aimdist;

	if (cl_wpn_bfg_mini)
	{
		ABFG9000::States[S_BFG1].Tics = 8;
		ABFG9000::States[S_BFG1+1].Tics = 4;
		ABFG9000::States[S_BFG1+2].Tics = 4;
		ABFG9000::States[S_BFG1+3].Tics = 8;
	}
	else
	{
		ABFG9000::States[S_BFG1].Tics = 20;
		ABFG9000::States[S_BFG1+1].Tics = 10;
		ABFG9000::States[S_BFG1+2].Tics = 10;
		ABFG9000::States[S_BFG1+3].Tics = 20;
	}

	for (int i = 0; i < (cl_wpn_bfg_spreadfire ? 8 : 1); i++)
	{
		if (player->ammo[am_cell] < wpnlev1info[wp_bfg]->ammouse)
			return;

		player->UseAmmo ();
	}

	if (dmflags2 & DF2_NO_FREEAIMBFG)
	{
		player->mo->pitch = 0;
		player->userinfo.aimdist = ANGLE_1*35;
	}

	for (i = 0; i < (cl_wpn_bfg_spreadfire ? 8 : 1); i++)
	{
		missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ABFGBall), (cl_wpn_bfg_spreadfire ? player->mo->angle + (PS_Random (pr_cposattack) << 20) : player->mo->angle));

		if (!missile)
			continue;

		// [GRB]
		if (cl_wpn_bfg_spreadfire)
		{
			missile->momz += ((M_Random () - 128) * FRACUNIT / 50);
		}
		if (cl_wpn_bfg_bounce)
		{
			missile->flags &= ~MF_NOGRAVITY;
			missile->flags2 |= MF2_FLOORBOUNCE;
		}
		if (cl_wpn_bfg_killable)
		{
			missile->flags |= MF_SHOOTABLE|MF_SOLID;
			missile->health = 1;
		}
		if (cl_wpn_bfg_bobbing)
		{
			missile->flags2 |= MF2_FLOATBOB;
		}
		if (cl_wpn_bfg_mini)
		{
			missile->SetState (&ABFGBall::States[S_BFGMINI]);
		}
	}

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
		P_AimLineAttack (mo->target, an, 16*64*FRACUNIT, ANGLE_1*32);

		if (!linetarget)
			continue;

		Spawn<ABFGExtra> (linetarget->x, linetarget->y,
			linetarget->z + (linetarget->height>>2));
		
		damage = 0;
		for (j=0;j<15;j++)
			damage += (P_Random (pr_bfgspray) & 7) + 1;

		P_DamageMobj (linetarget, mo->target, mo->target, damage, MOD_BFG_SPLASH);
		P_TraceBleed (damage, linetarget, mo->target);
	}
}

//
// A_BFGsound
//
void A_BFGsound (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/bfgf", 1, ATTN_NORM);
}

/************************
*	Grubber's Weapons	*
************************/

// Duke's kick ---------------------------------------------------------------------

void A_Kick (player_t *, pspdef_t *);

class ADukeKick : public AWeapon
{
	DECLARE_ACTOR (ADukeKick, AWeapon)
	AT_GAME_SET_FRIEND (DukeKick)
protected:
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukeKick::States[] =
{
#define S_DUKE_KICK 0
	S_NORMAL (KICK, 'A',	1, A_WeaponReady		, &States[S_DUKE_KICK]),

#define S_DUKE_KICKDOWN (S_DUKE_KICK+1)
	S_NORMAL (KICK, 'A',	1, A_Lower				, &States[S_DUKE_KICKDOWN]),

#define S_DUKE_KICKUP (S_DUKE_KICKDOWN+1)
	S_NORMAL (KICK, 'A',	1, A_Raise				, &States[S_DUKE_KICKUP]),

#define S_DUKE_KICK1 (S_DUKE_KICKUP+1)
	S_NORMAL (KICK, 'B',	6, NULL 				, &States[S_DUKE_KICK1+1]),
	S_NORMAL (KICK, 'C',	6, A_Kick				, &States[S_DUKE_KICK1+2]),
	S_NORMAL (KICK, 'B',	6, A_ReFire 			, &States[S_DUKE_KICK])
};

FWeaponInfo ADukeKick::WeaponInfo =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_DUKE_KICKUP],
	&States[S_DUKE_KICKDOWN],
	&States[S_DUKE_KICK],
	&States[S_DUKE_KICK1],
	&States[S_DUKE_KICK1],
	NULL,
	NULL,
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukeKick),
	-1
};

IMPLEMENT_ACTOR (ADukeKick, Doom, -1, 0)
END_DEFAULTS

AT_GAME_SET (DukeKick)
{
	wpnlev1info[wp_duke_kick] = wpnlev2info[wp_duke_kick] = &ADukeKick::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[1].AddWeapon (wp_duke_kick, 6);
	}
}

weapontype_t ADukeKick::OldStyleID () const
{
	return wp_duke_kick;
}

//
// A_Kick
//
void A_Kick (player_t *player, pspdef_t *psp)
{
	angle_t 	angle;
	int 		damage;
	int 		slope;
	int			t;
		
	damage = (P_Random (pr_punch)%10+6)<<1;

	if (player->powers[pw_strength])	
		damage *= 10;

	angle = player->mo->angle;

	t = P_Random (pr_punch);
	angle += (t - P_Random (pr_punch)) << 18;
	slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
	P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);

	// turn to face target
	if (linetarget)
	{
		player->mo->angle = R_PointToAngle2 (player->mo->x,
											 player->mo->y,
											 linetarget->x,
											 linetarget->y);
	}
	S_Sound (player->mo, CHAN_WEAPON, "weapons/dnkick", 1, ATTN_NORM);
}

// Duke's Pistol -------------------------------------------------------------------

void A_FireDukePistol (player_t *, pspdef_t *);

class ADukePistol : public AWeapon
{
	DECLARE_ACTOR (ADukePistol, AWeapon)
	AT_GAME_SET_FRIEND (DukePistol)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukePistol::States[] =
{
#define S_DUKE_PISTOL 0
	S_NORMAL (PIST, 'A',	1, A_WeaponReady		, &States[S_DUKE_PISTOL]),

#define S_DUKE_PISTOLDOWN (S_DUKE_PISTOL+1)
	S_NORMAL (PIST, 'A',	1, A_Lower				, &States[S_DUKE_PISTOLDOWN]),

#define S_DUKE_PISTOLUP (S_DUKE_PISTOLDOWN+1)
	S_NORMAL (PIST, 'A',	1, A_Raise				, &States[S_DUKE_PISTOLUP]),

#define S_DUKE_PISTOL1 (S_DUKE_PISTOLUP+1)
	S_NORMAL (PIST, 'A',	4, NULL 				, &States[S_DUKE_PISTOL1+1]),
	S_NORMAL (PIST, 'B',	4, A_FireDukePistol 	, &States[S_DUKE_PISTOL1+2]),
	S_NORMAL (PIST, 'C',	4, NULL 				, &States[S_DUKE_PISTOL1+3]),
	S_NORMAL (PIST, 'B',	4, A_ReFire 			, &States[S_DUKE_PISTOL]),

#define S_DUKE_ITMPISTOL (S_DUKE_PISTOL1+4)
	S_NORMAL (DNPI, 'A',	-1,NULL					, NULL)
};

FWeaponInfo ADukePistol::WeaponInfo =
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
	NULL,
	RUNTIME_CLASS(AClip),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukePistol),
	-1
};

IMPLEMENT_ACTOR (ADukePistol, Doom, 500, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (20)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMPISTOL)
END_DEFAULTS

AT_GAME_SET (DukePistol)
{
	wpnlev1info[wp_duke_pistol] = wpnlev2info[wp_duke_pistol] = &ADukePistol::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[2].AddWeapon (wp_duke_pistol, 2);
	}
}

weapontype_t ADukePistol::OldStyleID () const
{
	return wp_duke_pistol;
}

const char *ADukePistol::PickupMessage ()
{
	return GStrings(GOTSHOTGUN);
}

//
// A_FireDukePistol
//
void A_FireDukePistol (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/dnpist", 1, ATTN_NORM);

	player->mo->PlayAttacking2 ();
	player->UseAmmo ();

	A_Light2 (player, psp);

	P_BulletSlope (player->mo);
	P_GunShot (player->mo, true);
}

// Duke's Shotgun ------------------------------------------------------------------

void A_FireDukeShotgun (player_t *, pspdef_t *);

class ADukeShotgun : public AWeapon
{
	DECLARE_ACTOR (ADukeShotgun, AWeapon)
	AT_GAME_SET_FRIEND (DukeShotgun)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukeShotgun::States[] =
{
#define S_DUKE_SGUN 0
	S_NORMAL (DSHT, 'A',	1, A_WeaponReady		, &States[S_DUKE_SGUN]),

#define S_DUKE_SGUNDOWN (S_DUKE_SGUN+1)
	S_NORMAL (DSHT, 'A',	1, A_Lower				, &States[S_DUKE_SGUNDOWN]),

#define S_DUKE_SGUNUP (S_DUKE_SGUNDOWN+1)
	S_NORMAL (DSHT, 'A',	1, A_Raise				, &States[S_DUKE_SGUNUP]),

#define S_DUKE_SGUN1 (S_DUKE_SGUNUP+1)
	S_NORMAL (DSHT, 'A',	3, NULL 				, &States[S_DUKE_SGUN1+1]),
	S_NORMAL (DSHT, 'A',	7, A_FireDukeShotgun		, &States[S_DUKE_SGUN1+2]),
	S_NORMAL (DSHT, 'D',	5, NULL 				, &States[S_DUKE_SGUN1+3]),
	S_NORMAL (DSHT, 'E',	5, NULL 				, &States[S_DUKE_SGUN1+4]),
	S_NORMAL (DSHT, 'F',	4, NULL 				, &States[S_DUKE_SGUN1+5]),
	S_NORMAL (DSHT, 'G',	5, NULL 				, &States[S_DUKE_SGUN1+6]),
	S_NORMAL (DSHT, 'F',	5, NULL 				, &States[S_DUKE_SGUN1+7]),
	S_NORMAL (DSHT, 'E',	3, NULL 				, &States[S_DUKE_SGUN1+8]),
	S_NORMAL (DSHT, 'D',	7, A_ReFire 			, &States[S_DUKE_SGUN]),

#define S_DUKE_SGUNFLASH (S_DUKE_SGUN1+9)
	S_BRIGHT (DSHT, 'B',	4, A_Light1 			, &States[S_DUKE_SGUNFLASH+1]),
	S_BRIGHT (DSHT, 'C',	3, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),

#define S_DUKE_ITMSHOTGUN (S_DUKE_SGUNFLASH+2)
	S_NORMAL (DNSG, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ADukeShotgun::WeaponInfo =
{
	0,
	am_shell,
	1,
	8,
	&States[S_DUKE_SGUNUP],
	&States[S_DUKE_SGUNDOWN],
	&States[S_DUKE_SGUN],
	&States[S_DUKE_SGUN1],
	&States[S_DUKE_SGUN1],
	&States[S_DUKE_SGUNFLASH],
	RUNTIME_CLASS(ADukeShotgun),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukeShotgun),
	-1
};

IMPLEMENT_ACTOR (ADukeShotgun, Doom, 501, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMSHOTGUN)
END_DEFAULTS

AT_GAME_SET (DukeShotgun)
{
	wpnlev1info[wp_duke_shotgun] = wpnlev2info[wp_duke_shotgun] = &ADukeShotgun::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[3].AddWeapon (wp_duke_shotgun, 3);
	}
}

weapontype_t ADukeShotgun::OldStyleID () const
{
	return wp_duke_shotgun;
}

const char *ADukeShotgun::PickupMessage ()
{
	return GStrings(GOTSHOTGUN);
}

//
// A_FireDukeShotgun
//
void A_FireDukeShotgun (player_t *player, pspdef_t *psp)
{
	int i;
		
	S_Sound (player->mo, CHAN_WEAPON,  "weapons/dnsgun", 1, ATTN_NORM);
	player->mo->PlayAttacking2 ();
	player->UseAmmo ();

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate);

	P_BulletSlope (player->mo);
		
	for (i=0 ; i<9 ; i++)
		P_GunShot (player->mo, false);
}

// Duke's Chaingun Cannon-----------------------------------------------------------------

void A_FireDukeCGun (player_t *, pspdef_t *);

class ADukeChaingun : public AWeapon
{
	DECLARE_ACTOR (ADukeChaingun, AWeapon)
	AT_GAME_SET_FRIEND (DukeChaingun)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukeChaingun::States[] =
{
#define S_DUKE_CHAIN 0
	S_NORMAL (CHGC, 'A',	1, A_WeaponReady		, &States[S_DUKE_CHAIN]),

#define S_DUKE_CHAINDOWN (S_DUKE_CHAIN+1)
	S_NORMAL (CHGC, 'A',	1, A_Lower				, &States[S_DUKE_CHAINDOWN]),

#define S_DUKE_CHAINUP (S_DUKE_CHAINDOWN+1)
	S_NORMAL (CHGC, 'A',	1, A_Raise				, &States[S_DUKE_CHAINUP]),

#define S_DUKE_CHAIN1 (S_DUKE_CHAINUP+1)
	S_NORMAL (CHGC, 'B',	4, A_FireDukeCGun			, &States[S_DUKE_CHAIN1+1]),
	S_NORMAL (CHGC, 'C',	4, A_FireDukeCGun			, &States[S_DUKE_CHAIN1+2]),
	S_NORMAL (CHGC, 'D',	4, A_FireDukeCGun			, &States[S_DUKE_CHAIN1+3]),
	S_NORMAL (CHGC, 'C',	0, A_ReFire 			, &States[S_DUKE_CHAIN]),

#define S_DUKE_CHAINFLASH (S_DUKE_CHAIN1+4)
	S_BRIGHT (CHGC, 'E',	5, A_Light1 			, &AWeapon::States[S_LIGHTDONE]),
	S_BRIGHT (CHGC, 'F',	5, A_Light2 			, &AWeapon::States[S_LIGHTDONE]),
	S_BRIGHT (CHGC, 'G',	5, A_Light1 			, &AWeapon::States[S_LIGHTDONE]),

#define S_DUKE_ITMCHAIN (S_DUKE_CHAINFLASH+3)
	S_NORMAL (DNCG, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ADukeChaingun::WeaponInfo =
{
	0,
	am_clip,
	1,
	20,
	&States[S_DUKE_CHAINUP],
	&States[S_DUKE_CHAINDOWN],
	&States[S_DUKE_CHAIN],
	&States[S_DUKE_CHAIN1],
	&States[S_DUKE_CHAIN1],
	&States[S_DUKE_CHAINFLASH],
	RUNTIME_CLASS(ADukeChaingun),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukeChaingun),
	-1
};

IMPLEMENT_ACTOR (ADukeChaingun, Doom, 502, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMCHAIN)
END_DEFAULTS

AT_GAME_SET (DukeChaingun)
{
	wpnlev1info[wp_duke_chaingun] = wpnlev2info[wp_duke_chaingun] = &ADukeChaingun::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[4].AddWeapon (wp_duke_chaingun, 5);
	}
}

weapontype_t ADukeChaingun::OldStyleID () const
{
	return wp_duke_chaingun;
}

const char *ADukeChaingun::PickupMessage ()
{
	return GStrings(GOTCHAINGUN);
}

//
// A_FireCGun
//
void A_FireDukeCGun (player_t *player, pspdef_t *psp)
{
	S_Sound (player->mo, CHAN_WEAPON, "weapons/dnchan", 1, ATTN_NORM);

	if (wpnlev1info[player->readyweapon]->ammo < NUMAMMO &&
		!player->ammo[wpnlev1info[player->readyweapon]->ammo])
		return;
				
	player->mo->PlayAttacking2 ();
	player->UseAmmo ();

	P_SetPsprite (player,
				  ps_flash,
				  wpnlev1info[player->readyweapon]->flashstate
				  + (psp->state - wpnlev1info[player->readyweapon]->atkstate));

	P_BulletSlope (player->mo);
		
	P_GunShot (player->mo, !player->refire);
}

// Duke's Rpg ---------------------------------------------------------

void A_FireRpgMissile (player_t *, pspdef_t *);
void A_Explode (AActor *);
void A_Guide (AActor *);

class ADukeRpg : public AWeapon
{
	DECLARE_ACTOR (ADukeRpg, AWeapon)
	AT_GAME_SET_FRIEND (DukeRpg)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukeRpg::States[] =
{
#define S_DUKE_RPG 0
	S_NORMAL (RPG1, 'A',	1, A_WeaponReady		, &States[S_DUKE_RPG]),

#define S_DUKE_RPGDOWN (S_DUKE_RPG+1)
	S_NORMAL (RPG1, 'A',	1, A_Lower				, &States[S_DUKE_RPGDOWN]),

#define S_DUKE_RPGUP (S_DUKE_RPGDOWN+1)
	S_NORMAL (RPG1, 'A',	1, A_Raise				, &States[S_DUKE_RPGUP]),

#define S_DUKE_RPG1 (S_DUKE_RPGUP+1)
	S_NORMAL (RPG1, 'B',	4, A_Light1				, &States[S_DUKE_RPG1+1]),
	S_NORMAL (RPG1, 'C',	4, A_FireRpgMissile		, &States[S_DUKE_RPG1+2]),
	S_NORMAL (RPG1, 'B',	4, A_Light1 			, &States[S_DUKE_RPG1+3]),
	S_NORMAL (RPG1, 'A',	0, A_ReFire 			, &States[S_DUKE_RPG]),

#define S_DUKE_ITMRPG (S_DUKE_RPG1+4)
	S_NORMAL (DNRL, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ADukeRpg::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_misl,
	1,
	2,
	&States[S_DUKE_RPGUP],
	&States[S_DUKE_RPGDOWN],
	&States[S_DUKE_RPG],
	&States[S_DUKE_RPG1],
	&States[S_DUKE_RPG1],
	NULL,
	RUNTIME_CLASS(ADukeRpg),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukeRpg),
	-1
};

IMPLEMENT_ACTOR (ADukeRpg, Doom, 503, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMRPG)
END_DEFAULTS

AT_GAME_SET (DukeRpg)
{
	wpnlev1info[wp_duke_rpg] = wpnlev2info[wp_duke_rpg] = &ADukeRpg::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[5].AddWeapon (wp_duke_rpg, 6);
	}
}

weapontype_t ADukeRpg::OldStyleID () const
{
	return wp_duke_rpg;
}

const char *ADukeRpg::PickupMessage ()
{
	return GStrings(GOTLAUNCHER);
}

class ADukeRpgRocket : public AActor
{
	DECLARE_ACTOR (ADukeRpgRocket, AActor)
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState ADukeRpgRocket::States[] =
{
#define S_DUKE_RPGROCKET 0
	S_BRIGHT (RPGR, 'A',	1, NULL 						, &States[S_DUKE_RPGROCKET]),

#define S_DUKE_RPGROCKETX (S_DUKE_RPGROCKET+1)
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[S_DUKE_RPGROCKETX+1]),
	S_BRIGHT (MISL, 'C',	6, NULL							, &States[S_DUKE_RPGROCKETX+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ADukeRpgRocket, Doom, -1, 0)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (30)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_DUKE_RPGROCKET)
	PROP_DeathState (S_DUKE_RPGROCKETX)

	PROP_SeeSound ("weapons/dnrpg")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

void ADukeRpgRocket::BeginPlay ()
{
	Super::BeginPlay ();
	effects |= FX_ROCKET;
}

//
// A_FireRpgMissile
//
void A_FireRpgMissile (player_t *player, pspdef_t *psp)
{
	fixed_t x, y;
	AActor *missile;

	player->UseAmmo ();

	A_Light2 (player, psp);

	x = FixedMul (finecosine[player->mo->angle>>ANGLETOFINESHIFT], 12*FRACUNIT);
	y = FixedMul (finesine[player->mo->angle>>ANGLETOFINESHIFT], 12*FRACUNIT);

	Printf (PRINT_HIGH, "X: %i, Y: %i\n", x / FRACUNIT, y / FRACUNIT);

	missile = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ADukeRpgRocket));

	missile->x += x;
	missile->y += y;

	A_Light0 (player,psp);
}


// Duke's Pipebombs ---------------------------------------------------------

void A_FireDukePipe (player_t *, pspdef_t *);
void A_DetDukePipe (player_t *, pspdef_t *);
void A_CheckForScrapPipe (player_t *, pspdef_t *);
void A_Explode (AActor *);

class ADukePipe : public AWeapon
{
	DECLARE_ACTOR (ADukePipe, AWeapon)
	AT_GAME_SET_FRIEND (DukePipe)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukePipe::States[] =
{
#define S_DUKE_PIPE 0
	S_NORMAL (PIPB, 'A',	1, A_WeaponReady		, &States[S_DUKE_PIPE]),

#define S_DUKE_PIPEDOWN (S_DUKE_PIPE+1)
	S_NORMAL (PIPB, 'A',	1, A_Lower				, &States[S_DUKE_PIPEDOWN]),

#define S_DUKE_PIPEUP (S_DUKE_PIPEDOWN+1)
	S_NORMAL (PIPB, 'A',	1, A_Raise				, &States[S_DUKE_PIPEUP]),

#define S_DUKE_PIPE1 (S_DUKE_PIPEUP+1)
	S_NORMAL (PIPB, 'B',	4, NULL					, &States[S_DUKE_PIPE1+1]),
	S_NORMAL (PIPB, 'C',	4, A_FireDukePipe		, &States[S_DUKE_PIPE1+2]),
	S_NORMAL (PIPB, 'A',	0, A_ReFire				, &States[S_DUKE_PIPE]),

#define S_DUKE_ITMPIPE (S_DUKE_PIPE1+3)
	S_NORMAL (PIPE, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ADukePipe::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_misl,
	1,
	2,
	&States[S_DUKE_PIPEUP],
	&States[S_DUKE_PIPEDOWN],
	&States[S_DUKE_PIPE],
	&States[S_DUKE_PIPE1],
	&States[S_DUKE_PIPE1],
	NULL,
	RUNTIME_CLASS(ADukePipe),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukePipe),
	-1
};

IMPLEMENT_ACTOR (ADukePipe, Doom, 504, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMPIPE)
END_DEFAULTS

AT_GAME_SET (DukePipe)
{
	wpnlev1info[wp_duke_pipe] = wpnlev2info[wp_duke_pipe] = &ADukePipe::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[8].AddWeapon (wp_duke_pipe, 6);
		WeaponSlots[8].AddWeapon (wp_duke_pipe_det, 5);
	}
}

weapontype_t ADukePipe::OldStyleID () const
{
	return wp_duke_pipe;
}

const char *ADukePipe::PickupMessage ()
{
	return GStrings(GOTLAUNCHER);
}

class ADukePipeBomb : public AActor
{
	DECLARE_ACTOR (ADukePipeBomb, AActor)
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState ADukePipeBomb::States[] =
{
#define S_DUKE_PIPEBOMB 0
	S_NORMAL (PIPE, 'A',	1, NULL 						, &States[S_DUKE_PIPEBOMB]),

#define S_DUKE_PIPEBOMBX (S_DUKE_PIPEBOMB+1)
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[S_DUKE_PIPEBOMBX+1]),
	S_BRIGHT (MISL, 'C',	6, NULL		 					, &States[S_DUKE_PIPEBOMBX+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL),
};

IMPLEMENT_ACTOR (ADukePipeBomb, Doom, -1, 0)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (30)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT|MF2_FLOORBOUNCE)

	PROP_SpawnState (S_DUKE_PIPEBOMB)
	PROP_DeathState (S_DUKE_PIPEBOMB)
	PROP_XDeathState (S_DUKE_PIPEBOMBX)

	PROP_SeeSound ("weapons/dnpipe")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

void ADukePipeBomb::BeginPlay ()
{
	Super::BeginPlay ();
}

class ADukePipeDet : public AWeapon
{
	DECLARE_ACTOR (ADukePipeDet, AWeapon)
	AT_GAME_SET_FRIEND (DukePipeDet)
protected:
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukePipeDet::States[] =
{
#define S_DUKE_DET 0
	S_NORMAL (PIPB, 'D',	1, A_WeaponReady		, &States[S_DUKE_DET]),

#define S_DUKE_DETDOWN (S_DUKE_DET+1)
	S_NORMAL (PIPB, 'D',	1, A_Lower				, &States[S_DUKE_DETDOWN]),

#define S_DUKE_DETUP (S_DUKE_DETDOWN+1)
	S_NORMAL (PIPB, 'D',	1, A_Raise				, &States[S_DUKE_DETUP]),

#define S_DUKE_DET1 (S_DUKE_DETUP+1)
	S_NORMAL (PIPB, 'E',	4, NULL					, &States[S_DUKE_DET1+1]),
	S_NORMAL (PIPB, 'F',	4, A_DetDukePipe		, &States[S_DUKE_DET1+2]),
	S_NORMAL (PIPB, 'E',	4, A_CheckForScrapPipe	, &States[S_DUKE_DET1+3]),
	S_NORMAL (PIPB, 'D',	0, A_ReFire				, &States[S_DUKE_DET])
};

FWeaponInfo ADukePipeDet::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_noammo,
	0,
	0,
	&States[S_DUKE_DETUP],
	&States[S_DUKE_DETDOWN],
	&States[S_DUKE_DET],
	&States[S_DUKE_DET1],
	&States[S_DUKE_DET1],
	NULL,
	RUNTIME_CLASS(ADukePipeDet),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukePipeDet),
	-1
};

IMPLEMENT_ACTOR (ADukePipeDet, Doom, -1, 0)
END_DEFAULTS

AT_GAME_SET (DukePipeDet)
{
	wpnlev1info[wp_duke_pipe_det] = wpnlev2info[wp_duke_pipe_det] = &ADukePipeDet::WeaponInfo;
}

weapontype_t ADukePipeDet::OldStyleID () const
{
	return wp_duke_pipe_det;
}

//
// A_FireDukePipe
//
AActor *pipebomb[99];
int pipesfired;

void A_FireDukePipe (player_t *player, pspdef_t *psp)
{
	player->UseAmmo ();

	if (pipesfired <= 100)
	{
		pipesfired++;
		pipebomb[pipesfired - 1] = P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ADukePipeBomb));
		player->weaponowned[wp_duke_pipe_det] = true;
		player->pendingweapon = wp_duke_pipe_det;
	}
}

//
// A_DetDukePipe
//
void A_DetDukePipe (player_t *player, pspdef_t *psp)
{
	int i;

	if (pipesfired == 1)
	{
		pipebomb[0]->SetState (pipebomb[0]->XDeathState);
	} else if (pipesfired > 1) {
		for (i = 1; i <= pipesfired; i++)
		{
			pipebomb[i - 1]->SetState (pipebomb[i - 1]->XDeathState);
		}
	}
	pipesfired = 0;

	player->pendingweapon = wp_duke_pipe;
	player->weaponowned[wp_duke_pipe_det] = false;
}

//
// A_CheckForScrapPipe
//
void A_CheckForScrapPipe (player_t *player, pspdef_t *psp)
{
	int i;

	for (i = 0; i < 100; i++)
	{
		if (pipebomb[i] != NULL)
		{
			pipebomb[i]->Destroy ();
			pipebomb[i] = NULL;
		}
	}
}

// Duke's Devastator ---------------------------------------------------------

void A_FireDevLeft (player_t *, pspdef_t *);
void A_FireDevRight (player_t *, pspdef_t *);
void A_Explode (AActor *);

class ADukeDev : public AWeapon
{
	DECLARE_ACTOR (ADukeDev, AWeapon)
	AT_GAME_SET_FRIEND (DukeDev)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukeDev::States[] =
{
#define S_DUKE_DEV 0
	S_NORMAL (DEVS, 'A',	1, A_WeaponReady		, &States[S_DUKE_DEV]),

#define S_DUKE_DEVDOWN (S_DUKE_DEV+1)
	S_NORMAL (DEVS, 'A',	1, A_Lower				, &States[S_DUKE_DEVDOWN]),

#define S_DUKE_DEVUP (S_DUKE_DEVDOWN+1)
	S_NORMAL (DEVS, 'A',	1, A_Raise				, &States[S_DUKE_DEVUP]),

#define S_DUKE_DEV1 (S_DUKE_DEVUP+1)
	S_NORMAL (DEVS, 'B',	3, A_FireDevRight		, &States[S_DUKE_DEV1+1]),
	S_NORMAL (DEVS, 'C',	3, A_FireDevLeft		, &States[S_DUKE_DEV1+2]),
	S_NORMAL (DEVS, 'B',	0, A_ReFire				, &States[S_DUKE_DEV]),

#define S_DUKE_ITMDEV (S_DUKE_DEV1+3)
	S_NORMAL (DNDV, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ADukeDev::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_misl,
	1,
	2,
	&States[S_DUKE_DEVUP],
	&States[S_DUKE_DEVDOWN],
	&States[S_DUKE_DEV],
	&States[S_DUKE_DEV1],
	&States[S_DUKE_DEV1],
	NULL,
	RUNTIME_CLASS(ADukeDev),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukeDev),
	-1
};

IMPLEMENT_ACTOR (ADukeDev, Doom, 505, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMDEV)
END_DEFAULTS

AT_GAME_SET (DukeDev)
{
	wpnlev1info[wp_duke_dev] = wpnlev2info[wp_duke_dev] = &ADukeDev::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[5].AddWeapon (wp_duke_dev, 5);
	}
}

weapontype_t ADukeDev::OldStyleID () const
{
	return wp_duke_dev;
}

const char *ADukeDev::PickupMessage ()
{
	return GStrings(GOTLAUNCHER);
}

class ADukeDevRocket : public AActor
{
	DECLARE_ACTOR (ADukeDevRocket, AActor)
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState ADukeDevRocket::States[] =
{
#define S_DUKE_DEVROCKET 0
	S_BRIGHT (DEVR, 'A',	1, NULL 						, &States[S_DUKE_DEVROCKET]),

#define S_DUKE_DEVROCKETX (S_DUKE_DEVROCKET+1)
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[S_DUKE_DEVROCKETX+1]),
	S_BRIGHT (MISL, 'C',	6, NULL 						, &States[S_DUKE_DEVROCKETX+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ADukeDevRocket, Doom, -1, 0)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (40)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_DUKE_DEVROCKET)
	PROP_DeathState (S_DUKE_DEVROCKETX)

	PROP_SeeSound ("weapons/dnrpg")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

void ADukeDevRocket::BeginPlay ()
{
	Super::BeginPlay ();
	effects |= FX_ROCKET;
}

//
// A_FireDevLeft
//
void A_FireDevLeft (player_t *player, pspdef_t *psp)
{
	fixed_t x, y;

	player->UseAmmo ();

	A_Light2 (player, psp);

	x = FixedMul (finecosine[player->mo->angle>>ANGLETOFINESHIFT], 12*FRACUNIT);
	y = FixedMul (finesine[player->mo->angle>>ANGLETOFINESHIFT], 12*FRACUNIT);

	Printf (PRINT_HIGH, "X: %i, Y: %i\n", x / FRACUNIT, y / FRACUNIT);

	P_SpawnPlayerMissile (player->mo, player->mo->x + x, player->mo->y + y, player->mo->z, RUNTIME_CLASS(ADukeRpgRocket), player->mo->angle);
}

//
// A_FireDevRight
//
void A_FireDevRight (player_t *player, pspdef_t *psp)
{
	fixed_t x, y;

	player->UseAmmo ();

	A_Light2 (player, psp);

	x = FixedMul (finecosine[player->mo->angle>>ANGLETOFINESHIFT], -12*FRACUNIT);
	y = FixedMul (finesine[player->mo->angle>>ANGLETOFINESHIFT], -12*FRACUNIT);

	Printf (PRINT_HIGH, "X: %i, Y: %i\n", x / FRACUNIT, y / FRACUNIT);

	P_SpawnPlayerMissile (player->mo, player->mo->x + x, player->mo->y + y, player->mo->z, RUNTIME_CLASS(ADukeRpgRocket), player->mo->angle);
}

// Duke's Freezethrower ---------------------------------------------------------

void A_FireFreeze (player_t *, pspdef_t *);
void A_FreezeDie (AActor *);

class ADukeFreeze : public AWeapon
{
	DECLARE_ACTOR (ADukeFreeze, AWeapon)
	AT_GAME_SET_FRIEND (DukeFreeze)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ADukeFreeze::States[] =
{
#define S_DUKE_FRZ 0
	S_NORMAL (FREZ, 'A',	1, A_WeaponReady		, &States[S_DUKE_FRZ]),

#define S_DUKE_FRZDOWN (S_DUKE_FRZ+1)
	S_NORMAL (FREZ, 'A',	1, A_Lower				, &States[S_DUKE_FRZDOWN]),

#define S_DUKE_FRZUP (S_DUKE_FRZDOWN+1)
	S_NORMAL (FREZ, 'A',	1, A_Raise				, &States[S_DUKE_FRZUP]),

#define S_DUKE_FRZ1 (S_DUKE_FRZUP+1)
	S_NORMAL (FREZ, 'B',	1, NULL					, &States[S_DUKE_FRZ1+1]),
	S_NORMAL (FREZ, 'C',	1, A_FireFreeze			, &States[S_DUKE_FRZ1+2]),
	S_NORMAL (FREZ, 'D',	1, NULL					, &States[S_DUKE_FRZ1+3]),
	S_NORMAL (FREZ, 'B',	0, A_ReFire				, &States[S_DUKE_FRZ]),

#define S_DUKE_ITMFRZ (S_DUKE_FRZ1+4)
	S_NORMAL (DNFT, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ADukeFreeze::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_cell,
	1,
	2,
	&States[S_DUKE_FRZUP],
	&States[S_DUKE_FRZDOWN],
	&States[S_DUKE_FRZ],
	&States[S_DUKE_FRZ1],
	&States[S_DUKE_FRZ1],
	NULL,
	RUNTIME_CLASS(ADukeFreeze),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ADukeFreeze),
	-1
};

IMPLEMENT_ACTOR (ADukeFreeze, Doom, 506, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_DUKE_ITMFRZ)
END_DEFAULTS

AT_GAME_SET (DukeFreeze)
{
	wpnlev1info[wp_duke_freeze] = wpnlev2info[wp_duke_freeze] = &ADukeFreeze::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[6].AddWeapon (wp_duke_freeze, 7);
	}
}

weapontype_t ADukeFreeze::OldStyleID () const
{
	return wp_duke_freeze;
}

const char *ADukeFreeze::PickupMessage ()
{
	return GStrings(GOTLAUNCHER);
}

class ADukeFreezeBlast : public AActor
{
	DECLARE_ACTOR (ADukeFreezeBlast, AActor)
public:
	void BeginPlay ();
	int GetMOD () { return MOD_ROCKET; }
};

FState ADukeFreezeBlast::States[] =
{
#define S_DUKE_FBLAST 0
	S_BRIGHT (FICE, 'A',	1, NULL 						, &States[S_DUKE_FBLAST+1]),
	S_BRIGHT (FICE, 'B',	1, NULL 						, &States[S_DUKE_FBLAST+2]),
	S_BRIGHT (FICE, 'C',	1, NULL 						, &States[S_DUKE_FBLAST]),

#define S_DUKE_FBLASTX (S_DUKE_FBLAST+3)
	S_BRIGHT (FICE, 'A',	1, A_FreezeDie					, NULL)
};

IMPLEMENT_ACTOR (ADukeFreezeBlast, Doom, -1, 0)
	PROP_RadiusFixed (11)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (50)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_DUKE_FBLAST)
	PROP_DeathState (S_DUKE_FBLASTX)

	PROP_SeeSound ("weapons/dnfrez")
END_DEFAULTS

void ADukeFreezeBlast::BeginPlay ()
{
	Super::BeginPlay ();
}

//
// A_FireFreeze
//
void A_FireFreeze (player_t *player, pspdef_t *psp)
{
	player->UseAmmo ();

	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ADukeFreezeBlast));
}

//
// A_FreezeDie
//
void A_FreezeDie (AActor *self)
{
	if (self->target)
		self->target->SetShade (191, 223, 255);
}

// Wang's Fist ---------------------------------------------------------------------

void A_Punch (player_t *, pspdef_t *);

class ASwFist : public AWeapon
{
	DECLARE_ACTOR (ASwFist, AWeapon)
	AT_GAME_SET_FRIEND (SwFist)
protected:
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ASwFist::States[] =
{
#define S_SW_FIST 0
	S_NORMAL (SFST, 'A',	1, A_WeaponReady		, &States[S_SW_FIST]),
	S_NORMAL (SFST, 'B',	1, A_WeaponReady		, &States[S_SW_FIST+1]),

#define S_SW_FISTDOWN (S_SW_FIST+2)
	S_NORMAL (SFST, 'B',	1, A_Lower				, &States[S_SW_FISTDOWN]),

#define S_SW_FISTUP (S_SW_FISTDOWN+1)
	S_NORMAL (SFST, 'A',	1, A_Raise				, &States[S_SW_FISTUP]),

#define S_SW_FIST1 (S_SW_FISTUP+1)
	S_NORMAL (SFST, 'C',	2, NULL 				, &States[S_SW_FIST1+1]),
	S_NORMAL (SFST, 'D',	2, NULL					, &States[S_SW_FIST1+2]),
	S_NORMAL (SFST, 'E',	2, A_Punch				, &States[S_SW_FIST1+3]),
	S_NORMAL (SFST, 'D',	2, NULL		 			, &States[S_SW_FIST1+4]),
	S_NORMAL (SFST, 'C',	2, NULL					, &States[S_SW_FIST1+5]),
	S_NORMAL (SFST, 'F',	2, NULL 				, &States[S_SW_FIST1+6]),
	S_NORMAL (SFST, 'G',	2, NULL					, &States[S_SW_FIST1+7]),
	S_NORMAL (SFST, 'H',	2, A_Punch				, &States[S_SW_FIST1+8]),
	S_NORMAL (SFST, 'G',	2, NULL		 			, &States[S_SW_FIST1+9]),
	S_NORMAL (SFST, 'F',	2, NULL					, &States[S_SW_FIST1+10]),
	S_NORMAL (SFST, 'F',	0, A_ReFire				, &States[S_SW_FIST+1])
};

FWeaponInfo ASwFist::WeaponInfo =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_SW_FISTUP],
	&States[S_SW_FISTDOWN],
	&States[S_SW_FIST],
	&States[S_SW_FIST1],
	&States[S_SW_FIST1],
	NULL,
	NULL,
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ASwFist),
	-1
};

IMPLEMENT_ACTOR (ASwFist, Doom, -1, 0)
END_DEFAULTS

AT_GAME_SET (SwFist)
{
	wpnlev1info[wp_sw_fist] = wpnlev2info[wp_sw_fist] = &ASwFist::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[1].AddWeapon (wp_sw_fist, 5);
	}
}

weapontype_t ASwFist::OldStyleID () const
{
	return wp_sw_fist;
}

// Wang's Sword ---------------------------------------------------------------------

void A_SwSword (player_t *, pspdef_t *);

class ASwSword : public AWeapon
{
	DECLARE_ACTOR (ASwSword, AWeapon)
	AT_GAME_SET_FRIEND (SwSword)
protected:
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ASwSword::States[] =
{
#define S_SW_SWRD 0
	S_NORMAL (SWRD, 'A',	1, A_WeaponReady		, &States[S_SW_SWRD]),

#define S_SW_SWRDDOWN (S_SW_SWRD+1)
	S_NORMAL (SWRD, 'A',	1, A_Lower				, &States[S_SW_SWRDDOWN]),

#define S_SW_SWRDUP (S_SW_SWRDDOWN+1)
	S_NORMAL (SWRD, 'A',	1, A_Raise				, &States[S_SW_SWRDUP]),

#define S_SW_SWRD1 (S_SW_SWRDUP+1)
	S_NORMAL (SWRD, 'B',	4, NULL 				, &States[S_SW_SWRD1+1]),
	S_NORMAL (SWRD, 'C',	4, NULL					, &States[S_SW_SWRD1+2]),
	S_NORMAL (SWRD, 'D',	4, A_SwSword			, &States[S_SW_SWRD1+3]),
	S_NORMAL (SWRD, 'E',	4, NULL					, &States[S_SW_SWRD1+4]),
	S_NORMAL (SWRD, 'A',	0, A_ReFire				, &States[S_SW_SWRD]),
};

FWeaponInfo ASwSword::WeaponInfo =
{
	0,
	am_noammo,
	0,
	0,
	&States[S_SW_SWRDUP],
	&States[S_SW_SWRDDOWN],
	&States[S_SW_SWRD],
	&States[S_SW_SWRD1],
	&States[S_SW_SWRD1],
	NULL,
	NULL,
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ASwSword),
	-1
};

IMPLEMENT_ACTOR (ASwSword, Doom, -1, 0)
END_DEFAULTS

AT_GAME_SET (SwSword)
{
	wpnlev1info[wp_sw_sword] = wpnlev2info[wp_sw_sword] = &ASwSword::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[1].AddWeapon (wp_sw_sword, 4);
	}
}

weapontype_t ASwSword::OldStyleID () const
{
	return wp_sw_sword;
}

//
// A_SwSword
//
void A_SwSword (player_t *player, pspdef_t *psp)
{
	angle_t 	angle;
	int 		damage;

	player->UseAmmo ();

	damage = 5 * (P_Random (pr_saw)%10+1);
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
}

// Wang's Shurikens ----------------------------------------------------------------

void A_ThrowShuriken (player_t *, pspdef_t *);

class ASwShur : public AWeapon
{
	DECLARE_ACTOR (ASwShur, AWeapon)
	AT_GAME_SET_FRIEND (SwShur)
protected:
	const char *PickupMessage ();
	weapontype_t OldStyleID () const;
private:
	static FWeaponInfo WeaponInfo;
};

FState ASwShur::States[] =
{
#define S_SW_SHUR 0
	S_NORMAL (HSHR, 'A',	1, A_WeaponReady		, &States[S_SW_SHUR]),

#define S_SW_SHURDOWN (S_SW_SHUR+1)
	S_NORMAL (HSHR, 'A',	1, A_Lower				, &States[S_SW_SHURDOWN]),

#define S_SW_SHURUP (S_SW_SHURDOWN+1)
	S_NORMAL (HSHR, 'A',	1, A_Raise				, &States[S_SW_SHURUP]),

#define S_SW_SHUR1 (S_SW_SHURUP+1)
	S_NORMAL (HSHR, 'B',	1, NULL					, &States[S_SW_SHUR1+1]),
	S_NORMAL (HSHR, 'C',	1, NULL					, &States[S_SW_SHUR1+2]),
	S_NORMAL (HSHR, 'D',	1, A_ThrowShuriken		, &States[S_SW_SHUR1+3]),
	S_NORMAL (HSHR, 'E',	1, NULL					, &States[S_SW_SHUR1+4]),
	S_NORMAL (HSHR, 'F',	1, NULL					, &States[S_SW_SHUR1+5]),
	S_NORMAL (HSHR, 'G',	1, NULL					, &States[S_SW_SHUR1+6]),
	S_NORMAL (HSHR, 'H',	1, NULL 				, &States[S_SW_SHUR1+7]),
	S_NORMAL (HSHR, 'H',	0, A_ReFire				, &States[S_SW_SHUR]),

#define S_SW_ITMSHUR (S_SW_SHUR1+7)
	S_NORMAL (LAUN, 'A',   -1, NULL 				, NULL)
};

FWeaponInfo ASwShur::WeaponInfo =
{
	WIF_NOAUTOFIRE,
	am_clip,
	1,
	2,
	&States[S_SW_SHURUP],
	&States[S_SW_SHURDOWN],
	&States[S_SW_SHUR],
	&States[S_SW_SHUR1],
	&States[S_SW_SHUR1],
	NULL,
	RUNTIME_CLASS(ASwShur),
	100,
	0,
	NULL,
	NULL,
	RUNTIME_CLASS(ASwShur),
	-1
};

IMPLEMENT_ACTOR (ASwShur, Doom, 507, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (16)
	PROP_Flags (MF_SPECIAL)

	PROP_SpawnState (S_SW_ITMSHUR)
END_DEFAULTS

AT_GAME_SET (SwShur)
{
	wpnlev1info[wp_sw_shur] = wpnlev2info[wp_sw_shur] = &ASwShur::WeaponInfo;
	if (gameinfo.gametype == GAME_Doom)
	{
		WeaponSlots[2].AddWeapon (wp_sw_shur, 1);
	}
}

weapontype_t ASwShur::OldStyleID () const
{
	return wp_sw_shur;
}

const char *ASwShur::PickupMessage ()
{
	return GStrings(GOTSHOTGUN);
}

class ASwShuriken : public AActor
{
	DECLARE_ACTOR (ASwShuriken, AActor)
public:
	int GetMOD () { return MOD_ROCKET; }
};

FState ASwShuriken::States[] =
{
#define S_SW_SHURIKEN 0
	S_NORMAL (SHUR, 'A',	1, NULL 						, &States[S_SW_SHURIKEN+1]),
	S_NORMAL (SHUR, 'B',	1, NULL 						, &States[S_SW_SHURIKEN+2]),
	S_NORMAL (SHUR, 'C',	1, NULL 						, &States[S_SW_SHURIKEN+3]),
	S_NORMAL (SHUR, 'D',	1, NULL 						, &States[S_SW_SHURIKEN]),

#define S_SW_SHURIKEN_DIE (S_SW_SHURIKEN+4)
	S_NORMAL (SHUR, 'A',	0, NULL							, NULL)
};

IMPLEMENT_ACTOR (ASwShuriken, Doom, -1, 0)
	PROP_RadiusFixed (1)
	PROP_HeightFixed (2)
	PROP_SpeedFixed (40)
	PROP_Damage (20)
	PROP_Flags (MF_MISSILE|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_SW_SHURIKEN)
	PROP_DeathState (S_SW_SHURIKEN_DIE)

	PROP_SeeSound ("weapons/rocklf")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

//
// A_ThrowShuriken
//
void A_ThrowShuriken (player_t *player, pspdef_t *psp)
{
	player->UseAmmo ();
	P_SpawnPlayerMissile (player->mo, RUNTIME_CLASS(ASwShuriken));
}
