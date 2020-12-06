#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "dstrings.h"
#include "a_action.h"

void A_TroopAttack (AActor *);

class ADoomImp : public AActor
{
	DECLARE_ACTOR (ADoomImp, AActor);
public:
	const char *GetObituary () { return OB_IMP; }
	const char *GetHitObituary () { return OB_IMPHIT; }
};

IMPLEMENT_DEF_SERIAL (ADoomImp, AActor);
REGISTER_ACTOR (ADoomImp, Doom);

FState ADoomImp::States[] =
{
#define S_TROO_STND 0
	S_NORMAL (TROO, 'A',   10, A_Look						, &States[S_TROO_STND+1]),
	S_NORMAL (TROO, 'B',   10, A_Look						, &States[S_TROO_STND]),

#define S_TROO_RUN (S_TROO_STND+2)
	S_NORMAL (TROO, 'A',	3, A_Chase						, &States[S_TROO_RUN+1]),
	S_NORMAL (TROO, 'A',	3, A_Chase						, &States[S_TROO_RUN+2]),
	S_NORMAL (TROO, 'B',	3, A_Chase						, &States[S_TROO_RUN+3]),
	S_NORMAL (TROO, 'B',	3, A_Chase						, &States[S_TROO_RUN+4]),
	S_NORMAL (TROO, 'C',	3, A_Chase						, &States[S_TROO_RUN+5]),
	S_NORMAL (TROO, 'C',	3, A_Chase						, &States[S_TROO_RUN+6]),
	S_NORMAL (TROO, 'D',	3, A_Chase						, &States[S_TROO_RUN+7]),
	S_NORMAL (TROO, 'D',	3, A_Chase						, &States[S_TROO_RUN+0]),

#define S_TROO_ATK (S_TROO_RUN+8)
	S_NORMAL (TROO, 'E',	8, A_FaceTarget 				, &States[S_TROO_ATK+1]),
	S_NORMAL (TROO, 'F',	8, A_FaceTarget 				, &States[S_TROO_ATK+2]),
	S_NORMAL (TROO, 'G',	6, A_TroopAttack				, &States[S_TROO_RUN+0]),

#define S_TROO_PAIN (S_TROO_ATK+3)
	S_NORMAL (TROO, 'H',	2, NULL 						, &States[S_TROO_PAIN+1]),
	S_NORMAL (TROO, 'H',	2, A_Pain						, &States[S_TROO_RUN+0]),

#define S_TROO_DIE (S_TROO_PAIN+2)
	S_NORMAL (TROO, 'I',	8, NULL 						, &States[S_TROO_DIE+1]),
	S_NORMAL (TROO, 'J',	8, A_Scream 					, &States[S_TROO_DIE+2]),
	S_NORMAL (TROO, 'K',	6, NULL 						, &States[S_TROO_DIE+3]),
	S_NORMAL (TROO, 'L',	6, A_NoBlocking					, &States[S_TROO_DIE+4]),
	S_NORMAL (TROO, 'M',   -1, NULL 						, NULL),

#define S_TROO_XDIE (S_TROO_DIE+5)
	S_NORMAL (TROO, 'N',	5, NULL 						, &States[S_TROO_XDIE+1]),
	S_NORMAL (TROO, 'O',	5, A_XScream					, &States[S_TROO_XDIE+2]),
	S_NORMAL (TROO, 'P',	5, NULL 						, &States[S_TROO_XDIE+3]),
	S_NORMAL (TROO, 'Q',	5, A_NoBlocking					, &States[S_TROO_XDIE+4]),
	S_NORMAL (TROO, 'R',	5, NULL 						, &States[S_TROO_XDIE+5]),
	S_NORMAL (TROO, 'S',	5, NULL 						, &States[S_TROO_XDIE+6]),
	S_NORMAL (TROO, 'T',	5, NULL 						, &States[S_TROO_XDIE+7]),
	S_NORMAL (TROO, 'U',   -1, NULL 						, NULL),

#define S_TROO_RAISE (S_TROO_XDIE+8)
	S_NORMAL (TROO, 'M',	8, NULL 						, &States[S_TROO_RAISE+1]),
	S_NORMAL (TROO, 'L',	8, NULL 						, &States[S_TROO_RAISE+2]),
	S_NORMAL (TROO, 'K',	6, NULL 						, &States[S_TROO_RAISE+3]),
	S_NORMAL (TROO, 'J',	6, NULL 						, &States[S_TROO_RAISE+4]),
	S_NORMAL (TROO, 'I',	6, NULL 						, &States[S_TROO_RUN+0])
};

void ADoomImp::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 3001;
	info->spawnid = 5;
	info->spawnstate = &States[S_TROO_STND];
	info->spawnhealth = 60;
	info->seestate = &States[S_TROO_RUN];
	info->seesound = "imp/sight1";
	info->painstate = &States[S_TROO_PAIN];
	info->painchance = 200;
	info->painsound = "imp/pain";
	info->meleestate = &States[S_TROO_ATK];
	info->missilestate = &States[S_TROO_ATK];
	info->deathstate = &States[S_TROO_DIE];
	info->xdeathstate = &States[S_TROO_XDIE];
	info->deathsound = "imp/death1";
	info->speed = 8;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->activesound = "imp/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_TROO_RAISE];
}

class AStealthDoomImp : public ADoomImp
{
	DECLARE_STATELESS_ACTOR (AStealthDoomImp, ADoomImp);
public:
	const char *GetObituary () { return OB_STEALTHIMP; }
	const char *GetHitObituary () { return OB_STEALTHIMP; }
};

IMPLEMENT_DEF_SERIAL (AStealthDoomImp, ADoomImp);
REGISTER_ACTOR (AStealthDoomImp, Doom);

void AStealthDoomImp::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9057;
	info->spawnid = 122;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class ADoomImpBall : public AActor
{
	DECLARE_ACTOR (ADoomImpBall, AActor);
};

IMPLEMENT_DEF_SERIAL (ADoomImpBall, AActor);
REGISTER_ACTOR (ADoomImpBall, Doom);

FState ADoomImpBall::States[] =
{
#define S_TBALL 0
	S_BRIGHT (BAL1, 'A',	4, NULL 						, &States[S_TBALL+1]),
	S_BRIGHT (BAL1, 'B',	4, NULL 						, &States[S_TBALL+0]),

#define S_TBALLX (S_TBALL+2)
	S_BRIGHT (BAL1, 'C',	6, NULL 						, &States[S_TBALLX+1]),
	S_BRIGHT (BAL1, 'D',	6, NULL 						, &States[S_TBALLX+2]),
	S_BRIGHT (BAL1, 'E',	6, NULL 						, NULL)
};

void ADoomImpBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 10;
	info->spawnstate = &States[S_TBALL];
	info->seesound = "imp/attack";
	info->deathstate = &States[S_TBALLX];
	info->deathsound = "imp/shotx";
	info->speed = GameSpeed != SPEED_Fast ? 10 * FRACUNIT : 20 * FRACUNIT;
	info->radius = 6 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 3;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC75;
}

//
// A_TroopAttack
//
void A_TroopAttack (AActor *self)
{
	if (!self->target)
		return;
				
	A_FaceTarget (self);
	if (P_CheckMeleeRange (self))
	{
		int damage = (P_Random (pr_troopattack)%8+1)*3;
		S_Sound (self, CHAN_WEAPON, "imp/melee", 1, ATTN_NORM);
		P_DamageMobj (self->target, self, self, damage, MOD_HIT);
		return;
	}
	
	// launch a missile
	P_SpawnMissile (self, self->target, RUNTIME_CLASS(ADoomImpBall));
}

// Dead imp ----------------------------------------------------------------

class ADeadDoomImp : public ADoomImp
{
	DECLARE_STATELESS_ACTOR (ADeadDoomImp, ADoomImp);
};

IMPLEMENT_DEF_SERIAL (ADeadDoomImp, ADoomImp);
REGISTER_ACTOR (ADeadDoomImp, Doom);

void ADeadDoomImp::SetDefaults (FActorInfo *info)
{
	AActor::SetDefaults (info);
	info->OwnedStates = NULL;
	info->NumOwnedStates = 0;
	info->doomednum = 20;
	info->spawnstate = &States[S_TROO_DIE+4];
}
