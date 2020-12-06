#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "a_doomglobal.h"

void A_Fire (AActor *);		// from m_archvile.cpp

void A_BrainAwake (AActor *);
void A_BrainPain (AActor *);
void A_BrainScream (AActor *);
void A_BrainExplode (AActor *);
void A_BrainDie (AActor *);
void A_BrainSpit (AActor *);
void A_SpawnFly (AActor *);
void A_SpawnSound (AActor *);

IMPLEMENT_DEF_SERIAL (ABossBrain, AActor);
REGISTER_ACTOR (ABossBrain, Doom);

FState ABossBrain::States[] =
{
#define S_BRAINEXPLODE 0
	S_BRIGHT (MISL, 'B',   10, NULL 			, &States[S_BRAINEXPLODE+1]),
	S_BRIGHT (MISL, 'C',   10, NULL 			, &States[S_BRAINEXPLODE+2]),
	S_BRIGHT (MISL, 'D',   10, A_BrainExplode	, NULL),

#define S_BRAIN (S_BRAINEXPLODE+3)
	S_NORMAL (BBRN, 'A',   -1, NULL 			, NULL),

#define S_BRAIN_PAIN (S_BRAIN+1)
	S_NORMAL (BBRN, 'B',   36, A_BrainPain		, &States[S_BRAIN]),

#define S_BRAIN_DIE (S_BRAIN_PAIN+1)
	S_NORMAL (BBRN, 'A',  100, A_BrainScream	, &States[S_BRAIN_DIE+1]),
	S_NORMAL (BBRN, 'A',   10, NULL 			, &States[S_BRAIN_DIE+2]),
	S_NORMAL (BBRN, 'A',   10, NULL 			, &States[S_BRAIN_DIE+3]),
	S_NORMAL (BBRN, 'A',   -1, A_BrainDie		, NULL)
};

void ABossBrain::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 88;
	info->spawnstate = &States[S_BRAIN];
	info->spawnhealth = 250;
	info->painstate = &States[S_BRAIN_PAIN];
	info->painchance = 255;
	info->painsound = "brain/pain";
	info->deathstate = &States[S_BRAIN_DIE];
	info->deathsound = "brain/death";
	//info->height = 86 * FRACUNIT;		// don't do this; it messes up some non-id levels
	info->mass = 10000000;
	info->flags = MF_SOLID|MF_SHOOTABLE;
}

class ABossEye : public AActor
{
	DECLARE_ACTOR (ABossEye, AActor);
};

IMPLEMENT_DEF_SERIAL (ABossEye, AActor);
REGISTER_ACTOR (ABossEye, Doom);

FState ABossEye::States[] =
{
#define S_BRAINEYE 0
	S_NORMAL (SSWV, 'A',   10, A_Look						, &States[S_BRAINEYE]),

#define S_BRAINEYESEE (S_BRAINEYE+1)
	S_NORMAL (SSWV, 'A',  181, A_BrainAwake 				, &States[S_BRAINEYESEE+1]),
	S_NORMAL (SSWV, 'A',  150, A_BrainSpit					, &States[S_BRAINEYESEE+1])
};

void ABossEye::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 89;
	info->spawnstate = &States[S_BRAINEYE];
	info->seestate = &States[S_BRAINEYESEE];
	info->radius = 20 * FRACUNIT;
	info->height = 32 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP | MF_NOSECTOR;
}

class ABossTarget : public AActor
{
	DECLARE_STATELESS_ACTOR (ABossTarget, AActor);
};

IMPLEMENT_DEF_SERIAL (ABossTarget, AActor);
REGISTER_ACTOR (ABossTarget, Doom);

void ABossTarget::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 87;
	info->radius = 20 * FRACUNIT;
	info->height = 32 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP | MF_NOSECTOR;
}

class ASpawnShot : public AActor
{
	DECLARE_ACTOR (ASpawnShot, AActor);
};

IMPLEMENT_DEF_SERIAL (ASpawnShot, AActor);
REGISTER_ACTOR (ASpawnShot, Doom);

FState ASpawnShot::States[] =
{
	S_BRIGHT (BOSF, 'A',	3, A_SpawnSound 				, &States[1]),
	S_BRIGHT (BOSF, 'B',	3, A_SpawnFly					, &States[2]),
	S_BRIGHT (BOSF, 'C',	3, A_SpawnFly					, &States[3]),
	S_BRIGHT (BOSF, 'D',	3, A_SpawnFly					, &States[0])
};

void ASpawnShot::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->seesound = "brain/spit";
	info->deathsound = "brain/spawn";
	info->speed = 10 * FRACUNIT;
	info->radius = 6 * FRACUNIT;
	info->height = 32 * FRACUNIT;
	info->damage = 3;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_NOCLIP;
}

class ASpawnFire : public AActor
{
	DECLARE_ACTOR (ASpawnFire, AActor);
};

IMPLEMENT_DEF_SERIAL (ASpawnFire, AActor);
REGISTER_ACTOR (ASpawnFire, Doom);

FState ASpawnFire::States[] =
{
	S_BRIGHT (FIRE, 'A',	4, A_Fire						, &States[1]),
	S_BRIGHT (FIRE, 'B',	4, A_Fire						, &States[2]),
	S_BRIGHT (FIRE, 'C',	4, A_Fire						, &States[3]),
	S_BRIGHT (FIRE, 'D',	4, A_Fire						, &States[4]),
	S_BRIGHT (FIRE, 'E',	4, A_Fire						, &States[5]),
	S_BRIGHT (FIRE, 'F',	4, A_Fire						, &States[6]),
	S_BRIGHT (FIRE, 'G',	4, A_Fire						, &States[7]),
	S_BRIGHT (FIRE, 'H',	4, A_Fire						, NULL)
};

void ASpawnFire::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->radius = 20 * FRACUNIT;
	info->height = 78 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->translucency = TRANSLUC66;
}

static TArray<AActor *> BrainTargets (32);
struct brain_s brain;   // killough 3/26/98: global state of boss brain

// killough 3/26/98: initialize icon landings at level startup,
// rather than at boss wakeup, to prevent savegame-related crashes

void P_SpawnBrainTargets (void)	// killough 3/26/98: renamed old function
{
	AActor *other;
	TThinkerIterator<AActor> iterator;

	// find all the target spots
	BrainTargets.Clear ();
	brain.targeton = 0;
	brain.easy = 0;				// killough 3/26/98: always init easy to 0

	while ( (other = iterator.Next ()) )
	{
		if (other->IsKindOf (RUNTIME_CLASS(ABossTarget)))
		{
			BrainTargets.Push (other);
		}
	}
}

void A_BrainAwake (AActor *self)
{
	// killough 3/26/98: only generates sound now
	S_Sound (self, CHAN_VOICE, "brain/sight", 1, ATTN_SURROUND);
}

void A_BrainPain (AActor *self)
{
	S_Sound (self, CHAN_VOICE, "brain/pain", 1, ATTN_SURROUND);
}

static void BrainishExplosion (fixed_t x, fixed_t y, fixed_t z)
{
	AActor *boom = Spawn<ARocket> (x, y, z);
	if (boom != NULL)
	{
		boom->momz = P_Random (pr_brainscream) << 9;
		boom->SetState (&ABossBrain::States[S_BRAINEXPLODE]);
		boom->effects = 0;
		boom->tics -= P_Random (pr_brainscream) & 7;
		if (boom->tics < 1)
			boom->tics = 1;
	}
}

void A_BrainScream (AActor *self)
{
	fixed_t x;
		
	for (x = self->x - 196*FRACUNIT; x < self->x + 320*FRACUNIT; x += 8*FRACUNIT)
	{
		BrainishExplosion (x, self->y - 320*FRACUNIT,
			128 + (P_Random (pr_brainscream) << (FRACBITS + 1)));
	}
	S_Sound (self, CHAN_VOICE, "brain/death", 1, ATTN_SURROUND);
}

void A_BrainExplode (AActor *self)
{
	BrainishExplosion (self->x + PS_Random (pr_brainexplode)*2048,
		self->y, 128 + P_Random (pr_brainexplode)*2*FRACUNIT);
}

void A_BrainDie (AActor *self)
{
	// [RH] If noexit, then don't end the level.
	if ((deathmatch.value || alwaysapplydmflags.value) && (dmflags & DF_NO_EXIT))
		return;

	G_ExitLevel (0);
}

void A_BrainSpit (AActor *self)
{
	AActor *targ;
	AActor *newmobj;
	
	// [RH] Do nothing if there are no brain targets.
	if (BrainTargets.Size() == 0)
		return;

	brain.easy ^= 1;		// killough 3/26/98: use brain struct
	if (gameskill.value <= sk_easy && (!brain.easy))
		return;
				
	// shoot a cube at current target
	targ = BrainTargets[brain.targeton++];	// killough 3/26/98:
	brain.targeton %= BrainTargets.Size();	// Use brain struct for targets

	// spawn brain missile
	newmobj = P_SpawnMissile (self, targ, RUNTIME_CLASS(ASpawnShot));

	newmobj->target = targ;
	newmobj->reactiontime =
		((targ->y - self->y)/newmobj->momy) / newmobj->state->tics;

	S_Sound (self, CHAN_WEAPON, "brain/spit", 1, ATTN_SURROUND);
}

void A_SpawnFly (AActor *self)
{
	AActor *newmobj;
	AActor *fog;
	AActor *targ;
	int r;
	const char *type;
		
	if (--self->reactiontime)
		return; // still flying
		
	targ = self->target;

	// First spawn teleport fog.
	fog = Spawn<ASpawnFire> (targ->x, targ->y, targ->z);
	S_Sound (fog, CHAN_BODY, "misc/teleport", 1, ATTN_NORM);

	// Randomly select monster to spawn.
	r = P_Random (pr_spawnfly);

	// Probability distribution (kind of :),
	// decreasing likelihood.
		 if (r < 50)  type = "DoomImp";
	else if (r < 90)  type = "Demon";
	else if (r < 120) type = "Spectre";
	else if (r < 130) type = "PainElemental";
	else if (r < 160) type = "Cacodemon";
	else if (r < 162) type = "Archvile";
	else if (r < 172) type = "Revenant";
	else if (r < 192) type = "Arachnotron";
	else if (r < 222) type = "Fatso";
	else if (r < 246) type = "HellKnight";
	else			  type = "BaronOfHell";

	newmobj = Spawn (type, targ->x, targ->y, targ->z);
	if (P_LookForPlayers (newmobj, true))
		newmobj->SetState (RUNTIME_TYPE(newmobj)->ActorInfo->seestate);
		
	// telefrag anything in this spot
	P_TeleportMove (newmobj, newmobj->x, newmobj->y, newmobj->z, true);

	// remove self (i.e., cube).
	self->Destroy ();
}

// travelling cube sound
void A_SpawnSound (AActor *self)	
{
	S_Sound (self, CHAN_BODY, "brain/cube", 1, ATTN_IDLE);
	A_SpawnFly (self);
}
