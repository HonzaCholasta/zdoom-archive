#include "actor.h"
#include "info.h"
#include "gi.h"
#include "m_random.h"

void A_BridgeOrbit (AActor *);
void A_BridgeInit (AActor *);

static int orbitTableX[256];
static int orbitTableY[256];

// The Hexen (and Heretic) version of the bridge spawns extra floating
// "balls" orbiting the bridge. The Doom version only shows the bridge
// itself.

// Bridge ball -------------------------------------------------------------

class ABridgeBall : public AActor
{
	DECLARE_ACTOR (ABridgeBall, AActor);
};

IMPLEMENT_DEF_SERIAL (ABridgeBall, AActor);
REGISTER_ACTOR (ABridgeBall, Any);

FState ABridgeBall::States[] =
{
	S_NORMAL (TLGL, 'A',    2, NULL                         , &States[1]),
	S_NORMAL (TLGL, 'A',    5, A_BridgeOrbit                , &States[1])
};

void ABridgeBall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
};

// The bridge itself -------------------------------------------------------

class ABridge : public AActor
{
	DECLARE_ACTOR (ABridge, AActor);
};

IMPLEMENT_DEF_SERIAL (ABridge, AActor);
REGISTER_ACTOR (ABridge, Any);

FState ABridge::States[] =
{
#define S_DBRIDGE 0
	S_BRIGHT (TLGL, 'A',	4, NULL 						, &States[S_DBRIDGE+1]),
	S_BRIGHT (TLGL, 'B',	4, NULL 						, &States[S_DBRIDGE+2]),
	S_BRIGHT (TLGL, 'C',	4, NULL 						, &States[S_DBRIDGE+3]),
	S_BRIGHT (TLGL, 'D',	4, NULL 						, &States[S_DBRIDGE+4]),
	S_BRIGHT (TLGL, 'E',	4, NULL 						, &States[S_DBRIDGE+0]),

#define S_BRIDGE (S_DBRIDGE+5)
	S_NORMAL (TLGL, 'A',    2, NULL                         , &States[S_BRIDGE+1]),
	S_NORMAL (TLGL, 'A',    2, A_BridgeInit                 , &States[S_BRIDGE+2]),
	S_NORMAL (TLGL, 'A',   -1, NULL                         , NULL),

#define S_FREE_BRIDGE (S_BRIDGE+3)
	S_NORMAL (TLGL, 'A',    2, NULL                         , &States[S_FREE_BRIDGE+1]),
	S_NORMAL (TLGL, 'A',  300, NULL                         , NULL)
};

void ABridge::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS (info);
	info->doomednum = 118;
	info->flags = MF_SOLID|MF_NOGRAVITY;
	if (gameinfo.gametype != GAME_Doom)
	{
		info->spawnstate = &States[S_BRIDGE];
		info->radius = 32 * FRACUNIT;
		info->height = 2 * FRACUNIT;
		info->flags2 = MF2_DONTDRAW;
	}
	else
	{
		info->spawnstate = &States[S_DBRIDGE];
		info->radius = 36 * FRACUNIT;
		info->height = 4 * FRACUNIT;
	}
}

// Action functions for the non-Doom bridge --------------------------------

#define ORBIT_RADIUS	(15*FRACUNIT)
void GenerateOrbitTable(void)
{
	static bool inited = false;
	int angle;

	if (inited)
		return;

	inited = true;

	for (angle = 0; angle < 256; angle++)
	{
		orbitTableX[angle] = FixedMul (ORBIT_RADIUS, finecosine[angle<<5]);
		orbitTableY[angle] = FixedMul (ORBIT_RADIUS, finesine[angle<<5]);
	}
}

// New bridge stuff
//	Parent
//		special1	true == removing from world
//
//	Child
//		target		pointer to center mobj
//		args[0]		angle of ball

void A_BridgeOrbit (AActor *self)
{
	if (self->target->special1)
	{
		self->SetState (NULL);
	}
	self->args[0] = (self->args[0] + 3) & 255;
	self->x = self->target->x + orbitTableX[self->args[0]];
	self->y = self->target->y + orbitTableY[self->args[0]];
	self->z = self->target->z;
}

void A_BridgeInit (AActor *self)
{
	byte startangle;
	AActor *ball1, *ball2, *ball3;
	fixed_t cx, cy, cz;

	GenerateOrbitTable ();

	cx = self->x;
	cy = self->y;
	cz = self->z;
	startangle = P_Random ();
	self->special1 = 0;

	// Spawn triad into world
	ball1 = Spawn<ABridgeBall> (cx, cy, cz);
	ball1->args[0] = startangle;
	ball1->target = self;

	ball2 = Spawn<ABridgeBall> (cx, cy, cz);
	ball2->args[0] = (startangle + 85) & 255;
	ball2->target = self;

	ball3 = Spawn<ABridgeBall> (cx, cy, cz);
	ball3->args[0] = (startangle + 170) & 255;
	ball3->target = self;

	A_BridgeOrbit (ball1);
	A_BridgeOrbit (ball2);
	A_BridgeOrbit (ball3);
}

void A_BridgeRemove (AActor *self)
{
	self->special1 = true;		// Removing the bridge
	self->flags &= ~MF_SOLID;
	self->SetState (&ABridge::States[S_FREE_BRIDGE]);
}

