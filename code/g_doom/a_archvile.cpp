#include "actor.h"
#include "info.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "dstrings.h"
#include "a_action.h"

void A_VileChase (AActor *);
void A_VileStart (AActor *);
void A_StartFire (AActor *);
void A_FireCrackle (AActor *);
void A_Fire (AActor *);
void A_VileTarget (AActor *);
void A_VileAttack (AActor *);

IMPLEMENT_DEF_SERIAL (AArchvile, AActor);
REGISTER_ACTOR (AArchvile, Doom);

FState AArchvile::States[] =
{
#define S_VILE_STND 0
	S_NORMAL (VILE, 'A',   10, A_Look						, &States[S_VILE_STND+1]),
	S_NORMAL (VILE, 'B',   10, A_Look						, &States[S_VILE_STND]),

#define S_VILE_RUN (S_VILE_STND+2)
	S_NORMAL (VILE, 'A',	2, A_VileChase					, &States[S_VILE_RUN+1]),
	S_NORMAL (VILE, 'A',	2, A_VileChase					, &States[S_VILE_RUN+2]),
	S_NORMAL (VILE, 'B',	2, A_VileChase					, &States[S_VILE_RUN+3]),
	S_NORMAL (VILE, 'B',	2, A_VileChase					, &States[S_VILE_RUN+4]),
	S_NORMAL (VILE, 'C',	2, A_VileChase					, &States[S_VILE_RUN+5]),
	S_NORMAL (VILE, 'C',	2, A_VileChase					, &States[S_VILE_RUN+6]),
	S_NORMAL (VILE, 'D',	2, A_VileChase					, &States[S_VILE_RUN+7]),
	S_NORMAL (VILE, 'D',	2, A_VileChase					, &States[S_VILE_RUN+8]),
	S_NORMAL (VILE, 'E',	2, A_VileChase					, &States[S_VILE_RUN+9]),
	S_NORMAL (VILE, 'E',	2, A_VileChase					, &States[S_VILE_RUN+10]),
	S_NORMAL (VILE, 'F',	2, A_VileChase					, &States[S_VILE_RUN+11]),
	S_NORMAL (VILE, 'F',	2, A_VileChase					, &States[S_VILE_RUN+0]),

#define S_VILE_ATK (S_VILE_RUN+12)
	S_BRIGHT (VILE, 'G',	0, A_VileStart					, &States[S_VILE_ATK+1]),
	S_BRIGHT (VILE, 'G',   10, A_FaceTarget 				, &States[S_VILE_ATK+2]),
	S_BRIGHT (VILE, 'H',	8, A_VileTarget 				, &States[S_VILE_ATK+3]),
	S_BRIGHT (VILE, 'I',	8, A_FaceTarget 				, &States[S_VILE_ATK+4]),
	S_BRIGHT (VILE, 'J',	8, A_FaceTarget 				, &States[S_VILE_ATK+5]),
	S_BRIGHT (VILE, 'K',	8, A_FaceTarget 				, &States[S_VILE_ATK+6]),
	S_BRIGHT (VILE, 'L',	8, A_FaceTarget 				, &States[S_VILE_ATK+7]),
	S_BRIGHT (VILE, 'M',	8, A_FaceTarget 				, &States[S_VILE_ATK+8]),
	S_BRIGHT (VILE, 'N',	8, A_FaceTarget 				, &States[S_VILE_ATK+9]),
	S_BRIGHT (VILE, 'O',	8, A_VileAttack 				, &States[S_VILE_ATK+10]),
	S_BRIGHT (VILE, 'P',   20, NULL 						, &States[S_VILE_RUN+0]),

#define S_VILE_HEAL (S_VILE_ATK+11)
	S_BRIGHT (VILE, '[',   10, NULL 						, &States[S_VILE_HEAL+1]),
	S_BRIGHT (VILE, '\\',  10, NULL 						, &States[S_VILE_HEAL+2]),
	S_BRIGHT (VILE, ']',   10, NULL 						, &States[S_VILE_RUN+0]),

#define S_VILE_PAIN (S_VILE_HEAL+3)
	S_NORMAL (VILE, 'Q',	5, NULL 						, &States[S_VILE_PAIN+1]),
	S_NORMAL (VILE, 'Q',	5, A_Pain						, &States[S_VILE_RUN+0]),

#define S_VILE_DIE (S_VILE_PAIN+2)
	S_NORMAL (VILE, 'Q',	7, NULL 						, &States[S_VILE_DIE+1]),
	S_NORMAL (VILE, 'R',	7, A_Scream 					, &States[S_VILE_DIE+2]),
	S_NORMAL (VILE, 'S',	7, A_NoBlocking					, &States[S_VILE_DIE+3]),
	S_NORMAL (VILE, 'T',	7, NULL 						, &States[S_VILE_DIE+4]),
	S_NORMAL (VILE, 'U',	7, NULL 						, &States[S_VILE_DIE+5]),
	S_NORMAL (VILE, 'V',	7, NULL 						, &States[S_VILE_DIE+6]),
	S_NORMAL (VILE, 'W',	7, NULL 						, &States[S_VILE_DIE+7]),
	S_NORMAL (VILE, 'X',	5, NULL 						, &States[S_VILE_DIE+8]),
	S_NORMAL (VILE, 'Y',	5, NULL 						, &States[S_VILE_DIE+9]),
	S_NORMAL (VILE, 'Z',   -1, NULL 						, NULL)
};

void AArchvile::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 64;
	info->spawnid = 111;
	info->spawnstate = &States[S_VILE_STND];
	info->spawnhealth = 700;
	info->seestate = &States[S_VILE_RUN];
	info->seesound = "vile/sight";
	info->painstate = &States[S_VILE_PAIN];
	info->painchance = 10;
	info->painsound = "vile/pain";
	info->missilestate = &States[S_VILE_ATK];
	info->deathstate = &States[S_VILE_DIE];
	info->deathsound = "vile/death";
	info->speed = 15;
	info->radius = 20 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 500;
	info->activesound = "vile/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
}

const char *AArchvile::GetObituary ()
{
	return OB_VILE;
}

class AStealthArchvile : public AArchvile
{
	DECLARE_STATELESS_ACTOR (AStealthArchvile, AArchvile);
public:
	const char *GetObituary () { return OB_STEALTHVILE; }
};

IMPLEMENT_DEF_SERIAL (AStealthArchvile, AArchvile);
REGISTER_ACTOR (AStealthArchvile, Doom);

void AStealthArchvile::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9051;
	info->spawnid = 118;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class AArchvileFire : public AActor
{
	DECLARE_ACTOR (AArchvileFire, AActor);
};

IMPLEMENT_DEF_SERIAL (AArchvileFire, AActor);
REGISTER_ACTOR (AArchvileFire, Doom);

FState AArchvileFire::States[] =
{
	S_BRIGHT (FIRE, 'A',	2, A_StartFire					, &States[1]),
	S_BRIGHT (FIRE, 'B',	2, A_Fire						, &States[2]),
	S_BRIGHT (FIRE, 'A',	2, A_Fire						, &States[3]),
	S_BRIGHT (FIRE, 'B',	2, A_Fire						, &States[4]),
	S_BRIGHT (FIRE, 'C',	2, A_FireCrackle				, &States[5]),
	S_BRIGHT (FIRE, 'B',	2, A_Fire						, &States[6]),
	S_BRIGHT (FIRE, 'C',	2, A_Fire						, &States[7]),
	S_BRIGHT (FIRE, 'B',	2, A_Fire						, &States[8]),
	S_BRIGHT (FIRE, 'C',	2, A_Fire						, &States[9]),
	S_BRIGHT (FIRE, 'D',	2, A_Fire						, &States[10]),
	S_BRIGHT (FIRE, 'C',	2, A_Fire						, &States[11]),
	S_BRIGHT (FIRE, 'D',	2, A_Fire						, &States[12]),
	S_BRIGHT (FIRE, 'C',	2, A_Fire						, &States[13]),
	S_BRIGHT (FIRE, 'D',	2, A_Fire						, &States[14]),
	S_BRIGHT (FIRE, 'E',	2, A_Fire						, &States[15]),
	S_BRIGHT (FIRE, 'D',	2, A_Fire						, &States[16]),
	S_BRIGHT (FIRE, 'E',	2, A_Fire						, &States[17]),
	S_BRIGHT (FIRE, 'D',	2, A_Fire						, &States[18]),
	S_BRIGHT (FIRE, 'E',	2, A_FireCrackle				, &States[19]),
	S_BRIGHT (FIRE, 'F',	2, A_Fire						, &States[20]),
	S_BRIGHT (FIRE, 'E',	2, A_Fire						, &States[21]),
	S_BRIGHT (FIRE, 'F',	2, A_Fire						, &States[22]),
	S_BRIGHT (FIRE, 'E',	2, A_Fire						, &States[23]),
	S_BRIGHT (FIRE, 'F',	2, A_Fire						, &States[24]),
	S_BRIGHT (FIRE, 'G',	2, A_Fire						, &States[25]),
	S_BRIGHT (FIRE, 'H',	2, A_Fire						, &States[26]),
	S_BRIGHT (FIRE, 'G',	2, A_Fire						, &States[27]),
	S_BRIGHT (FIRE, 'H',	2, A_Fire						, &States[28]),
	S_BRIGHT (FIRE, 'G',	2, A_Fire						, &States[29]),
	S_BRIGHT (FIRE, 'H',	2, A_Fire						, NULL)
};

void AArchvileFire::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 98;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->translucency = TRANSLUC66;
}

bool AArchvile::SuggestMissileAttack (fixed_t dist)
{
	if (dist > 14*64*FRACUNIT)
		return false;		// too far away
	return Super::SuggestMissileAttack (dist);
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
static AActor *corpsehit;
static AActor *vileobj;
static fixed_t viletryx;
static fixed_t viletryy;

BOOL PIT_VileCheck (AActor *thing)
{
	int maxdist;
	BOOL check;
		
	if (!(thing->flags & MF_CORPSE) )
		return true;	// not a monster
	
	if (thing->tics != -1)
		return true;	// not lying still yet
	
	if (GetInfo (thing)->raisestate == NULL)
		return true;	// monster doesn't have a raise state
	
	maxdist = GetInfo (thing)->radius + RUNTIME_CLASS(AArchvile)->ActorInfo->radius;
		
	if ( abs(thing->x - viletryx) > maxdist
		 || abs(thing->y - viletryy) > maxdist )
		return true;			// not actually touching
				
	corpsehit = thing;
	corpsehit->momx = corpsehit->momy = 0;
	// [RH] Check against real height and radius

	fixed_t oldheight = corpsehit->height;
	fixed_t oldradius = corpsehit->radius;
	int oldflags = corpsehit->flags;

	corpsehit->flags |= MF_SOLID;
	corpsehit->height = GetInfo (corpsehit)->height;
	check = P_CheckPosition (corpsehit, corpsehit->x, corpsehit->y);
	corpsehit->flags = oldflags;
	corpsehit->radius = oldradius;
	corpsehit->height = oldheight;

	return !check;
}



//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase (AActor *self)
{
	int xl, xh, yl, yh;
	int bx, by;

	const FActorInfo *info;
	AActor *temp;
		
	if (self->movedir != DI_NODIR)
	{
		// check for corpses to raise
		viletryx =
			self->x + GetInfo (self)->speed*xspeed[self->movedir];
		viletryy =
			self->y + GetInfo (self)->speed*yspeed[self->movedir];

		xl = (viletryx - bmaporgx - MAXRADIUS*2)>>MAPBLOCKSHIFT;
		xh = (viletryx - bmaporgx + MAXRADIUS*2)>>MAPBLOCKSHIFT;
		yl = (viletryy - bmaporgy - MAXRADIUS*2)>>MAPBLOCKSHIFT;
		yh = (viletryy - bmaporgy + MAXRADIUS*2)>>MAPBLOCKSHIFT;
		
		vileobj = self;
		for (bx=xl ; bx<=xh ; bx++)
		{
			for (by=yl ; by<=yh ; by++)
			{
				// Call PIT_VileCheck to check
				// whether object is a corpse
				// that canbe raised.
				if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
				{
					// got one!
					temp = self->target;
					self->target = corpsehit;
					A_FaceTarget (self);
					self->target = temp;
										
					self->SetState (&AArchvile::States[S_VILE_HEAL]);
					S_Sound (corpsehit, CHAN_BODY, "vile/raise", 1, ATTN_IDLE);
					info = GetInfo (corpsehit);
					
					corpsehit->SetState (GetInfo(corpsehit)->raisestate);
					corpsehit->height = info->height;	// [RH] Use real mobj height
					corpsehit->radius = info->radius;	// [RH] Use real radius
					if (corpsehit->translucency > TRANSLUC50)
						corpsehit->translucency /= 2;
					corpsehit->flags = info->flags;
					corpsehit->flags2 = info->flags2;
					corpsehit->health = info->spawnhealth;
					corpsehit->target = NULL;

					return;
				}
			}
		}
	}

	// Return to normal attack.
	A_Chase (self);
}


//
// A_VileStart
//
void A_VileStart (AActor *self)
{
	S_Sound (self, CHAN_VOICE, "vile/start", 1, ATTN_NORM);
}


//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_StartFire (AActor *self)
{
	S_Sound (self, CHAN_BODY, "vile/firestrt", 1, ATTN_NORM);
	A_Fire (self);
}

void A_FireCrackle (AActor *self)
{
	S_Sound (self, CHAN_BODY, "vile/firecrkl", 1, ATTN_NORM);
	A_Fire (self);
}

void A_Fire (AActor *self)
{
	AActor *dest;
	angle_t an;
				
	dest = self->tracer;
	if (!dest)
		return;
				
	// don't move it if the vile lost sight
	if (!P_CheckSight (self->target, dest, false) )
		return;

	an = dest->angle >> ANGLETOFINESHIFT;

	self->SetOrigin (dest->x + FixedMul (24*FRACUNIT, finecosine[an]),
					 dest->y + FixedMul (24*FRACUNIT, finesine[an]),
					 dest->z);
}



//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget (AActor *actor)
{
	AActor *fog;
		
	if (!actor->target)
		return;

	A_FaceTarget (actor);

	fog = Spawn<AArchvileFire> (actor->target->x, actor->target->x,
		actor->target->z);
	
	actor->tracer = fog;
	fog->target = actor;
	fog->tracer = actor->target;
	A_Fire (fog);
}




//
// A_VileAttack
//
void A_VileAttack (AActor *actor)
{		
	AActor *fire;
	int an;
		
	if (!actor->target)
		return;
	
	A_FaceTarget (actor);

	if (!P_CheckSight (actor, actor->target, false) )
		return;

	S_Sound (actor, CHAN_WEAPON, "vile/stop", 1, ATTN_NORM);
	P_DamageMobj (actor->target, actor, actor, 20, MOD_UNKNOWN);
	actor->target->momz = 1000 * FRACUNIT / GetInfo(actor->target)->mass;
		
	an = actor->angle >> ANGLETOFINESHIFT;

	fire = actor->tracer;

	if (!fire)
		return;
				
	// move the fire between the vile and the player
	fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[an]);
	fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[an]);	
	P_RadiusAttack (fire, actor, 70, 70, true, MOD_UNKNOWN);
}
