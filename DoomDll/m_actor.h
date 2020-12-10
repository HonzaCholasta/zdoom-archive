#ifndef __M_ACTOR_H__
#define __M_ACTOR_H__

/*
// Map Object definition.
class AActor : public DThinker
{
	DECLARE_CLASS (AActor, DThinker)
	HAS_OBJECT_POINTERS
public:
	AActor () throw();
	AActor (const AActor &other) throw();
	AActor &operator= (const AActor &other);
	void Destroy ();
	~AActor ();

	void Serialize (FArchive &arc);

	static AActor *StaticSpawn (const TypeInfo *type, fixed_t x, fixed_t y, fixed_t z);

	inline AActor *GetDefault () const
	{
		return (AActor *)(RUNTIME_TYPE(this)->ActorInfo->Defaults);
	}

	// BeginPlay: Called just after the actor is created
	virtual void BeginPlay ();
	// LevelSpawned: Called after BeginPlay if this actor was spawned by the world
	virtual void LevelSpawned ();

	virtual void Activate (AActor *activator);
	virtual void Deactivate (AActor *activator);

	virtual void Tick ();

	// Smallest yaw interval for a mapthing to be spawned with
	virtual angle_t AngleIncrements ();

	// Means-of-death for this actor
	virtual int GetMOD ();

	// Normal/ranged obituary if this actor is the attacker
	virtual const char *GetObituary ();

	// Melee obituary if this actor is the attacker
	virtual const char *GetHitObituary ();

	// Return true if the monster should use a missile attack, false for melee
	virtual bool SuggestMissileAttack (fixed_t dist);

	// Called when actor dies
	virtual void Die (AActor *source, AActor *inflictor);

	// Called by A_Explode just before exploding the actor
	virtual void PreExplode ();

	// Called by A_Explode to find out how much damage to do
	virtual void GetExplodeParms (int &damage, int &dist, bool &hurtSource);

	// Perform some special damage action. Returns the amount of damage to do.
	// Returning -1 signals the damage routine to exit immediately
	virtual int DoSpecialDamage (AActor *target, int damage);

	// Centaurs and ettins squeal when electrocuted, poisoned, or "holy"-ed
	virtual void Howl ();

	// Returns true if other should become the actor's new target
	virtual bool NewTarget (AActor *other);

	// Called by A_NoBlocking in case the actor wants to drop some presents
	virtual void NoBlockingSet ();

	// Called by A_SinkMobj
	virtual fixed_t GetSinkSpeed ();

	// Called by A_RaiseMobj
	virtual fixed_t GetRaiseSpeed ();

	// Actor just hit the floor
	virtual void HitFloor ();

	virtual void ChangeSpecial (byte special, byte data1, byte data2,
		byte data3, byte data4, byte data5);

	// [GRB] Support for using things
	virtual bool Use (AActor *activator);
	virtual bool UnUse (AActor *activator);
	virtual void UseThink (AActor *activator);

	// Set the alphacolor field properly
	void SetShade (DWORD rgb);
	void SetShade (int r, int g, int b);

// info for drawing
// NOTE: The first member variable *must* be x.
	fixed_t	 		x,y,z;
	AActor			*snext, **sprev;	// links in sector (if needed)
	angle_t			angle;
	WORD			sprite;				// used to find patch_t and flip value
	BYTE			frame;				// sprite frame to draw
	BYTE			xscale, yscale;		// Scaling values; 63 is normal size
	BYTE			RenderStyle;		// Style to draw this actor with
	WORD			renderflags;		// Different rendering flags
	WORD			picnum;				// Draw this instead of sprite if != 0xffff
	DWORD			effects;			// [RH] see p_effect.h
	fixed_t			alpha;
	DWORD			alphacolor;			// Color to draw when STYLE_Shaded

// interaction info
	fixed_t			pitch, roll;
	AActor			*bnext, **bprev;	// links in blocks (if needed)
	struct sector_t	*Sector;
	fixed_t			floorz, ceilingz;	// closest together of contacted secs
	fixed_t			dropoffz;		// killough 11/98: the lowest floor over all contacted Sectors.

	struct sector_t	*floorsector;
	int				floorpic;			// contacted sec floorpic
	fixed_t			radius, height;		// for movement checking
	fixed_t			momx, momy, momz;	// momentums
	int				validcount;			// if == validcount, already checked
	int				tics;				// state tic counter
	FState			*state;
	int				damage;			// For missiles
	int				flags;
	int				flags2;			// Heretic flags
	int				flags3;			// Hexen/Heretic actor-dependant behavior made flagged
	int				mapflags;		// Flags from map (MTF_*)
	int				special1;		// Special info
	int				special2;		// Special info
	int 			health;
	byte			movedir;		// 0-7
	char			visdir;
	short			movecount;		// when 0, select a new dir
	AActor			*target;		// thing being chased/attacked (or NULL)
									// also the originator for missiles
	AActor			*lastenemy;		// Last known enemy -- killogh 2/15/98
	int				reactiontime;	// if non 0, don't attack yet
									// used by player to freeze a bit after
									// teleporting
	int				threshold;		// if > 0, the target will be chased
									// no matter what (even if shot)
	player_s		*player;		// only valid if type of APlayerPawn
	int				lastlook;		// player number last looked for
	WORD			SpawnPoint[3]; 	// For nightmare respawn
	WORD			SpawnAngle;
	AActor			*tracer;		// Thing being chased/attacked for tracers
	fixed_t			floorclip;		// value to use for floor clipping
	WORD			tid;			// thing identifier
	byte			special;		// special
	byte			args[5];		// special arguments

	AActor			*inext, **iprev;// Links to other mobjs in same bucket
	AActor			*goal;			// Monster's goal if not chasing anything
	byte			waterlevel;		// 0=none, 1=feet, 2=waist, 3=eyes
	BYTE			SpawnFlags;
	SWORD			gear;			// killough 11/98: used in torque simulation

	// a linked list of sectors where this object appears
	struct msecnode_s	*touching_sectorlist;				// phares 3/14/98

	//Added by MC:
	int id;							// Player ID (for items, # in list.)

	BYTE FloatBobPhase;
	WORD Translation;

	// [RH] Stuff that used to be part of an Actor Info
	WORD SeeSound;
	WORD AttackSound;
	WORD PainSound;
	WORD DeathSound;
	WORD ActiveSound;

	int ReactionTime;
	fixed_t Speed;
	int Mass;
	SWORD PainChance;

	FState *SpawnState;
	FState *SeeState;
	FState *PainState;
	FState *MeleeState;
	FState *MissileState;
	FState *CrashState;
	FState *DeathState;
	FState *XDeathState;
	FState *BDeathState;
	FState *IDeathState;
	FState *RaiseState;

	// [RH] Decal(s) this weapon/projectile generates on impact.
	FDecalBase *DecalGenerator;

	// Public functions
	bool IsTeammate (AActor *other);

	// ThingIDs
	static void ClearTIDHashes ();
	void AddToHash ();
	void RemoveFromHash ();

private:
	static AActor *TIDHash[128];
	static inline int TIDHASH (int key) { return key & 127; }

	friend class FActorIterator;

public:
	void LinkToWorld ();
	void UnlinkFromWorld ();
	void AdjustFloorClip ();
	void SetOrigin (fixed_t x, fixed_t y, fixed_t z);
	bool SetState (FState *newstate);
	bool SetStateNF (FState *newstate);
	bool UpdateWaterLevel (fixed_t oldz);

	static FState States[];
};
*/

#endif	// __M_ACTOR_H__
