#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_action.h"
#include "a_sharedglobal.h"

void A_KnightAttack (AActor *);
void A_DripBlood (AActor *);
void A_AxeSound (AActor *);

// Knight -------------------------------------------------------------------

class AKnight : public AActor
{
	DECLARE_ACTOR (AKnight, AActor);
public:
	void NoBlockingSet ();
};

IMPLEMENT_DEF_SERIAL (AKnight, AActor);
REGISTER_ACTOR (AKnight, Heretic);

FState AKnight::States[] =
{
#define S_KNIGHT_STND 0
	S_NORMAL (KNIG, 'A',   10, A_Look					, &States[S_KNIGHT_STND+1]),
	S_NORMAL (KNIG, 'B',   10, A_Look					, &States[S_KNIGHT_STND+0]),

#define S_KNIGHT_WALK (S_KNIGHT_STND+2)
	S_NORMAL (KNIG, 'A',	4, A_Chase					, &States[S_KNIGHT_WALK+1]),
	S_NORMAL (KNIG, 'B',	4, A_Chase					, &States[S_KNIGHT_WALK+2]),
	S_NORMAL (KNIG, 'C',	4, A_Chase					, &States[S_KNIGHT_WALK+3]),
	S_NORMAL (KNIG, 'D',	4, A_Chase					, &States[S_KNIGHT_WALK+0]),

#define S_KNIGHT_ATK (S_KNIGHT_WALK+4)
	S_NORMAL (KNIG, 'E',   10, A_FaceTarget 			, &States[S_KNIGHT_ATK+1]),
	S_NORMAL (KNIG, 'F',	8, A_FaceTarget 			, &States[S_KNIGHT_ATK+2]),
	S_NORMAL (KNIG, 'G',	8, A_KnightAttack			, &States[S_KNIGHT_ATK+3]),
	S_NORMAL (KNIG, 'E',   10, A_FaceTarget 			, &States[S_KNIGHT_ATK+4]),
	S_NORMAL (KNIG, 'F',	8, A_FaceTarget 			, &States[S_KNIGHT_ATK+5]),
	S_NORMAL (KNIG, 'G',	8, A_KnightAttack			, &States[S_KNIGHT_WALK+0]),

#define S_KNIGHT_PAIN (S_KNIGHT_ATK+6)
	S_NORMAL (KNIG, 'H',	3, NULL 					, &States[S_KNIGHT_PAIN+1]),
	S_NORMAL (KNIG, 'H',	3, A_Pain					, &States[S_KNIGHT_WALK+0]),

#define S_KNIGHT_DIE (S_KNIGHT_PAIN+2)
	S_NORMAL (KNIG, 'I',	6, NULL 					, &States[S_KNIGHT_DIE+1]),
	S_NORMAL (KNIG, 'J',	6, A_Scream 				, &States[S_KNIGHT_DIE+2]),
	S_NORMAL (KNIG, 'K',	6, NULL 					, &States[S_KNIGHT_DIE+3]),
	S_NORMAL (KNIG, 'L',	6, A_NoBlocking 			, &States[S_KNIGHT_DIE+4]),
	S_NORMAL (KNIG, 'M',	6, NULL 					, &States[S_KNIGHT_DIE+5]),
	S_NORMAL (KNIG, 'N',	6, NULL 					, &States[S_KNIGHT_DIE+6]),
	S_NORMAL (KNIG, 'O',   -1, NULL 					, NULL)
};

void AKnight::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 64;
	info->spawnstate = &States[S_KNIGHT_STND];
	info->spawnhealth = 200;
	info->seestate = &States[S_KNIGHT_WALK];
	info->seesound = "knight/sight";
	info->attacksound = "knight/attack";
	info->painstate = &States[S_KNIGHT_PAIN];
	info->painchance = 100;
	info->painsound = "knight/pain";
	info->meleestate = &States[S_KNIGHT_ATK];
	info->missilestate = &States[S_KNIGHT_ATK];
	info->deathstate = &States[S_KNIGHT_DIE];
	info->deathsound = "knight/death";
	info->speed = 12;
	info->radius = 24 * FRACUNIT;
	info->height = 78 * FRACUNIT;
	info->mass = 150;
	info->activesound = "knight/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_FLOORCLIP|MF2_PASSMOBJ;
}

void AKnight::NoBlockingSet ()
{
	P_DropItem (this, "CrossbowWimpy", 5, 84);
}

// Knight ghost -------------------------------------------------------------

class AKnightGhost : public AKnight
{
	DECLARE_STATELESS_ACTOR (AKnightGhost, AKnight);
};

IMPLEMENT_DEF_SERIAL (AKnightGhost, AKnight);
REGISTER_ACTOR (AKnightGhost, Heretic);

void AKnightGhost::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 65;
	info->flags |= MF_SHADOW;
	info->flags3 = MF3_GHOST;
	info->translucency = HR_SHADOW;
}

// Knight axe ---------------------------------------------------------------

class AKnightAxe : public AActor
{
	DECLARE_ACTOR (AKnightAxe, AActor);
};

IMPLEMENT_DEF_SERIAL (AKnightAxe, AActor);
REGISTER_ACTOR (AKnightAxe, Heretic);

FState AKnightAxe::States[] =
{
#define S_SPINAXE 0
	S_BRIGHT (SPAX, 'A',	3, A_AxeSound				, &States[S_SPINAXE+1]),
	S_BRIGHT (SPAX, 'B',	3, NULL 					, &States[S_SPINAXE+2]),
	S_BRIGHT (SPAX, 'C',	3, NULL 					, &States[S_SPINAXE+0]),

#define S_SPINAXEX (S_SPINAXE+3)
	S_BRIGHT (SPAX, 'D',	6, NULL 					, &States[S_SPINAXEX+1]),
	S_BRIGHT (SPAX, 'E',	6, NULL 					, &States[S_SPINAXEX+2]),
	S_BRIGHT (SPAX, 'F',	6, NULL 					, NULL)
};

void AKnightAxe::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SPINAXE];
	info->deathstate = &States[S_SPINAXEX];
	info->deathsound = "knight/hit";
	info->speed = GameSpeed != SPEED_Fast ? 9 * FRACUNIT : 18 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_WINDTHRUST|MF2_NOTELEPORT|MF2_THRUGHOST;
}

// Red axe ------------------------------------------------------------------

class ARedAxe : public AKnightAxe
{
	DECLARE_ACTOR (ARedAxe, AKnightAxe);
};

IMPLEMENT_DEF_SERIAL (ARedAxe, AKnightAxe);
REGISTER_ACTOR (ARedAxe, Heretic);

FState ARedAxe::States[] =
{
#define S_REDAXE 0
	S_BRIGHT (RAXE, 'A',	5, A_DripBlood				, &States[S_REDAXE+1]),
	S_BRIGHT (RAXE, 'B',	5, A_DripBlood				, &States[S_REDAXE+0]),

#define S_REDAXEX (S_REDAXE+2)
	S_BRIGHT (RAXE, 'C',	6, NULL 					, &States[S_REDAXEX+1]),
	S_BRIGHT (RAXE, 'D',	6, NULL 					, &States[S_REDAXEX+2]),
	S_BRIGHT (RAXE, 'E',	6, NULL 					, NULL)
};

void ARedAxe::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_REDAXE];
	info->deathstate = &States[S_REDAXEX];
	info->damage = 7;
	info->flags2 &= ~MF2_WINDTHRUST;
}

//----------------------------------------------------------------------------
//
// PROC A_DripBlood
//
//----------------------------------------------------------------------------

void A_DripBlood (AActor *actor)
{
	AActor *mo;
	fixed_t x, y;

	x = actor->x + (PS_Random () << 11);
	y = actor->y + (PS_Random () << 11);
	mo = Spawn<ABlood> (x, y, actor->z);
	mo->momx = PS_Random () << 10;
	mo->momy = PS_Random () << 10;
	mo->flags2 |= MF2_LOGRAV;
}

//----------------------------------------------------------------------------
//
// PROC A_KnightAttack
//
//----------------------------------------------------------------------------

void A_KnightAttack (AActor *actor)
{
	if (!actor->target)
	{
		return;
	}
	if (P_CheckMeleeRange (actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(3));
		S_Sound (actor, CHAN_BODY, "knight/melee", 1, ATTN_NORM);
		return;
	}
	// Throw axe
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->attacksound, 1, ATTN_NORM);
	if (actor->flags & MF_SHADOW || P_Random () < 40)
	{ // Red axe
		P_SpawnMissileZ (actor, actor->z + 36*FRACUNIT, actor->target, RUNTIME_CLASS(ARedAxe));
		return;
	}
	// Green axe
	P_SpawnMissileZ (actor, actor->z + 36*FRACUNIT, actor->target, RUNTIME_CLASS(AKnightAxe));
}

//---------------------------------------------------------------------------
//
// PROC A_AxeSound
//
//---------------------------------------------------------------------------

void A_AxeSound (AActor *actor)
{
	S_Sound (actor, CHAN_BODY, "knight/axewhoosh", 1, ATTN_NORM);
}
