#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "p_enemy.h"
#include "dstrings.h"
#include "a_action.h"

void A_SargAttack (AActor *);

class ADemon : public AActor
{
	DECLARE_ACTOR (ADemon, AActor);
public:
	const char *GetHitObituary () { return OB_DEMONHIT; }
};

IMPLEMENT_DEF_SERIAL (ADemon, AActor);
REGISTER_ACTOR (ADemon, Doom);

FState ADemon::States[] =
{
#define S_SARG_STND 0
	S_NORMAL (SARG, 'A',   10, A_Look						, &States[S_SARG_STND+1]),
	S_NORMAL (SARG, 'B',   10, A_Look						, &States[S_SARG_STND]),

#define S_SARG_RUN (S_SARG_STND+2)
	S_NORMAL (SARG, 'A',	2, A_Chase						, &States[S_SARG_RUN+1]),
	S_NORMAL (SARG, 'A',	2, A_Chase						, &States[S_SARG_RUN+2]),
	S_NORMAL (SARG, 'B',	2, A_Chase						, &States[S_SARG_RUN+3]),
	S_NORMAL (SARG, 'B',	2, A_Chase						, &States[S_SARG_RUN+4]),
	S_NORMAL (SARG, 'C',	2, A_Chase						, &States[S_SARG_RUN+5]),
	S_NORMAL (SARG, 'C',	2, A_Chase						, &States[S_SARG_RUN+6]),
	S_NORMAL (SARG, 'D',	2, A_Chase						, &States[S_SARG_RUN+7]),
	S_NORMAL (SARG, 'D',	2, A_Chase						, &States[S_SARG_RUN+0]),

#define S_SARG_ATK (S_SARG_RUN+8)
	S_NORMAL (SARG, 'E',	8, A_FaceTarget 				, &States[S_SARG_ATK+1]),
	S_NORMAL (SARG, 'F',	8, A_FaceTarget 				, &States[S_SARG_ATK+2]),
	S_NORMAL (SARG, 'G',	8, A_SargAttack 				, &States[S_SARG_RUN+0]),

#define S_SARG_PAIN (S_SARG_ATK+3)
	S_NORMAL (SARG, 'H',	2, NULL 						, &States[S_SARG_PAIN+1]),
	S_NORMAL (SARG, 'H',	2, A_Pain						, &States[S_SARG_RUN+0]),

#define S_SARG_DIE (S_SARG_PAIN+2)
	S_NORMAL (SARG, 'I',	8, NULL 						, &States[S_SARG_DIE+1]),
	S_NORMAL (SARG, 'J',	8, A_Scream 					, &States[S_SARG_DIE+2]),
	S_NORMAL (SARG, 'K',	4, NULL 						, &States[S_SARG_DIE+3]),
	S_NORMAL (SARG, 'L',	4, A_NoBlocking					, &States[S_SARG_DIE+4]),
	S_NORMAL (SARG, 'M',	4, NULL 						, &States[S_SARG_DIE+5]),
	S_NORMAL (SARG, 'N',   -1, NULL 						, NULL),

#define S_SARG_RAISE (S_SARG_DIE+6)
	S_NORMAL (SARG, 'N',	5, NULL 						, &States[S_SARG_RAISE+1]),
	S_NORMAL (SARG, 'M',	5, NULL 						, &States[S_SARG_RAISE+2]),
	S_NORMAL (SARG, 'L',	5, NULL 						, &States[S_SARG_RAISE+3]),
	S_NORMAL (SARG, 'K',	5, NULL 						, &States[S_SARG_RAISE+4]),
	S_NORMAL (SARG, 'J',	5, NULL 						, &States[S_SARG_RAISE+5]),
	S_NORMAL (SARG, 'I',	5, NULL 						, &States[S_SARG_RUN+0])
};

void ADemon::SetDefaults (FActorInfo *info)
{
	int i;

	INHERIT_DEFS;
	info->doomednum = 3002;
	info->spawnid = 8;
	info->spawnstate = &States[S_SARG_STND];
	info->spawnhealth = 150;
	info->seestate = &States[S_SARG_RUN];
	info->seesound = "demon/sight";
	info->attacksound = "demon/melee";
	info->painstate = &States[S_SARG_PAIN];
	info->painchance = 180;
	info->painsound = "demon/pain";
	info->meleestate = &States[S_SARG_ATK];
	info->deathstate = &States[S_SARG_DIE];
	info->deathsound = "demon/death";
	info->speed = 10;
	info->radius = 30 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 400;
	info->activesound = "demon/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_SARG_RAISE];

	if (GameSpeed == SPEED_Fast)
	{
		for (i = S_SARG_RUN; i < S_SARG_PAIN; i++)
			States[i].tics <<= 1;
	}
	else if (GameSpeed == SPEED_Normal)
	{
		for (i = S_SARG_RUN; i < S_SARG_PAIN; i++)
			States[i].tics >>= 1;
	}
}

class AStealthDemon : public ADemon
{
	DECLARE_STATELESS_ACTOR (AStealthDemon, ADemon);
public:
	const char *GetObituary () { return OB_STEALTHDEMON; }
	const char *GetHitObituary () { return OB_STEALTHDEMON; }
};

IMPLEMENT_DEF_SERIAL (AStealthDemon, ADemon);
REGISTER_ACTOR (AStealthDemon, Doom);

void AStealthDemon::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9055;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class ASpectre : public ADemon
{
	DECLARE_STATELESS_ACTOR (ASpectre, ADemon);
public:
	const char *GetHitObituary () { return OB_SPECTREHIT; }
};

IMPLEMENT_DEF_SERIAL (ASpectre, ADemon);
REGISTER_ACTOR (ASpectre, Doom);

void ASpectre::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 58;
	info->spawnid = 9;
	info->flags |= MF_SHADOW;
}

void A_SargAttack (AActor *self)
{
	if (!self->target)
		return;
				
	A_FaceTarget (self);
	if (P_CheckMeleeRange (self))
	{
		int damage = ((P_Random (pr_sargattack)%10)+1)*4;
		P_DamageMobj (self->target, self, self, damage, MOD_HIT);
	}
}

// Dead demon --------------------------------------------------------------

class ADeadDemon : public ADemon
{
	DECLARE_STATELESS_ACTOR (ADeadDemon, ADemon);
};

IMPLEMENT_DEF_SERIAL (ADeadDemon, ADemon);
REGISTER_ACTOR (ADeadDemon, Doom);

void ADeadDemon::SetDefaults (FActorInfo *info)
{
	AActor::SetDefaults (info);
	info->OwnedStates = NULL;
	info->NumOwnedStates = 0;
	info->doomednum = 21;
	info->spawnstate = &States[S_SARG_DIE+5];
}
