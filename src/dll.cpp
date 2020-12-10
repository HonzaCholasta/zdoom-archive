#include <windows.h>
#include "c_dispatch.h"
#include "w_wad.h"
#include "z_zone.h"
#include "doomstat.h"
#include "d_player.h"
#include "templates.h"
#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_enemy.h"
#include "gstrings.h"
#include "p_effect.h"
#include "v_video.h"
#include "sc_man.h"
#include "d_player.h"
#include "doomstat.h"
#include "c_cvars.h"
#include "p_trace.h"
#include "i_system.h"
#include "p_lnspec.h"
#include "r_sky.h"
#include "dll.h"
#include "dllfunc.h"

#define DLL_API_VERSION 1

static HINSTANCE library;

dllexport_t *ge;
dllimport_t import;

//
// Main functions
//

void Dll_Unload (void)
{
	if (!FreeLibrary (library))
		I_FatalError ("FreeLibrary failed");
	library = NULL;
}

void *Dll_GetAPI (void *parms)
{
	void *(*GetDllAPI) (void *);

	if (library)
		I_FatalError ("Dll_GetApi without Dll_Unload");

	library = LoadLibrary ("doomdll.dll");
	if (library)
	{
		Printf ("LoadLibrary (DoomDll.dll)\n");
	}

//	GetDllAPI = (void *)GetProcAddress (library, "GetDllAPI");
	GetDllAPI = (void *(__cdecl *)(void *))GetProcAddress (library, "GetDllAPI");
//	GetDllAPI = NULL;
	if (!GetDllAPI)
	{
		Dll_Unload ();
		return NULL;
	}

	return GetDllAPI (parms);
}

#define CLASSPROC(cls, proc) import.class_##cls.##proc = ##cls##_##proc
#define CLASSPROCOL(cls, proc, ol) import.class_##cls.##proc##_##ol = ##cls##_##proc##_##ol

void Dll_Init (void)
{
	import.Con_RunString = Con_RunString;
	import.Con_CmdAdd = Con_CmdAdd;
	import.Con_CVarSet = Con_CVarSet;
	import.Con_CVarUnSet = Con_CVarUnSet;
	import.Con_CVarGet = Con_CVarGet;
	import.Con_ParseCommandLine = Con_ParseCommandLine;

	import.Z_Malloc = Z_Malloc;
	import.Z_Free = Z_Free;

	import.Printf = Printf;

/*
	CLASSPROC(AActor, Activate);
	CLASSPROC(AActor, AddToHash);
	CLASSPROC(AActor, AdjustFloorClip);
	CLASSPROC(AActor, AngleIncrements);
	CLASSPROC(AActor, BeginPlay);
	CLASSPROC(AActor, ChangeSpecial);
	CLASSPROC(AActor, ClearTIDHashes);
	CLASSPROC(AActor, Deactivate);
	CLASSPROC(AActor, Destroy);
	CLASSPROC(AActor, Die);
	CLASSPROC(AActor, DoSpecialDamage);
	CLASSPROC(AActor, GetExplodeParms);
	CLASSPROC(AActor, GetHitObituary);
	CLASSPROC(AActor, GetMOD);
	CLASSPROC(AActor, GetObituary);
	CLASSPROC(AActor, GetRaiseSpeed);
	CLASSPROC(AActor, GetSinkSpeed);
	CLASSPROC(AActor, HitFloor);
	CLASSPROC(AActor, Howl);
	CLASSPROC(AActor, IsTeammate);
	CLASSPROC(AActor, LevelSpawned);
	CLASSPROC(AActor, LinkToWorld);
	CLASSPROC(AActor, NewTarget);
	CLASSPROC(AActor, NoBlockingSet);
	CLASSPROC(AActor, PreExplode);
	CLASSPROC(AActor, RemoveFromHash);
	CLASSPROC(AActor, Serialize);
	CLASSPROC(AActor, SetOrigin);
	CLASSPROCOL(AActor, SetShade, a);
	CLASSPROCOL(AActor, SetShade, b);
	CLASSPROC(AActor, SetState);
	CLASSPROC(AActor, SetStateNF);
	CLASSPROC(AActor, StaticSpawn);
	CLASSPROC(AActor, SuggestMissileAttack);
	CLASSPROC(AActor, Tick);
	CLASSPROC(AActor, UnlinkFromWorld);
	CLASSPROC(AActor, UnUse);
	CLASSPROC(AActor, UpdateWaterLevel);
	CLASSPROC(AActor, Use);
	CLASSPROC(AActor, UseThink);
*/

	ge = (dllexport_t *)Dll_GetAPI (&import);

	if (!ge)
		I_FatalError ("Failed to load game DLL");

	if (ge->apiversion != DLL_API_VERSION)
		I_FatalError ("Game DLL version is version %i, but game is version %i", ge->apiversion, DLL_API_VERSION);

	ge->Init ();
}
