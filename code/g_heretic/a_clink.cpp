#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_action.h"

void A_ClinkAttack (AActor *);

// Clink --------------------------------------------------------------------

class AClink : public AActor
{
	DECLARE_ACTOR (AClink, AActor);
public:
	void NoBlockingSet ();
};

IMPLEMENT_DEF_SERIAL (AClink, AActor);
REGISTER_ACTOR (AClink, Heretic);

FState AClink::States[] =
{
#define S_CLINK_LOOK 0
	S_NORMAL (CLNK, 'A',   10, A_Look					, &States[S_CLINK_LOOK+1]),
	S_NORMAL (CLNK, 'B',   10, A_Look					, &States[S_CLINK_LOOK+0]),

#define S_CLINK_WALK (S_CLINK_LOOK+2)
	S_NORMAL (CLNK, 'A',	3, A_Chase					, &States[S_CLINK_WALK+1]),
	S_NORMAL (CLNK, 'B',	3, A_Chase					, &States[S_CLINK_WALK+2]),
	S_NORMAL (CLNK, 'C',	3, A_Chase					, &States[S_CLINK_WALK+3]),
	S_NORMAL (CLNK, 'D',	3, A_Chase					, &States[S_CLINK_WALK+0]),

#define S_CLINK_ATK (S_CLINK_WALK+4)
	S_NORMAL (CLNK, 'E',	5, A_FaceTarget 			, &States[S_CLINK_ATK+1]),
	S_NORMAL (CLNK, 'F',	4, A_FaceTarget 			, &States[S_CLINK_ATK+2]),
	S_NORMAL (CLNK, 'G',	7, A_ClinkAttack			, &States[S_CLINK_WALK+0]),

#define S_CLINK_PAIN (S_CLINK_ATK+3)
	S_NORMAL (CLNK, 'H',	3, NULL 					, &States[S_CLINK_PAIN+1]),
	S_NORMAL (CLNK, 'H',	3, A_Pain					, &States[S_CLINK_WALK+0]),

#define S_CLINK_DIE (S_CLINK_PAIN+2)
	S_NORMAL (CLNK, 'I',	6, NULL 					, &States[S_CLINK_DIE+1]),
	S_NORMAL (CLNK, 'J',	6, NULL 					, &States[S_CLINK_DIE+2]),
	S_NORMAL (CLNK, 'K',	5, A_Scream 				, &States[S_CLINK_DIE+3]),
	S_NORMAL (CLNK, 'L',	5, A_NoBlocking 			, &States[S_CLINK_DIE+4]),
	S_NORMAL (CLNK, 'M',	5, NULL 					, &States[S_CLINK_DIE+5]),
	S_NORMAL (CLNK, 'N',	5, NULL 					, &States[S_CLINK_DIE+6]),
	S_NORMAL (CLNK, 'O',   -1, NULL 					, NULL)
};

void AClink::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 90;
	info->spawnstate = &States[S_CLINK_LOOK];
	info->spawnhealth = 150;
	info->seestate = &States[S_CLINK_WALK];
	info->seesound = "clink/sight";
	info->attacksound = "clink/attack";
	info->painstate = &States[S_CLINK_PAIN];
	info->painchance = 32;
	info->painsound = "clink/pain";
	info->meleestate = &States[S_CLINK_ATK];
	info->deathstate = &States[S_CLINK_DIE];
	info->deathsound = "clink/death";
	info->speed = 14;
	info->radius = 20 * FRACUNIT;
	info->height = 64 * FRACUNIT;
	info->mass = 75;
	info->activesound = "clink/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_NOBLOOD;
	info->flags2 = MF2_FLOORCLIP|MF2_PASSMOBJ;
}

void AClink::NoBlockingSet ()
{
	P_DropItem (this, "SkullRodWimpy", 20, 84);
}

//----------------------------------------------------------------------------
//
// PROC A_ClinkAttack
//
//----------------------------------------------------------------------------

void A_ClinkAttack (AActor *actor)
{
	int damage;

	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->attacksound, 1, ATTN_NORM);
	if (P_CheckMeleeRange (actor))
	{
		damage = ((P_Random()%7)+3);
		P_DamageMobj (actor->target, actor, actor, damage);
	}
}
