#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "dstrings.h"
#include "a_action.h"

void A_SkelMissile (AActor *);
void A_Tracer (AActor *);
void A_SkelWhoosh (AActor *);
void A_SkelFist (AActor *);

class ARevenant : public AActor
{
	DECLARE_ACTOR (ARevenant, AActor);
public:
	bool SuggestMissileAttack (fixed_t dist);
	const char *GetObituary () { return OB_UNDEAD; }
	const char *GetHitObituary () { return OB_UNDEADHIT; }
};

IMPLEMENT_DEF_SERIAL (ARevenant, AActor);
REGISTER_ACTOR (ARevenant, Doom);

FState ARevenant::States[] =
{
#define S_SKEL_STND 0
	S_NORMAL (SKEL, 'A',   10, A_Look						, &States[S_SKEL_STND+1]),
	S_NORMAL (SKEL, 'B',   10, A_Look						, &States[S_SKEL_STND]),

#define S_SKEL_RUN (S_SKEL_STND+2)
	S_NORMAL (SKEL, 'A',	2, A_Chase						, &States[S_SKEL_RUN+1]),
	S_NORMAL (SKEL, 'A',	2, A_Chase						, &States[S_SKEL_RUN+2]),
	S_NORMAL (SKEL, 'B',	2, A_Chase						, &States[S_SKEL_RUN+3]),
	S_NORMAL (SKEL, 'B',	2, A_Chase						, &States[S_SKEL_RUN+4]),
	S_NORMAL (SKEL, 'C',	2, A_Chase						, &States[S_SKEL_RUN+5]),
	S_NORMAL (SKEL, 'C',	2, A_Chase						, &States[S_SKEL_RUN+6]),
	S_NORMAL (SKEL, 'D',	2, A_Chase						, &States[S_SKEL_RUN+7]),
	S_NORMAL (SKEL, 'D',	2, A_Chase						, &States[S_SKEL_RUN+8]),
	S_NORMAL (SKEL, 'E',	2, A_Chase						, &States[S_SKEL_RUN+9]),
	S_NORMAL (SKEL, 'E',	2, A_Chase						, &States[S_SKEL_RUN+10]),
	S_NORMAL (SKEL, 'F',	2, A_Chase						, &States[S_SKEL_RUN+11]),
	S_NORMAL (SKEL, 'F',	2, A_Chase						, &States[S_SKEL_RUN+0]),

#define S_SKEL_FIST (S_SKEL_RUN+12)
	S_NORMAL (SKEL, 'G',	0, A_FaceTarget 				, &States[S_SKEL_FIST+1]),
	S_NORMAL (SKEL, 'G',	6, A_SkelWhoosh 				, &States[S_SKEL_FIST+2]),
	S_NORMAL (SKEL, 'H',	6, A_FaceTarget 				, &States[S_SKEL_FIST+3]),
	S_NORMAL (SKEL, 'I',	6, A_SkelFist					, &States[S_SKEL_RUN+0]),

#define S_SKEL_MISS (S_SKEL_FIST+4)
	S_BRIGHT (SKEL, 'J',	0, A_FaceTarget 				, &States[S_SKEL_MISS+1]),
	S_BRIGHT (SKEL, 'J',   10, A_FaceTarget 				, &States[S_SKEL_MISS+2]),
	S_NORMAL (SKEL, 'K',   10, A_SkelMissile				, &States[S_SKEL_MISS+3]),
	S_NORMAL (SKEL, 'K',   10, A_FaceTarget 				, &States[S_SKEL_RUN+0]),

#define S_SKEL_PAIN (S_SKEL_MISS+4)
	S_NORMAL (SKEL, 'L',	5, NULL 						, &States[S_SKEL_PAIN+1]),
	S_NORMAL (SKEL, 'L',	5, A_Pain						, &States[S_SKEL_RUN+0]),

#define S_SKEL_DIE (S_SKEL_PAIN+2)
	S_NORMAL (SKEL, 'L',	7, NULL 						, &States[S_SKEL_DIE+1]),
	S_NORMAL (SKEL, 'M',	7, NULL 						, &States[S_SKEL_DIE+2]),
	S_NORMAL (SKEL, 'N',	7, A_Scream 					, &States[S_SKEL_DIE+3]),
	S_NORMAL (SKEL, 'O',	7, A_NoBlocking					, &States[S_SKEL_DIE+4]),
	S_NORMAL (SKEL, 'P',	7, NULL 						, &States[S_SKEL_DIE+5]),
	S_NORMAL (SKEL, 'Q',   -1, NULL 						, NULL),

#define S_SKEL_RAISE (S_SKEL_DIE+6)
	S_NORMAL (SKEL, 'Q',	5, NULL 						, &States[S_SKEL_RAISE+1]),
	S_NORMAL (SKEL, 'P',	5, NULL 						, &States[S_SKEL_RAISE+2]),
	S_NORMAL (SKEL, 'O',	5, NULL 						, &States[S_SKEL_RAISE+3]),
	S_NORMAL (SKEL, 'N',	5, NULL 						, &States[S_SKEL_RAISE+4]),
	S_NORMAL (SKEL, 'M',	5, NULL 						, &States[S_SKEL_RAISE+5]),
	S_NORMAL (SKEL, 'L',	5, NULL 						, &States[S_SKEL_RUN+0])
};

void ARevenant::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 66;
	info->spawnid = 20;
	info->spawnstate = &States[S_SKEL_STND];
	info->spawnhealth = 300;
	info->seestate = &States[S_SKEL_RUN];
	info->seesound = "skeleton/sight";
	info->painstate = &States[S_SKEL_PAIN];
	info->painchance = 100;
	info->painsound = "skeleton/pain";
	info->meleestate = &States[S_SKEL_FIST];
	info->missilestate = &States[S_SKEL_MISS];
	info->deathstate = &States[S_SKEL_DIE];
	info->deathsound = "skeleton/death";
	info->speed = 10;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 500;
	info->activesound = "skeleton/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_SKEL_RAISE];
}

bool ARevenant::SuggestMissileAttack (fixed_t dist)
{
	if (dist < 196*FRACUNIT)
		return false;		// close for fist attack
	return P_Random (pr_checkmissilerange) >= MIN (dist >> (FRACBITS + 1), 200);
}

class AStealthRevenant : public ARevenant
{
	DECLARE_STATELESS_ACTOR (AStealthRevenant, ARevenant);
public:
	const char *GetObituary () { return OB_STEALTHUNDEAD; }
	const char *GetHitObituary () { return OB_STEALTHUNDEAD; }
};

IMPLEMENT_DEF_SERIAL (AStealthRevenant, ARevenant);
REGISTER_ACTOR (AStealthRevenant, Doom);

void AStealthRevenant::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9059;
	info->spawnid = 124;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class ARevenantTracer : public AActor
{
	DECLARE_ACTOR (ARevenantTracer, AActor);
};

IMPLEMENT_DEF_SERIAL (ARevenantTracer, AActor);
REGISTER_ACTOR (ARevenantTracer, Doom);

FState ARevenantTracer::States[] =
{
#define S_TRACER 0
	S_BRIGHT (FATB, 'A',	2, A_Tracer 					, &States[S_TRACER+1]),
	S_BRIGHT (FATB, 'B',	2, A_Tracer 					, &States[S_TRACER]),

#define S_TRACEEXP (S_TRACER+2)
	S_BRIGHT (FBXP, 'A',	8, NULL 						, &States[S_TRACEEXP+1]),
	S_BRIGHT (FBXP, 'B',	6, NULL 						, &States[S_TRACEEXP+2]),
	S_BRIGHT (FBXP, 'C',	4, NULL 						, NULL)
};

void ARevenantTracer::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 53;
	info->spawnstate = &States[S_TRACER];
	info->seesound = "skeleton/attack";
	info->deathstate = &States[S_TRACEEXP];
	info->deathsound = "skeleton/tracex";
	info->speed = 10 * FRACUNIT;
	info->radius = 11 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 10;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC75;
}

class ARevenantTracerSmoke : public AActor
{
	DECLARE_ACTOR (ARevenantTracerSmoke, AActor);
};

IMPLEMENT_DEF_SERIAL (ARevenantTracerSmoke, AActor);
REGISTER_ACTOR (ARevenantTracerSmoke, Doom);

FState ARevenantTracerSmoke::States[] =
{
	S_NORMAL (PUFF, 'B',	4, NULL 						, &States[1]),
	S_NORMAL (PUFF, 'C',	4, NULL 						, &States[2]),
	S_NORMAL (PUFF, 'B',	4, NULL 						, &States[3]),
	S_NORMAL (PUFF, 'C',	4, NULL 						, &States[4]),
	S_NORMAL (PUFF, 'D',	4, NULL 						, NULL)
};

void ARevenantTracerSmoke::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->translucency = TRANSLUC50;
}

//
// A_SkelMissile
//
void A_SkelMissile (AActor *self)
{		
	AActor *missile;
		
	if (!self->target)
		return;
				
	A_FaceTarget (self);
	self->z += 16*FRACUNIT;		// so missile spawns higher
	missile = P_SpawnMissile (self, self->target, RUNTIME_CLASS(ARevenantTracer));
	self->z -= 16*FRACUNIT;		// back to normal

	missile->x += missile->momx;
	missile->y += missile->momy;
	missile->tracer = self->target;
}

#define TRACEANGLE (0xc000000)

void A_Tracer (AActor *self)
{
	angle_t exact;
	fixed_t dist;
	fixed_t slope;
	AActor *dest;
	AActor *smoke;
				
	// killough 1/18/98: this is why some missiles do not have smoke
	// and some do. Also, internal demos start at random gametics, thus
	// the bug in which revenants cause internal demos to go out of sync.
	//
	// killough 3/6/98: fix revenant internal demo bug by subtracting
	// levelstarttic from gametic:
	//
	// [RH] level.time is always 0-based, so nothing special to do here.

	if (level.time & 3)
		return;
	
	// spawn a puff of smoke behind the rocket			
	P_SpawnPuff (self->x, self->y, self->z, 0, 3);
		
	smoke = Spawn<ARevenantTracerSmoke> (self->x - self->momx,
		self->y - self->momy, self->z);
	
	smoke->momz = FRACUNIT;
	smoke->tics -= P_Random (pr_tracer)&3;
	if (smoke->tics < 1)
		smoke->tics = 1;
	
	// adjust direction
	dest = self->tracer;
		
	if (!dest || dest->health <= 0)
		return;
	
	// change angle 	
	exact = R_PointToAngle2 (self->x,
							 self->y,
							 dest->x,
							 dest->y);

	if (exact != self->angle)
	{
		if (exact - self->angle > 0x80000000)
		{
			self->angle -= TRACEANGLE;
			if (exact - self->angle < 0x80000000)
				self->angle = exact;
		}
		else
		{
			self->angle += TRACEANGLE;
			if (exact - self->angle > 0x80000000)
				self->angle = exact;
		}
	}
		
	exact = self->angle>>ANGLETOFINESHIFT;
	self->momx = FixedMul (GetInfo (self)->speed, finecosine[exact]);
	self->momy = FixedMul (GetInfo (self)->speed, finesine[exact]);
	
	// change slope
	dist = P_AproxDistance (dest->x - self->x,
							dest->y - self->y);
	
	dist = dist / GetInfo (self)->speed;

	if (dist < 1)
		dist = 1;
	slope = (dest->z+40*FRACUNIT - self->z) / dist;

	if (slope < self->momz)
		self->momz -= FRACUNIT/8;
	else
		self->momz += FRACUNIT/8;
}


void A_SkelWhoosh (AActor *self)
{
	if (!self->target)
		return;
	A_FaceTarget (self);
	S_Sound (self, CHAN_WEAPON, "skeleton/swing", 1, ATTN_NORM);
}

void A_SkelFist (AActor *self)
{
	if (!self->target)
		return;
				
	A_FaceTarget (self);
		
	if (P_CheckMeleeRange (self))
	{
		int damage = ((P_Random (pr_skelfist)%10)+1)*6;
		S_Sound (self, CHAN_WEAPON, "skeleton/melee", 1, ATTN_NORM);
		P_DamageMobj (self->target, self, self, damage, MOD_HIT);
	}
}
