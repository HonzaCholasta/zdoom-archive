#include "info.h"
#include "a_pickups.h"
#include "a_artifacts.h"
#include "hstrings.h"
#include "p_local.h"
#include "gi.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "m_random.h"
#include "a_sharedglobal.h"

#define MORPHTICS (40*TICRATE)

//---------------------------------------------------------------------------
//
// FUNC P_MorphPlayer
//
// Returns true if the player gets turned into a chicken/pig.
//
//---------------------------------------------------------------------------

bool P_MorphPlayer (player_t *p)
{
	AActor *morphed;
	AActor *actor;
	const TypeInfo *spawntype;

	actor = p->mo;
	if (actor->flags3 & MF3_DONTMORPH)
	{
		return false;
	}
	if (p->powers[pw_invulnerability])
	{ // Immune when invulnerable
		return false;
	}
	if (p->morphTics)
	{ // Player is already a beast
		return false;
	}

	if (gameinfo.gametype == GAME_Heretic)
		spawntype = TypeInfo::FindType ("ChickenPlayer");
	else
		spawntype = TypeInfo::FindType ("PigPlayer");

	if (spawntype == NULL)
	{
		return false;
	}

	morphed = Spawn (spawntype, actor->x, actor->y, actor->z);
	morphed->angle = actor->angle;
	morphed->target = actor->target;
	morphed->tracer = actor;
	morphed->special1 = p->readyweapon;
	morphed->special2 = actor->flags & ~MF_JUSTHIT;
	morphed->player = p;
	if (actor->flags2 & MF2_DONTDRAW)
	{
		morphed->special2 |= MF_JUSTHIT;
	}
	if (actor->flags2 & MF2_FLY)
	{
		morphed->flags |= MF2_FLY;
	}
	if (actor->flags3 & MF3_GHOST)
	{
		morphed->flags3 |= MF3_GHOST;
	}
	if (actor->flags & MF_SHADOW)
	{
		morphed->flags |= MF_SHADOW;
	}
	Spawn<ATeleportFog> (actor->x, actor->y, actor->z + TELEFOGHEIGHT);
	actor->player = NULL;
	actor->flags &= ~(MF_SOLID|MF_SHOOTABLE);
	actor->flags |= MF_UNMORPHED;
	actor->flags2 |= MF2_DONTDRAW;
	p->morphTics = MORPHTICS;
	p->health = morphed->health;
	p->mo = static_cast<APlayerPawn *>(morphed);
	memset (&p->armorpoints[0], 0, NUMARMOR*sizeof(int));
	P_ActivateMorphWeapon (p);
	if (p->camera == actor)
	{
		p->camera = morphed;
	}
	return true;
}

//----------------------------------------------------------------------------
//
// FUNC P_UndoPlayerMorph
//
//----------------------------------------------------------------------------

bool P_UndoPlayerMorph (player_t *player, bool force)
{
	APlayerPawn *mo;
	AActor *pmo;
	angle_t angle;

	pmo = player->mo;
	if (pmo->tracer == NULL)
	{
		return false;
	}
	mo = static_cast<APlayerPawn *>(pmo->tracer);
	mo->SetOrigin (pmo->x, pmo->y, pmo->z);
	mo->flags |= MF_SOLID;
	pmo->flags &= ~MF_SOLID;
	if (!force && P_TestMobjLocation (mo) == false)
	{ // Didn't fit
		mo->flags &= ~MF_SOLID;
		pmo->flags |= MF_SOLID;
		player->morphTics = 2*TICRATE;
		return false;
	}
	pmo->player = NULL;

	mo->angle = pmo->angle;
	mo->player = player;
	mo->reactiontime = 18;
	mo->flags = pmo->special2 & ~MF_JUSTHIT;
	mo->momx = 0;
	mo->momy = 0;
	player->momx = 0;
	player->momy = 0;
	mo->momz = pmo->momz;
	if (!(pmo->special2 & MF_JUSTHIT))
	{
		mo->flags2 &= ~MF2_DONTDRAW;
	}

	player->morphTics = 0;
	player->powers[pw_weaponlevel2] = 0;
	player->health = mo->health = GetInfo (mo)->spawnhealth;
	player->mo = mo;
	if (player->camera == pmo)
	{
		player->camera = mo;
	}
	angle = mo->angle >> ANGLETOFINESHIFT;
	Spawn<ATeleportFog> (pmo->x + 20*finecosine[angle],
		pmo->y + 20*finesine[angle], pmo->z + TELEFOGHEIGHT);
	P_PostMorphWeapon (player, (weapontype_t)pmo->special1);
	pmo->tracer = NULL;
	pmo->Destroy ();
	return true;
}

//---------------------------------------------------------------------------
//
// FUNC P_MorphMonster
//
// Returns true if the monster gets turned into a chicken/pig.
//
//---------------------------------------------------------------------------

bool P_MorphMonster (AActor *actor)
{
	AActor *morphed;
	const TypeInfo *spawntype;

	if (actor->player || actor->flags3 & MF3_DONTMORPH)
	{
		return false;
	}

	if (gameinfo.gametype == GAME_Heretic)
		spawntype = TypeInfo::FindType ("Chicken");
	else
		spawntype = TypeInfo::FindType ("Pig");

	morphed = Spawn (spawntype, actor->x, actor->y, actor->z);
	morphed->tid = actor->tid;
	morphed->angle = actor->angle;
	morphed->target = actor->target;
	morphed->tracer = actor;
	morphed->special1 = MORPHTICS + P_Random();
	morphed->special2 = actor->flags & ~MF_JUSTHIT;
	morphed->special = actor->special;
	memcpy (morphed->args, actor->args, sizeof(actor->args));
	if (actor->flags2 & MF2_DONTDRAW)
	{
		morphed->special2 |= MF_JUSTHIT;
	}
	if (actor->flags3 & MF3_GHOST)
	{
		morphed->flags3 |= MF3_GHOST;
		if (actor->flags & MF_SHADOW)
			morphed->flags |= MF_SHADOW;
	}
	morphed->AddToHash ();
	actor->RemoveFromHash ();
	actor->tid = 0;
	actor->flags &= ~(MF_SOLID|MF_SHOOTABLE);
	actor->flags |= MF_UNMORPHED;
	actor->flags2 |= MF2_DONTDRAW;
	Spawn<ATeleportFog> (actor->x, actor->y, actor->z + TELEFOGHEIGHT);
	return true;
}

//----------------------------------------------------------------------------
//
// FUNC P_UpdateMorphedMonster
//
// Returns true if the monster unmorphs.
//
//----------------------------------------------------------------------------

bool P_UpdateMorphedMonster (AActor *beast, int tics)
{
	AActor *actor;

	beast->special1 -= tics;
	if (beast->special1 > 0 ||
		beast->tracer == NULL ||
		beast->flags3 & MF3_STAYMORPHED)
	{
		return false;
	}
	actor = beast->tracer;
	actor->SetOrigin (beast->x, beast->y, beast->z);
	actor->flags |= MF_SOLID;
	beast->flags &= ~MF_SOLID;
	if (P_TestMobjLocation (actor) == false)
	{ // Didn't fit
		actor->flags &= ~MF_SOLID;
		beast->flags |= MF_SOLID;
		beast->special1 = 5*TICRATE; // Next try in 5 seconds
		return false;
	}
	actor->angle = beast->angle;
	actor->target = beast->target;
	actor->flags = beast->special2 & ~MF_JUSTHIT;
	if (!(beast->special2 & MF_JUSTHIT))
		actor->flags2 &= ~MF2_DONTDRAW;
	actor->health = GetInfo (actor)->spawnhealth;
	actor->momx = beast->momx;
	actor->momy = beast->momy;
	actor->momz = beast->momz;
	actor->tid = beast->tid;
	actor->special = beast->special;
	memcpy (actor->args, beast->args, sizeof(actor->args));
	actor->AddToHash ();
	beast->tracer = NULL;
	beast->Destroy ();
	Spawn<ATeleportFog> (beast->x, beast->y, beast->z + TELEFOGHEIGHT);
	return true;
}

// Egg ----------------------------------------------------------------------

class AEggFX : public AActor
{
	DECLARE_ACTOR (AEggFX, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (AEggFX, AActor);
REGISTER_ACTOR (AEggFX, Raven);

FState AEggFX::States[] =
{
#define S_EGGFX 0
	S_NORMAL (EGGM, 'A',	4, NULL, &States[S_EGGFX+1]),
	S_NORMAL (EGGM, 'B',	4, NULL, &States[S_EGGFX+2]),
	S_NORMAL (EGGM, 'C',	4, NULL, &States[S_EGGFX+3]),
	S_NORMAL (EGGM, 'D',	4, NULL, &States[S_EGGFX+4]),
	S_NORMAL (EGGM, 'E',	4, NULL, &States[S_EGGFX+0]),

#define S_EGGFXI1 (S_EGGFX+5)
	S_BRIGHT (FX01, 'E',	3, NULL, &States[S_EGGFXI1+1]),
	S_BRIGHT (FX01, 'F',	3, NULL, &States[S_EGGFXI1+2]),
	S_BRIGHT (FX01, 'G',	3, NULL, &States[S_EGGFXI1+3]),
	S_BRIGHT (FX01, 'H',	3, NULL, NULL),

#define S_EGGFXI2 (S_EGGFXI1+4)
	S_BRIGHT (FHFX, 'I',	3, NULL, &States[S_EGGFXI2+1]),
	S_BRIGHT (FHFX, 'J',	3, NULL, &States[S_EGGFXI2+2]),
	S_BRIGHT (FHFX, 'K',	3, NULL, &States[S_EGGFXI2+3]),
	S_BRIGHT (FHFX, 'L',	3, NULL, NULL)
};

void AEggFX::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_EGGFX];
	info->speed = 18 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 1;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
	info->deathstate = (gameinfo.gametype == GAME_Heretic) ?
		&States[S_EGGFXI1] : &States[S_EGGFXI2];
}

int AEggFX::DoSpecialDamage (AActor *target, int damage)
{
	if (target->player)
	{
		P_MorphPlayer (target->player);
	}
	else
	{
		P_MorphMonster (target);
	}
	return -1;
}

BASIC_ARTI (Egg, arti_egg, TXT_ARTIEGG)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		AActor *mo = player->mo;
		P_SpawnPlayerMissile (mo, RUNTIME_CLASS(AEggFX));
		P_SpawnPlayerMissile (mo, RUNTIME_CLASS(AEggFX), mo->angle-(ANG45/6));
		P_SpawnPlayerMissile (mo, RUNTIME_CLASS(AEggFX), mo->angle+(ANG45/6));
		P_SpawnPlayerMissile (mo, RUNTIME_CLASS(AEggFX), mo->angle-(ANG45/3));
		P_SpawnPlayerMissile (mo, RUNTIME_CLASS(AEggFX), mo->angle+(ANG45/3));
		return true;
	}
};

ARTI_SETUP (Egg, Raven);

FState AArtiEgg::States[] =
{
#define S_ARTI_EGGC 0
	S_NORMAL (EGGC, 'A',	6, NULL, &States[S_ARTI_EGGC+1]),
	S_NORMAL (EGGC, 'B',	6, NULL, &States[S_ARTI_EGGC+2]),
	S_NORMAL (EGGC, 'C',	6, NULL, &States[S_ARTI_EGGC+3]),
	S_NORMAL (EGGC, 'B',	6, NULL, &States[S_ARTI_EGGC+0]),

#define S_ARTI_EGGP (S_ARTI_EGGC+4)
	S_NORMAL (PORK, 'A',	5, NULL, &States[S_ARTI_EGGP+1]),
	S_NORMAL (PORK, 'B',	5, NULL, &States[S_ARTI_EGGP+2]),
	S_NORMAL (PORK, 'C',	5, NULL, &States[S_ARTI_EGGP+3]),
	S_NORMAL (PORK, 'D',	5, NULL, &States[S_ARTI_EGGP+4]),
	S_NORMAL (PORK, 'E',	5, NULL, &States[S_ARTI_EGGP+5]),
	S_NORMAL (PORK, 'F',	5, NULL, &States[S_ARTI_EGGP+6]),
	S_NORMAL (PORK, 'G',	5, NULL, &States[S_ARTI_EGGP+7]),
	S_NORMAL (PORK, 'H',	5, NULL, &States[S_ARTI_EGGP+0])
};

void AArtiEgg::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 30;
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	info->spawnstate = (gameinfo.gametype == GAME_Heretic) ?
		&States[S_ARTI_EGGC] : &States[S_ARTI_EGGP];
	ArtiDispatch[arti_egg] = ActivateArti;
	ArtiPics[arti_egg] = "ARTIEGGC";
}

