#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "doomstat.h"
#include "dstrings.h"
#include "a_action.h"

void A_BruisAttack (AActor *);

class ABaronOfHell : public AActor
{
	DECLARE_ACTOR (ABaronOfHell, AActor);
public:
	const char *GetObituary () { return OB_BARON; }
	const char *GetHitObituary () { return OB_BARONHIT; }
};

IMPLEMENT_DEF_SERIAL (ABaronOfHell, AActor);
REGISTER_ACTOR (ABaronOfHell, Doom);

FState ABaronOfHell::States[] =
{
#define S_BOSS_STND 0
	S_NORMAL (BOSS, 'A',   10, A_Look						, &States[S_BOSS_STND+1]),
	S_NORMAL (BOSS, 'B',   10, A_Look						, &States[S_BOSS_STND]),

#define S_BOSS_RUN (S_BOSS_STND+2)
	S_NORMAL (BOSS, 'A',	3, A_Chase						, &States[S_BOSS_RUN+1]),
	S_NORMAL (BOSS, 'A',	3, A_Chase						, &States[S_BOSS_RUN+2]),
	S_NORMAL (BOSS, 'B',	3, A_Chase						, &States[S_BOSS_RUN+3]),
	S_NORMAL (BOSS, 'B',	3, A_Chase						, &States[S_BOSS_RUN+4]),
	S_NORMAL (BOSS, 'C',	3, A_Chase						, &States[S_BOSS_RUN+5]),
	S_NORMAL (BOSS, 'C',	3, A_Chase						, &States[S_BOSS_RUN+6]),
	S_NORMAL (BOSS, 'D',	3, A_Chase						, &States[S_BOSS_RUN+7]),
	S_NORMAL (BOSS, 'D',	3, A_Chase						, &States[S_BOSS_RUN+0]),

#define S_BOSS_ATK (S_BOSS_RUN+8)
	S_NORMAL (BOSS, 'E',	8, A_FaceTarget 				, &States[S_BOSS_ATK+1]),
	S_NORMAL (BOSS, 'F',	8, A_FaceTarget 				, &States[S_BOSS_ATK+2]),
	S_NORMAL (BOSS, 'G',	8, A_BruisAttack				, &States[S_BOSS_RUN+0]),

#define S_BOSS_PAIN (S_BOSS_ATK+3)
	S_NORMAL (BOSS, 'H',	2, NULL 						, &States[S_BOSS_PAIN+1]),
	S_NORMAL (BOSS, 'H',	2, A_Pain						, &States[S_BOSS_RUN+0]),

#define S_BOSS_DIE (S_BOSS_PAIN+2)
	S_NORMAL (BOSS, 'I',	8, NULL 						, &States[S_BOSS_DIE+1]),
	S_NORMAL (BOSS, 'J',	8, A_Scream 					, &States[S_BOSS_DIE+2]),
	S_NORMAL (BOSS, 'K',	8, NULL 						, &States[S_BOSS_DIE+3]),
	S_NORMAL (BOSS, 'L',	8, A_NoBlocking					, &States[S_BOSS_DIE+4]),
	S_NORMAL (BOSS, 'M',	8, NULL 						, &States[S_BOSS_DIE+5]),
	S_NORMAL (BOSS, 'N',	8, NULL 						, &States[S_BOSS_DIE+6]),
	S_NORMAL (BOSS, 'O',   -1, A_BossDeath					, NULL),

#define S_BOSS_RAISE (S_BOSS_DIE+7)
	S_NORMAL (BOSS, 'O',	8, NULL 						, &States[S_BOSS_RAISE+1]),
	S_NORMAL (BOSS, 'N',	8, NULL 						, &States[S_BOSS_RAISE+2]),
	S_NORMAL (BOSS, 'M',	8, NULL 						, &States[S_BOSS_RAISE+3]),
	S_NORMAL (BOSS, 'L',	8, NULL 						, &States[S_BOSS_RAISE+4]),
	S_NORMAL (BOSS, 'K',	8, NULL 						, &States[S_BOSS_RAISE+5]),
	S_NORMAL (BOSS, 'J',	8, NULL 						, &States[S_BOSS_RAISE+6]),
	S_NORMAL (BOSS, 'I',	8, NULL 						, &States[S_BOSS_RUN+0])
};

void ABaronOfHell::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 3003;
	info->spawnid = 3;
	info->spawnstate = &States[S_BOSS_STND];
	info->spawnhealth = 1000;
	info->seestate = &States[S_BOSS_RUN];
	info->seesound = "baron/sight";
	info->painstate = &States[S_BOSS_PAIN];
	info->painchance = 50;
	info->painsound = "baron/pain";
	info->meleestate = &States[S_BOSS_ATK];
	info->missilestate = &States[S_BOSS_ATK];
	info->deathstate = &States[S_BOSS_DIE];
	info->deathsound = "baron/death";
	info->speed = 8;
	info->radius = 24 * FRACUNIT;
	info->height = 64 * FRACUNIT;
	info->mass = 1000;
	info->activesound = "baron/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_BOSS_RAISE];
};

class AStealthBaron : public ABaronOfHell
{
	DECLARE_STATELESS_ACTOR (AStealthBaron, ABaronOfHell);
public:
	const char *GetObituary () { return OB_STEALTHBARON; }
	const char *GetHitObituary () { return OB_STEALTHBARON; }
};

IMPLEMENT_DEF_SERIAL (AStealthBaron, ABaronOfHell);
REGISTER_ACTOR (AStealthBaron, Doom);

void AStealthBaron::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9052;
	info->spawnid = 100;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class ABaronBall : public AActor
{
	DECLARE_ACTOR (ABaronBall, AActor);
};

IMPLEMENT_DEF_SERIAL (ABaronBall, AActor);
REGISTER_ACTOR (ABaronBall, Doom);

FState ABaronBall::States[] =
{
#define S_BRBALL 0
	S_BRIGHT (BAL7, 'A',	4, NULL 						, &States[S_BRBALL+1]),
	S_BRIGHT (BAL7, 'B',	4, NULL 						, &States[S_BRBALL+0]),

#define S_BRBALLX (S_BRBALL+2)
	S_BRIGHT (BAL7, 'C',	6, NULL 						, &States[S_BRBALLX+1]),
	S_BRIGHT (BAL7, 'D',	6, NULL 						, &States[S_BRBALLX+2]),
	S_BRIGHT (BAL7, 'E',	6, NULL 						, NULL)
};

void ABaronBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BRBALL];
	info->seesound = "baron/attack";
	info->deathstate = &States[S_BRBALLX];
	info->deathsound = "baron/shotx";
	info->speed = GameSpeed != SPEED_Fast ? 15 * FRACUNIT : 20 * FRACUNIT;
	info->radius = 6 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->damage = 8;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC75;
}

class AHellKnight : public ABaronOfHell
{
	DECLARE_ACTOR (AHellKnight, ABaronOfHell);
public:
	const char *GetObituary () { return OB_KNIGHT; }
	const char *GetHitObituary () { return OB_KNIGHTHIT; }
};

IMPLEMENT_DEF_SERIAL (AHellKnight, ABaronOfHell);
REGISTER_ACTOR (AHellKnight, Doom);

FState AHellKnight::States[] =
{
#define S_BOS2_STND 0
	S_NORMAL (BOS2, 'A',   10, A_Look						, &States[S_BOS2_STND+1]),
	S_NORMAL (BOS2, 'B',   10, A_Look						, &States[S_BOS2_STND]),

#define S_BOS2_RUN (S_BOS2_STND+2)
	S_NORMAL (BOS2, 'A',	3, A_Chase						, &States[S_BOS2_RUN+1]),
	S_NORMAL (BOS2, 'A',	3, A_Chase						, &States[S_BOS2_RUN+2]),
	S_NORMAL (BOS2, 'B',	3, A_Chase						, &States[S_BOS2_RUN+3]),
	S_NORMAL (BOS2, 'B',	3, A_Chase						, &States[S_BOS2_RUN+4]),
	S_NORMAL (BOS2, 'C',	3, A_Chase						, &States[S_BOS2_RUN+5]),
	S_NORMAL (BOS2, 'C',	3, A_Chase						, &States[S_BOS2_RUN+6]),
	S_NORMAL (BOS2, 'D',	3, A_Chase						, &States[S_BOS2_RUN+7]),
	S_NORMAL (BOS2, 'D',	3, A_Chase						, &States[S_BOS2_RUN+0]),

#define S_BOS2_ATK (S_BOS2_RUN+8)
	S_NORMAL (BOS2, 'E',	8, A_FaceTarget 				, &States[S_BOS2_ATK+1]),
	S_NORMAL (BOS2, 'F',	8, A_FaceTarget 				, &States[S_BOS2_ATK+2]),
	S_NORMAL (BOS2, 'G',	8, A_BruisAttack				, &States[S_BOS2_RUN+0]),

#define S_BOS2_PAIN (S_BOS2_ATK+3)
	S_NORMAL (BOS2, 'H',	2, NULL 						, &States[S_BOS2_PAIN+1]),
	S_NORMAL (BOS2, 'H',	2, A_Pain						, &States[S_BOS2_RUN+0]),

#define S_BOS2_DIE (S_BOS2_PAIN+2)
	S_NORMAL (BOS2, 'I',	8, NULL 						, &States[S_BOS2_DIE+1]),
	S_NORMAL (BOS2, 'J',	8, A_Scream 					, &States[S_BOS2_DIE+2]),
	S_NORMAL (BOS2, 'K',	8, NULL 						, &States[S_BOS2_DIE+3]),
	S_NORMAL (BOS2, 'L',	8, A_NoBlocking						, &States[S_BOS2_DIE+4]),
	S_NORMAL (BOS2, 'M',	8, NULL 						, &States[S_BOS2_DIE+5]),
	S_NORMAL (BOS2, 'N',	8, NULL 						, &States[S_BOS2_DIE+6]),
	S_NORMAL (BOS2, 'O',   -1, NULL 						, NULL),

#define S_BOS2_RAISE (S_BOS2_DIE+7)
	S_NORMAL (BOS2, 'O',	8, NULL 						, &States[S_BOS2_RAISE+1]),
	S_NORMAL (BOS2, 'N',	8, NULL 						, &States[S_BOS2_RAISE+2]),
	S_NORMAL (BOS2, 'M',	8, NULL 						, &States[S_BOS2_RAISE+3]),
	S_NORMAL (BOS2, 'L',	8, NULL 						, &States[S_BOS2_RAISE+4]),
	S_NORMAL (BOS2, 'K',	8, NULL 						, &States[S_BOS2_RAISE+5]),
	S_NORMAL (BOS2, 'J',	8, NULL 						, &States[S_BOS2_RAISE+6]),
	S_NORMAL (BOS2, 'I',	8, NULL 						, &States[S_BOS2_RUN+0])
};

void AHellKnight::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 69;
	info->spawnid = 113;
	info->spawnstate = &States[S_BOS2_STND];
	info->spawnhealth = 500;
	info->seestate = &States[S_BOS2_RUN];
	info->seesound = "knight/sight";
	info->painstate = &States[S_BOS2_PAIN];
	info->painchance = 50;
	info->painsound = "knight/pain";
	info->meleestate = &States[S_BOS2_ATK];
	info->missilestate = &States[S_BOS2_ATK];
	info->deathstate = &States[S_BOS2_DIE];
	info->deathsound = "knight/death";
	info->speed = 8;
	info->radius = 24 * FRACUNIT;
	info->height = 64 * FRACUNIT;
	info->mass = 1000;
	info->activesound = "knight/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_BOS2_RAISE];
}

class AStealthHellKnight : public AHellKnight
{
	DECLARE_STATELESS_ACTOR (AStealthHellKnight, AHellKnight);
public:
	const char *GetObituary () { return OB_STEALTHKNIGHT; }
	const char *GetHitObituary () { return OB_STEALTHKNIGHT; }
};

IMPLEMENT_DEF_SERIAL (AStealthHellKnight, AHellKnight);
REGISTER_ACTOR (AStealthHellKnight, Doom);

void AStealthHellKnight::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9056;
	info->spawnid = 101;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

void A_BruisAttack (AActor *self)
{
	if (!self->target)
		return;
				
	if (P_CheckMeleeRange (self))
	{
		int damage = (P_Random (pr_bruisattack)%8+1)*10;
		S_Sound (self, CHAN_WEAPON, "baron/melee", 1, ATTN_NORM);
		P_DamageMobj (self->target, self, self, damage, MOD_HIT);
		return;
	}
	
	// launch a missile
	P_SpawnMissile (self, self->target, RUNTIME_CLASS(ABaronBall));
}
