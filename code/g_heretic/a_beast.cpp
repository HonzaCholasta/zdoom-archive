#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_action.h"

void A_BeastAttack (AActor *);
void A_BeastPuff (AActor *);

// Beast --------------------------------------------------------------------

class ABeast : public AActor
{
	DECLARE_ACTOR (ABeast, AActor);
public:
	void NoBlockingSet ();
};

IMPLEMENT_DEF_SERIAL (ABeast, AActor);
REGISTER_ACTOR (ABeast, Heretic);

FState ABeast::States[] =
{
#define S_BEAST_LOOK 0
	S_NORMAL (BEAS, 'A',   10, A_Look					, &States[S_BEAST_LOOK+1]),
	S_NORMAL (BEAS, 'B',   10, A_Look					, &States[S_BEAST_LOOK+0]),

#define S_BEAST_WALK (S_BEAST_LOOK+2)
	S_NORMAL (BEAS, 'A',	3, A_Chase					, &States[S_BEAST_WALK+1]),
	S_NORMAL (BEAS, 'B',	3, A_Chase					, &States[S_BEAST_WALK+2]),
	S_NORMAL (BEAS, 'C',	3, A_Chase					, &States[S_BEAST_WALK+3]),
	S_NORMAL (BEAS, 'D',	3, A_Chase					, &States[S_BEAST_WALK+4]),
	S_NORMAL (BEAS, 'E',	3, A_Chase					, &States[S_BEAST_WALK+5]),
	S_NORMAL (BEAS, 'F',	3, A_Chase					, &States[S_BEAST_WALK+0]),

#define S_BEAST_ATK (S_BEAST_WALK+6)
	S_NORMAL (BEAS, 'H',   10, A_FaceTarget 			, &States[S_BEAST_ATK+1]),
	S_NORMAL (BEAS, 'I',   10, A_BeastAttack			, &States[S_BEAST_WALK+0]),

#define S_BEAST_PAIN (S_BEAST_ATK+2)
	S_NORMAL (BEAS, 'G',	3, NULL 					, &States[S_BEAST_PAIN+1]),
	S_NORMAL (BEAS, 'G',	3, A_Pain					, &States[S_BEAST_WALK+0]),

#define S_BEAST_DIE (S_BEAST_PAIN+2)
	S_NORMAL (BEAS, 'R',	6, NULL 					, &States[S_BEAST_DIE+1]),
	S_NORMAL (BEAS, 'S',	6, A_Scream 				, &States[S_BEAST_DIE+2]),
	S_NORMAL (BEAS, 'T',	6, NULL 					, &States[S_BEAST_DIE+3]),
	S_NORMAL (BEAS, 'U',	6, NULL 					, &States[S_BEAST_DIE+4]),
	S_NORMAL (BEAS, 'V',	6, NULL 					, &States[S_BEAST_DIE+5]),
	S_NORMAL (BEAS, 'W',	6, A_NoBlocking 			, &States[S_BEAST_DIE+6]),
	S_NORMAL (BEAS, 'X',	6, NULL 					, &States[S_BEAST_DIE+7]),
	S_NORMAL (BEAS, 'Y',	6, NULL 					, &States[S_BEAST_DIE+8]),
	S_NORMAL (BEAS, 'Z',   -1, NULL 					, NULL),

#define S_BEAST_XDIE (S_BEAST_DIE+9)
	S_NORMAL (BEAS, 'J',	5, NULL 					, &States[S_BEAST_XDIE+1]),
	S_NORMAL (BEAS, 'K',	6, A_Scream 				, &States[S_BEAST_XDIE+2]),
	S_NORMAL (BEAS, 'L',	5, NULL 					, &States[S_BEAST_XDIE+3]),
	S_NORMAL (BEAS, 'M',	6, NULL 					, &States[S_BEAST_XDIE+4]),
	S_NORMAL (BEAS, 'N',	5, NULL 					, &States[S_BEAST_XDIE+5]),
	S_NORMAL (BEAS, 'O',	6, A_NoBlocking 			, &States[S_BEAST_XDIE+6]),
	S_NORMAL (BEAS, 'P',	5, NULL 					, &States[S_BEAST_XDIE+7]),
	S_NORMAL (BEAS, 'Q',   -1, NULL 					, NULL)
};

void ABeast::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 70;
	info->spawnstate = &States[S_BEAST_LOOK];
	info->spawnhealth = 220;
	info->seestate = &States[S_BEAST_WALK];
	info->seesound = "beast/sight";
	info->attacksound = "beast/attack";
	info->painstate = &States[S_BEAST_PAIN];
	info->painchance = 100;
	info->painsound = "beast/pain";
	info->missilestate = &States[S_BEAST_ATK];
	info->deathstate = &States[S_BEAST_DIE];
	info->xdeathstate = &States[S_BEAST_XDIE];
	info->deathsound = "beast/death";
	info->speed = 14;
	info->radius = 32 * FRACUNIT;
	info->height = 74 * FRACUNIT;
	info->mass = 200;
	info->activesound = "beast/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_FLOORCLIP|MF2_PASSMOBJ;
}

void ABeast::NoBlockingSet ()
{
	P_DropItem (this, "CrossbowWimpy", 10, 84);
}

// Beast ball ---------------------------------------------------------------

// Heretic also had a MT_BURNBALL and MT_BURNBALLFB based on the beast ball,
// but it didn't use them anywhere.

class ABeastBall : public AActor
{
	DECLARE_ACTOR (ABeastBall, AActor);
};

IMPLEMENT_DEF_SERIAL (ABeastBall, AActor);
REGISTER_ACTOR (ABeastBall, Heretic);

FState ABeastBall::States[] =
{
#define S_BEASTBALL 0
	S_NORMAL (FRB1, 'A',	2, A_BeastPuff				, &States[S_BEASTBALL+1]),
	S_NORMAL (FRB1, 'A',	2, A_BeastPuff				, &States[S_BEASTBALL+2]),
	S_NORMAL (FRB1, 'B',	2, A_BeastPuff				, &States[S_BEASTBALL+3]),
	S_NORMAL (FRB1, 'B',	2, A_BeastPuff				, &States[S_BEASTBALL+4]),
	S_NORMAL (FRB1, 'C',	2, A_BeastPuff				, &States[S_BEASTBALL+5]),
	S_NORMAL (FRB1, 'C',	2, A_BeastPuff				, &States[S_BEASTBALL+0]),

#define S_BEASTBALLX (S_BEASTBALL+6)
	S_NORMAL (FRB1, 'D',	4, NULL 					, &States[S_BEASTBALLX+1]),
	S_NORMAL (FRB1, 'E',	4, NULL 					, &States[S_BEASTBALLX+2]),
	S_NORMAL (FRB1, 'F',	4, NULL 					, &States[S_BEASTBALLX+3]),
	S_NORMAL (FRB1, 'G',	4, NULL 					, &States[S_BEASTBALLX+4]),
	S_NORMAL (FRB1, 'H',	4, NULL 					, NULL)
};

void ABeastBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BEASTBALL];
	info->deathstate = &States[S_BEASTBALLX];
	info->speed = GameSpeed != SPEED_Fast ? 12 * FRACUNIT : 20 * FRACUNIT;
	info->radius = 9 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 4;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_WINDTHRUST|MF2_NOTELEPORT;
}

// Puffy --------------------------------------------------------------------

class APuffy : public AActor
{
	DECLARE_ACTOR (APuffy, AActor);
};

IMPLEMENT_DEF_SERIAL (APuffy, AActor);
REGISTER_ACTOR (APuffy, Heretic);

FState APuffy::States[] =
{
	S_NORMAL (FRB1, 'D',	4, NULL 					, &States[1]),
	S_NORMAL (FRB1, 'E',	4, NULL 					, &States[2]),
	S_NORMAL (FRB1, 'F',	4, NULL 					, &States[3]),
	S_NORMAL (FRB1, 'G',	4, NULL 					, &States[4]),
	S_NORMAL (FRB1, 'H',	4, NULL 					, NULL)
};

void APuffy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->speed = 10 * FRACUNIT;
	info->radius = 6 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE;
	info->flags2 = MF2_NOTELEPORT;
}

//----------------------------------------------------------------------------
//
// PROC A_BeastAttack
//
//----------------------------------------------------------------------------

void A_BeastAttack (AActor *actor)
{
	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->attacksound, 1, ATTN_NORM);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(3));
		return;
	}
	P_SpawnMissile (actor, actor->target, RUNTIME_CLASS(ABeastBall));
}

//----------------------------------------------------------------------------
//
// PROC A_BeastPuff
//
//----------------------------------------------------------------------------

void A_BeastPuff (AActor *actor)
{
	if (P_Random() > 64)
	{
		fixed_t x, y, z;

		x = actor->x + (PS_Random () << 10);
		y = actor->y + (PS_Random () << 10);
		z = actor->z + (PS_Random () << 10);
		Spawn<APuffy> (x, y, z);
	}
}
