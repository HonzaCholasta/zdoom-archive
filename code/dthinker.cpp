#include "dthinker.h"
#include "z_zone.h"
#include "stats.h"
#include "p_local.h"

static cycle_t ThinkCycles;
extern cycle_t BotSupportCycles;

IMPLEMENT_SERIAL (DThinker, DObject)

static DThinker *NextToThink;

DThinker *DThinker::FirstThinker = NULL;
DThinker *DThinker::LastThinker = NULL;

void DThinker::Serialize (FArchive &arc)
{
	Super::Serialize (arc);

	// We do not serialize m_Next or m_Prev, because the DThinker
	// constructor handles them for us.
}

void DThinker::SerializeAll (FArchive &arc, bool hubLoad)
{
	DThinker *thinker;
	BYTE id;

	if (arc.IsStoring ())
	{
		thinker = FirstThinker;
		id = 1;
		while (thinker)
		{
			arc << id;
			arc << thinker;
			thinker = thinker->m_Next;
		}
		id = 0;
		arc << id;
	}
	else
	{
		if (hubLoad)
			DestroyMostThinkers ();
		else
			DestroyAllThinkers ();

		arc << id;
		while (id)
		{
			DThinker *thinker;
			arc << thinker;
			arc << id;
		}

		// killough 3/26/98: Spawn icon landings:
		P_SpawnBrainTargets ();
	}
}

DThinker::DThinker ()
{
	// Add a new thinker at the end of the list.
	m_Prev = LastThinker;
	m_Next = NULL;
	if (LastThinker)
		LastThinker->m_Next = this;
	if (!FirstThinker)
		FirstThinker = this;
	LastThinker = this;
}

DThinker::~DThinker ()
{
	if (FirstThinker == this)
		FirstThinker = m_Next;
	if (LastThinker == this)
		LastThinker = m_Prev;
	if (m_Next)
		m_Next->m_Prev = m_Prev;
	if (m_Prev)
		m_Prev->m_Next = m_Next;
}

void DThinker::Destroy ()
{
	if (this == NextToThink)
		NextToThink = m_Next;
	if (FirstThinker == this)
		FirstThinker = m_Next;
	if (LastThinker == this)
		LastThinker = m_Prev;
	if (m_Next)
		m_Next->m_Prev = m_Prev;
	if (m_Prev)
		m_Prev->m_Next = m_Next;
	m_Next = m_Prev = NULL;
	Super::Destroy ();
}

// Destroy every thinker
void DThinker::DestroyAllThinkers ()
{
	DThinker *currentthinker = FirstThinker;
	while (currentthinker)
	{
		DThinker *next = currentthinker->m_Next;
		DObject::BeginFrame ();
		currentthinker->Destroy ();
		DObject::EndFrame ();
		currentthinker = next;
	}
}

// Destroy all thinkers except for player-controlled actors
void DThinker::DestroyMostThinkers ()
{
	DThinker *thinker = FirstThinker;
	while (thinker)
	{
		DThinker *next = thinker->m_Next;
		if (!thinker->IsKindOf (RUNTIME_CLASS (AActor)) ||
			static_cast<AActor *>(thinker)->player == NULL ||
			static_cast<AActor *>(thinker)->player->mo
			 != static_cast<AActor *>(thinker))
		{
			thinker->Destroy ();
		}
		thinker = next;
	}
	DObject::EndFrame ();
}

void DThinker::RunThinkers ()
{
	DThinker *currentthinker;

	ThinkCycles = BotSupportCycles = 0;

	clock (ThinkCycles);
	currentthinker = FirstThinker;
	while (currentthinker)
	{
		NextToThink = currentthinker->m_Next;
		if (currentthinker->ObjectFlags & OF_JustSpawned)
		{	// OF_JustSpawned is valid with actors only
			currentthinker->ObjectFlags &= ~OF_JustSpawned;
			(static_cast<AActor *> (currentthinker))->PostBeginPlay ();
		}
		currentthinker->RunThink ();
		currentthinker = NextToThink;
	}
	unclock (ThinkCycles);
}

void *DThinker::operator new (size_t size)
{
	return Z_Malloc (size, PU_LEVSPEC, 0);
}

// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
void DThinker::operator delete (void *mem)
{
	Z_Free (mem);
}

BEGIN_STAT (think)
{
	sprintf (out, "Think time = %04.1f ms",
		SecondsPerCycle * (double)ThinkCycles * 1000);
}
END_STAT (think)