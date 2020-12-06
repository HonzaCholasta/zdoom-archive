#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_enemy.h"
#include "p_local.h"

// Convenient macros --------------------------------------------------------

#define _DEBCOMMON(cls,ns,dstate,sid) \
	class cls : public AActor { DECLARE_STATELESS_ACTOR (cls, AActor); static FState States[ns]; }; \
	IMPLEMENT_DEF_SERIAL (cls, AActor); \
	REGISTER_ACTOR (cls, Hexen); \
	void cls::SetDefaults (FActorInfo *info) { \
		INHERIT_DEFS; \
		info->spawnid = sid; \
		info->deathstate = &States[dstate]; \
		info->spawnstate = &States[0];
		

#define _DEBSTARTSTATES(cls,ns) \
	FState cls::States[ns] =

#define DEBRIS(cls,spawnnum,ns,dstate) \
	_DEBCOMMON(cls,ns,dstate,spawnnum) \
	info->flags = MF_NOBLOCKMAP|MF_DROPOFF|MF_MISSILE; \
	info->flags2 = MF2_NOTELEPORT; } \
	 _DEBSTARTSTATES(cls,ns)

#define SHARD(cls,spawnnum,ns,dstate) \
	_DEBCOMMON(cls,ns,dstate,spawnnum) \
	info->radius = 5 * FRACUNIT; \
	info->flags = MF_NOBLOCKMAP|MF_DROPOFF|MF_MISSILE|MF_NOGRAVITY; \
	info->flags2 = MF2_NOTELEPORT|MF2_FLOORBOUNCE; } \
	 _DEBSTARTSTATES(cls,ns)

// Rocks --------------------------------------------------------------------

#define S_ROCK1 0
#define S_ROCK1_D (S_ROCK1+1)

DEBRIS (ARock1, 41, 2, S_ROCK1_D)
{
	S_NORMAL (ROCK, 'A',   20, NULL, &States[S_ROCK1]),
	S_NORMAL (ROCK, 'A',   10, NULL, NULL)
};

#define S_ROCK2 0
#define S_ROCK2_D (S_ROCK2+1)

DEBRIS (ARock2, 42, 2, S_ROCK2_D)
{
	S_NORMAL (ROCK, 'B',   20, NULL, &States[S_ROCK2]),
	S_NORMAL (ROCK, 'B',   10, NULL, NULL)
};

#define S_ROCK3 0
#define S_ROCK3_D (S_ROCK3+1)

DEBRIS (ARock3, 43, 2, S_ROCK3_D)
{
	S_NORMAL (ROCK, 'C',   20, NULL, &States[S_ROCK3+0]),
	S_NORMAL (ROCK, 'C',   10, NULL, NULL)
};

// Dirt ---------------------------------------------------------------------

#define S_DIRT1 0
#define S_DIRT1_D (S_DIRT1+1)

DEBRIS (ADirt1, 44, 2, S_DIRT1_D)
{
	S_NORMAL (ROCK, 'D',   20, NULL, &States[S_DIRT1]),
	S_NORMAL (ROCK, 'D',   10, NULL, NULL)
};

#define S_DIRT2 0
#define S_DIRT2_D (S_DIRT2+1)

DEBRIS (ADirt2, 45, 2, S_DIRT2_D)
{
	S_NORMAL (ROCK, 'E',   20, NULL, &States[S_DIRT2]),
	S_NORMAL (ROCK, 'E',   10, NULL, NULL)
};

#define S_DIRT3 0
#define S_DIRT3_D (S_DIRT3+1)

DEBRIS (ADirt3, 46, 2, S_DIRT3_D)
{
	S_NORMAL (ROCK, 'F',   20, NULL, &States[S_DIRT3]),
	S_NORMAL (ROCK, 'F',   10, NULL, NULL)
};

#define S_DIRT4 0
#define S_DIRT4_D (S_DIRT4+1)

DEBRIS (ADirt4, 47, 2, S_DIRT4_D)
{
	S_NORMAL (ROCK, 'G',   20, NULL, &States[S_DIRT4]),
	S_NORMAL (ROCK, 'G',   10, NULL, NULL)
};

#define S_DIRT5 0
#define S_DIRT5_D (S_DIRT5+1)

DEBRIS (ADirt5, 48, 2, S_DIRT5_D)
{
	S_NORMAL (ROCK, 'H',   20, NULL, &States[S_DIRT5]),
	S_NORMAL (ROCK, 'H',   10, NULL, NULL)
};

#define S_DIRT6 0
#define S_DIRT6_D (S_DIRT6+1)

DEBRIS (ADirt6, 49, 2, S_DIRT6_D)
{
	S_NORMAL (ROCK, 'I',   20, NULL, &States[S_DIRT6]),
	S_NORMAL (ROCK, 'I',   10, NULL, NULL)
};

// Stained glass ------------------------------------------------------------

#define S_SGSHARD1 0
#define S_SGSHARD1_D (S_SGSHARD1+5)

SHARD (ASGShard1, 54, 6, S_SGSHARD1_D)
{
	S_NORMAL (SGSA, 'A',	4, NULL 					, &States[S_SGSHARD1+1]),
	S_NORMAL (SGSA, 'B',	4, NULL 					, &States[S_SGSHARD1+2]),
	S_NORMAL (SGSA, 'C',	4, NULL 					, &States[S_SGSHARD1+3]),
	S_NORMAL (SGSA, 'D',	4, NULL 					, &States[S_SGSHARD1+4]),
	S_NORMAL (SGSA, 'E',	4, NULL 					, &States[S_SGSHARD1+0]),

	S_NORMAL (SGSA, 'E',   30, NULL 					, NULL)
};

#define S_SGSHARD2 0
#define S_SGSHARD2_D (S_SGSHARD2+5)

SHARD (ASGShard2, 55, 6, S_SGSHARD2_D)
{
	S_NORMAL (SGSA, 'F',	4, NULL 					, &States[S_SGSHARD2+1]),
	S_NORMAL (SGSA, 'G',	4, NULL 					, &States[S_SGSHARD2+2]),
	S_NORMAL (SGSA, 'H',	4, NULL 					, &States[S_SGSHARD2+3]),
	S_NORMAL (SGSA, 'I',	4, NULL 					, &States[S_SGSHARD2+4]),
	S_NORMAL (SGSA, 'J',	4, NULL 					, &States[S_SGSHARD2+0]),

	S_NORMAL (SGSA, 'J',   30, NULL 					, NULL)
};

#define S_SGSHARD3 0
#define S_SGSHARD3_D (S_SGSHARD3+5)

SHARD (ASGShard3, 56, 6, S_SGSHARD3_D)
{
	S_NORMAL (SGSA, 'K',	4, NULL 					, &States[S_SGSHARD3+1]),
	S_NORMAL (SGSA, 'L',	4, NULL 					, &States[S_SGSHARD3+2]),
	S_NORMAL (SGSA, 'M',	4, NULL 					, &States[S_SGSHARD3+3]),
	S_NORMAL (SGSA, 'N',	4, NULL 					, &States[S_SGSHARD3+4]),
	S_NORMAL (SGSA, 'O',	4, NULL 					, &States[S_SGSHARD3+0]),

	S_NORMAL (SGSA, 'O',   30, NULL 					, NULL)
};

#define S_SGSHARD4 0
#define S_SGSHARD4_D (S_SGSHARD4+5)

SHARD (ASGShard4, 57, 6, S_SGSHARD4_D)
{
	S_NORMAL (SGSA, 'P',	4, NULL 					, &States[S_SGSHARD4+1]),
	S_NORMAL (SGSA, 'Q',	4, NULL 					, &States[S_SGSHARD4+2]),
	S_NORMAL (SGSA, 'R',	4, NULL 					, &States[S_SGSHARD4+3]),
	S_NORMAL (SGSA, 'S',	4, NULL 					, &States[S_SGSHARD4+4]),
	S_NORMAL (SGSA, 'T',	4, NULL 					, &States[S_SGSHARD4+0]),

	S_NORMAL (SGSA, 'T',   30, NULL 					, NULL)
};

#define S_SGSHARD5 0
#define S_SGSHARD5_D (S_SGSHARD5+5)

SHARD (ASGShard5, 58, 6, S_SGSHARD5_D)
{
	S_NORMAL (SGSA, 'U',	4, NULL 					, &States[S_SGSHARD5+1]),
	S_NORMAL (SGSA, 'V',	4, NULL 					, &States[S_SGSHARD5+2]),
	S_NORMAL (SGSA, 'W',	4, NULL 					, &States[S_SGSHARD5+3]),
	S_NORMAL (SGSA, 'X',	4, NULL 					, &States[S_SGSHARD5+4]),
	S_NORMAL (SGSA, 'Y',	4, NULL 					, &States[S_SGSHARD5+0]),

	S_NORMAL (SGSA, 'Y',   30, NULL 					, NULL)
};

#define S_SGSHARD6 0
#define S_SGSHARD6_D (S_SGSHARD6+1)

SHARD (ASGShard6, 59, 2, S_SGSHARD6_D)
{
	S_NORMAL (SGSB, 'A',	4, NULL 					, &States[S_SGSHARD6+0]),
	S_NORMAL (SGSB, 'A',   30, NULL 					, NULL)
};

#define S_SGSHARD7 0
#define S_SGSHARD7_D (S_SGSHARD7+1)

SHARD (ASGShard7, 60, 2, S_SGSHARD7_D)
{
	S_NORMAL (SGSB, 'B',	4, NULL 					, &States[S_SGSHARD7+0]),
	S_NORMAL (SGSB, 'B',   30, NULL 					, NULL)
};

#define S_SGSHARD8 0
#define S_SGSHARD8_D (S_SGSHARD8+1)

SHARD (ASGShard8, 61, 2, S_SGSHARD8_D)
{
	S_NORMAL (SGSB, 'C',	4, NULL 					, &States[S_SGSHARD8+0]),
	S_NORMAL (SGSB, 'C',   30, NULL 					, NULL)
};

#define S_SGSHARD9 0
#define S_SGSHARD9_D (S_SGSHARD9+1)

SHARD (ASGShard9, 62, 2, S_SGSHARD9_D)
{
	S_NORMAL (SGSB, 'D',	4, NULL 					, &States[S_SGSHARD9+0]),
	S_NORMAL (SGSB, 'D',   30, NULL 					, NULL)
};

#define S_SGSHARD0 0
#define S_SGSHARD0_D (S_SGSHARD0+1)

SHARD (ASGShard0, 63, 2, S_SGSHARD0_D)
{
	S_NORMAL (SGSB, 'E',	4, NULL 					, &States[S_SGSHARD0+0]),
	S_NORMAL (SGSB, 'E',   30, NULL 					, NULL)
};

// Dirt clump (spawned by spike) --------------------------------------------

class ADirtClump : public AActor
{
	DECLARE_ACTOR (ADirtClump, AActor);
};

IMPLEMENT_DEF_SERIAL (ADirtClump, AActor);
REGISTER_ACTOR (ADirtClump, Hexen)

FState ADirtClump::States[] =
{
	S_NORMAL (TSPK, 'C',   20, NULL, &States[0])
};

void ADirtClump::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP;
	info->flags2 = MF2_NOTELEPORT;
}

// Dirt stuff

void P_SpawnDirt (AActor *actor, fixed_t radius)
{
	fixed_t x,y,z;
	const TypeInfo *dtype = NULL;
	AActor *mo;
	angle_t angle;

	angle = P_Random()<<5;		// <<24 >>19
	x = actor->x + FixedMul(radius,finecosine[angle]);
	y = actor->y + FixedMul(radius,finesine[angle]);
	z = actor->z + (P_Random()<<9) + FRACUNIT;
	switch (P_Random()%6)
	{
		case 0:
			dtype = RUNTIME_CLASS(ADirt1);
			break;
		case 1:
			dtype = RUNTIME_CLASS(ADirt2);
			break;
		case 2:
			dtype = RUNTIME_CLASS(ADirt3);
			break;
		case 3:
			dtype = RUNTIME_CLASS(ADirt4);
			break;
		case 4:
			dtype = RUNTIME_CLASS(ADirt5);
			break;
		case 5:
			dtype = RUNTIME_CLASS(ADirt6);
			break;
	}
	mo = Spawn (dtype, x, y, z);
	if (mo)
	{
		mo->momz = P_Random()<<10;
	}
}

// Spike (thrust floor) -----------------------------------------------------

void A_ThrustInitUp (AActor *);
void A_ThrustInitDn (AActor *);
void A_ThrustRaise (AActor *);
void A_ThrustLower (AActor *);
void A_ThrustBlock (AActor *);
void A_ThrustImpale (AActor *);

AActor *tsthing;

BOOL PIT_ThrustStompThing (AActor *thing)
{
	fixed_t blockdist;

	if (!(thing->flags & MF_SHOOTABLE) )
		return true;

	blockdist = thing->radius + tsthing->radius;
	if ( abs(thing->x - tsthing->x) >= blockdist || 
		  abs(thing->y - tsthing->y) >= blockdist ||
			(thing->z > tsthing->z+tsthing->height) )
		return true;            // didn't hit it

	if (thing == tsthing)
		return true;            // don't clip against self

	P_DamageMobj (thing, tsthing, tsthing, 10001);
	tsthing->args[1] = 1;	// Mark thrust thing as bloody

	return true;
}

void PIT_ThrustSpike (AActor *actor)
{
	int xl,xh,yl,yh,bx,by;
	int x0,x2,y0,y2;

	tsthing = actor;

	x0 = actor->x - GetInfo (actor)->radius;
	x2 = actor->x + GetInfo (actor)->radius;
	y0 = actor->y - GetInfo (actor)->radius;
	y2 = actor->y + GetInfo (actor)->radius;

	xl = (x0 - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (x2 - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (y0 - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (y2 - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

	// stomp on any things contacted
	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			P_BlockThingsIterator (bx,by,PIT_ThrustStompThing);
}

// AThrustFloor is just a container for all the spike states.
// All the real spikes subclass it.

class AThrustFloor : public AActor
{
	DECLARE_ACTOR (AThrustFloor, AActor);
public:
	fixed_t GetSinkSpeed () { return 6*FRACUNIT; }
	fixed_t GetRaiseSpeed () { return special2*FRACUNIT; }

	ADirtClump *DirtClump;
};

IMPLEMENT_POINTY_SERIAL (AThrustFloor, AActor)
 DECLARE_POINTER (DirtClump)
END_POINTERS

REGISTER_ACTOR (AThrustFloor, Hexen);

void AThrustFloor::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << DirtClump;
}

FState AThrustFloor::States[] =
{
#define S_THRUSTRAISING 0
	S_NORMAL (TSPK, 'A',	2, A_ThrustRaise			, &States[S_THRUSTRAISING]),

#define S_BTHRUSTRAISING (S_THRUSTRAISING+1)
	S_NORMAL (TSPK, 'B',	2, A_ThrustRaise			, &States[S_BTHRUSTRAISING]),

#define S_THRUSTIMPALE (S_BTHRUSTRAISING+1)
	S_NORMAL (TSPK, 'A',	2, A_ThrustImpale			, &States[S_THRUSTRAISING]),

#define S_BTHRUSTIMPALE (S_THRUSTIMPALE+1)
	S_NORMAL (TSPK, 'B',	2, A_ThrustImpale			, &States[S_BTHRUSTRAISING]),

#define S_THRUSTBLOCK (S_BTHRUSTIMPALE+1)
	S_NORMAL (TSPK, 'A',   10, NULL 					, &States[S_THRUSTBLOCK]),

#define S_BTHRUSTBLOCK (S_THRUSTBLOCK+1)
	S_NORMAL (TSPK, 'B',   10, NULL 					, &States[S_BTHRUSTBLOCK]),

#define S_THRUSTLOWER (S_BTHRUSTBLOCK+1)
	S_NORMAL (TSPK, 'A',	2, A_ThrustLower			, &States[S_THRUSTLOWER]),

#define S_BTHRUSTLOWER (S_THRUSTLOWER+1)
	S_NORMAL (TSPK, 'B',	2, A_ThrustLower			, &States[S_BTHRUSTLOWER]),

#define S_THRUSTSTAY (S_BTHRUSTLOWER+1)
	S_NORMAL (TSPK, 'A',   -1, NULL 					, &States[S_THRUSTSTAY]),

#define S_BTHRUSTSTAY (S_THRUSTSTAY+1)
	S_NORMAL (TSPK, 'B',   -1, NULL 					, &States[S_BTHRUSTSTAY]),

#define S_THRUSTINIT2 (S_BTHRUSTSTAY+1)
	S_NORMAL (TSPK, 'A',	3, NULL 					, &States[S_THRUSTINIT2+1]),
	S_NORMAL (TSPK, 'A',	4, A_ThrustInitUp			, &States[S_THRUSTBLOCK]),

#define S_BTHRUSTINIT2 (S_THRUSTINIT2+2)
	S_NORMAL (TSPK, 'B',	3, NULL 					, &States[S_BTHRUSTINIT2+1]),
	S_NORMAL (TSPK, 'B',	4, A_ThrustInitUp			, &States[S_BTHRUSTBLOCK]),

#define S_THRUSTINIT1 (S_BTHRUSTINIT2+2)
	S_NORMAL (TSPK, 'A',	3, NULL 					, &States[S_THRUSTINIT1+1]),
	S_NORMAL (TSPK, 'A',	4, A_ThrustInitDn			, &States[S_THRUSTSTAY]),

#define S_BTHRUSTINIT1 (S_THRUSTINIT1+2)
	S_NORMAL (TSPK, 'B',	3, NULL 					, &States[S_BTHRUSTINIT1+1]),
	S_NORMAL (TSPK, 'B',	4, A_ThrustInitDn			, &States[S_BTHRUSTSTAY]),

#define S_THRUSTRAISE (S_BTHRUSTINIT1+2)
	S_NORMAL (TSPK, 'A',	8, A_ThrustRaise			, &States[S_THRUSTRAISE+1]),
	S_NORMAL (TSPK, 'A',	6, A_ThrustRaise			, &States[S_THRUSTRAISE+2]),
	S_NORMAL (TSPK, 'A',	4, A_ThrustRaise			, &States[S_THRUSTRAISE+3]),
	S_NORMAL (TSPK, 'A',	3, A_ThrustBlock			, &States[S_THRUSTIMPALE]),

#define S_BTHRUSTRAISE (S_THRUSTRAISE+4)
	S_NORMAL (TSPK, 'B',	8, A_ThrustRaise			, &States[S_BTHRUSTRAISE+1]),
	S_NORMAL (TSPK, 'B',	6, A_ThrustRaise			, &States[S_BTHRUSTRAISE+2]),
	S_NORMAL (TSPK, 'B',	4, A_ThrustRaise			, &States[S_BTHRUSTRAISE+3]),
	S_NORMAL (TSPK, 'B',	3, A_ThrustBlock			, &States[S_BTHRUSTIMPALE]),

};

void AThrustFloor::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->radius = 20 * FRACUNIT;
	info->height = 128 * FRACUNIT;
}

// Spike up -----------------------------------------------------------------

class AThrustFloorUp : public AThrustFloor
{
	DECLARE_STATELESS_ACTOR (AThrustFloorUp, AThrustFloor);
};

IMPLEMENT_DEF_SERIAL (AThrustFloorUp, AThrustFloor);
REGISTER_ACTOR (AThrustFloorUp, Hexen);

void AThrustFloorUp::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 10091;
	info->spawnstate = &Super::States[S_THRUSTINIT2];
	info->flags = MF_SOLID;
	info->flags2 = MF2_NOTELEPORT|MF2_FLOORCLIP;
}

// Spike down ---------------------------------------------------------------

class AThrustFloorDown : public AThrustFloor
{
	DECLARE_STATELESS_ACTOR (AThrustFloorDown, AThrustFloor);
};

IMPLEMENT_DEF_SERIAL (AThrustFloorDown, AThrustFloor);
REGISTER_ACTOR (AThrustFloorDown, Hexen);

void AThrustFloorDown::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 10090;
	info->spawnstate = &Super::States[S_THRUSTINIT1];
	info->flags2 = MF2_NOTELEPORT|MF2_FLOORCLIP|MF2_DONTDRAW;
}

//===========================================================================
//
// Thrust floor stuff
//
// Thrust Spike Variables
//		DirtClump	pointer to dirt clump actor
//		special2	speed of raise
//		args[0]		0 = lowered,  1 = raised
//		args[1]		0 = normal,   1 = bloody
//===========================================================================

void A_ThrustInitUp (AActor *actor)
{
	actor->special2 = 5;		// Raise speed
	actor->args[0] = 1;		// Mark as up
	actor->floorclip = 0;
	actor->flags = MF_SOLID;
	actor->flags2 = MF2_NOTELEPORT|MF2_FLOORCLIP;
	actor->special1 = 0L;
}

void A_ThrustInitDn (AActor *actor)
{
	actor->special2 = 5;		// Raise speed
	actor->args[0] = 0;		// Mark as down
	actor->floorclip = GetInfo (actor)->height;
	actor->flags = 0;
	actor->flags2 = MF2_NOTELEPORT|MF2_FLOORCLIP|MF2_DONTDRAW;
	static_cast<AThrustFloor *>(actor)->DirtClump =
		Spawn<ADirtClump> (actor->x, actor->y, actor->z);
}


void A_ThrustRaise (AActor *self)
{
	AThrustFloor *actor = static_cast<AThrustFloor *>(self);

	if (A_RaiseMobj (actor))
	{	// Reached it's target height
		actor->args[0] = 1;
		if (actor->args[1])
			actor->SetStateNF (&AThrustFloor::States[S_BTHRUSTINIT2]);
		else
			actor->SetStateNF (&AThrustFloor::States[S_THRUSTINIT2]);
	}

	// Lose the dirt clump
	if ((actor->floorclip < actor->height) && actor->DirtClump)
	{
		actor->DirtClump->Destroy ();
		actor->DirtClump = NULL;
	}

	// Spawn some dirt
	if (P_Random()<40)
		P_SpawnDirt (actor, actor->radius);
	actor->special2++;							// Increase raise speed
}

void A_ThrustLower (AActor *actor)
{
	if (A_SinkMobj (actor))
	{
		actor->args[0] = 0;
		if (actor->args[1])
			actor->SetStateNF (&AThrustFloor::States[S_BTHRUSTINIT1]);
		else
			actor->SetStateNF (&AThrustFloor::States[S_THRUSTINIT1]);
	}
}

void A_ThrustBlock (AActor *actor)
{
	actor->flags |= MF_SOLID;
}

void A_ThrustImpale (AActor *actor)
{
	// Impale all shootables in radius
	PIT_ThrustSpike (actor);
}

