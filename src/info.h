/*
** info.h
**
**---------------------------------------------------------------------------
** Copyright 1998-2001 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
** Important restrictions because of the way FState is structured:
**
** The range of Frame is [0,63]. Since sprite naming conventions
** are even more restrictive than this, this isn't something to
** really worry about.
**
** The range of Tics is [-1,65534]. If Misc1 is important, then
** the range of Tics is reduced to [-1,254], because Misc1 also
** doubles as the high byte of the tic.
**
** The range of Misc1 is [-128,127] and Misc2's range is [0,255].
**
** When compiled with Visual C++, this struct is 16 bytes. With
** any other compiler (assuming a 32-bit architecture), it is 20 bytes.
** This is because with VC++, I can use the charizing operator to
** initialize the name array to exactly 4 chars. If GCC would
** compile something like char t = "PLYR"[0]; as char t = 'P'; then GCC
** could also use the 16-byte version. Unfortunately, GCC compiles it
** more like:
**
** char t;
** void initializer () {
**     static const char str[]="PLYR";
**     t = str[0];
** }
**
** While this does allow the use of a 16-byte FState, the additional
** code amounts to more than 4 bytes.
**
** If C++ would allow char name[4] = "PLYR"; without an error (as C does),
** I could just initialize the name as a regular string and be done with it.
*/

#ifndef __INFO_H__
#define __INFO_H__

#include "dobject.h"
#include "dthinker.h"
#include "farchive.h"

const BYTE SF_FULLBRIGHT = 0x40;
const BYTE SF_BIGTIC	 = 0x80;

struct FState
{
	union
	{
#if _MSC_VER
		char name[4];
#else
		char name[8];	// 4 for name, 1 for '\0', 3 for pad
#endif
		int index;
	} sprite;
	BYTE		Tics;
	SBYTE		Misc1;
	BYTE		Misc2;
	BYTE		Frame;
	actionf_t	Action;
	FState		*NextState;

	inline int GetFrame() const
	{
		return Frame & ~(SF_FULLBRIGHT|SF_BIGTIC);
	}
	inline int GetFullbright() const
	{
		return Frame & SF_FULLBRIGHT ? 0x10 /*RF_FULLBRIGHT*/ : 0;
	}
	inline int GetTics() const
	{
#ifdef __BIG_ENDIAN__
		return Frame & SF_BIGTIC ? (Tics|((BYTE)Misc1<<8))-1 : Tics-1;
#else
		// Use some trickery to help the compiler create this without
		// using any jumps.
		return ((*(int *)&Tics) & ((*(int *)&Tics) < 0 ? 0xffff : 0xff)) - 1;
#endif
	}
	inline int GetMisc1() const
	{
		return Frame & SF_BIGTIC ? 0 : Misc1;
	}
	inline int GetMisc2() const
	{
		return Misc2;
	}
	inline FState *GetNextState() const
	{
		return NextState;
	}
	inline actionf_t GetAction() const
	{
		return Action;
	}
};

FArchive &operator<< (FArchive &arc, FState *&state);

#if _MSC_VER
#define _S__SPRITE_(spr) \
	{ {{(char)(#@spr>>24),(char)(#@spr>>16),(char)(#@spr>>8),(char)#@spr}}
#else
#define _S__SPRITE_(spr) \
	{ {{#spr}}
#endif

#define _S__FR_TIC_(spr,frm,tic,m1,m2,cmd,next) \
	_S__SPRITE_(spr), (tic+1)&255, m1|((tic+1)>>8), m2, (tic>254)?SF_BIGTIC|(frm):(frm), \
	{cmd}, next }

#define S_NORMAL2(spr,frm,tic,cmd,next,m1,m2) \
	_S__FR_TIC_(spr, (frm) - 'A', tic, m1, m2, (void *)cmd, next)

#define S_BRIGHT2(spr,frm,tic,cmd,next,m1,m2) \
	_S__FR_TIC_(spr, (frm) - 'A' | SF_FULLBRIGHT, tic, m1, m2, (void *)cmd, next)

/* <winbase.h> #defines its own, completely unrelated S_NORMAL.
 * Since winbase.h will only be included in Win32-specific files that
 * don't define any actors, we can safely avoid defining it here.
 */

#ifndef S_NORMAL
#define S_NORMAL(spr,frm,tic,cmd,next)	S_NORMAL2(spr,frm,tic,(void *)cmd,next,0,0)
#endif
#define S_BRIGHT(spr,frm,tic,cmd,next)	S_BRIGHT2(spr,frm,tic,(void *)cmd,next,0,0)


#ifndef EGAMETYPE
#define EGAMETYPE
enum EGameType
{
	GAME_Any	 = 0,
	GAME_Doom	 = 1,
	GAME_Heretic = 2,
	GAME_Hexen	 = 4,
	GAME_Raven	 = 6
};
#endif

enum
{
	ADEFTYPE_Byte		= 0,
	ADEFTYPE_FixedMul	= 64,		// one byte, multiplied by FRACUNIT
	ADEFTYPE_Word		= 128,
	ADEFTYPE_Long		= 192,
	ADEFTYPE_MASK		= 192,

	// These first properties are always strings
	ADEF_SeeSound = 1,
	ADEF_AttackSound,
	ADEF_PainSound,
	ADEF_DeathSound,
	ADEF_ActiveSound,
	ADEF_LastString = ADEF_ActiveSound,

	// The rest of the properties use their type field (upper 2 bits)
	ADEF_XScale,
	ADEF_YScale,
	ADEF_SpawnHealth,
	ADEF_ReactionTime,
	ADEF_PainChance,
	ADEF_Speed,
	ADEF_Radius,
	ADEF_Height,
	ADEF_Mass,
	ADEF_Damage,
	ADEF_Flags,			// Use these flags exactly
	ADEF_Flags2,		// "
	ADEF_Flags3,		// "
	ADEF_FlagsSet,		// Or these flags with previous
	ADEF_Flags2Set,		// "
	ADEF_Flags3Set,		// "
	ADEF_FlagsClear,	// Clear these flags from previous
	ADEF_Flags2Clear,	// "
	ADEF_Flags3Clear,	// "
	ADEF_Alpha,
	ADEF_RenderStyle,
	ADEF_RenderFlags,
	ADEF_Translation,

	ADEF_SpawnState,
	ADEF_SeeState,
	ADEF_PainState,
	ADEF_MeleeState,
	ADEF_MissileState,
	ADEF_CrashState,
	ADEF_DeathState,
	ADEF_XDeathState,
	ADEF_BDeathState,
	ADEF_IDeathState,
	ADEF_RaiseState,

	// The following are not properties but effect how the list is parsed
	ADEF_FirstCommand,
	ADEF_LimitGame = ADEF_FirstCommand,
	ADEF_SkipSuper,		// Take defaults from AActor instead of superclass(es)
	ADEF_StateBase,		// Use states not owned by this actor

	ADEF_EOL = 0		// End Of List
};

#if _MSC_VER
#pragma warning(disable:4200)	// nonstandard extension used : zero-sized array in struct/union
#endif

struct FActorInfo
{
	static void StaticInit ();
	static void StaticGameSet ();
	static void StaticSetActorNums ();
	static void StaticSpeedSet ();

	void BuildDefaults ();
	void ApplyDefaults (BYTE *defaults);

	TypeInfo *Class;
	FState *OwnedStates;
	BYTE *Defaults;
	int NumOwnedStates;
	BYTE GameFilter;
	BYTE SpawnID;
	SWORD DoomEdNum;

	// Followed by a 0-terminated list of default properties
	BYTE DefaultList[];
};

#if _MSC_VER
#pragma warning(default:4200)
#endif

class FDoomEdMap
{
public:
	const TypeInfo *FindType (int doomednum) const;
	void AddType (int doomednum, const TypeInfo *type);
	void DelType (int doomednum);
	void Empty ();

	static void DumpMapThings ();

private:
	enum { DOOMED_HASHSIZE = 256 };

	struct FDoomEdEntry
	{
		FDoomEdEntry *HashNext;
		const TypeInfo *Type;
		int DoomEdNum;
	};

	static FDoomEdEntry *DoomEdHash[DOOMED_HASHSIZE];
};

extern FDoomEdMap DoomEdMap;

#include "infomacros.h"

#endif	// __INFO_H__
