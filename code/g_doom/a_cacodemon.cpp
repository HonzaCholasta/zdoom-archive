#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "p_enemy.h"
#include "dstrings.h"
#include "a_action.h"

void A_HeadAttack (AActor *);

class ACacodemon : public AActor
{
	DECLARE_ACTOR (ACacodemon, AActor);
public:
	const char *GetObituary () { return OB_CACO; }
	const char *GetHitObituary () { return OB_CACOHIT; }
};

IMPLEMENT_DEF_SERIAL (ACacodemon, AActor);
REGISTER_ACTOR (ACacodemon, Doom);

FState ACacodemon::States[] =
{
#define S_HEAD_STND 0
	S_NORMAL (HEAD, 'A',   10, A_Look						, &States[S_HEAD_STND]),

#define S_HEAD_RUN (S_HEAD_STND+1)
	S_NORMAL (HEAD, 'A',	3, A_Chase						, &States[S_HEAD_RUN+0]),

#define S_HEAD_ATK (S_HEAD_RUN+1)
	S_NORMAL (HEAD, 'B',	5, A_FaceTarget 				, &States[S_HEAD_ATK+1]),
	S_NORMAL (HEAD, 'C',	5, A_FaceTarget 				, &States[S_HEAD_ATK+2]),
	S_BRIGHT (HEAD, 'D',	5, A_HeadAttack 				, &States[S_HEAD_RUN+0]),

#define S_HEAD_PAIN (S_HEAD_ATK+3)
	S_NORMAL (HEAD, 'E',	3, NULL 						, &States[S_HEAD_PAIN+1]),
	S_NORMAL (HEAD, 'E',	3, A_Pain						, &States[S_HEAD_PAIN+2]),
	S_NORMAL (HEAD, 'F',	6, NULL 						, &States[S_HEAD_RUN+0]),

#define S_HEAD_DIE (S_HEAD_PAIN+3)
	S_NORMAL (HEAD, 'G',	8, NULL 						, &States[S_HEAD_DIE+1]),
	S_NORMAL (HEAD, 'H',	8, A_Scream 					, &States[S_HEAD_DIE+2]),
	S_NORMAL (HEAD, 'I',	8, NULL 						, &States[S_HEAD_DIE+3]),
	S_NORMAL (HEAD, 'J',	8, NULL 						, &States[S_HEAD_DIE+4]),
	S_NORMAL (HEAD, 'K',	8, A_NoBlocking					, &States[S_HEAD_DIE+5]),
	S_NORMAL (HEAD, 'L',   -1, NULL 						, NULL),

#define S_HEAD_RAISE (S_HEAD_DIE+6)
	S_NORMAL (HEAD, 'L',	8, NULL 						, &States[S_HEAD_RAISE+1]),
	S_NORMAL (HEAD, 'K',	8, NULL 						, &States[S_HEAD_RAISE+2]),
	S_NORMAL (HEAD, 'J',	8, NULL 						, &States[S_HEAD_RAISE+3]),
	S_NORMAL (HEAD, 'I',	8, NULL 						, &States[S_HEAD_RAISE+4]),
	S_NORMAL (HEAD, 'H',	8, NULL 						, &States[S_HEAD_RAISE+5]),
	S_NORMAL (HEAD, 'G',	8, NULL 						, &States[S_HEAD_RUN+0])
};

void ACacodemon::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 3005;
	info->spawnid = 19;
	info->spawnstate = &States[S_HEAD_STND];
	info->spawnhealth = 400;
	info->seestate = &States[S_HEAD_RUN];
	info->seesound = "caco/sight";
	info->painstate = &States[S_HEAD_PAIN];
	info->painchance = 128;
	info->painsound = "caco/pain";
	info->missilestate = &States[S_HEAD_ATK];
	info->deathstate = &States[S_HEAD_DIE];
	info->deathsound = "caco/death";
	info->speed = 8;
	info->radius = 31 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 400;
	info->activesound = "caco/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_HEAD_RAISE];
}

class AStealthCacodemon : public ACacodemon
{
	DECLARE_STATELESS_ACTOR (AStealthCacodemon, ACacodemon);
public:
	const char *GetObituary () { return OB_STEALTHCACO; }
	const char *GetHitObituary () { return OB_STEALTHCACO; }
};

IMPLEMENT_DEF_SERIAL (AStealthCacodemon, ACacodemon);
REGISTER_ACTOR (AStealthCacodemon, Doom);

void AStealthCacodemon::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9053;
	info->spawnid = 119;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class ACacodemonBall : public AActor
{
	DECLARE_ACTOR (ACacodemonBall, AActor);
};

IMPLEMENT_DEF_SERIAL (ACacodemonBall, AActor);
REGISTER_ACTOR (ACacodemonBall, Doom);

FState ACacodemonBall::States[] =
{
#define S_RBALL 0
	S_BRIGHT (BAL2, 'A',	4, NULL 						, &States[S_RBALL+1]),
	S_BRIGHT (BAL2, 'B',	4, NULL 						, &States[S_RBALL+0]),

#define S_RBALLX (S_RBALL+2)
	S_BRIGHT (BAL2, 'C',	6, NULL 						, &States[S_RBALLX+1]),
	S_BRIGHT (BAL2, 'D',	6, NULL 						, &States[S_RBALLX+2]),
	S_BRIGHT (BAL2, 'E',	6, NULL 						, NULL)
};

void ACacodemonBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 126;
	info->spawnstate = &States[S_RBALL];
	info->seesound = "caco/attack";
	info->deathstate = &States[S_RBALLX];
	info->deathsound = "caco/shotx";
	info->speed = GameSpeed != SPEED_Fast ? 10 * FRACUNIT : 20 * FRACUNIT;
	info->radius = 6 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 5;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC75;
}

void A_HeadAttack (AActor *self)
{
	if (!self->target)
		return;
				
	A_FaceTarget (self);
	if (P_CheckMeleeRange (self))
	{
		int damage = (P_Random (pr_headattack)%6+1)*10;
		P_DamageMobj (self->target, self, self, damage, MOD_HIT);
		return;
	}
	
	// launch a missile
	P_SpawnMissile (self, self->target, RUNTIME_CLASS(ACacodemonBall));
}

// Dead cacodemon ----------------------------------------------------------

class ADeadCacodemon : public ACacodemon
{
	DECLARE_STATELESS_ACTOR (ADeadCacodemon, ACacodemon);
};

IMPLEMENT_DEF_SERIAL (ADeadCacodemon, ACacodemon);
REGISTER_ACTOR (ADeadCacodemon, Doom);

void ADeadCacodemon::SetDefaults (FActorInfo *info)
{
	AActor::SetDefaults (info);
	info->OwnedStates = NULL;
	info->NumOwnedStates = 0;
	info->doomednum = 22;
	info->spawnstate = &States[S_HEAD_DIE+5];
}
