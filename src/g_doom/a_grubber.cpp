#include "templates.h"
#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "gstrings.h"
#include "a_action.h"
#include "p_effect.h"
#include "v_video.h"

void A_ThrowGibs (AActor *);

class ABloodGib : public AActor
{
	DECLARE_ACTOR (ABloodGib, AActor)
public:
	void BeginPlay ();
};

FState ABloodGib::States[] =
{
#define S_GRB_BLOODGIB1 0
	S_NORMAL (BLUD, 'A',	1, NULL							, &States[S_GRB_BLOODGIB1]),
#define S_GRB_BLOODGIB2 (S_GRB_BLOODGIB1+1)
	S_NORMAL (BLUD, 'B',	1, NULL							, &States[S_GRB_BLOODGIB2]),
#define S_GRB_BLOODGIB3 (S_GRB_BLOODGIB2+1)
	S_NORMAL (BLUD, 'C',	1, NULL							, &States[S_GRB_BLOODGIB3])
};

IMPLEMENT_ACTOR (ABloodGib, Doom, -1, 0)
	PROP_RadiusFixed (6)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (100)
	PROP_Damage (3)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF)
	PROP_Flags2 (MF2_PCROSS|MF2_NOTELEPORT|MF2_FLOORBOUNCE)

	PROP_SpawnState (S_GRB_BLOODGIB1)
	PROP_DeathState (S_GRB_BLOODGIB1)
END_DEFAULTS

void ABloodGib::BeginPlay ()
{
	int rnd = M_Random ();

	Super::BeginPlay ();
	effects |= FX_GIB;

	if (rnd < 96)
		SetState (&States[S_GRB_BLOODGIB1]);
	else if (rnd < 192)
		SetState (&States[S_GRB_BLOODGIB2]);
	else
		SetState (&States[S_GRB_BLOODGIB3]);
}

//
// A_ThrowGibs
//

void A_ThrowGibs (AActor *actor)
{
	int r = M_Random ();

	for (int i = 0; i <= 25; i++)
	{
		P_SpawnMissileAngle (actor, RUNTIME_CLASS (ABloodGib),
			r, actor->momz);
	}
}

// Explodemon

void A_ExSargAttack (AActor *);

class AExplodemon : public AActor
{
	DECLARE_ACTOR (AExplodemon, AActor)
public:
	const char *GetHitObituary () { return GStrings(OB_DEMONHIT); }
};

FState AExplodemon::States[] =
{
#define S_EXSARG_STND 0
	S_NORMAL (ESRG, 'A',   10, A_Look						, &States[S_EXSARG_STND+1]),
	S_NORMAL (ESRG, 'B',   10, A_Look						, &States[S_EXSARG_STND]),

#define S_EXSARG_RUN (S_EXSARG_STND+2)
	S_NORMAL (ESRG, 'A',	2, A_Chase						, &States[S_EXSARG_RUN+1]),
	S_NORMAL (ESRG, 'A',	2, A_Chase						, &States[S_EXSARG_RUN+2]),
	S_NORMAL (ESRG, 'B',	2, A_Chase						, &States[S_EXSARG_RUN+3]),
	S_NORMAL (ESRG, 'B',	2, A_Chase						, &States[S_EXSARG_RUN+4]),
	S_NORMAL (ESRG, 'C',	2, A_Chase						, &States[S_EXSARG_RUN+5]),
	S_NORMAL (ESRG, 'C',	2, A_Chase						, &States[S_EXSARG_RUN+6]),
	S_NORMAL (ESRG, 'D',	2, A_Chase						, &States[S_EXSARG_RUN+7]),
	S_NORMAL (ESRG, 'D',	2, A_Chase						, &States[S_EXSARG_RUN+0]),

#define S_EXSARG_ATK (S_EXSARG_RUN+8)
	S_NORMAL (ESRG, 'E',	8, A_FaceTarget 				, &States[S_EXSARG_ATK+1]),
	S_NORMAL (ESRG, 'F',	8, A_FaceTarget 				, &States[S_EXSARG_ATK+2]),
	S_NORMAL (ESRG, 'G',	8, A_ExSargAttack 				, &States[S_EXSARG_RUN+0]),

#define S_EXSARG_PAIN (S_EXSARG_ATK+3)
	S_NORMAL (ESRG, 'H',	2, NULL 						, &States[S_EXSARG_PAIN+1]),
	S_NORMAL (ESRG, 'H',	2, A_Pain						, &States[S_EXSARG_RUN+0]),

#define S_EXSARG_DIE (S_EXSARG_PAIN+2)
	S_BRIGHT (MISL, 'B',	8, A_Explode					, &States[S_EXSARG_DIE+1]),
	S_BRIGHT (MISL, 'C',	6, A_Scream 					, &States[S_EXSARG_DIE+2]),
	S_BRIGHT (MISL, 'D',	4, NULL 						, NULL),

#define S_EXSARG_RAISE (S_EXSARG_DIE+3)
	S_NORMAL (ESRG, 'N',	5, NULL 						, &States[S_EXSARG_RAISE+1]),
	S_NORMAL (ESRG, 'M',	5, NULL 						, &States[S_EXSARG_RAISE+2]),
	S_NORMAL (ESRG, 'L',	5, NULL 						, &States[S_EXSARG_RAISE+3]),
	S_NORMAL (ESRG, 'K',	5, NULL 						, &States[S_EXSARG_RAISE+4]),
	S_NORMAL (ESRG, 'J',	5, NULL 						, &States[S_EXSARG_RAISE+5]),
	S_NORMAL (ESRG, 'I',	5, NULL 						, &States[S_EXSARG_RUN+0])
};

IMPLEMENT_ACTOR (AExplodemon, Doom, 400, 0)
	PROP_SpawnHealth (75)
	PROP_PainChance (180)
	PROP_SpeedFixed (10)
	PROP_RadiusFixed (30)
	PROP_HeightFixed (56)
	PROP_Mass (400)
	PROP_Flags (MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL)
	PROP_Flags2 (MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL|MF2_FLOORCLIP)

	PROP_SpawnState (S_EXSARG_STND)
	PROP_SeeState (S_EXSARG_RUN)
	PROP_PainState (S_EXSARG_PAIN)
	PROP_MeleeState (S_EXSARG_ATK)
	PROP_DeathState (S_EXSARG_DIE)
	PROP_RaiseState (S_EXSARG_RAISE)

	PROP_SeeSound ("demon/sight")
	PROP_AttackSound ("demon/melee")
	PROP_PainSound ("demon/pain")
	PROP_DeathSound ("weapons/rocklx")
	PROP_ActiveSound ("demon/active")
END_DEFAULTS

void A_ExSargAttack (AActor *self)
{
	self->SetState (self->DeathState);
}

// Dukai Sushi

class ASushi : public AActor
{
	DECLARE_ACTOR (ASushi, AActor)
public:
	bool Use (AActor *activator);
};

FState ASushi::States[] =
{
#define S_SUSHI 0
	S_NORMAL (SHSI, 'A',	1, NULL							, &States[S_SUSHI]),
#define S_SUSHI_USE (S_SUSHI+1)
	S_NORMAL (SHSI, 'B',   48, NULL							, &States[S_SUSHI]),
#define S_SUSHI_DIE (S_SUSHI_USE+1)
	S_NORMAL (SHSI, 'A',	0, NULL							, NULL)
};

IMPLEMENT_ACTOR (ASushi, Doom, 401, 0)
	PROP_SpawnHealth (10)
	PROP_PainChance (180)
	PROP_RadiusFixed (12)
	PROP_HeightFixed (56)
	PROP_Mass (200)
	PROP_Flags (MF_SOLID|MF_SHOOTABLE)
	PROP_Flags2 (MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL|MF2_FLOORCLIP|MF2_PUSHABLE)
	PROP_XScale (41)
	PROP_YScale (41)

	PROP_SpawnState (S_SUSHI)
	PROP_MeleeState (S_SUSHI_USE)
	PROP_DeathState (S_SUSHI_DIE)
END_DEFAULTS

bool ASushi::Use (AActor *activator)
{
	SetState (MeleeState);
	return true;
}

// Useable cannon

void A_Explode (AActor *);
void A_CannonBall (AActor *);

class ACannonBall : public AActor
{
	DECLARE_ACTOR (ACannonBall, AActor)
};

FState ACannonBall::States [] =
{
#define S_CBALL 0
	S_NORMAL (CANB,	'A',	1, NULL							, &States[S_CBALL]),
#define S_CBALL_DIE (S_CBALL+1)
	S_NORMAL (CANB,	'A',	1, NULL							, NULL)
};

IMPLEMENT_ACTOR (ACannonBall, Doom, -1, 0)
	PROP_RadiusFixed (8)
	PROP_HeightFixed (8)
	PROP_SpeedFixed (20)
	PROP_Damage (100)
	PROP_Flags (MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY)
	PROP_Flags2 (MF2_PCROSS|MF2_IMPACT|MF2_NOTELEPORT)

	PROP_SpawnState (S_CBALL)
	PROP_DeathState (S_CBALL_DIE)

	PROP_SeeSound ("weapons/rocklf")
	PROP_DeathSound ("weapons/rocklx")
END_DEFAULTS

class ACannon : public AActor
{
	DECLARE_ACTOR (ACannon, AActor)
public:
	bool Use (AActor *activator);
	bool UnUse (AActor *activator);
	void UseThink (AActor *activator);
};

FState ACannon::States[] =
{
#define S_CANNON 0
	S_NORMAL (CANN,	'A',	1, NULL							, &States[S_CANNON]),
};

IMPLEMENT_ACTOR (ACannon, Doom, 402, 0)
	PROP_RadiusFixed (20)
	PROP_HeightFixed (56)
	PROP_Flags (MF_SOLID)

	PROP_SpawnState (S_CANNON)
END_DEFAULTS

bool ACannon::Use (AActor *activator)
{
	return true;
}

bool ACannon::UnUse (AActor *activator)
{
	P_SpawnMissileAngle (this, RUNTIME_CLASS(ACannonBall), angle, FixedMul (Speed, finesine[-pitch>>ANGLETOFINESHIFT]));
	return true;
}

void ACannon::UseThink (AActor *activator)
{
	angle = activator->angle;
	pitch = activator->pitch;
}

// Railgun cannon

class ARailgun : public AActor
{
	DECLARE_STATELESS_ACTOR (ARailgun, AActor)
public:
	void PostBeginPlay ();
	void Activate (AActor *activator);
};

IMPLEMENT_STATELESS_ACTOR (ARailgun, Doom, 403, 0)
	PROP_Flags (MF_NOBLOCKMAP|MF_NOGRAVITY)
	PROP_RenderStyle (STYLE_None)
END_DEFAULTS

void ARailgun::PostBeginPlay ()
{
	pitch = (signed int)((char)args[0]) * ANGLE_1;
	if (pitch <= -ANGLE_90)
		pitch = -ANGLE_90 + ANGLE_1;
	else if (pitch >= ANGLE_90)
		pitch = ANGLE_90 - ANGLE_1;
}

void ARailgun::Activate (AActor *activator)
{
	P_RailAttack (this, args[1], 0);
}

// Trees

#define _DECCOMMON(cls,ednum,rad,hi,ns,id) \
	class cls : public AActor { DECLARE_STATELESS_ACTOR (cls, AActor) static FState States[ns]; }; \
	IMPLEMENT_ACTOR (cls, Doom, ednum, id) \
		PROP_SpawnState(0) \
		PROP_RadiusFixed(rad) \
		PROP_HeightFixed(hi)

#define _DECSTARTSTATES(cls,ns) \
	FState cls::States[ns] =

#define DECID(cls,ednum,id,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns,id) \
	PROP_Flags (MF_SOLID) \
	END_DEFAULTS \
	_DECSTARTSTATES(cls,ns)

#define DECIDNS(cls,ednum,id,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns,id) \
	END_DEFAULTS \
	_DECSTARTSTATES(cls,ns)

#define DEC(cls,ednum,rad,hi,ns) \
	DECID(cls,ednum,0,rad,hi,ns)

#define DECNS(cls,ednum,rad,hi,ns) \
	DECIDNS(cls,ednum,0,rad,hi,ns)


DEC (ATree3, 404, 16, 96, 1)
{
	S_NORMAL (TRE3, 'A', -1, NULL, NULL)
};

DECNS (ATree4, 405, 32, 96, 1)
{
	S_NORMAL (TRE4, 'A', -1, NULL, NULL)
};

DEC (ATree5, 406, 32, 64, 1)
{
	S_NORMAL (TRE5, 'A', -1, NULL, NULL)
};

DECNS (ATree6, 407, 32, 64, 1)
{
	S_NORMAL (TRE6, 'A', -1, NULL, NULL)
};

DEC (ATree7, 408, 16, 96, 1)
{
	S_NORMAL (TRE7, 'A', -1, NULL, NULL)
};

DEC (ATree8, 409, 16, 96, 1)
{
	S_NORMAL (TRE8, 'A', -1, NULL, NULL)
};

DEC (ATree9, 410, 32, 32, 1)
{
	S_NORMAL (TRE9, 'A', -1, NULL, NULL)
};

DECNS (ATree10, 411, 32, 32, 1)
{
	S_NORMAL (TRE0, 'A', -1, NULL, NULL)
};

// Beams

class ABeam : public AActor
{
	DECLARE_STATELESS_ACTOR (ABeam, AActor)
public:
	void PostBeginPlay ();
	void Activate (AActor *activator);
	void Deactivate (AActor *activator);
};

IMPLEMENT_STATELESS_ACTOR (ABeam, Doom, 412, 0)
	PROP_Flags (MF_NOBLOCKMAP|MF_NOGRAVITY)
END_DEFAULTS

void ABeam::PostBeginPlay ()
{
	FActorIterator iterator (args[0]);
	AActor *targ;

	while ( (targ = iterator.Next ()) )
		goal = targ;

	radius = args[2];
	alpha = args[3];
	height = args[4];

	if (!radius)
		radius = 3;
	if (!alpha)
		alpha = 127;
	if (!height)
		height = 8;
}

void ABeam::Activate (AActor *activator)
{
	switch (args[1])
	{
	case 0:
		effects |= FX_BEAMRED;
		break;
	case 1:
		effects |= FX_BEAMGREEN;
		break;
	case 2:
		effects |= FX_BEAMBLUE;
		break;
	case 3:
		effects |= FX_BEAMYELLOW;
		break;
	case 4:
		effects |= FX_BEAMWHITE;
		break;
	}
}

void ABeam::Deactivate (AActor *activator)
{
	switch (args[1])
	{
	case 0:
		effects &= ~FX_BEAMRED;
		break;
	case 1:
		effects &= ~FX_BEAMGREEN;
		break;
	case 2:
		effects &= ~FX_BEAMBLUE;
		break;
	case 3:
		effects &= ~FX_BEAMYELLOW;
		break;
	case 4:
		effects &= ~FX_BEAMWHITE;
		break;
	}
}
