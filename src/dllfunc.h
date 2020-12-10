#ifndef __DLLFUNC_H__
#define __DLLFUNC_H__

#include "doomtype.h"
#include "info.h"
#include "c_dispatch.h"
#include "tables.h"

// Console
void Con_RunString (char *cmd);
void Con_CmdAdd (const char *name, CCmdRun callback);
void Con_CVarSet (const char *name, char *value, int flags, void(*callback)(char *));
bool Con_CVarUnSet (const char *name);
char *Con_CVarGet (const char *name);
long Con_ParseCommandLine (const char *, int *, char **);	// defined in c_dispatch.cpp

// Class AActor (defined in p_mobj.cpp)
void AActor_Destroy ();
void AActor_Serialize (FArchive &arc);
static AActor *AActor_StaticSpawn (const TypeInfo *type, fixed_t x, fixed_t y, fixed_t z);
void AActor_BeginPlay ();	
void AActor_LevelSpawned ();
void AActor_Activate (AActor *activator);
void AActor_Deactivate (AActor *activator);
void AActor_Tick ();
angle_t AActor_AngleIncrements ();
int AActor_GetMOD ();
const char *AActor_GetObituary ();
const char *AActor_GetHitObituary ();
bool AActor_SuggestMissileAttack (fixed_t dist);
void AActor_Die (AActor *source, AActor *inflictor);
void AActor_PreExplode ();
void AActor_GetExplodeParms (int &damage, int &dist, bool &hurtSource);
int AActor_DoSpecialDamage (AActor *target, int damage);
void AActor_Howl ();
bool AActor_NewTarget (AActor *other);
void AActor_NoBlockingSet ();
fixed_t AActor_GetSinkSpeed ();
fixed_t AActor_GetRaiseSpeed ();
void AActor_HitFloor ();
void AActor_ChangeSpecial (byte special, byte data1, byte data2, byte data3, byte data4, byte data5);
bool AActor_Use (AActor *activator);
bool AActor_UnUse (AActor *activator);
void AActor_UseThink (AActor *activator);
void AActor_SetShade_a (DWORD rgb);
void AActor_SetShade_b (int r, int g, int b);
bool AActor_IsTeammate (AActor *other);
static void AActor_ClearTIDHashes ();
void AActor_AddToHash ();
void AActor_RemoveFromHash ();
void AActor_LinkToWorld ();
void AActor_UnlinkFromWorld ();
void AActor_AdjustFloorClip ();
void AActor_SetOrigin (fixed_t x, fixed_t y, fixed_t z);
bool AActor_SetState (FState *newstate);
bool AActor_SetStateNF (FState *newstate);
bool AActor_UpdateWaterLevel (fixed_t oldz);

#endif	// __DLLFUNC_H__
