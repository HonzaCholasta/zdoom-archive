#include "c_dispatch.h"
#include "w_wad.h"
#include "z_zone.h"
#include "doomstat.h"
#include "templates.h"
#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_enemy.h"
#include "gstrings.h"
#include "p_effect.h"
#include "v_video.h"
#include "sc_man.h"
#include "d_player.h"
#include "c_cvars.h"
#include "p_trace.h"
#include "i_system.h"
#include "p_lnspec.h"
#include "r_sky.h"
#include "a_doomglobal.h"

#include <math.h>
#include "p_grubber.h"

//
// Shell Ejecting
//

class AEjectBullet : public AActor
{
	DECLARE_ACTOR (AEjectBullet, AActor)
public:
	void BeginPlay ();
};

FState AEjectBullet::States[] =
{
#define S_BULLET 0
	S_NORMAL (PBUL, 'A',	3, NULL 						, &States[S_BULLET+1]),
	S_NORMAL (PBUL, 'B',	3, NULL							, &States[S_BULLET+2]),
	S_NORMAL (PBUL, 'C',	3, NULL							, &States[S_BULLET+3]),
	S_NORMAL (PBUL, 'D',	3, NULL							, &States[S_BULLET]),
#define S_BULLET_DIE (S_BULLET+4)
	S_NORMAL (PBUL, 'A',	1, NULL							, &States[S_BULLET_DIE])
};

IMPLEMENT_ACTOR (AEjectBullet, Doom, -1, 0)
	PROP_RadiusFixed (1)
	PROP_HeightFixed (1)
	PROP_Flags (MF_MISSILE|MF_DROPOFF)
	PROP_Flags2 (MF2_IMPACT|MF2_NOTELEPORT|MF2_FLOORBOUNCE)

	PROP_SpawnState (S_BULLET)
	PROP_DeathState (S_BULLET_DIE)
	PROP_XDeathState (S_BULLET_DIE)
END_DEFAULTS

void AEjectBullet::BeginPlay ()
{
	Super::BeginPlay ();
	SetState (&States[S_BULLET+M_Random()&3]);
}

class AEjectShell : public AActor
{
	DECLARE_ACTOR (AEjectShell, AActor)
public:
	void BeginPlay ();
};

FState AEjectShell::States[] =
{
#define S_SHELL 0
	S_NORMAL (PSHE, 'A',	3, NULL 						, &States[S_SHELL+1]),
	S_NORMAL (PSHE, 'B',	3, NULL							, &States[S_SHELL+2]),
	S_NORMAL (PSHE, 'C',	3, NULL							, &States[S_SHELL+3]),
	S_NORMAL (PSHE, 'D',	3, NULL							, &States[S_SHELL]),
#define S_SHELL_DIE (S_SHELL+4)
	S_NORMAL (PSHE, 'A',	1, NULL							, &States[S_SHELL_DIE])
};

IMPLEMENT_ACTOR (AEjectShell, Doom, -1, 0)
	PROP_RadiusFixed (1)
	PROP_HeightFixed (1)
	PROP_Flags (MF_MISSILE|MF_DROPOFF)
	PROP_Flags2 (MF2_IMPACT|MF2_NOTELEPORT|MF2_FLOORBOUNCE)

	PROP_SpawnState (S_SHELL)
	PROP_DeathState (S_SHELL_DIE)
	PROP_XDeathState (S_SHELL_DIE)
END_DEFAULTS

void AEjectShell::BeginPlay ()
{
	Super::BeginPlay ();
	SetState (&States[S_SHELL+M_Random()&3]);
}

CVAR (Bool, cl_shellejecting, true, CVAR_ARCHIVE);

void P_EjectShell (AActor *actor, int type, fixed_t dist)
{
	angle_t ang = actor->angle - ANGLE_90;
	AActor *missile;

	if (!cl_shellejecting)
		return;

	missile = P_SpawnPlayerMissile (actor, actor->x, actor->y, actor->z, type ? RUNTIME_CLASS(AEjectBullet) : RUNTIME_CLASS(AEjectShell), ang);

	missile->momx = FixedMul (FRACUNIT*dist/2 + (P_Random() << 10), finecosine[ang>>ANGLETOFINESHIFT]);
	missile->momy = FixedMul (FRACUNIT*dist/2 + (P_Random() << 10), finesine[ang>>ANGLETOFINESHIFT]);
	missile->momz = FRACUNIT*dist/2 + (P_Random() << 10);
}

void A_EjectShell (AActor *self)
{
	P_EjectShell (self, 0, 5);
}

void A_EjectBullet (AActor *self)
{
	P_EjectShell (self, 1, 5);
}

/*
CCMD (tracetest)
{
	AActor *plr = players[consoleplayer].mo;
	FTraceResults trace;
	int x, y, z, vx, vy, vz;
	angle_t angle, pitch;

	pitch = (angle_t)(-plr->pitch) >> ANGLETOFINESHIFT;
	angle = plr->angle >> ANGLETOFINESHIFT;

	vx = FixedMul (finecosine[pitch], finecosine[angle]);
	vy = FixedMul (finecosine[pitch], finesine[angle]);
	vz = finesine[pitch];

	x = plr->x;
	y = plr->y;
	z = plr->z - plr->floorclip + (plr->height >> 1) + 8*FRACUNIT;

	Trace (x, y, z, plr->Sector, vx, vy, vz, MISSILERANGE,
		MF_SHOOTABLE, ML_BLOCKEVERYTHING, plr, trace, 0);

//	Printf (PRINT_HIGH, "Player: [%i,%i,%i], Trace: [%i,%i,%i]\n",
//		plr->x / FRACUNIT, plr->y / FRACUNIT, plr->z / FRACUNIT,
//		trace.X / FRACUNIT, trace.Y / FRACUNIT, trace.Z / FRACUNIT);

	if (trace.Sector->ceilingplane.ZatPoint (trace.X, trace.Y) -
		trace.Sector->floorplane.ZatPoint (trace.X, trace.Y) >=
		plr->height)
	{
		plr->x = trace.X - FixedMul (plr->radius, finecosine[angle]);
		plr->y = trace.Y - FixedMul (plr->radius, finesine[angle]);
		plr->z = trace.Z;
	}

	if (plr->z < plr->Sector->floorplane.ZatPoint (plr->x, plr->y))
		plr->z = plr->Sector->floorplane.ZatPoint (plr->z, plr->y);
}
*/

//
// Weapon options
//

CVAR (Bool, cl_wpn_fist_thrust, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_chainsaw_throw, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_pistol_burst, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_pistol_expuffs, false, CVAR_ARCHIVE);
CVAR (Int, cl_wpn_pistol_spread, 1, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_shotgun_auto, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_shotgun_expuffs, false, CVAR_ARCHIVE);
CVAR (Int, cl_wpn_shotgun_spread, 2, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_sshotgun_fireone, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_sshotgun_auto, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_sshotgun_expuffs, false, CVAR_ARCHIVE);
CVAR (Int, cl_wpn_sshotgun_spread, 3, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_chaingun_shell, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_chaingun_expuffs, false, CVAR_ARCHIVE);
CVAR (Int, cl_wpn_chaingun_spread, 1, CVAR_ARCHIVE);
CVAR (Int, cl_wpn_rocket_mode, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_rocket_spreadfire, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_rocket_bounce, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_rocket_guide, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_rocket_killable, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_rocket_bobbing, false, CVAR_ARCHIVE);
CVAR (Int, cl_wpn_plasma_mode, 0, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_plasma_spreadfire, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_plasma_green, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_plasma_bounce, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_plasma_bobbing, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_plasma_killable, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_bfg_spreadfire, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_bfg_mini, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_bfg_bounce, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_bfg_bobbing, false, CVAR_ARCHIVE);
CVAR (Bool, cl_wpn_bfg_killable, false, CVAR_ARCHIVE);

void A_Guide (AActor *self)
{
	AActor *plr = players[consoleplayer].mo;
	FTraceResults trace;
	int x, y, z, vx, vy, vz;
	angle_t ang, angle, pitch;

	pitch = (angle_t)(-plr->pitch) >> ANGLETOFINESHIFT;
	angle = plr->angle >> ANGLETOFINESHIFT;

	vx = FixedMul (finecosine[pitch], finecosine[angle]);
	vy = FixedMul (finecosine[pitch], finesine[angle]);
	vz = finesine[pitch];

	x = self->x;
	y = self->y;
	z =	self->z - self->floorclip + (self->height >> 1) + 8*FRACUNIT;

	if (cl_wpn_rocket_guide)
	{
		Trace (x, y, z, plr->Sector, vx, vy, vz, MISSILERANGE,
			MF_SOLID, ML_BLOCKEVERYTHING, plr, trace, 0);

		ang = R_PointToAngle2 (self->x, self->y, trace.X, trace.Y);
		self->angle = ang;
		self->pitch = plr->pitch;
		self->momx = FixedMul (self->Speed, finecosine[ang>>ANGLETOFINESHIFT]);
		self->momy = FixedMul (self->Speed, finesine[ang>>ANGLETOFINESHIFT]);
		self->momz = FixedMul (self->Speed, finesine[pitch]);
	}
}

class AExplosion : public AActor
{
	DECLARE_ACTOR (AExplosion, AActor)
public:
	int GetMOD () { return MOD_ROCKET; }
};

FState AExplosion::States[] =
{
	S_BRIGHT (MISL, 'A',	0, NULL							, &States[1]),
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[2]),
	S_BRIGHT (MISL, 'C',	6, NULL 						, &States[3]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL)
};

IMPLEMENT_ACTOR (AExplosion, Doom, -1, 0)
	PROP_RadiusFixed (1)
	PROP_HeightFixed (1)
	PROP_Damage (20)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (0)
	PROP_DeathState (0)

	PROP_SeeSound ("weapons/rocklx")
END_DEFAULTS

void P_SpawnExplosion (AActor *self)
{
	FTraceResults pnt;
	int x, y, z, vx, vy, vz;
	angle_t angle, pitch;

	pitch = (angle_t)(-self->pitch) >> ANGLETOFINESHIFT;
	//pitch += PS_Random (pr_fireshotgun2) << 18;
	angle = self->angle >> ANGLETOFINESHIFT;
	//angle += PS_Random (pr_fireshotgun2) << 19;

	vx = FixedMul (finecosine[pitch], finecosine[angle]);
	vy = FixedMul (finecosine[pitch], finesine[angle]);
	vz = finesine[pitch];

	x = self->x;
	y = self->y;
	z =	self->z - self->floorclip + (self->height >> 1) + 8*FRACUNIT;

	Trace (x, y, z, self->Sector, vx, vy, vz, MISSILERANGE,
		MF_SOLID, ML_BLOCKEVERYTHING, self, pnt, 0);

	if (self->player && self->player->ammo[am_misl] > 0)
	{
		P_SpawnPlayerMissile (self, pnt.X, pnt.Y, pnt.Z, RUNTIME_CLASS(AExplosion), self->angle);
		self->player->ammo[am_misl]--;
	}
}

//
// Monster options
//
CVAR (Int, cl_mon_zombieman_fire, 0, CVAR_ARCHIVE);
CVAR (Int, cl_mon_shotgunguy_fire, 1, CVAR_ARCHIVE);
CVAR (Int, cl_mon_heavyweapondude_fire, 3, CVAR_ARCHIVE);
CVAR (Int, cl_mon_imp_fire, 10, CVAR_ARCHIVE);
//CVAR (Int, cl_mon_demon_fire, 0, CVAR_ARCHIVE);
CVAR (Int, cl_mon_lostsoul_fire, 16, CVAR_ARCHIVE);
CVAR (Int, cl_mon_cacodemon_fire, 9, CVAR_ARCHIVE);
CVAR (Int, cl_mon_hellknight_fire, 8, CVAR_ARCHIVE);
CVAR (Int, cl_mon_baronofhell_fire, 8, CVAR_ARCHIVE);
CVAR (Int, cl_mon_arachnotron_fire, 7, CVAR_ARCHIVE);
CVAR (Int, cl_mon_painelemental_fire, 12, CVAR_ARCHIVE);
CVAR (Int, cl_mon_revenant_fire, 13, CVAR_ARCHIVE);
CVAR (Int, cl_mon_mancubus_fire, 11, CVAR_ARCHIVE);
//CVAR (Int, cl_mon_archvile_fire, 0, CVAR_ARCHIVE);
CVAR (Int, cl_mon_spiderboss_fire, 1, CVAR_ARCHIVE);
CVAR (Int, cl_mon_cyberdemon_fire, 4, CVAR_ARCHIVE);
CVAR (Int, cl_mon_ss_fire, 3, CVAR_ARCHIVE);

AActor *Grb_PosAttack (AActor *self, int angle, int pitch);
AActor *Grb_SPosAttack (AActor *self, int angle, int pitch);
AActor *Grb_CPosAttack (AActor *self, int angle, int pitch);
AActor *Grb_TroopAttack (AActor *self);
AActor *Grb_FatAttack (AActor *self);
AActor *Grb_HeadAttack (AActor *self);
AActor *Grb_BruisAttack (AActor *self);
AActor *Grb_BspiAttack (AActor *self);
AActor *Grb_PainAttack (AActor *self, int angle);
AActor *Grb_SkelMissile (AActor *self);
AActor *Grb_CyberAttack (AActor *self);
AActor *Grb_SkullAttack (AActor *self, int angle);

AActor *P_MonsterFire (int cvar, AActor *self, int angle, int slope)
{
	int i, slopeb;
	AActor *ret = NULL;

	switch (cvar)
	{
	case 0:
		Grb_PosAttack (self, angle, slope);
		break;
	case 1:
		Grb_SPosAttack (self, angle, slope);
		break;
	case 2:
		for (i=0; i<2; i++)
		{
			slopeb = slope + (PS_Random (pr_fireshotgun2) << 18);
			Grb_SPosAttack (self, angle, slopeb);
		}
		break;
	case 3:
		Grb_CPosAttack (self, angle, slope);
		break;
	case 4:
		ret = Grb_CyberAttack (self);
		break;
	case 5:
		ret = P_SpawnMissile (self, self->target, RUNTIME_CLASS(APlasmaBall));
		break;
	case 6:
		ret = P_SpawnMissile (self, self->target, RUNTIME_CLASS(ABFGBall));
		break;
	case 7:
		ret = Grb_BspiAttack (self);
		break;
	case 8:
		ret = Grb_BruisAttack (self);
		break;
	case 9:
		ret = Grb_HeadAttack (self);
		break;
	case 10:
		ret = Grb_TroopAttack (self);
		break;
	case 11:
		ret = Grb_FatAttack (self);
		break;
	case 12:
		ret = Grb_PainAttack (self, angle);
		break;
	case 13:
		ret = Grb_SkelMissile (self);
		break;
	case 14:
		self->damage = 150;
		A_MonsterRail (self);
		break;
	case 15:
		for (i = 0; i < 8; i++)
			ret = P_SpawnMissileAngle (self, RUNTIME_CLASS(ARocket), angle + (PS_Random (pr_cposattack) << 20), ((M_Random () - 128) * FRACUNIT / 50));
		break;
	case 16:
		ret = Grb_SkullAttack (self, angle);
		break;
	}

	return ret;
}

//
// Linear map teleporter stuff
// Teleport_LineMap (map, position, endspot)
//

bool PosSaved;
angle_t EndAngle;
fixed_t EndPitch;
fixed_t EndDist;
angle_t PlrAngle;
fixed_t PlrPitch;
angle_t PlrMomAng;
fixed_t PlrMomDist;
fixed_t PlrMomZ;
pspdef_t PlrPsp[NUMPSPRITES];

void P_SavePosData (WORD tid)
{
	FActorIterator iterator (tid);
	FTraceResults trace;
	AActor *endspot;
	AActor *pmo = players[consoleplayer].mo;
	int i;

	if (!(endspot = iterator.Next ()))
	{
		PosSaved = false;
		return;
	}

	Trace (endspot->x, endspot->y, endspot->z, endspot->Sector,
		pmo->x, pmo->y, pmo->z, MISSILERANGE, MF_SOLID, 0, endspot,
		trace, 0);

	EndAngle = R_PointToAngle2 (endspot->x, endspot->y, trace.X, trace.Y) - endspot->angle;
	EndPitch = -(fixed_t)R_PointToAngle2 (0, endspot->z, 0, trace.Z);
	EndDist = trace.Distance;
	PlrAngle = pmo->angle - endspot->angle;
	PlrPitch = pmo->pitch;
	PlrMomAng = R_PointToAngle2 (0, 0, pmo->momx, pmo->momy) - endspot->angle;
	PlrMomDist = (fixed_t)sqrt (pmo->momx * pmo->momx + pmo->momy * pmo->momy);
	PlrMomZ = pmo->momz;
	for (i = 0; i < NUMPSPRITES; i++)
		PlrPsp[i] = pmo->player->psprites[i];

	PosSaved = true;
}

/*
void P_LoadPosData ()
{
	AActor *pmo = players[displayplayer].mo;

	pmo->x += (EndDist, finesine[(pmo->angle + EndAngle) >> ANGLETOFINESHIFT]);
	pmo->y += (EndDist, finecosine[(pmo->angle + EndAngle) >> ANGLETOFINESHIFT]);
	pmo->angle += PlrAngle;
	pmo->pitch = PlrPitch;
}
*/

//
// Sky changing
//

CCMD (sky1)
{
	if (argv.argc() < 2)
	{
		Printf ("Usage: sky1 <texture>\n");
		return;
	}

	sky1texture = R_TextureNumForName (argv[1]);

	R_InitSkyMap ();
}

CCMD (sky2)
{
	if (argv.argc() < 2)
	{
		Printf ("Usage: sky2 <texture>\n");
		return;
	}

	sky2texture = R_TextureNumForName (argv[1]);

	R_InitSkyMap ();
}
