#include "actor.h"
#include "info.h"
#include "p_lnspec.h"
#include "a_action.h"
#include "m_random.h"
#include "s_sound.h"
#include "d_player.h"
#include "p_local.h"
#include "p_terrain.h"

/***************************** IceChunk ************************************/

class AIceChunk : public APlayerPawn
{
	DECLARE_ACTOR (AIceChunk, APlayerPawn);
public:
	void PlayIdle () {}
	void PlayRunning () {}
	void PlayAttacking () {}
	void PlayAttacking2 () {}
};

IMPLEMENT_DEF_SERIAL (AIceChunk, APlayerPawn);
REGISTER_ACTOR (AIceChunk, Any);

FState AIceChunk::States[] =
{
#define S_ICECHUNK 0
	S_NORMAL (ICEC, 'A',   10, NULL 					, &States[S_ICECHUNK+1]),
	S_NORMAL (ICEC, 'B',   10, A_IceSetTics 			, &States[S_ICECHUNK+2]),
	S_NORMAL (ICEC, 'C',   10, A_IceSetTics 			, &States[S_ICECHUNK+3]),
	S_NORMAL (ICEC, 'D',   10, A_IceSetTics 			, NULL),

#define S_ICECHUNK_HEAD (S_ICECHUNK+4)
	S_NORMAL (ICEC, 'A',   10, A_IceCheckHeadDone		, &States[S_ICECHUNK_HEAD]),

#define S_ICECHUNK_HEAD2 (S_ICECHUNK_HEAD+1)
	S_NORMAL (ICEC, 'A', 1050, NULL 					, NULL)
};

void AIceChunk::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_ICECHUNK];
	info->radius = 3 * FRACUNIT;
	info->height = 4 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_CANNOTPUSH|MF2_FLOORCLIP;
}

/***************************************************************************/

//----------------------------------------------------------------------------
//
// PROC A_NoBlocking
//
//----------------------------------------------------------------------------

void A_NoBlocking (AActor *actor)
{
	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		actor->translucency = OPAQUE;
		actor->visdir = 0;
	}

	actor->flags &= ~MF_SOLID;
	actor->NoBlockingSet ();
}

//==========================================================================
//
// A_HideThing
//
//==========================================================================

void A_HideThing (AActor *actor)
{
	actor->flags2 |= MF2_DONTDRAW;
}

//==========================================================================
//
// A_UnHideThing
//
//==========================================================================

void A_UnHideThing (AActor *actor)
{
	actor->flags2 &= ~MF2_DONTDRAW;
}

//===========================================================================
//
// A_CheckFloor - Checks if an object hit the floor
//
//===========================================================================

void A_CheckFloor (AActor *actor)
{
	if (actor->z <= actor->floorz)
	{
		actor->z = actor->floorz;
		actor->flags2 &= ~MF2_LOGRAV;
		actor->SetState (GetInfo (actor)->deathstate);
	}
}

//============================================================================
//
// A_FreezeDeath
//
//============================================================================

void A_FreezeDeath (AActor *actor)
{
	actor->tics = 75+P_Random()+P_Random();
	actor->flags |= MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
	actor->flags2 |= MF2_PUSHABLE|MF2_TELESTOMP|MF2_PASSMOBJ|MF2_SLIDE;
	actor->height <<= 2;
	S_Sound (actor, CHAN_BODY, "FreezeDeath", 1, ATTN_NORM);

	if (actor->player)
	{
		actor->player->damagecount = 0;
		actor->player->poisoncount = 0;
		actor->player->bonuscount = 0;
	}
	else if (actor->flags&MF_COUNTKILL && actor->special)
	{ // Initiate monster death actions
		LineSpecials [actor->special] (NULL, actor, actor->args[0],
			actor->args[1], actor->args[2], actor->args[3], actor->args[4]);
		actor->special = 0;
	}
}

//============================================================================
//
// A_IceSetTics
//
//============================================================================

void A_IceSetTics (AActor *actor)
{
	int floor;

	actor->tics = 70+(P_Random()&63);
	floor = P_GetThingFloorType (actor);
	if (Terrains[floor].DamageMOD == MOD_LAVA)
	{
		actor->tics >>= 2;
	}
	else if (Terrains[floor].DamageMOD == MOD_ICE)
	{
		actor->tics <<= 1;
	}
}

//============================================================================
//
// A_IceCheckHeadDone
//
//============================================================================

void A_IceCheckHeadDone (AActor *actor)
{
	if (actor->special2 == 666)
	{
		actor->SetState (&AIceChunk::States[S_ICECHUNK_HEAD2]);
	}
}

//============================================================================
//
// A_FreezeDeathChunks
//
//============================================================================

void A_FreezeDeathChunks (AActor *actor)
{
	int i;
	AActor *mo;
	
	if (actor->momx || actor->momy || actor->momz)
	{
		actor->tics = 105;
		return;
	}
	S_Sound (actor, CHAN_BODY, "FreezeShatter", 1, ATTN_NORM);

	for (i = 12 + (P_Random()&15); i >= 0; i--)
	{
		mo = Spawn<AIceChunk> (
			actor->x + (((P_Random()-128)*actor->radius)>>7), 
			actor->y + (((P_Random()-128)*actor->radius)>>7), 
			actor->z + (P_Random()*actor->height/255));
		mo->SetState (GetInfo (mo)->spawnstate + (P_Random()%3));
		if (mo)
		{
			mo->momz = FixedDiv(mo->z-actor->z, actor->height)<<2;
			mo->momx = PS_Random () << (FRACBITS-7);
			mo->momy = PS_Random () << (FRACBITS-7);
			A_IceSetTics (mo); // set a random tic wait
		}
	}
	for (i = 12 + (P_Random()&15); i >= 0; i--)
	{
		mo = Spawn<AIceChunk> (
			actor->x + (((P_Random()-128)*actor->radius)>>7), 
			actor->y + (((P_Random()-128)*actor->radius)>>7), 
			actor->z + (P_Random()*actor->height/255));
		mo->SetState (GetInfo (mo)->spawnstate + (P_Random()%3));
		if (mo)
		{
			mo->momz = FixedDiv (mo->z-actor->z, actor->height)<<2;
			mo->momx = PS_Random() << (FRACBITS-7);
			mo->momy = PS_Random() << (FRACBITS-7);
			A_IceSetTics (mo); // set a random tic wait
		}
	}
	if (actor->player)
	{ // attach the player's view to a chunk of ice
		AIceChunk *head = Spawn<AIceChunk> (actor->x, actor->y, actor->z+VIEWHEIGHT);
		head->SetState (&AIceChunk::States[S_ICECHUNK_HEAD]);
		head->momz = FixedDiv(head->z-actor->z, actor->height)<<2;
		head->momx = PS_Random() << (FRACBITS-7);
		head->momy = PS_Random() << (FRACBITS-7);
		head->flags2 |= MF2_ICEDAMAGE; // used to force blue palette
		head->flags2 &= ~MF2_FLOORCLIP;
		head->player = actor->player;
		actor->player = NULL;
		head->health = actor->health;
		head->angle = actor->angle;
		head->player->mo = head;
		head->pitch = 0;
	}
	actor->RemoveFromHash ();
	actor->SetState (&AActor::States[S_FREETARGMOBJ]);
	actor->flags2 |= MF2_DONTDRAW;
}

//----------------------------------------------------------------------------
//
// CorpseQueue Routines (used by Hexen)
//
//----------------------------------------------------------------------------

// Corpse queue for monsters - this should be saved out
#define CORPSEQUEUESIZE	64

class DCorpseQueue : public DThinker
{
	DECLARE_SERIAL (DCorpseQueue, DThinker);
public:
	DCorpseQueue ();
	void Enqueue (AActor *);
	void Dequeue (AActor *);
protected:
	AActor *CorpseQueue[CORPSEQUEUESIZE];
	int CorpseQueueSlot;
};

IMPLEMENT_POINTY_SERIAL (DCorpseQueue, DThinker)
 DECLARE_POINTER (CorpseQueue[0])
 DECLARE_POINTER (CorpseQueue[1])
 DECLARE_POINTER (CorpseQueue[2])
 DECLARE_POINTER (CorpseQueue[3])
 DECLARE_POINTER (CorpseQueue[4])
 DECLARE_POINTER (CorpseQueue[5])
 DECLARE_POINTER (CorpseQueue[6])
 DECLARE_POINTER (CorpseQueue[7])
 DECLARE_POINTER (CorpseQueue[8])
 DECLARE_POINTER (CorpseQueue[9])
 DECLARE_POINTER (CorpseQueue[10])
 DECLARE_POINTER (CorpseQueue[11])
 DECLARE_POINTER (CorpseQueue[12])
 DECLARE_POINTER (CorpseQueue[13])
 DECLARE_POINTER (CorpseQueue[14])
 DECLARE_POINTER (CorpseQueue[15])
 DECLARE_POINTER (CorpseQueue[16])
 DECLARE_POINTER (CorpseQueue[17])
 DECLARE_POINTER (CorpseQueue[18])
 DECLARE_POINTER (CorpseQueue[19])
 DECLARE_POINTER (CorpseQueue[20])
 DECLARE_POINTER (CorpseQueue[21])
 DECLARE_POINTER (CorpseQueue[22])
 DECLARE_POINTER (CorpseQueue[23])
 DECLARE_POINTER (CorpseQueue[24])
 DECLARE_POINTER (CorpseQueue[25])
 DECLARE_POINTER (CorpseQueue[26])
 DECLARE_POINTER (CorpseQueue[27])
 DECLARE_POINTER (CorpseQueue[28])
 DECLARE_POINTER (CorpseQueue[29])
 DECLARE_POINTER (CorpseQueue[30])
 DECLARE_POINTER (CorpseQueue[31])
 DECLARE_POINTER (CorpseQueue[32])
 DECLARE_POINTER (CorpseQueue[33])
 DECLARE_POINTER (CorpseQueue[34])
 DECLARE_POINTER (CorpseQueue[35])
 DECLARE_POINTER (CorpseQueue[36])
 DECLARE_POINTER (CorpseQueue[37])
 DECLARE_POINTER (CorpseQueue[38])
 DECLARE_POINTER (CorpseQueue[39])
 DECLARE_POINTER (CorpseQueue[40])
 DECLARE_POINTER (CorpseQueue[41])
 DECLARE_POINTER (CorpseQueue[42])
 DECLARE_POINTER (CorpseQueue[43])
 DECLARE_POINTER (CorpseQueue[44])
 DECLARE_POINTER (CorpseQueue[45])
 DECLARE_POINTER (CorpseQueue[46])
 DECLARE_POINTER (CorpseQueue[47])
 DECLARE_POINTER (CorpseQueue[48])
 DECLARE_POINTER (CorpseQueue[49])
 DECLARE_POINTER (CorpseQueue[50])
 DECLARE_POINTER (CorpseQueue[51])
 DECLARE_POINTER (CorpseQueue[52])
 DECLARE_POINTER (CorpseQueue[53])
 DECLARE_POINTER (CorpseQueue[54])
 DECLARE_POINTER (CorpseQueue[55])
 DECLARE_POINTER (CorpseQueue[56])
 DECLARE_POINTER (CorpseQueue[57])
 DECLARE_POINTER (CorpseQueue[58])
 DECLARE_POINTER (CorpseQueue[59])
 DECLARE_POINTER (CorpseQueue[60])
 DECLARE_POINTER (CorpseQueue[61])
 DECLARE_POINTER (CorpseQueue[62])
 DECLARE_POINTER (CorpseQueue[63])
END_POINTERS

DCorpseQueue::DCorpseQueue ()
{
	CorpseQueueSlot = 0;
	memset (CorpseQueue, 0, sizeof(CorpseQueue));
}

void DCorpseQueue::Serialize (FArchive &arc)
{
	int i;

	Super::Serialize (arc);
	for (i = 0; i < CORPSEQUEUESIZE; i++)
		arc << CorpseQueue[i];
	arc << CorpseQueueSlot;
}

// throw another corpse on the queue
void DCorpseQueue::Enqueue (AActor *actor)
{
	if (CorpseQueue[CorpseQueueSlot] != NULL)
	{ // Too many corpses - remove an old one
		CorpseQueue[CorpseQueueSlot]->Destroy ();
	}
	CorpseQueue[CorpseQueueSlot] = actor;
	CorpseQueueSlot = (CorpseQueueSlot + 1) % CORPSEQUEUESIZE;
}

// Remove a mobj from the queue (for resurrection)
void DCorpseQueue::Dequeue (AActor *actor)
{
	int slot;

	for (slot = 0; slot < CORPSEQUEUESIZE; slot++)
	{
		if (CorpseQueue[slot] == actor)
		{
			CorpseQueue[slot] = NULL;
			break;
		}
	}
}

void A_QueueCorpse (AActor *actor)
{
	DCorpseQueue *queue;
	TThinkerIterator<DCorpseQueue> iterator;

	queue = iterator.Next ();
	if (queue == NULL)
	{
		queue = new DCorpseQueue;
	}
	queue->Enqueue (actor);
}

void A_DeQueueCorpse (AActor *actor)
{
	DCorpseQueue *queue;
	TThinkerIterator<DCorpseQueue> iterator;

	queue = iterator.Next ();
	if (queue == NULL)
	{
		queue = new DCorpseQueue;
	}
	queue->Dequeue (actor);
}
