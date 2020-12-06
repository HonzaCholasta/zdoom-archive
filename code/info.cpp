// info.cpp: Keep track of available actors and their states
//
// This is completely different from the Doom original.


#include "m_fixed.h"
#include "c_dispatch.h"

#include "info.h"
#include "gi.h"

#include "actor.h"
#include "r_state.h"
#include "i_system.h"
#include "p_local.h"

// Each state is owned by an actor. Actors can own any number of
// states, but a single state cannot be owned by more than one
// actor. States are archived by recording the actor they belong
// to and the index into that actor's list of states.

// For NULL states, which aren't owned by any actor, the owner
// is recorded as AActor with the following state. AActor should
// never actually have this many states of its own, so this
// is (relatively) safe.

#define NULL_STATE_INDEX	127

FArchive &operator<< (FArchive &arc, FState *&state)
{
	if (arc.IsStoring ())
	{
		if (state == NULL)
		{
			arc.UserWriteClass (RUNTIME_CLASS(AActor));
			arc.WriteCount (NULL_STATE_INDEX);
			return arc;
		}

		FActorInfo *info = RUNTIME_CLASS(AActor)->ActorInfo;

		if (state >= info->OwnedStates &&
			state < info->OwnedStates + info->NumOwnedStates)
		{
			arc.UserWriteClass (RUNTIME_CLASS(AActor));
			arc.WriteCount (state - info->OwnedStates);
			return arc;
		}

		FActorInfoInitializer *init = FActorInfoInitializer::StaticInitList;
		while (init != NULL)
		{
			if (init->info)
			{
				info = init->info->ActorInfo;
				if (info != NULL)
				{
					if (state >= info->OwnedStates &&
						state < info->OwnedStates + info->NumOwnedStates)
					{
						arc.UserWriteClass (init->info);
						arc.WriteCount (state - info->OwnedStates);
						return arc;
					}
				}
			}
			init = init->next;
		}
		I_Error ("Cannot find owner for state %p\n", state);
	}
	else
	{
		const TypeInfo *info;
		DWORD ofs;

		arc.UserReadClass (info);
		ofs = arc.ReadCount ();
		if (ofs == NULL_STATE_INDEX && info == RUNTIME_CLASS(AActor))
		{
			state = NULL;
		}
		else if (info->ActorInfo)
		{
			state = info->ActorInfo->OwnedStates + ofs;
		}
		else
		{
			state = NULL;
		}
	}
	return arc;
}

FActorInfoInitializer *FActorInfoInitializer::StaticInitList = NULL;

static void ProcessStates (FState *states, int numstates)
{
	int sprite = -1;

	if (states == NULL)
		return;

	while (--numstates >= 0)
	{
		if (sprite == -1 || strncmp (sprites[sprite].name, states->sprite.name, 4) != 0)
		{
			size_t i;

			sprite = -1;
			for (i = 0; i < sprites.Size (); i++)
			{
				if (strncmp (sprites[i].name, states->sprite.name, 4) == 0)
				{
					sprite = i;
					break;
				}
			}
			if (sprite == -1)
			{
				spritedef_t temp;
				strncpy (temp.name, states->sprite.name, 4);
				temp.name[5] = 0;
				sprite = sprites.Push (temp);
			}
		}
		states->sprite.index = sprite;
		states++;
	}
}

void FActorInfoInitializer::StaticInit (EGameType game)
{
	// Make sure the AActor class has its ActorInfo created
	AActor::SetDefaults ((RUNTIME_CLASS(AActor)->ActorInfo = new FActorInfo));
	ProcessStates (RUNTIME_CLASS(AActor)->ActorInfo->OwnedStates, RUNTIME_CLASS(AActor)->ActorInfo->NumOwnedStates);

	// Find all actors valid for the current gametype and
	// create ActorInfos for them.
	FActorInfoInitializer *init = StaticInitList;
	while (init != NULL)
	{
		if (init->gamemode == GAME_Any || (init->gamemode & game))
		{
			TypeInfo *info = const_cast<TypeInfo *>(init->info);
			info->ActorInfo = new FActorInfo;
			init->active = 1;
			init->setdefaults (info->ActorInfo);
			if (info->ActorInfo->spawnid != 0 && info->ActorInfo->spawnid < MAX_SPAWNABLES)
				SpawnableThings[info->ActorInfo->spawnid] = info;
			if (info->ActorInfo->doomednum != -1)
				DoomEdMap.AddType (info->ActorInfo->doomednum, info);
			if (info->ActorInfo->OwnedStates && info->ActorInfo->NumOwnedStates)
				ProcessStates (info->ActorInfo->OwnedStates, info->ActorInfo->NumOwnedStates);
		}
		init = init->next;
	}

	// Find classes for all actors

	// For any actors without infos, use the info from the closest
	// ancestor class.
	int i;

	for (i = 0; i < TypeInfo::m_NumTypes; i++)
	{
		if (TypeInfo::m_Types[i]->ActorInfo == NULL)
		{
			const TypeInfo *type = TypeInfo::m_Types[i]->ParentType;
			while (type && type->ActorInfo == NULL)
				type = type->ParentType;
			if (type)
				TypeInfo::m_Types[i]->ActorInfo = type->ActorInfo;
		}
	}
}

void FActorInfoInitializer::StaticSetDefaults (EGameType game)
{
	FActorInfoInitializer *init = StaticInitList;
	while (init != NULL)
	{
		if (init->active)
		{
			init->setdefaults (init->info->ActorInfo);
		}
		init = init->next;
	}
}

FDoomEdMap DoomEdMap;

FDoomEdMap::FDoomEdEntry *FDoomEdMap::DoomEdHash[DOOMED_HASHSIZE];

void FDoomEdMap::AddType (int doomednum, const TypeInfo *type)
{
	int hash = doomednum % DOOMED_HASHSIZE;
	FDoomEdEntry *entry = DoomEdHash[hash];
	while (entry && entry->DoomEdNum != doomednum)
		entry = entry->HashNext;
	if (entry == NULL)
	{
		entry = new FDoomEdEntry;
		entry->HashNext = DoomEdHash[hash];
		entry->DoomEdNum = doomednum;
		DoomEdHash[hash] = entry;
	}
	entry->Type = type;
}

const TypeInfo *FDoomEdMap::FindType (int doomednum)
{
	int hash = doomednum % DOOMED_HASHSIZE;
	FDoomEdEntry *entry = DoomEdHash[hash];
	while (entry && entry->DoomEdNum != doomednum)
		entry = entry->HashNext;
	return entry ? entry->Type : NULL;
}

static int STACK_ARGS sortnums (const void *a, const void *b)
{
	return (*((const TypeInfo **)a))->ActorInfo->doomednum -
		(*((const TypeInfo **)b))->ActorInfo->doomednum;
}

BEGIN_COMMAND (dumpmapthings)
{
	TArray<const TypeInfo *> infos (TypeInfo::m_NumTypes);
	int i;

	for (i = 0; i < TypeInfo::m_NumTypes; i++)
	{
		if (TypeInfo::m_Types[i]->ActorInfo &&
			TypeInfo::m_Types[i]->ActorInfo->doomednum != -1)
		{
			infos.Push (TypeInfo::m_Types[i]);
		}
	}

	if (infos.Size () == 0)
	{
		Printf (PRINT_HIGH, "No map things registered\n");
	}
	else
	{
		qsort (&infos[0], infos.Size (), sizeof(TypeInfo *), sortnums);

		for (i = 0; i < (int)infos.Size (); i++)
		{
			Printf (PRINT_HIGH, "%6d %s\n",
				infos[i]->ActorInfo->doomednum, infos[i]->Name + 1);
		}
	}
}
END_COMMAND (dumpmapthings)

BOOL CheckCheatmode ();

BEGIN_COMMAND (summon)
{
	if (CheckCheatmode ())
		return;

	if (argc > 1)
	{
		// Don't use FindType, because we want a case-insensitive search
		const TypeInfo *type = TypeInfo::IFindType (argv[1]);
		if (type == NULL)
		{
			Printf (PRINT_HIGH, "Unknown class '%s'\n", argv[1]);
			return;
		}
		Net_WriteByte (DEM_SUMMON);
		Net_WriteString (type->Name + 1);
	}
}
END_COMMAND (summon)
