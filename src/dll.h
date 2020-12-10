#ifndef __DLL_H__
#define __DLL_H__

typedef struct
{
	void (*Destroy) ();
	void (*Serialize) (FArchive &arc);
	static AActor *(*StaticSpawn) (const TypeInfo *type, fixed_t x, fixed_t y, fixed_t z);
	void (*BeginPlay) ();
	void (*LevelSpawned) ();
	void (*Activate) (AActor *activator);
	void (*Deactivate) (AActor *activator);
	void (*Tick) ();
	angle_t (*AngleIncrements) ();
	int (*GetMOD) ();
	const char *(*GetObituary) ();
	const char *(*GetHitObituary) ();
	bool (*SuggestMissileAttack) (fixed_t dist);
	void (*Die) (AActor *source, AActor *inflictor);
	void (*PreExplode) ();
	void (*GetExplodeParms) (int &damage, int &dist, bool &hurtSource);
	int (*DoSpecialDamage) (AActor *target, int damage);
	void (*Howl) ();
	bool (*NewTarget) (AActor *other);
	void (*NoBlockingSet) ();
	fixed_t (*GetSinkSpeed) ();
	fixed_t (*GetRaiseSpeed) ();
	void (*HitFloor) ();
	void (*ChangeSpecial) (byte special, byte data1, byte data2,
		byte data3, byte data4, byte data5);
	bool (*Use) (AActor *activator);
	bool (*UnUse) (AActor *activator);
	void (*UseThink) (AActor *activator);
	void (*SetShade_a) (DWORD rgb);
	void (*SetShade_b) (int r, int g, int b);
	bool (*IsTeammate) (AActor *other);
	static void (*ClearTIDHashes) ();
	void (*AddToHash) ();
	void (*RemoveFromHash) ();
	void (*LinkToWorld) ();
	void (*UnlinkFromWorld) ();
	void (*AdjustFloorClip) ();
	void (*SetOrigin) (fixed_t x, fixed_t y, fixed_t z);
	bool (*SetState) (FState *newstate);
	bool (*SetStateNF) (FState *newstate);
	bool (*UpdateWaterLevel) (fixed_t oldz);
} dllimport_AActor_t;

typedef struct
{
	void (*Con_RunString) (char *cmd);
	void (*Con_CmdAdd) (const char *name, CCmdRun callback);
	void (*Con_CVarSet) (const char *name, char *value, int flags,void(*callback)(char *));
	bool (*Con_CVarUnSet) (const char *name);
	char *(*Con_CVarGet) (const char *name);
	long (*Con_ParseCommandLine) (const char *, int *, char **);

	void* (*Z_Malloc) (size_t, int, void *);
	void (*Z_Free) (void *);

	int (*Printf) (int printlevel, const char *format, ...);

	dllimport_AActor_t class_AActor;
} dllimport_t;

typedef struct
{
	int apiversion;

	void (*Init) (void);
} dllexport_t;

extern dllexport_t *ge;

void Dll_Unload (void);
void *Dll_GetAPI (void *);
void Dll_Init (void);

#endif	// __DLL_H__
