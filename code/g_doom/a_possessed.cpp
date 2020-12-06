#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "dstrings.h"
#include "a_action.h"

void A_PosAttack (AActor *);
void A_SPosAttack (AActor *);
void A_CPosAttack (AActor *);
void A_CPosRefire (AActor *);

// Zombie man --------------------------------------------------------------

class AZombieMan : public AActor
{
	DECLARE_ACTOR (AZombieMan, AActor);
public:
	void NoBlockingSet ();
	const char *GetObituary () { return OB_ZOMBIE; }
};

IMPLEMENT_DEF_SERIAL (AZombieMan, AActor);
REGISTER_ACTOR (AZombieMan, Doom);

FState AZombieMan::States[] =
{
#define S_POSS_STND 0
	S_NORMAL (POSS, 'A',   10, A_Look						, &States[S_POSS_STND+1]),
	S_NORMAL (POSS, 'B',   10, A_Look						, &States[S_POSS_STND]),

#define S_POSS_RUN (S_POSS_STND+2)
	S_NORMAL (POSS, 'A',	4, A_Chase						, &States[S_POSS_RUN+1]),
	S_NORMAL (POSS, 'A',	4, A_Chase						, &States[S_POSS_RUN+2]),
	S_NORMAL (POSS, 'B',	4, A_Chase						, &States[S_POSS_RUN+3]),
	S_NORMAL (POSS, 'B',	4, A_Chase						, &States[S_POSS_RUN+4]),
	S_NORMAL (POSS, 'C',	4, A_Chase						, &States[S_POSS_RUN+5]),
	S_NORMAL (POSS, 'C',	4, A_Chase						, &States[S_POSS_RUN+6]),
	S_NORMAL (POSS, 'D',	4, A_Chase						, &States[S_POSS_RUN+7]),
	S_NORMAL (POSS, 'D',	4, A_Chase						, &States[S_POSS_RUN+0]),

#define S_POSS_ATK (S_POSS_RUN+8)
	S_NORMAL (POSS, 'E',   10, A_FaceTarget 				, &States[S_POSS_ATK+1]),
	S_NORMAL (POSS, 'F',	8, A_PosAttack					, &States[S_POSS_ATK+2]),
	S_NORMAL (POSS, 'E',	8, NULL 						, &States[S_POSS_RUN+0]),

#define S_POSS_PAIN (S_POSS_ATK+3)
	S_NORMAL (POSS, 'G',	3, NULL 						, &States[S_POSS_PAIN+1]),
	S_NORMAL (POSS, 'G',	3, A_Pain						, &States[S_POSS_RUN+0]),

#define S_POSS_DIE (S_POSS_PAIN+2)
	S_NORMAL (POSS, 'H',	5, NULL 						, &States[S_POSS_DIE+1]),
	S_NORMAL (POSS, 'I',	5, A_Scream 					, &States[S_POSS_DIE+2]),
	S_NORMAL (POSS, 'J',	5, A_NoBlocking					, &States[S_POSS_DIE+3]),
	S_NORMAL (POSS, 'K',	5, NULL 						, &States[S_POSS_DIE+4]),
	S_NORMAL (POSS, 'L',   -1, NULL 						, NULL),

#define S_POSS_XDIE (S_POSS_DIE+5)
	S_NORMAL (POSS, 'M',	5, NULL 						, &States[S_POSS_XDIE+1]),
	S_NORMAL (POSS, 'N',	5, A_XScream					, &States[S_POSS_XDIE+2]),
	S_NORMAL (POSS, 'O',	5, A_NoBlocking					, &States[S_POSS_XDIE+3]),
	S_NORMAL (POSS, 'P',	5, NULL 						, &States[S_POSS_XDIE+4]),
	S_NORMAL (POSS, 'Q',	5, NULL 						, &States[S_POSS_XDIE+5]),
	S_NORMAL (POSS, 'R',	5, NULL 						, &States[S_POSS_XDIE+6]),
	S_NORMAL (POSS, 'S',	5, NULL 						, &States[S_POSS_XDIE+7]),
	S_NORMAL (POSS, 'T',	5, NULL 						, &States[S_POSS_XDIE+8]),
	S_NORMAL (POSS, 'U',   -1, NULL 						, NULL),

#define S_POSS_RAISE (S_POSS_XDIE+9)
	S_NORMAL (POSS, 'K',	5, NULL 						, &States[S_POSS_RAISE+1]),
	S_NORMAL (POSS, 'J',	5, NULL 						, &States[S_POSS_RAISE+2]),
	S_NORMAL (POSS, 'I',	5, NULL 						, &States[S_POSS_RAISE+3]),
	S_NORMAL (POSS, 'H',	5, NULL 						, &States[S_POSS_RUN+0])
};

void AZombieMan::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 3004;
	info->spawnid = 4;
	info->spawnstate = &States[S_POSS_STND];
	info->spawnhealth = 20;
	info->seestate = &States[S_POSS_RUN];
	info->seesound = "grunt/sight1";
	info->attacksound = "grunt/attack";
	info->painstate = &States[S_POSS_PAIN];
	info->painchance = 200;
	info->painsound = "grunt/pain";
	info->missilestate = &States[S_POSS_ATK];
	info->deathstate = &States[S_POSS_DIE];
	info->xdeathstate = &States[S_POSS_XDIE];
	info->deathsound = "grunt/death1";
	info->speed = 8;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->activesound = "grunt/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_POSS_RAISE];
}

void AZombieMan::NoBlockingSet ()
{
	P_DropItem (this, "Clip", 0, 256);
}

class AStealthZombieMan : public AZombieMan
{
	DECLARE_STATELESS_ACTOR (AStealthZombieMan, AZombieMan);
public:
	const char *GetObituary () { return OB_STEALTHZOMBIE; }
};

IMPLEMENT_DEF_SERIAL (AStealthZombieMan, AZombieMan);
REGISTER_ACTOR (AStealthZombieMan, Doom);

void AStealthZombieMan::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9061;
	info->spawnid = 102;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

//
// A_PosAttack
//
void A_PosAttack (AActor *self)
{
	int angle;
	int damage;
	int slope;
		
	if (!self->target)
		return;
				
	A_FaceTarget (self);
	angle = self->angle;
	slope = P_AimLineAttack (self, angle, MISSILERANGE);

	S_Sound (self, CHAN_WEAPON, "grunt/attack", 1, ATTN_NORM);
	angle += PS_Random (pr_posattack) << 20;
	damage = ((P_Random (pr_posattack)%5)+1)*3;
	P_LineAttack (self, angle, MISSILERANGE, slope, damage);
}

// Dead zombie man ---------------------------------------------------------

class ADeadZombieMan : public AZombieMan
{
	DECLARE_STATELESS_ACTOR (ADeadZombieMan, AZombieMan);
public:
	void NoBlockingSet () {}
};

IMPLEMENT_DEF_SERIAL (ADeadZombieMan, AZombieMan);
REGISTER_ACTOR (ADeadZombieMan, Doom);

void ADeadZombieMan::SetDefaults (FActorInfo *info)
{
	AActor::SetDefaults (info);
	info->OwnedStates = NULL;
	info->NumOwnedStates = 0;
	info->doomednum = 18;
	info->spawnstate = &States[S_POSS_DIE+4];
}

// Shotgun guy -------------------------------------------------------------

class AShotgunGuy : public AActor
{
	DECLARE_ACTOR (AShotgunGuy, AActor);
public:
	void NoBlockingSet ();
	const char *GetObituary () { return OB_SHOTGUY; }
};

IMPLEMENT_DEF_SERIAL (AShotgunGuy, AActor);
REGISTER_ACTOR (AShotgunGuy, Doom);

FState AShotgunGuy::States[] =
{

#define S_SPOS_STND 0
	S_NORMAL (SPOS, 'A',   10, A_Look						, &States[S_SPOS_STND+1]),
	S_NORMAL (SPOS, 'B',   10, A_Look						, &States[S_SPOS_STND]),

#define S_SPOS_RUN (S_SPOS_STND+2)
	S_NORMAL (SPOS, 'A',	3, A_Chase						, &States[S_SPOS_RUN+1]),
	S_NORMAL (SPOS, 'A',	3, A_Chase						, &States[S_SPOS_RUN+2]),
	S_NORMAL (SPOS, 'B',	3, A_Chase						, &States[S_SPOS_RUN+3]),
	S_NORMAL (SPOS, 'B',	3, A_Chase						, &States[S_SPOS_RUN+4]),
	S_NORMAL (SPOS, 'C',	3, A_Chase						, &States[S_SPOS_RUN+5]),
	S_NORMAL (SPOS, 'C',	3, A_Chase						, &States[S_SPOS_RUN+6]),
	S_NORMAL (SPOS, 'D',	3, A_Chase						, &States[S_SPOS_RUN+7]),
	S_NORMAL (SPOS, 'D',	3, A_Chase						, &States[S_SPOS_RUN+0]),

#define S_SPOS_ATK (S_SPOS_RUN+8)
	S_NORMAL (SPOS, 'E',   10, A_FaceTarget 				, &States[S_SPOS_ATK+1]),
	S_BRIGHT (SPOS, 'F',   10, A_SPosAttack 				, &States[S_SPOS_ATK+2]),
	S_NORMAL (SPOS, 'E',   10, NULL 						, &States[S_SPOS_RUN+0]),

#define S_SPOS_PAIN (S_SPOS_ATK+3)
	S_NORMAL (SPOS, 'G',	3, NULL 						, &States[S_SPOS_PAIN+1]),
	S_NORMAL (SPOS, 'G',	3, A_Pain						, &States[S_SPOS_RUN+0]),

#define S_SPOS_DIE (S_SPOS_PAIN+2)
	S_NORMAL (SPOS, 'H',	5, NULL 						, &States[S_SPOS_DIE+1]),
	S_NORMAL (SPOS, 'I',	5, A_Scream 					, &States[S_SPOS_DIE+2]),
	S_NORMAL (SPOS, 'J',	5, A_NoBlocking					, &States[S_SPOS_DIE+3]),
	S_NORMAL (SPOS, 'K',	5, NULL 						, &States[S_SPOS_DIE+4]),
	S_NORMAL (SPOS, 'L',   -1, NULL 						, NULL),

#define S_SPOS_XDIE (S_SPOS_DIE+5)
	S_NORMAL (SPOS, 'M',	5, NULL 						, &States[S_SPOS_XDIE+1]),
	S_NORMAL (SPOS, 'N',	5, A_XScream					, &States[S_SPOS_XDIE+2]),
	S_NORMAL (SPOS, 'O',	5, A_NoBlocking					, &States[S_SPOS_XDIE+3]),
	S_NORMAL (SPOS, 'P',	5, NULL 						, &States[S_SPOS_XDIE+4]),
	S_NORMAL (SPOS, 'Q',	5, NULL 						, &States[S_SPOS_XDIE+5]),
	S_NORMAL (SPOS, 'R',	5, NULL 						, &States[S_SPOS_XDIE+6]),
	S_NORMAL (SPOS, 'S',	5, NULL 						, &States[S_SPOS_XDIE+7]),
	S_NORMAL (SPOS, 'T',	5, NULL 						, &States[S_SPOS_XDIE+8]),
	S_NORMAL (SPOS, 'U',   -1, NULL 						, NULL),

#define S_SPOS_RAISE (S_SPOS_XDIE+9)
	S_NORMAL (SPOS, 'L',	5, NULL 						, &States[S_SPOS_RAISE+1]),
	S_NORMAL (SPOS, 'K',	5, NULL 						, &States[S_SPOS_RAISE+2]),
	S_NORMAL (SPOS, 'J',	5, NULL 						, &States[S_SPOS_RAISE+3]),
	S_NORMAL (SPOS, 'I',	5, NULL 						, &States[S_SPOS_RAISE+4]),
	S_NORMAL (SPOS, 'H',	5, NULL 						, &States[S_SPOS_RUN+0])
};

void AShotgunGuy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 9;
	info->spawnid = 1;
	info->spawnstate = &States[S_SPOS_STND];
	info->spawnhealth = 30;
	info->seestate = &States[S_SPOS_RUN];
	info->seesound = "shotguy/sight1";
	info->painstate = &States[S_SPOS_PAIN];
	info->painchance = 170;
	info->painsound = "shotguy/pain";
	info->missilestate = &States[S_SPOS_ATK];
	info->deathstate = &States[S_SPOS_DIE];
	info->xdeathstate = &States[S_SPOS_XDIE];
	info->deathsound = "shotguy/death1";
	info->speed = 8;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->activesound = "shotguy/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_SPOS_RAISE];
}

void AShotgunGuy::NoBlockingSet ()
{
	P_DropItem (this, "Shotgun", 0, 256);
}

class AStealthShotgunGuy : public AShotgunGuy
{
	DECLARE_STATELESS_ACTOR (AStealthShotgunGuy, AShotgunGuy);
public:
	const char *GetObituary () { return OB_STEALTHSHOTGUY; }
};

IMPLEMENT_DEF_SERIAL (AStealthShotgunGuy, AShotgunGuy);
REGISTER_ACTOR (AStealthShotgunGuy, Doom);

void AStealthShotgunGuy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9060;
	info->spawnid = 103;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

void A_SPosAttack (AActor *self)
{
	int i;
	int bangle;
	int slope;
		
	if (!self->target)
		return;

	S_Sound (self, CHAN_WEAPON, "shotguy/attack", 1, ATTN_NORM);
	A_FaceTarget (self);
	bangle = self->angle;
	slope = P_AimLineAttack (self, bangle, MISSILERANGE);

	for (i=0 ; i<3 ; i++)
    {
		int angle = bangle + (PS_Random (pr_sposattack) << 20);
		int damage = ((P_Random (pr_sposattack)%5)+1)*3;
		P_LineAttack(self, angle, MISSILERANGE, slope, damage);
    }
}

// Dead shotgun guy --------------------------------------------------------

class ADeadShotgunGuy : public AShotgunGuy
{
	DECLARE_STATELESS_ACTOR (ADeadShotgunGuy, AShotgunGuy);
public:
	void NoBlockingSet () {}
};

IMPLEMENT_DEF_SERIAL (ADeadShotgunGuy, AShotgunGuy);
REGISTER_ACTOR (ADeadShotgunGuy, Doom);

void ADeadShotgunGuy::SetDefaults (FActorInfo *info)
{
	AActor::SetDefaults (info);
	info->OwnedStates = NULL;
	info->NumOwnedStates = 0;
	info->doomednum = 19;
	info->spawnstate = &States[S_SPOS_DIE+4];
}

// Chaingun guy ------------------------------------------------------------

class AChaingunGuy : public AActor
{
	DECLARE_ACTOR (AChaingunGuy, AActor);
public:
	void NoBlockingSet ();
	const char *GetObituary () { return OB_CHAINGUY; }
};

IMPLEMENT_DEF_SERIAL (AChaingunGuy, AActor);
REGISTER_ACTOR (AChaingunGuy, Doom);

FState AChaingunGuy::States[] =
{
#define S_CPOS_STND 0
	S_NORMAL (CPOS, 'A',   10, A_Look						, &States[S_CPOS_STND+1]),
	S_NORMAL (CPOS, 'B',   10, A_Look						, &States[S_CPOS_STND]),

#define S_CPOS_RUN (S_CPOS_STND+2)
	S_NORMAL (CPOS, 'A',	3, A_Chase						, &States[S_CPOS_RUN+1]),
	S_NORMAL (CPOS, 'A',	3, A_Chase						, &States[S_CPOS_RUN+2]),
	S_NORMAL (CPOS, 'B',	3, A_Chase						, &States[S_CPOS_RUN+3]),
	S_NORMAL (CPOS, 'B',	3, A_Chase						, &States[S_CPOS_RUN+4]),
	S_NORMAL (CPOS, 'C',	3, A_Chase						, &States[S_CPOS_RUN+5]),
	S_NORMAL (CPOS, 'C',	3, A_Chase						, &States[S_CPOS_RUN+6]),
	S_NORMAL (CPOS, 'D',	3, A_Chase						, &States[S_CPOS_RUN+7]),
	S_NORMAL (CPOS, 'D',	3, A_Chase						, &States[S_CPOS_RUN+0]),

#define S_CPOS_ATK (S_CPOS_RUN+8)
	S_NORMAL (CPOS, 'E',   10, A_FaceTarget 				, &States[S_CPOS_ATK+1]),
	S_BRIGHT (CPOS, 'F',	4, A_CPosAttack 				, &States[S_CPOS_ATK+2]),
	S_BRIGHT (CPOS, 'E',	4, A_CPosAttack 				, &States[S_CPOS_ATK+3]),
	S_NORMAL (CPOS, 'F',	1, A_CPosRefire 				, &States[S_CPOS_ATK+1]),

#define S_CPOS_PAIN (S_CPOS_ATK+4)
	S_NORMAL (CPOS, 'G',	3, NULL 						, &States[S_CPOS_PAIN+1]),
	S_NORMAL (CPOS, 'G',	3, A_Pain						, &States[S_CPOS_RUN+0]),

#define S_CPOS_DIE (S_CPOS_PAIN+2)
	S_NORMAL (CPOS, 'H',	5, NULL 						, &States[S_CPOS_DIE+1]),
	S_NORMAL (CPOS, 'I',	5, A_Scream 					, &States[S_CPOS_DIE+2]),
	S_NORMAL (CPOS, 'J',	5, A_NoBlocking					, &States[S_CPOS_DIE+3]),
	S_NORMAL (CPOS, 'K',	5, NULL 						, &States[S_CPOS_DIE+4]),
	S_NORMAL (CPOS, 'L',	5, NULL 						, &States[S_CPOS_DIE+5]),
	S_NORMAL (CPOS, 'M',	5, NULL 						, &States[S_CPOS_DIE+6]),
	S_NORMAL (CPOS, 'N',   -1, NULL 						, NULL),

#define S_CPOS_XDIE (S_CPOS_DIE+7)
	S_NORMAL (CPOS, 'O',	5, NULL 						, &States[S_CPOS_XDIE+1]),
	S_NORMAL (CPOS, 'P',	5, A_XScream					, &States[S_CPOS_XDIE+2]),
	S_NORMAL (CPOS, 'Q',	5, A_NoBlocking					, &States[S_CPOS_XDIE+3]),
	S_NORMAL (CPOS, 'R',	5, NULL 						, &States[S_CPOS_XDIE+4]),
	S_NORMAL (CPOS, 'S',	5, NULL 						, &States[S_CPOS_XDIE+5]),
	S_NORMAL (CPOS, 'T',   -1, NULL 						, NULL),

#define S_CPOS_RAISE (S_CPOS_XDIE+6)
	S_NORMAL (CPOS, 'N',	5, NULL 						, &States[S_CPOS_RAISE+1]),
	S_NORMAL (CPOS, 'M',	5, NULL 						, &States[S_CPOS_RAISE+2]),
	S_NORMAL (CPOS, 'L',	5, NULL 						, &States[S_CPOS_RAISE+3]),
	S_NORMAL (CPOS, 'K',	5, NULL 						, &States[S_CPOS_RAISE+4]),
	S_NORMAL (CPOS, 'J',	5, NULL 						, &States[S_CPOS_RAISE+5]),
	S_NORMAL (CPOS, 'I',	5, NULL 						, &States[S_CPOS_RAISE+6]),
	S_NORMAL (CPOS, 'H',	5, NULL 						, &States[S_CPOS_RUN+0])
};

void AChaingunGuy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 65;
	info->spawnid = 2;
	info->spawnstate = &States[S_CPOS_STND];
	info->spawnhealth = 70;
	info->seestate = &States[S_CPOS_RUN];
	info->seesound = "chainguy/sight1";
	info->painstate = &States[S_CPOS_PAIN];
	info->painchance = 170;
	info->painsound = "chainguy/pain";
	info->missilestate = &States[S_CPOS_ATK];
	info->deathstate = &States[S_CPOS_DIE];
	info->xdeathstate = &States[S_CPOS_XDIE];
	info->deathsound = "chainguy/death1";
	info->speed = 8;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->activesound = "chainguy/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_CPOS_RAISE];
}

void AChaingunGuy::NoBlockingSet ()
{
	P_DropItem (this, "Chaingun", 0, 256);
}

class AStealthChaingunGuy : public AChaingunGuy
{
	DECLARE_STATELESS_ACTOR (AStealthChaingunGuy, AChaingunGuy);
public:
	const char *GetObituary () { return OB_STEALTHCHAINGUY; }
};

IMPLEMENT_DEF_SERIAL (AStealthChaingunGuy, AChaingunGuy);
REGISTER_ACTOR (AStealthChaingunGuy, Doom);

void AStealthChaingunGuy::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9054;
	info->spawnid = 120;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

// Wolfenstein SS ----------------------------------------------------------

class AWolfensteinSS : public AActor
{
	DECLARE_ACTOR (AWolfensteinSS, AActor);
public:
	const char *GetObituary () { return OB_WOLFSS; }
	void NoBlockingSet ();
};

IMPLEMENT_DEF_SERIAL (AWolfensteinSS, AActor);
REGISTER_ACTOR (AWolfensteinSS, Doom);

FState AWolfensteinSS::States[] =
{
#define S_SSWV_STND 0
	S_NORMAL (SSWV, 'A',   10, A_Look						, &States[S_SSWV_STND+1]),
	S_NORMAL (SSWV, 'B',   10, A_Look						, &States[S_SSWV_STND]),

#define S_SSWV_RUN (S_SSWV_STND+2)
	S_NORMAL (SSWV, 'A',	3, A_Chase						, &States[S_SSWV_RUN+1]),
	S_NORMAL (SSWV, 'A',	3, A_Chase						, &States[S_SSWV_RUN+2]),
	S_NORMAL (SSWV, 'B',	3, A_Chase						, &States[S_SSWV_RUN+3]),
	S_NORMAL (SSWV, 'B',	3, A_Chase						, &States[S_SSWV_RUN+4]),
	S_NORMAL (SSWV, 'C',	3, A_Chase						, &States[S_SSWV_RUN+5]),
	S_NORMAL (SSWV, 'C',	3, A_Chase						, &States[S_SSWV_RUN+6]),
	S_NORMAL (SSWV, 'D',	3, A_Chase						, &States[S_SSWV_RUN+7]),
	S_NORMAL (SSWV, 'D',	3, A_Chase						, &States[S_SSWV_RUN+0]),

#define S_SSWV_ATK (S_SSWV_RUN+8)
	S_NORMAL (SSWV, 'E',   10, A_FaceTarget 				, &States[S_SSWV_ATK+1]),
	S_NORMAL (SSWV, 'F',   10, A_FaceTarget 				, &States[S_SSWV_ATK+2]),
	S_BRIGHT (SSWV, 'G',	4, A_CPosAttack 				, &States[S_SSWV_ATK+3]),
	S_NORMAL (SSWV, 'F',	6, A_FaceTarget 				, &States[S_SSWV_ATK+4]),
	S_BRIGHT (SSWV, 'G',	4, A_CPosAttack 				, &States[S_SSWV_ATK+5]),
	S_NORMAL (SSWV, 'F',	1, A_CPosRefire 				, &States[S_SSWV_ATK+1]),

#define S_SSWV_PAIN (S_SSWV_ATK+6)
	S_NORMAL (SSWV, 'H',	3, NULL 						, &States[S_SSWV_PAIN+1]),
	S_NORMAL (SSWV, 'H',	3, A_Pain						, &States[S_SSWV_RUN+0]),

#define S_SSWV_DIE (S_SSWV_PAIN+2)
	S_NORMAL (SSWV, 'I',	5, NULL 						, &States[S_SSWV_DIE+1]),
	S_NORMAL (SSWV, 'J',	5, A_Scream 					, &States[S_SSWV_DIE+2]),
	S_NORMAL (SSWV, 'K',	5, A_NoBlocking					, &States[S_SSWV_DIE+3]),
	S_NORMAL (SSWV, 'L',	5, NULL 						, &States[S_SSWV_DIE+4]),
	S_NORMAL (SSWV, 'M',   -1, NULL 						, NULL),

#define S_SSWV_XDIE (S_SSWV_DIE+5)
	S_NORMAL (SSWV, 'N',	5, NULL 						, &States[S_SSWV_XDIE+1]),
	S_NORMAL (SSWV, 'O',	5, A_XScream					, &States[S_SSWV_XDIE+2]),
	S_NORMAL (SSWV, 'P',	5, A_NoBlocking					, &States[S_SSWV_XDIE+3]),
	S_NORMAL (SSWV, 'Q',	5, NULL 						, &States[S_SSWV_XDIE+4]),
	S_NORMAL (SSWV, 'R',	5, NULL 						, &States[S_SSWV_XDIE+5]),
	S_NORMAL (SSWV, 'S',	5, NULL 						, &States[S_SSWV_XDIE+6]),
	S_NORMAL (SSWV, 'T',	5, NULL 						, &States[S_SSWV_XDIE+7]),
	S_NORMAL (SSWV, 'U',	5, NULL 						, &States[S_SSWV_XDIE+8]),
	S_NORMAL (SSWV, 'V',   -1, NULL 						, NULL),

#define S_SSWV_RAISE (S_SSWV_XDIE+9)
	S_NORMAL (SSWV, 'M',	5, NULL 						, &States[S_SSWV_RAISE+1]),
	S_NORMAL (SSWV, 'L',	5, NULL 						, &States[S_SSWV_RAISE+2]),
	S_NORMAL (SSWV, 'K',	5, NULL 						, &States[S_SSWV_RAISE+3]),
	S_NORMAL (SSWV, 'J',	5, NULL 						, &States[S_SSWV_RAISE+4]),
	S_NORMAL (SSWV, 'I',	5, NULL 						, &States[S_SSWV_RUN+0])
};

void AWolfensteinSS::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 84;
	info->spawnid = 116;
	info->spawnstate = &States[S_SSWV_STND];
	info->spawnhealth = 50;
	info->seestate = &States[S_SSWV_RUN];
	info->seesound = "wolfss/sight";
	info->painstate = &States[S_SSWV_PAIN];
	info->painchance = 170;
	info->painsound = "wolfss/pain";
	info->missilestate = &States[S_SSWV_ATK];
	info->deathstate = &States[S_SSWV_DIE];
	info->xdeathstate = &States[S_SSWV_XDIE];
	info->deathsound = "wolfss/death";
	info->speed = 8;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->activesound = "wolfss/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_SSWV_RAISE];
}

void AWolfensteinSS::NoBlockingSet ()
{
	P_DropItem (this, "Clip", 0, 256);
}

void A_CPosAttack (AActor *self)
{
	int angle;
	int bangle;
	int damage;
	int slope;
		
	if (!self->target)
		return;

	// [RH] Andy Baker's stealth monsters
	if (self->flags & MF_STEALTH)
	{
		self->visdir = 1;
	}

	S_Sound (self, CHAN_WEAPON, "chainguy/attack", 1, ATTN_NORM);
	A_FaceTarget (self);
	bangle = self->angle;
	slope = P_AimLineAttack (self, bangle, MISSILERANGE);

	angle = bangle + (PS_Random (pr_cposattack) << 20);
	damage = ((P_Random (pr_cposattack)%5)+1)*3;
	P_LineAttack (self, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire (AActor *self)
{		
	// keep firing unless target got out of sight
	A_FaceTarget (self);

	if (P_Random (pr_cposrefire) < 40)
		return;

	if (!self->target
		|| self->target->health <= 0
		|| !P_CheckSight (self, self->target, false) )
	{
		self->SetState (RUNTIME_TYPE(self)->ActorInfo->seestate);
	}
}
