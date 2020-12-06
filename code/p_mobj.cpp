// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//		Moving object handling. Spawn functions.
//
//-----------------------------------------------------------------------------

// HEADER FILES ------------------------------------------------------------

#include "m_alloc.h"
#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "doomdef.h"
#include "p_local.h"
#include "p_lnspec.h"
#include "p_effect.h"
#include "p_terrain.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "s_sound.h"
#include "doomstat.h"
#include "v_video.h"
#include "c_cvars.h"
#include "b_bot.h"	//Added by MC:
#include "stats.h"
#include "a_doomglobal.h"
#include "a_sharedglobal.h"
#include "gi.h"
#include "sbar.h"

// MACROS ------------------------------------------------------------------

#define WATER_SINK_FACTOR		3
#define WATER_SINK_SMALL_FACTOR	4
#define WATER_SINK_SPEED		(FRACUNIT/2)
#define WATER_JUMP_SPEED		(FRACUNIT*7/2)

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void G_PlayerReborn (int player);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern cycle_t BotSupportCycles;
extern fixed_t attackrange;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

BEGIN_CUSTOM_CVAR (sv_gravity, "800", CVAR_SERVERINFO)
{
	level.gravity = var.value;
}
END_CUSTOM_CVAR (sv_gravity)

CVAR (sv_friction, "0.90625", CVAR_SERVERINFO)

fixed_t FloatBobOffsets[64] =
{
	0, 51389, 102283, 152192,
	200636, 247147, 291278, 332604,
	370727, 405280, 435929, 462380,
	484378, 501712, 514213, 521763,
	524287, 521763, 514213, 501712,
	484378, 462380, 435929, 405280,
	370727, 332604, 291278, 247147,
	200636, 152192, 102283, 51389,
	-1, -51390, -102284, -152193,
	-200637, -247148, -291279, -332605,
	-370728, -405281, -435930, -462381,
	-484380, -501713, -514215, -521764,
	-524288, -521764, -514214, -501713,
	-484379, -462381, -435930, -405280,
	-370728, -332605, -291279, -247148,
	-200637, -152193, -102284, -51389
};

CVAR (cl_pufftype, "0", CVAR_ARCHIVE)
CVAR (cl_bloodtype, "0", CVAR_ARCHIVE)

const TypeInfo *PuffType = RUNTIME_CLASS(ABulletPuff);
const TypeInfo *HitPuffType = NULL;
AActor *MissileActor;

// CODE --------------------------------------------------------------------

IMPLEMENT_POINTY_SERIAL (AActor, DThinker)
 DECLARE_POINTER (target)
 DECLARE_POINTER (lastenemy)
 DECLARE_POINTER (tracer)
 DECLARE_POINTER (goal)
END_POINTERS

AActor::~AActor ()
{
	// Please avoid calling the destructor directly (or through delete)!
	// Use Destroy() instead.
}

void AActor::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << x
		<< y
		<< z
		<< pitch
		<< angle
		<< roll
		<< sprite
		<< frame
		<< effects
		<< floorz
		<< ceilingz
		<< floorpic
		<< radius
		<< height
		<< momx
		<< momy
		<< momz
		<< tics
		<< state
		<< damage
		<< flags
		<< flags2
		<< flags3
		<< mapflags
		<< special1
		<< special2
		<< health
		<< movedir
		<< visdir
		<< movecount
		<< target
		<< lastenemy
		<< reactiontime
		<< threshold
		<< player
		<< lastlook
		<< tracer
		<< floorclip
		<< tid
		<< special
		<< args[0] << args[1] << args[2] << args[3] << args[4]
		<< goal
		<< translucency
		<< waterlevel;
	arc.SerializePointer (translationtables, (BYTE **)&translation, 256);
	spawnpoint.Serialize (arc);

	if (arc.IsLoading ())
	{
		touching_sectorlist = NULL;
		LinkToWorld ();
		AddToHash ();
	}
}

void MapThing::Serialize (FArchive &arc)
{
	arc << thingid << x << y << z << angle << type << flags << special
		<< args[0] << args[1] << args[2] << args[3] << args[4];
}

AActor::AActor ()
{
	memset (&x, 0, (byte *)&this[1] - (byte *)&x);
}

AActor::AActor (const AActor &other)
{
	memcpy (&x, &other.x, (byte *)&this[1] - (byte *)&x);
}

AActor &AActor::operator= (const AActor &other)
{
	memcpy (&x, &other.x, (byte *)&this[1] - (byte *)&x);
	return *this;
}

//==========================================================================
//
// AActor::SetState
//
// Returns true if the mobj is still present.
//
//==========================================================================

bool AActor::SetState (FState *newstate)
{
	do
	{
		if (newstate == NULL)
		{
			state = NULL;
			Destroy ();
			return false;
		}

		state = newstate;
		tics = newstate->tics;
		frame = newstate->frame;

		// [RH] Only change sprite if not using a skin
		if (!player || (player->cheats & CF_NOSKIN))
			sprite = newstate->sprite.index;

		if (newstate->action.acp1)
			newstate->action.acp1 (this);

		newstate = newstate->nextstate;
	} while (tics == 0);

	return true;
}

//----------------------------------------------------------------------------
//
// FUNC AActor::SetStateNF
//
// Same as SetState, but does not call the state function.
//
//----------------------------------------------------------------------------

bool AActor::SetStateNF (FState *newstate)
{
	do
	{
		if (newstate == NULL)
		{
			state = NULL;
			Destroy ();
			return false;
		}
		state = newstate;
		tics = newstate->tics;
		frame = newstate->frame;
		// [RH] Only change sprite if not using a skin
		if (!player || (player->cheats & CF_NOSKIN))
			sprite = newstate->sprite.index;
		newstate = newstate->nextstate;
	} while (tics == 0);

	return true;
}

//----------------------------------------------------------------------------
//
// PROC P_ExplodeMissile
//
//----------------------------------------------------------------------------

void P_ExplodeMissile (AActor *mo)
{
	if (mo->flags3 & MF3_EXPLOCOUNT)
	{
		if (++mo->special2 < mo->special1)
		{
			return;
		}
	}
	FActorInfo *info = GetInfo (mo);

	mo->momx = mo->momy = mo->momz = 0;
	mo->effects = 0;		// [RH]

	mo->SetState (info->deathstate);
	if (info->deathstate != NULL)
	{
		// [RH] Change translucency of exploding rockets
		if (mo->IsKindOf (RUNTIME_CLASS(ARocket)))
			mo->translucency = TRANSLUC66;

		if (gameinfo.gametype == GAME_Doom)
		{
			mo->tics -= P_Random (pr_explodemissile) & 3;
			if (mo->tics < 1)
				mo->tics = 1;
		}

		mo->flags &= ~MF_MISSILE;

		if (info->deathsound)
			S_Sound (mo, CHAN_VOICE, info->deathsound, 1, ATTN_NORM);
	}
}

//----------------------------------------------------------------------------
//
// PROC P_FloorBounceMissile
//
//----------------------------------------------------------------------------

void P_FloorBounceMissile (AActor *mo)
{
	if (gameinfo.gametype == GAME_Heretic)
	{
		mo->momz = -mo->momz;
		mo->SetState (GetInfo (mo)->deathstate);
		return;
	}
/*
	if(P_HitFloor(mo))
	{
		switch(mo->type)
		{
			case MT_SORCFX1:
			case MT_SORCBALL1:
			case MT_SORCBALL2:
			case MT_SORCBALL3:
				break;
			default:
				mo->Destroy ();
				return;
		}
	}
*/
/*
	switch(mo->type)
	{
	case MT_SORCFX1:
		mo->momz = -mo->momz;		// no energy absorbed
		break;
	case MT_SGSHARD1:
	case MT_SGSHARD2:
	case MT_SGSHARD3:
	case MT_SGSHARD4:
	case MT_SGSHARD5:
	case MT_SGSHARD6:
	case MT_SGSHARD7:
	case MT_SGSHARD8:
	case MT_SGSHARD9:
	case MT_SGSHARD0:
		mo->momz = FixedMul(mo->momz, (fixed_t)(-0.3*FRACUNIT));
		if(abs(mo->momz) < (FRACUNIT/2))
		{
			mo->SetState (NULL);
			return;
		}
		break;
	default:
*/
		mo->momz = FixedMul(mo->momz, (fixed_t)(-0.7*FRACUNIT));
//		break;
//	}
	mo->momx = 2*mo->momx/3;
	mo->momy = 2*mo->momy/3;
	if (GetInfo (mo)->seesound)
	{
		S_Sound (mo, CHAN_VOICE, GetInfo (mo)->seesound, 1, ATTN_IDLE);
	}
	if (!(mo->flags & MF_NOGRAVITY) && (mo->momz < 3*FRACUNIT))
		mo->flags2 &= ~MF2_FLOORBOUNCE;
//	mo->SetState (GetInfo (mo)->deathstate);
}

//----------------------------------------------------------------------------
//
// PROC P_ThrustMobj
//
//----------------------------------------------------------------------------

void P_ThrustMobj (AActor *mo, angle_t angle, fixed_t move)
{
	angle >>= ANGLETOFINESHIFT;
	mo->momx += FixedMul (move, finecosine[angle]);
	mo->momy += FixedMul (move, finesine[angle]);
}

//----------------------------------------------------------------------------
//
// FUNC P_FaceMobj
//
// Returns 1 if 'source' needs to turn clockwise, or 0 if 'source' needs
// to turn counter clockwise.  'delta' is set to the amount 'source'
// needs to turn.
//
//----------------------------------------------------------------------------

int P_FaceMobj (AActor *source, AActor *target, angle_t *delta)
{
	angle_t diff;
	angle_t angle1;
	angle_t angle2;

	angle1 = source->angle;
	angle2 = R_PointToAngle2 (source->x, source->y, target->x, target->y);
	if (angle2 > angle1)
	{
		diff = angle2 - angle1;
		if (diff > ANGLE_180)
		{
			*delta = ANGLE_MAX - diff;
			return 0;
		}
		else
		{
			*delta = diff;
			return 1;
		}
	}
	else
	{
		diff = angle1 - angle2;
		if (diff > ANGLE_180)
		{
			*delta = ANGLE_MAX - diff;
			return 1;
		}
		else
		{
			*delta = diff;
			return 0;
		}
	}
}

//----------------------------------------------------------------------------
//
// FUNC P_SeekerMissile
//
// The missile's tracer field must be the target.  Returns true if
// target was tracked, false if not.
//
//----------------------------------------------------------------------------

bool P_SeekerMissile (AActor *actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	AActor *target;

	target = actor->tracer;
	if (target == NULL)
	{
		return false;
	}
	if (!(target->flags & MF_SHOOTABLE))
	{ // Target died
		actor->tracer = NULL;
		return false;
	}
	dir = P_FaceMobj (actor, target, &delta);
	if (delta > thresh)
	{
		delta >>= 1;
		if (delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if (dir)
	{ // Turn clockwise
		actor->angle += delta;
	}
	else
	{ // Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul (GetInfo (actor)->speed, finecosine[angle]);
	actor->momy = FixedMul (GetInfo (actor)->speed, finesine[angle]);
	if (actor->z + actor->height < target->z ||
		target->z + target->height < actor->z)
	{ // Need to seek vertically
		dist = P_AproxDistance (target->x - actor->x, target->y - actor->y);
		dist = dist / GetInfo (actor)->speed;
		if (dist < 1)
		{
			dist = 1;
		}
		actor->momz = (target->z - actor->z) / dist;
	}
	return true;
}

//
// P_XYMovement  
//
#define STOPSPEED			0x1000
#define FRICTION			0xe800

void P_XYMovement (AActor *mo) 
{
	angle_t angle;
	fixed_t ptryx, ptryy;
	player_t *player;
	fixed_t xmove, ymove;
	FActorInfo *info = GetInfo (mo);
	static const int windTab[3] = {2048*5, 2048*10, 2048*25};

	if (!mo->momx && !mo->momy)
	{
		if (mo->flags & MF_SKULLFLY)
		{
			// the skull slammed into something
			mo->flags &= ~MF_SKULLFLY;
			mo->momx = mo->momy = mo->momz = 0;

			mo->SetState (info->seestate);
		}
		return;
	}

	if (mo->flags2 & MF2_WINDTHRUST)
	{
		int special = mo->subsector->sector->special;
		switch (special)
		{
			case 40: case 41: case 42: // Wind_East
				P_ThrustMobj (mo, 0, windTab[special-40]);
				break;
			case 43: case 44: case 45: // Wind_North
				P_ThrustMobj (mo, ANG90, windTab[special-43]);
				break;
			case 46: case 47: case 48: // Wind_South
				P_ThrustMobj (mo, ANG270, windTab[special-46]);
				break;
			case 49: case 50: case 51: // Wind_West
				P_ThrustMobj (mo, ANG180, windTab[special-49]);
				break;
		}
	}

	player = mo->player;

	int maxmove = (mo->waterlevel < 2) || (mo->flags & MF_MISSILE) ? MAXMOVE : MAXMOVE/4;

	if (mo->momx > maxmove)
		mo->momx = maxmove;
	else if (mo->momx < -maxmove)
		mo->momx = -maxmove;

	if (mo->momy > maxmove)
		mo->momy = maxmove;
	else if (mo->momy < -maxmove)
		mo->momy = -maxmove;
	
	xmove = mo->momx;
	ymove = mo->momy;

	maxmove /= 2;

	do
	{
		if (xmove > maxmove || ymove > maxmove ||
			xmove < -maxmove || ymove < -maxmove)
		{
			ptryx = mo->x + xmove/2;
			ptryy = mo->y + ymove/2;
			xmove >>= 1;
			ymove >>= 1;
		}
		else
		{
			ptryx = mo->x + xmove;
			ptryy = mo->y + ymove;
			xmove = ymove = 0;
		}

		// killough 3/15/98: Allow objects to drop off
		if (!P_TryMove (mo, ptryx, ptryy, true))
		{
			// blocked move
			if (mo->flags2 & MF2_SLIDE)
			{
				// try to slide along it
				if (BlockingMobj == NULL)
				{ // slide against wall
					if (mo->player && mo->waterlevel && mo->waterlevel < 3
						&& (mo->player->cmd.ucmd.forwardmove | mo->player->cmd.ucmd.sidemove))
					{
						mo->momz = WATER_JUMP_SPEED;
					}
					P_SlideMove (mo);
				}
				else
				{ // slide against mobj
					if (P_TryMove (mo, mo->x, ptryy, true))
					{
						mo->momx = 0;
					}
					else if (P_TryMove (mo, ptryx, mo->y, true))
					{
						mo->momy = 0;
					}
					else
					{
						mo->momx = mo->momy = 0;
					}
					if (player && player->mo == mo)
					{
						if (mo->momx == 0)
							player->momx = 0;
						if (mo->momy == 0)
							player->momy = 0;
					}
				}
			}
			else if (mo->flags & MF_MISSILE)
			{
				if (gameinfo.gametype != GAME_Heretic)
				{
					if (mo->flags2 & MF2_FLOORBOUNCE)
					{
						if (BlockingMobj)
						{
							if ((BlockingMobj->flags2 & MF2_REFLECTIVE) ||
								((!BlockingMobj->player) &&
								(!(BlockingMobj->flags & MF_COUNTKILL))))
							{
								fixed_t speed;

								angle = R_PointToAngle2 (BlockingMobj->x,
									BlockingMobj->y, mo->x, mo->y)
									+ANGLE_1*((P_Random(pr_bounce)%16)-8);
								speed = P_AproxDistance (mo->momx, mo->momy);
								speed = FixedMul (speed, (fixed_t)(0.75*FRACUNIT));
								mo->angle = angle;
								angle >>= ANGLETOFINESHIFT;
								mo->momx = FixedMul (speed, finecosine[angle]);
								mo->momy = FixedMul (speed, finesine[angle]);
								if (info->seesound)
								{
									S_Sound (mo, CHAN_VOICE, info->seesound, 1, ATTN_IDLE);
								}
								return;
							}
							else
							{
								// Struck a player/creature
								P_ExplodeMissile (mo);
							}
						}
						else
						{
							// Struck a wall
							P_BounceWall (mo);
							if (info->seesound)
							{
								S_Sound (mo, CHAN_VOICE, info->seesound, 1, ATTN_IDLE);
							}
							return;
						}
					}
				}
				if(BlockingMobj &&
					(BlockingMobj->flags2 & MF2_REFLECTIVE))
				{
					angle = R_PointToAngle2(BlockingMobj->x,
													BlockingMobj->y,
													mo->x, mo->y);

					// Change angle for delflection/reflection
					angle += ANGLE_1 * ((P_Random(pr_bounce)%16)-8);

					// Reflect the missile along angle
					mo->angle = angle;
					angle >>= ANGLETOFINESHIFT;
					mo->momx = FixedMul (info->speed>>1, finecosine[angle]);
					mo->momy = FixedMul (info->speed>>1, finesine[angle]);
//					mo->momz = -mo->momz;
					if (mo->flags2 & MF2_SEEKERMISSILE)
					{
						mo->tracer = mo->target;
					}
					mo->target = BlockingMobj;
					return;
				}
				// explode a missile
				if (ceilingline &&
					ceilingline->backsector &&
					ceilingline->backsector->ceilingpic == skyflatnum &&
					mo->z >= ceilingline->backsector->ceilingheight) //killough
				{
					// Hack to prevent missiles exploding against the sky.
					// Does not handle sky floors.
					mo->Destroy ();
					return;
				}
				P_ExplodeMissile (mo);
			}
			else
			{
				mo->momx = mo->momy = 0;
			}
		}
	} while (xmove | ymove);
	
	// Friction

	if (player && player->mo == mo && player->cheats & CF_NOMOMENTUM)
	{ // debug option for no sliding at all
		mo->momx = mo->momy = 0;
		player->momx = player->momy = 0;
		return;
	}

	if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
	{ // no friction for missiles
		return;
	}

	if (mo->z > mo->floorz && !(mo->flags2 & MF2_ONMOBJ) &&
		!(mo->flags2 & MF2_FLY)	&& !mo->waterlevel)
	{ // no friction when falling
		return;
	}

	if (mo->flags & MF_CORPSE)
	{ // Don't stop sliding if halfway off a step with some momentum
		if (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4
			|| mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4)
		{
			if (mo->floorz != mo->subsector->sector->floorheight)
				return;
		}
	}

	// killough 11/98:
	// Stop voodoo dolls that have come to rest, despite any
	// moving corresponding player:
	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
		&& mo->momy > -STOPSPEED && mo->momy < STOPSPEED
		&& (!player || (player->mo != mo)
			|| !(player->cmd.ucmd.forwardmove | player->cmd.ucmd.sidemove)))
	{
		// if in a walking frame, stop moving
		// killough 10/98:
		// Don't affect main player when voodoo dolls stop:
		if (player && player->mo == mo)
		{
			player->mo->PlayIdle ();
		}

		mo->momx = mo->momy = 0;

		// killough 10/98: kill any bobbing momentum too (except in voodoo dolls)
		if (player && player->mo == mo)
			player->momx = player->momy = 0; 
	}
	else
	{
		// phares 3/17/98
		// Friction will have been adjusted by friction thinkers for icy
		// or muddy floors. Otherwise it was never touched and
		// remained set at ORIG_FRICTION
		//
		// killough 8/28/98: removed inefficient thinker algorithm,
		// instead using touching_sectorlist in P_GetFriction() to
		// determine friction (and thus only when it is needed).
		//
		// killough 10/98: changed to work with new bobbing method.
		// Reducing player momentum is no longer needed to reduce
		// bobbing, so ice works much better now.

		fixed_t friction = P_GetFriction (mo, NULL);

		mo->momx = FixedMul (mo->momx, friction);
		mo->momy = FixedMul (mo->momy, friction);

		// killough 10/98: Always decrease player bobbing by ORIG_FRICTION.
		// This prevents problems with bobbing on ice, where it was not being
		// reduced fast enough, leading to all sorts of kludges being developed.

		if (player && player->mo == mo)		//  Not voodoo dolls
		{
			player->momx = FixedMul (player->momx, ORIG_FRICTION);
			player->momy = FixedMul (player->momy, ORIG_FRICTION);
		}
	}
}

// Move this to p_inter ***
void P_MonsterFallingDamage (AActor *mo)
{
	int damage;
	int mom;

	if (gameinfo.gametype != GAME_Hexen)
		return;

	mom = abs(mo->momz);
	if (mom > 35*FRACUNIT)
	{ // automatic death
		damage=10000;
	}
	else
	{
		damage = ((mom - (23*FRACUNIT))*6)>>FRACBITS;
	}
	damage = 10000;	// always kill 'em
	P_DamageMobj (mo, NULL, NULL, damage);
}

//
// P_ZMovement
//
void P_ZMovement (AActor *mo)
{
	fixed_t dist;
	fixed_t delta;
	FActorInfo *info = GetInfo (mo);

//	
// check for smooth step up
//
	if (mo->player && mo->z < mo->floorz)
	{
		mo->player->viewheight -= mo->floorz - mo->z;
		mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight)>>3;
	}
//
// apply gravity
//
	if (mo->z > mo->floorz && !(mo->flags & MF_NOGRAVITY))
	{
		fixed_t startmomz = mo->momz;

		if (!mo->waterlevel || (mo->player &&
			!(mo->player->cmd.ucmd.forwardmove | mo->player->cmd.ucmd.sidemove)))
		{
			mo->momz -= (fixed_t)(level.gravity * mo->subsector->sector->gravity *
				(mo->flags2 & MF2_LOGRAV ? 10.24 : 81.92));
		}
		if (mo->waterlevel > 1)
		{
			if (mo->momz < -WATER_SINK_SPEED)
			{
				mo->momz = (startmomz < -WATER_SINK_SPEED) ? startmomz : -WATER_SINK_SPEED;
			}
			else
			{
				mo->momz = startmomz + ((mo->momz - startmomz) >>
					(mo->waterlevel == 1 ? WATER_SINK_SMALL_FACTOR : WATER_SINK_FACTOR));
			}
		}
	}
//
// adjust height
//
	mo->z += mo->momz;
	if ((mo->flags & MF_FLOAT) && mo->target)
	{	// float down towards target if too close
		if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
		{
			dist = P_AproxDistance (mo->x - mo->target->x, mo->y - mo->target->y);
			delta = (mo->target->z + (mo->height>>1)) - mo->z;
			if (delta < 0 && dist < -(delta*3))
				mo->z -= FLOATSPEED;
			else if (delta > 0 && dist < (delta*3))
				mo->z += FLOATSPEED;
		}
	}
	if (mo->player && (mo->flags2 & MF2_FLY) && (mo->z > mo->floorz))
	{
		mo->z += finesine[(FINEANGLES/80*level.time)&FINEMASK]/8;
		mo->momz = FixedMul (mo->momz, FRICTION_FLY);
	}
	if (mo->waterlevel && !(mo->flags & MF_NOGRAVITY))
	{
		mo->momz = FixedMul (mo->momz, mo->subsector->sector->friction);
	}

//
// clip movement
//
	if (mo->z <= mo->floorz)
	{	// Hit the floor
		if ((mo->flags & MF_MISSILE) &&
			(gameinfo.gametype != GAME_Doom || !(mo->flags & MF_NOCLIP)))
		{
			mo->z = mo->floorz;
			if (mo->flags2 & MF2_FLOORBOUNCE)
			{
				P_FloorBounceMissile (mo);
				return;
			}
			else if (mo->flags3 & MF3_FLOORHUGGER)
			{ // Floor huggers can go up steps
				return;
			}
			else
			{
				if (mo->subsector->sector->floorpic == skyflatnum)
				{
					// [RH] Just remove the missile without exploding it
					//		if this is a sky floor.
					mo->Destroy ();
					return;
				}
				P_HitFloor (mo);
				P_ExplodeMissile (mo);
				return;
			}
		}
		if (mo->flags & MF_COUNTKILL)		// Blasted mobj falling
		{
			if (mo->momz < -(23*FRACUNIT))
			{
				P_MonsterFallingDamage (mo);
			}
		}
		mo->z = mo->floorz;
		if (mo->momz < 0)
		{
			// [RH] avoid integer roundoff by doing comparisons with floats
			float minmom = level.gravity * mo->subsector->sector->gravity * -655.36f;
			float mom = (float)mo->momz;

			// Spawn splashes, etc.
			P_HitFloor (mo);
			if (mo->flags2 & MF2_ICEDAMAGE && mom < minmom)
			{
				mo->tics = 1;
				mo->momx = 0;
				mo->momy = 0;
				mo->momz = 0;
				return;
			}
			// Let the actor do something special for hitting the floor
			mo->HitFloor ();
			if (mo->player)
			{
				mo->player->jumpTics = 7;	// delay any jumping for a short while
				if (mom < minmom && !(mo->flags2 & MF2_FLY))
				{
					// Squat down.
					// Decrease viewheight for a moment after hitting the ground (hard),
					// and utter appropriate sound.
					mo->player->deltaviewheight = mo->momz >> 3;
					if (!mo->player->morphTics)
					{
						S_Sound (mo, CHAN_AUTO, "*land1", 1, ATTN_NORM);
					}
				}
			}
			mo->momz = 0;
		}
		if (mo->flags & MF_SKULLFLY)
		{ // The skull slammed into something
			mo->momz = -mo->momz;
		}
		if (info->crashstate &&
			(mo->flags & MF_CORPSE) &&
			!(mo->flags2 & MF2_ICEDAMAGE))
		{
			mo->SetState (info->crashstate);
		}
	}

	if (mo->flags2 & MF2_FLOORCLIP)
	{
		mo->AdjustFloorClip ();
	}

	if (mo->z + mo->height > mo->ceilingz)
	{
		// hit the ceiling
		mo->z = mo->ceilingz - mo->height;
		if (mo->flags2 & MF2_FLOORBOUNCE)
		{	// reverse momentum here for ceiling bounce
			mo->momz = FixedMul (mo->momz, (fixed_t)(-0.75*FRACUNIT));
			if (info->seesound)
			{
				S_Sound (mo, CHAN_BODY, info->seesound, 1, ATTN_IDLE);
			}
			return;
		}
		if (mo->momz > 0)
			mo->momz = 0;
		if (mo->flags & MF_SKULLFLY)
		{	// the skull slammed into something
			mo->momz = -mo->momz;
		}
		if (mo->flags & MF_MISSILE &&
			(gameinfo.gametype != GAME_Doom || !(mo->flags & MF_NOCLIP)))
		{
			if (!(mo->flags3 & MF3_SKYEXPLODE) &&
				mo->subsector->sector->ceilingpic == skyflatnum)
			{
				mo->Destroy ();
				return;
			}
			P_ExplodeMissile (mo);
			return;
		}
	}
}

//===========================================================================
//
// PlayerLandedOnThing
//
//===========================================================================

static void PlayerLandedOnThing (AActor *mo, AActor *onmobj)
{
	mo->player->deltaviewheight = mo->momz>>3;
	if (!mo->player->morphTics)
	{
		S_Sound (mo, CHAN_AUTO, "*land1", 1, ATTN_IDLE);
	}
//	mo->player->centering = true;
}



//
// P_NightmareRespawn
//
void P_NightmareRespawn (AActor *mobj)
{
	fixed_t x, y, z;
	subsector_t *ss;
	AActor *mo;
	mapthing2_t *mthing;
	FActorInfo *info = GetInfo (mobj);

	x = mobj->spawnpoint.x << FRACBITS;
	y = mobj->spawnpoint.y << FRACBITS;
	// something is occupying it's position?
	if (!P_CheckPosition (mobj, x, y)) 
		return;		// no respawn

	// spawn a teleport fog at old spot because of removal of the body?
	Spawn ("TeleportFog", mobj->x, mobj->y, ONFLOORZ);

	ss = R_PointInSubsector (x,y);

	// spawn a teleport fog at the new spot
	Spawn ("TeleportFog", x, y, ONFLOORZ);

	// spawn the new monster
	mthing = &mobj->spawnpoint;

	if (info->flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (info->flags2 & MF2_SPAWNFLOAT)
		z = FLOATRANDZ;
	else if (info->flags2 & MF2_FLOATBOB)
		z = mthing->z << FRACBITS;
	else
		z = ONFLOORZ;

	// spawn it
	// inherit attributes from deceased one
	mo = Spawn (RUNTIME_TYPE(mobj), x, y, ONFLOORZ);
	mo->spawnpoint = mobj->spawnpoint;
	mo->angle = ANG45 * (mthing->angle/45);

	if (z == ONFLOORZ)
		mo->z += mthing->z << FRACBITS;
	else if (z == ONCEILINGZ)
		mo->z -= mthing->z << FRACBITS;

	if (mo->flags2 & MF2_FLOATBOB)
	{ // Seed random starting index for bobbing motion
		mo->health = P_Random(pr_bobbing);
		mo->special1 = mthing->z << FRACBITS;
	}

	if (mthing->flags & MTF_AMBUSH)
		mo->flags |= MF_AMBUSH;

	mo->reactiontime = 18;

	// remove the old monster,
	mobj->Destroy ();
}


//
// [RH] Some new functions to work with Thing IDs. ------->
//
AActor *AActor::TIDHash[128];

//
// P_ClearTidHashes
//
// Clears the tid hashtable.
//

void AActor::ClearTIDHashes ()
{
	int i;

	for (i = 0; i < 128; i++)
		TIDHash[i] = NULL;
}

//
// P_AddMobjToHash
//
// Inserts an mobj into the correct chain based on its tid.
// If its tid is 0, this function does nothing.
//
void AActor::AddToHash ()
{
	if (tid == 0)
	{
		iprev = NULL;
		inext = NULL;
		return;
	}
	else
	{
		int hash = TIDHASH (tid);

		inext = TIDHash[hash];
		iprev = &TIDHash[hash];
		TIDHash[hash] = this;
		if (inext)
		{
			inext->iprev = &inext;
		}
	}
}

//
// P_RemoveMobjFromHash
//
// Removes an mobj from its hash chain.
//
void AActor::RemoveFromHash ()
{
	if (tid != 0 && iprev)
	{
		*iprev = inext;
		if (inext)
		{
			inext->iprev = iprev;
		}
		iprev = NULL;
		inext = NULL;
	}
	tid = 0;
}

// <------- [RH] End new functions


//
// P_MobjThinker
//
void AActor::RunThink ()
{
	AActor *onmo;
	int i;

	flags3 &= ~MF3_CARRIED;

	if (flags & MF_UNMORPHED)
	{
		return;
	}

	//Added by MC: Freeze mode.
	if (bglobal.freeze && !(player && !player->isbot))
	{
		return;
	}

	// [RH] Pulse in and out of visibility
	if (effects & FX_VISIBILITYPULSE)
	{
		if (special2 > 0)
		{
			translucency += 0x800;
			if (translucency >= OPAQUE)
			{
				translucency = OPAQUE;
				special2 = -1;
			}
		}
		else
		{
			translucency -= 0x800;
			if (translucency <= TRANSLUC25)
			{
				translucency = TRANSLUC25;
				special2 = 1;
			}
		}
	}

	// [RH] Fade a stealth monster in and out of visibility
	if (visdir > 0)
	{
		translucency += 2*FRACUNIT/TICRATE;
		if (translucency > OPAQUE)
		{
			translucency = OPAQUE;
			visdir = 0;
		}
	}
	else if (visdir < 0)
	{
		translucency -= 3*OPAQUE/TICRATE/2;
		if (translucency < 0)
		{
			translucency = 0;
			visdir = 0;
		}
	}

	if (!bglobal.itemsdone) //Added by MC: Initialize thing list
	{
		if (bglobal.thingnum >= MAXTHINGS)
		{
			bglobal.itemsdone = true;
			Printf (PRINT_HIGH, "Warning: Number of items known to the bot limited to %d.\n", MAXTHINGS);
		}
		else if (flags & MF_SPECIAL)
		{
			if (this == bglobal.firstthing)
				bglobal.itemsdone = true;
			bglobal.things[bglobal.thingnum] = this;
			bglobal.thingnum++;
			if (!bglobal.firstthing)
				bglobal.firstthing = this;
		}
	}

	if (bglobal.botnum && consoleplayer == Net_Arbitrator && !demoplayback &&
		flags & (MF_COUNTKILL|MF_SPECIAL|MF_MISSILE))
	{
		clock (BotSupportCycles);
		bglobal.m_Thinking = true;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || !players[i].isbot)
				continue;

			if (flags & MF_COUNTKILL)
			{
				if (health > 0
					&& !players[i].enemy
					&& player ? !IsTeammate (players[i].mo) : true
					&& P_AproxDistance (players[i].mo->x-x, players[i].mo->y-y) < MAX_MONSTER_TARGET_DIST
					&& P_CheckSight (players[i].mo, this))
				{ //Probably a monster, so go kill it.
					players[i].enemy = this;
				}
			}
			else if (flags & MF_SPECIAL)
			{ //Item pickup time
				bglobal.WhatToGet (players[i].mo, this);
			}
			else if (flags & MF_MISSILE)
			{
				if (!players[i].missile && IsKindOf (RUNTIME_CLASS(APlasmaBall)))
				{ //warn for incoming missiles.
					if (target != players[i].mo && bglobal.Check_LOS (players[i].mo, this, ANGLE_90))
						players[i].missile = this;
				}
			}
		}
		bglobal.m_Thinking = false;
		unclock (BotSupportCycles);
	}

	//End of MC

	// Handle X and Y momemtums
	BlockingMobj = NULL;
	if ((momx | momy) || (flags & MF_SKULLFLY))
	{
		P_XYMovement (this);

		if (ObjectFlags & OF_MassDestruction)
			return;		// actor was destroyed
	}
	else if (flags2 & MF2_BLASTED)
	{ // Reset to not blasted when momentums are gone
		flags2 &= ~MF2_BLASTED;
		if (!(flags & MF_ICECORPSE))
		{
			flags2 &= ~MF2_SLIDE;
		}
	}
	if (flags2 & MF2_FLOATBOB)
	{ // Floating item bobbing motion (special1 is height)
		z = floorz + special1 + FloatBobOffsets[(health++)&63];
	}
	else if ((z != floorz) || momz || BlockingMobj)
	{	// Handle Z momentum and gravity
		if (flags2 & MF2_PASSMOBJ)
		{
			if (!(onmo = P_CheckOnmobj (this)))
			{
				P_ZMovement (this);
				if (flags2 & MF2_ONMOBJ)
				{
					flags2 &= ~MF2_ONMOBJ;
				}
			}
			else
			{
				if (player)
				{
					if (momz < (fixed_t)(level.gravity * subsector->sector->gravity * -655.36f)
						&& !(flags2&MF2_FLY))
					{
						PlayerLandedOnThing (this, onmo);
					}
				}
				if (onmo->z + onmo->height - z <= 24 * FRACUNIT)
				{
					if (player)
					{
						player->viewheight -= onmo->z + onmo->height - z;
						player->deltaviewheight =
							(VIEWHEIGHT - player->viewheight)>>3;
					}
					z = onmo->z + onmo->height;
				}
				flags2 |= MF2_ONMOBJ;
				momz = 0;
			}
		}
		else
		{
			P_ZMovement (this);
		}

		if (ObjectFlags & OF_MassDestruction)
			return;		// actor was destroyed
	}

	//byte lastwaterlevel = waterlevel;

	waterlevel = 0;

	if (subsector)
	{
		if (subsector->sector->waterzone)
			waterlevel = 3;
		sector_t *hsec;
		if ( (hsec = subsector->sector->heightsec) )
		{
			if (hsec->waterzone && !subsector->sector->waterzone)
			{
				if (z < hsec->floorheight)
				{
					waterlevel = 1;
					if (z + height/2 < hsec->floorheight)
					{
						waterlevel = 2;
						if (z + height <= hsec->floorheight)
							waterlevel = 3;
					}
				}
				else if (z + height > hsec->ceilingheight)
				{
					waterlevel = 3;
				}
			}
		}
	}

	if (flags2 & MF2_DORMANT)
		return;

	// cycle through states, calling action functions at transitions
	if (tics != -1)
	{
		tics--;
				
		// you can cycle through multiple states in a tic
		if (!tics)
			if (!SetState (state->nextstate))
				return; 		// freed itself
	}
	else
	{
		// check for nightmare respawn
		if (!(flags & MF_COUNTKILL) || !respawnmonsters)
			return;

		movecount++;

		if (movecount < 12*TICRATE)
			return;

		if (level.time & 31)
			return;

		if (P_Random (pr_mobjthinker) > 4)
			return;

		P_NightmareRespawn (this);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FreeTargMobj
//
//----------------------------------------------------------------------------

void A_FreeTargMobj (AActor *mo)
{
	mo->momx = mo->momy = mo->momz = 0;
	mo->z = mo->ceilingz + 4*FRACUNIT;
	mo->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY|MF_SOLID);
	mo->flags |= MF_CORPSE|MF_DROPOFF|MF_NOGRAVITY;
	mo->flags2 &= ~(MF2_PASSMOBJ|MF2_LOGRAV);
	mo->player = NULL;
}

FState AActor::States[] =
{
	S_NORMAL (TNT1, 'A', -1, NULL, NULL),
	S_NORMAL (TNT1, 'E', 1050, A_FreeTargMobj, NULL),
	S_NORMAL (TNT1, 'A', 1, NULL, NULL)		// S_NULL
};

void AActor::SetDefaults (FActorInfo *info)
{
	info->doomednum = -1;
	info->spawnid = 0;
	info->spawnstate = &States[2];
	info->spawnhealth = 1000;
	info->seestate = NULL;
	info->seesound = NULL;
	info->reactiontime = 8;
	info->attacksound = NULL;
	info->painstate = NULL;
	info->painchance = 0;
	info->painsound = NULL;
	info->meleestate = NULL;
	info->missilestate = NULL;
	info->crashstate = NULL;
	info->deathstate = NULL;
	info->xdeathstate = NULL;
	info->ideathstate = NULL;
	info->bdeathstate = NULL;
	info->deathsound = NULL;
	info->speed = 0;
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->mass = 100;
	info->damage = 0;
	info->activesound = NULL;
	info->flags = 0;
	info->flags2 = 0;
	info->flags3 = 0;
	info->raisestate = NULL;
	info->translucency = OPAQUE;
	info->OwnedStates = &States[0];
	info->NumOwnedStates = sizeof(States) / sizeof(States[0]);
}

//==========================================================================
//
// P_SpawnMobj
//
//==========================================================================

AActor *AActor::StaticSpawn (const TypeInfo *type, fixed_t ix, fixed_t iy, fixed_t iz)
{
	if (type == NULL)
	{
		I_Error ("Tried to spawn a bad actor\n");
	}

	if (type->ActorInfo == NULL)
	{
		I_Error ("%s is not an actor\n", type->Name);
	}

	AActor *actor;

	actor = static_cast<AActor *>(type->CreateNew ());

	memset (&(actor->x), 0, (byte *)&actor[1] - (byte *)&(actor->x));
	actor->x = ix;
	actor->y = iy;
	actor->z = iz;
	actor->ObjectFlags |= OF_JustSpawned;
	actor->BeginPlay ();
	return actor;
}

void AActor::LevelSpawned ()
{
	if (tics > 0)
		tics = 1 + (P_Random (pr_spawnmapthing) % tics);
	if (flags & MF_COUNTKILL)
		level.total_monsters++;
	if (flags & MF_COUNTITEM)
		level.total_items++;
	angle_t incs = AngleIncrements ();
	angle -= angle % incs;
	if (mapflags & MTF_AMBUSH)
		flags |= MF_AMBUSH;
	if (mapflags & MTF_DORMANT)
		Deactivate (NULL);
}

void AActor::BeginPlay ()
{
	pr_class_t rngclass = bglobal.m_Thinking ? pr_botspawnmobj : pr_spawnmobj;
	FActorInfo *info = RUNTIME_TYPE(this)->ActorInfo;

	radius = info->radius;
	height = info->height;
	flags = info->flags;
	flags2 = info->flags2;
	flags3 = info->flags3;
	damage = info->damage;
	health = info->spawnhealth;
	translucency = info->translucency;

	if (gameskill.value != sk_nightmare)
		reactiontime = info->reactiontime;
	
	lastlook = P_Random (rngclass) % MAXPLAYERS;

	// Set the state, but do not use SetState, because action
	// routines can't be called yet.  If the spawnstate has an action
	// routine, it will not be called.
	FState *st = info->spawnstate;
	state = st;
	tics = st->tics;
	sprite = st->sprite.index;
	frame = st->frame;
	touching_sectorlist = NULL;	// NULL head of sector list // phares 3/13/98

	// set subsector and/or block links
	LinkToWorld ();
	floorz = subsector->sector->floorheight;
	ceilingz = subsector->sector->ceilingheight;

	if (z == ONFLOORZ)
	{
		z = floorz;
	}
	else if (z == ONCEILINGZ)
	{
		z = ceilingz - height;
	}
	else if (z == FLOATRANDZ)
	{
		fixed_t space = ceilingz - info->height - floorz;
		if (space > 48*FRACUNIT)
		{
			space -= 40*FRACUNIT;
			z = ((space * P_Random(rngclass))>>8) + floorz + 40*FRACUNIT;
		}
		else
		{
			z = floorz;
		}
	}
	else if (flags2 & MF2_FLOATBOB)
	{
		z = floorz + z;		// artifact z passed in as height
	}
	if (flags2 & MF2_FLOORCLIP)
	{
		AdjustFloorClip ();
	}
	else
	{
		floorclip = 0;
	}
}

void AActor::Activate (AActor *activator)
{
	if (flags & MF_COUNTKILL)
		flags2 &= ~(MF2_DORMANT | MF2_INVULNERABLE);
}

void AActor::Deactivate (AActor *activator)
{
	if (flags & MF_COUNTKILL)
		flags2 |= MF2_DORMANT | MF2_INVULNERABLE;
}

//
// P_RemoveMobj
//

void AActor::Destroy ()
{
	// [RH] Unlink from tid chain
	RemoveFromHash ();

	// unlink from sector and block lists
	UnlinkFromWorld ();

	// Delete all nodes on the current sector_list			phares 3/16/98
	if (sector_list)
	{
		P_DelSeclist (sector_list);
		sector_list = NULL;
	}

	// stop any playing sound
	S_RelinkSound (this, NULL);

	Super::Destroy ();
}

//===========================================================================
//
// AdjustFloorClip
//
//===========================================================================

void AActor::AdjustFloorClip ()
{
	fixed_t oldclip = floorclip;
	fixed_t shallowestclip = MAXINT;
	const msecnode_t *m;

	// [RH] clip based on shallowest floor player is standing on
	for (m = touching_sectorlist; m; m = m->m_tnext)
	{
		if (m->m_sector->floorheight == z)
		{
			fixed_t clip = Terrains[TerrainTypes[m->m_sector->floorpic]].FootClip;
			if (clip < shallowestclip)
			{
				shallowestclip = clip;
			}
		}
	}
	if (shallowestclip == MAXINT)
	{
		floorclip = 0;
	}
	else
	{
		floorclip = shallowestclip;
	}
	if (player && oldclip != floorclip)
	{
		player->viewheight -= oldclip - floorclip;
		player->deltaviewheight = (VIEWHEIGHT - player->viewheight) >> 3;
	}
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//	between levels.
//
EXTERN_CVAR (chasedemo)
extern BOOL demonew;

void P_SpawnPlayer (mapthing2_t *mthing)
{
	int		  playernum;
	player_t *p;
	APlayerPawn *mobj;
	int 	  i;

	// [RH] Things 4001-? are also multiplayer starts. Just like 1-4.
	//		To make things simpler, figure out which player is being
	//		spawned here.
	playernum = (mthing->type > 4000) ? mthing->type - 4001 + 4 : mthing->type - 1;

	// not playing?
	if (playernum >= MAXPLAYERS || !playeringame[playernum])
		return;

	p = &players[playernum];

	if (gameinfo.gametype == GAME_Doom)
	{
		p->cls = TypeInfo::FindType ("DoomPlayer");
	}
	else if (gameinfo.gametype == GAME_Heretic)
	{
		p->cls = TypeInfo::FindType ("HereticPlayer");
	}
	else
	{
		p->cls = TypeInfo::FindType ("FighterPlayer");
	}

	mobj = static_cast<APlayerPawn *>
		(Spawn (p->cls, mthing->x << FRACBITS, mthing->y << FRACBITS, ONFLOORZ));

	p->mo = mobj;
	mobj->player = p;
	if (p->playerstate == PST_REBORN)
		G_PlayerReborn (playernum);

	// set color translations for player sprites
	// [RH] Different now: MF_TRANSLATION is not used.
	mobj->translation = translationtables + 256*2*playernum;

	mobj->angle = ANG45 * (mthing->angle/45);
	mobj->pitch = mobj->roll = 0;
	mobj->health = p->health;

	//Added by MC: Identification (number in the players[MAXPLAYERS] array)
    mobj->id = playernum;

	// [RH] Set player sprite based on skin
	mobj->sprite = skins[p->userinfo.skin].sprite;

	p->fov = 90.0f;
	p->camera = p->mo;
	p->playerstate = PST_LIVE;
	p->refire = 0;
	p->damagecount = 0;
	p->bonuscount = 0;
	p->morphTics = 0;
	p->rain1 = NULL;
	p->rain2 = NULL;
	p->extralight = 0;
	p->fixedcolormap = 0;
	p->viewheight = VIEWHEIGHT;
	p->inconsistant = 0;
	p->attacker = NULL;

	p->momx = p->momy = 0;		// killough 10/98: initialize bobbing to 0.

	players[consoleplayer].camera = players[displayplayer].mo;

	// [RH] Allow chasecam for demo watching
	if ((demoplayback || demonew) && chasedemo.value)
		p->cheats = CF_CHASECAM;

	// setup gun psprite
	P_SetupPsprites (p);

	// give all cards in death match mode
	if (deathmatch.value)
		for (i = 0; i < NUMKEYS; i++)
			p->keys[i] = true;

	if (displayplayer == playernum)
	{
		StatusBar->AttachToPlayer (p);
	}

	if (multiplayer)
	{
		unsigned an = ( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT;
		Spawn ("TeleportFog", mobj->x+20*finecosine[an], mobj->y+20*finesine[an], mobj->z);
	}

	// [RH] If someone is in the way, kill them
	P_TeleportMove (mobj, mobj->x, mobj->y, mobj->z, true);
}


//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//
// [RH] position is used to weed out unwanted start spots
void P_SpawnMapThing (mapthing2_t *mthing, int position)
{
	const TypeInfo *i;
	int bit;
	AActor *mobj;
	fixed_t x, y, z;

	if (mthing->type == 0 || mthing->type == -1)
		return;

	// count deathmatch start positions
	if (mthing->type == 11)
	{
		deathmatchstarts.Push (*mthing);
		return;
	}

	// [RH] Record polyobject-related things
	if (HexenHack)
	{
		switch (mthing->type)
		{
		case PO_HEX_ANCHOR_TYPE:
			mthing->type = PO_ANCHOR_TYPE;
			break;
		case PO_HEX_SPAWN_TYPE:
			mthing->type = PO_SPAWN_TYPE;
			break;
		case PO_HEX_SPAWNCRUSH_TYPE:
			mthing->type = PO_SPAWNCRUSH_TYPE;
			break;
		}
	}

	if (mthing->type == PO_ANCHOR_TYPE ||
		mthing->type == PO_SPAWN_TYPE ||
		mthing->type == PO_SPAWNCRUSH_TYPE)
	{
		polyspawns_t *polyspawn = new polyspawns_t;
		polyspawn->next = polyspawns;
		polyspawn->x = mthing->x << FRACBITS;
		polyspawn->y = mthing->y << FRACBITS;
		polyspawn->angle = mthing->angle;
		polyspawn->type = mthing->type;
		polyspawns = polyspawn;
		if (mthing->type != PO_ANCHOR_TYPE)
			po_NumPolyobjs++;
		return;
	}

	// check for players specially
	if (mthing->type <= 4 && mthing->type > 0)
	{
		// [RH] Only spawn spots that match position.
		if (mthing->args[0] != position)
			return;

		// save spots for respawning in network games
		playerstarts[mthing->type-1] = *mthing;
		if (!deathmatch.value)
			P_SpawnPlayer (mthing);

		return;
	}
	else if (mthing->type >= 4001 && mthing->type <= 4001 + MAXPLAYERS - 4)
	{ // [RH] Multiplayer starts for players 5-?

		// [RH] Only spawn spots that match position.
		if (mthing->args[0] != position)
			return;

		playerstarts[mthing->type - 4001 + 4] = *mthing;
		if (!deathmatch.value)
			P_SpawnPlayer (mthing);
		return;
	}

	// [RH] sound sequence overrides
	if (mthing->type >= 1400 && mthing->type < 1410)
	{
		R_PointInSubsector (mthing->x<<FRACBITS,
			mthing->y<<FRACBITS)->sector->seqType = mthing->type - 1400;
		return;
	}
	else if (mthing->type == 1411)
	{
		int type;

		if (mthing->args[0] == 255)
			type = -1;
		else
			type = mthing->args[0];

		if (type > 63)
		{
			Printf (PRINT_HIGH, "Sound sequence %d out of range\n", type);
		}
		else
		{
			R_PointInSubsector (mthing->x << FRACBITS,
				mthing->y << FRACBITS)->sector->seqType = type;
		}
		return;
	}

	if (deathmatch.value) 
	{
		if (!(mthing->flags & MTF_DEATHMATCH))
			return;
	}
	else if (multiplayer)
	{
		if (!(mthing->flags & MTF_COOPERATIVE))
			return;
	}
	else
	{
		if (!(mthing->flags & MTF_SINGLE))
			return;
	}

	// check for apropriate skill level
	if (gameskill.value == sk_baby)
		bit = 1;
	else if (gameskill.value == sk_nightmare)
		bit = 4;
	else
		bit = 1 << ((int)gameskill.value - 1);

	if (!(mthing->flags & bit))
		return;

	// [RH] Determine if it is an old ambient thing, and if so,
	//		map it to MT_AMBIENT with the proper parameter.
	if (mthing->type >= 14001 && mthing->type <= 14064)
	{
		mthing->args[0] = mthing->type - 14000;
		mthing->type = 14065;
		i = RUNTIME_CLASS(AAmbientSound);
	}
	else
	{
		// find which type to spawn
		i = DoomEdMap.FindType (mthing->type);
	}

	if (i == NULL)
	{
		// [RH] Don't die if the map tries to spawn an unknown thing
		Printf (PRINT_HIGH, "Unknown type %i at (%i, %i)\n",
				 mthing->type,
				 mthing->x, mthing->y);
		i = RUNTIME_CLASS(AUnknown);
	}
	// [RH] If the thing's corresponding sprite has no frames, also map
	//		it to the unknown thing.
	else if (i->ActorInfo->spawnstate == NULL ||
		sprites[i->ActorInfo->spawnstate->sprite.index].numframes == 0)
	{
		Printf (PRINT_HIGH, "Type %s at (%i, %i) has no frames\n",
				i->Name, mthing->x, mthing->y);
		i = RUNTIME_CLASS(AUnknown);
	}

	const FActorInfo *info = i->ActorInfo;

	// don't spawn keycards and players in deathmatch
	if (deathmatch.value && info->flags & MF_NOTDMATCH)
		return;

	// [RH] don't spawn extra weapons in coop
	if (multiplayer && !deathmatch.value)
	{
		if (i->IsDescendantOf (RUNTIME_CLASS(AWeapon)))
		{
			if ((mthing->flags & (MTF_DEATHMATCH|MTF_SINGLE)) == MTF_DEATHMATCH)
				return;
		}
	}
				
	// don't spawn any monsters if -nomonsters
	if (dmflags & DF_NO_MONSTERS
		&& (i->IsDescendantOf (RUNTIME_CLASS(ALostSoul)) || (info->flags & MF_COUNTKILL)) )
	{
		return;
	}
	
	// [RH] Other things that shouldn't be spawned depending on dmflags
	if (deathmatch.value || alwaysapplydmflags.value)
	{
		if (dmflags & DF_NO_HEALTH)
		{
			if (i->IsDescendantOf (RUNTIME_CLASS(AHealth)))
				return;
			if (strcmp (i->Name, "Berserk") == 0)
				return;
			if (strcmp (i->Name, "Soulsphere") == 0)
				return;
			if (strcmp (i->Name, "Megasphere") == 0)
				return;
		}
		if (dmflags & DF_NO_ITEMS)
		{
			if (i->IsDescendantOf (RUNTIME_CLASS(AArtifact)))
				return;
		}
		if (dmflags & DF_NO_ARMOR)
		{
			if (i->IsDescendantOf (RUNTIME_CLASS(AArmor)))
				return;
			if (strcmp (i->Name, "Megasphere") == 0)
				return;
		}
	}

	// spawn it
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;

	if (info->flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (info->flags2 & MF2_SPAWNFLOAT)
		z = FLOATRANDZ;
	else if (info->flags2 & MF2_FLOATBOB)
		z = mthing->z << FRACBITS;
	else
		z = ONFLOORZ;

	mobj = Spawn (i, x, y, z);
	mobj->mapflags = mthing->flags;

	if (z == ONFLOORZ)
		mobj->z += mthing->z << FRACBITS;
	else if (z == ONCEILINGZ)
		mobj->z -= mthing->z << FRACBITS;
	mobj->spawnpoint = *mthing;

	if (mobj->flags2 & MF2_FLOATBOB)
	{ // Seed random starting index for bobbing motion
		mobj->health = P_Random(pr_bobbing);
		mobj->special1 = mthing->z << FRACBITS;
	}

	// [RH] Set the thing's special
	mobj->special = mthing->special;
	memcpy (mobj->args, mthing->args, sizeof(mobj->args));

	// [RH] Add ThingID to mobj and link it in with the others
	mobj->tid = mthing->thingid;
	mobj->AddToHash ();

	mobj->angle = (DWORD)((mthing->angle * (QWORD)0x100000000) / 360);
	mobj->LevelSpawned ();
}



//
// GAME SPAWN FUNCTIONS
//


//
// P_SpawnPuff
//

void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z, angle_t dir, int updown, bool hitthing)
{
	AActor *puff;

	z += PS_Random (pr_spawnpuff) << 10;

	if (!hitthing || HitPuffType == NULL)
		puff = Spawn (PuffType, x, y, z);
	else
		puff = Spawn (HitPuffType, x, y, z);
	puff->momz = FRACUNIT;
	puff->tics -= P_Random (pr_spawnpuff) & 3;

	if (puff->tics < 1)
		puff->tics = 1;
		
	if (gameinfo.gametype == GAME_Doom)
	{
		// don't make punches spark on the wall
		if (attackrange == MELEERANGE)
		{
			FState *state = puff->state;
			int i;

			for (i = 0; i < 2 && state->nextstate; i++)
				state = state->nextstate;

			puff->SetState (state);
		}
		if (cl_pufftype.value && updown != 3)
		{
			P_DrawSplash2 (32, x, y, z, dir, updown, 1);
			puff->flags2 |= MF2_DONTDRAW;
		}
	}

	if (linetarget && GetInfo (puff)->seesound)
	{ // Hit thing sound
		S_Sound (puff, CHAN_BODY, GetInfo (puff)->seesound, 1, ATTN_NORM);
	}
	else if (GetInfo (puff)->attacksound)
	{
		S_Sound (puff, CHAN_BODY, GetInfo (puff)->attacksound, 1, ATTN_NORM);
	}
}



//
// P_SpawnBlood
// 
void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, angle_t dir, int damage)
{
	AActor *th;

	if (cl_bloodtype.value <= 1) {
		z += PS_Random (pr_spawnblood) << 10;
		th = Spawn<ABlood> (x, y, z);
		th->momz = FRACUNIT*2;
		th->tics -= P_Random (pr_spawnblood) & 3;

		if (th->tics < 1)
			th->tics = 1;

		if (gameinfo.gametype == GAME_Doom)
		{
			if (damage <= 12 && damage >= 9)
				th->SetState (GetInfo (th)->spawnstate + 1);
			else if (damage < 9)
				th->SetState (GetInfo (th)->spawnstate + 2);
		}
	}

	if (cl_bloodtype.value >= 1)
		P_DrawSplash2 (32, x, y, z, dir, 2, 0);
}

// Blood splatter -----------------------------------------------------------

class ABloodSplatter : public AActor
{
	DECLARE_ACTOR (ABloodSplatter, AActor);
};

IMPLEMENT_DEF_SERIAL (ABloodSplatter, AActor);
REGISTER_ACTOR (ABloodSplatter, Raven);

FState ABloodSplatter::States[] =
{
#define S_BLOODSPLATTER 0
	S_NORMAL (BLOD, 'C',	8, NULL, &States[S_BLOODSPLATTER+1]),
	S_NORMAL (BLOD, 'B',	8, NULL, &States[S_BLOODSPLATTER+2]),
	S_NORMAL (BLOD, 'A',	8, NULL, NULL),

#define S_BLOODSPLATTERX (S_BLOODSPLATTER+3)
	S_NORMAL (BLOD, 'A',	6, NULL, NULL)
};

void ABloodSplatter::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BLOODSPLATTER];
	info->deathstate = &States[S_BLOODSPLATTERX];
	info->radius = 2 * FRACUNIT;
	info->height = 4 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_NOTELEPORT|MF2_CANNOTPUSH;
	info->mass = 5;
}

//---------------------------------------------------------------------------
//
// PROC P_BloodSplatter
//
//---------------------------------------------------------------------------

void P_BloodSplatter (fixed_t x, fixed_t y, fixed_t z, AActor *originator)
{
	AActor *mo;

	mo = Spawn<ABloodSplatter> (x, y, z);
	mo->target = originator;
	mo->momx = PS_Random() << 10;
	mo->momy = PS_Random() << 10;
	mo->momz = 3*FRACUNIT;
}

//---------------------------------------------------------------------------
//
// PROC P_RipperBlood
//
//---------------------------------------------------------------------------

void P_RipperBlood (AActor *mo)
{
	AActor *th;
	fixed_t x, y, z;

	x = mo->x + (PS_Random () << 12);
	y = mo->y + (PS_Random () << 12);
	z = mo->z + (PS_Random () << 12);
	th = Spawn<ABlood> (x, y, z);
	if (gameinfo.gametype == GAME_Heretic)
		th->flags |= MF_NOGRAVITY;
	th->momx = mo->momx >> 1;
	th->momy = mo->momy >> 1;
	th->tics += P_Random () & 3;
}

//---------------------------------------------------------------------------
//
// FUNC P_GetThingFloorType
//
//---------------------------------------------------------------------------

int P_GetThingFloorType (AActor *thing)
{
	if (thing->floorpic)
	{		
		return TerrainTypes[thing->floorpic];
	}
	else
	{
		return TerrainTypes[thing->subsector->sector->floorpic];
	}
}

//---------------------------------------------------------------------------
//
// FUNC P_HitFloor
//
// Returns true if hit liquid and splashed, false if not.
//---------------------------------------------------------------------------

bool P_HitFloor (AActor *thing)
{
	AActor *mo;
	FSplashDef *splash;
	int terrainnum;
	int splashnum;
	bool smallsplash = false;
	const msecnode_t *m;

	// don't splash if landing on the edge above water/lava/etc....
	for (m = thing->touching_sectorlist; m; m = m->m_tnext)
	{
		if (thing->z == m->m_sector->floorheight)
		{
			break;
		}
	}
	if (m == NULL)
	{ 
		return false;
	}

	if (thing->flags3 & MF3_DONTSPLASH)
		return false;

	terrainnum = TerrainTypes[m->m_sector->floorpic];
	splashnum = Terrains[terrainnum].Splash;

	if (Terrains[terrainnum].DamageAmount && thing->player &&
		(level.time & Terrains[terrainnum].DamageTimeMask))
	{
		P_DamageMobj (thing, NULL, NULL, Terrains[terrainnum].DamageAmount,
			Terrains[terrainnum].DamageMOD, Terrains[terrainnum].DamageFlags);
	}

	if (splashnum == -1)
		return Terrains[terrainnum].IsLiquid;

	splash = &Splashes[splashnum];

	// Small splash for small masses
	if (GetInfo (thing)->mass < 10)
		smallsplash = true;

	if (smallsplash && splash->SmallSplash)
	{
		mo = Spawn (splash->SmallSplash, thing->x, thing->y, ONFLOORZ);
		if (mo) mo->floorclip += splash->SmallSplashClip;
	}
	else
	{
		if (splash->SplashBase)
		{
			mo = Spawn (splash->SplashBase, thing->x, thing->y, ONFLOORZ);
		}
		if (splash->SplashChunk)
		{
			mo = Spawn (splash->SplashChunk, thing->x, thing->y, ONFLOORZ);
			mo->target = thing;
			if (splash->ChunkXVelShift != 255)
				mo->momx = PS_Random() << splash->ChunkXVelShift;
			if (splash->ChunkYVelShift != 255)
				mo->momy = PS_Random() << splash->ChunkYVelShift;
			mo->momz = splash->ChunkBaseZVel +
				(P_Random() << splash->ChunkZVelShift);
		}
		if (!smallsplash && thing->player)
		{
			P_NoiseAlert (thing, thing);
		}
	}
	if (mo)
	{
		S_SoundID (mo, CHAN_BODY, smallsplash ?
			splash->SmallSplashSound : splash->NormalSplashSound,
			1, ATTN_IDLE);
	}

	return Terrains[terrainnum].IsLiquid;
}

//---------------------------------------------------------------------------
//
// FUNC P_CheckMissileSpawn
//
// Returns true if the missile is at a valid spawn point, otherwise
// explodes it and returns false.
//
//---------------------------------------------------------------------------

bool P_CheckMissileSpawn (AActor* th)
{
	if (gameinfo.gametype == GAME_Doom)
	{
		th->tics -= P_Random (pr_checkmissilespawn) & 3;
		if (th->tics < 1)
			th->tics = 1;
	}

	// move a little forward so an angle can
	// be computed if it immediately explodes
	if (th->flags3 & MF3_VERYFAST)
	{ // Ultra-fast ripper spawning missile
		th->x += th->momx>>3;
		th->y += th->momy>>3;
		th->z += th->momz>>3;
	}
	else
	{ // Normal missile
		th->x += th->momx>>1;
		th->y += th->momy>>1;
		th->z += th->momz>>1;
	}

	// killough 3/15/98: no dropoff (really = don't care for missiles)

	if (!P_TryMove (th, th->x, th->y, false))
	{
		P_ExplodeMissile (th);
		return false;
	}
	return true;
}


//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissile
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

AActor *P_SpawnMissile (AActor *source, AActor *dest, const TypeInfo *type)
{
	return P_SpawnMissileZ (source, source->z + 32*FRACUNIT, dest, type);
}

AActor *P_SpawnMissileZ (AActor *source, fixed_t z, AActor *dest, const TypeInfo *type)
{
	if (type->ActorInfo->flags3 & MF3_FLOORHUGGER)
	{
		z = ONFLOORZ;
	}
	else if (type->ActorInfo->flags3 & MF3_CEILINGHUGGER)
	{
		z = ONCEILINGZ;
	}
	if (z != ONFLOORZ)
	{
		z -= source->floorclip;
	}

	AActor *th = Spawn (type, source->x, source->y, z);
	
	if (GetInfo (th)->seesound)
		S_Sound (th, CHAN_VOICE, GetInfo (th)->seesound, 1, ATTN_NORM);

	th->target = source;		// where it came from

	vec3_t velocity;
	float speed = FIXED2FLOAT (GetInfo (th)->speed);

	velocity[0] = FIXED2FLOAT(dest->x - source->x);
	velocity[1] = FIXED2FLOAT(dest->y - source->y);
	velocity[2] = FIXED2FLOAT(dest->z - source->z);
	VectorNormalize (velocity);
	th->momx = FLOAT2FIXED(velocity[0] * speed);
	th->momy = FLOAT2FIXED(velocity[1] * speed);
	th->momz = FLOAT2FIXED(velocity[2] * speed);

	// invisible target: rotate velocity vector in 2D
	if (dest->flags & MF_SHADOW)
	{
		angle_t an = PS_Random (pr_spawnmissile) << 20;
		an >>= ANGLETOFINESHIFT;
		
		fixed_t newx = FixedMul (th->momx, finecosine[an]) - FixedMul (th->momy, finesine[an]);
		fixed_t newy = FixedMul (th->momx, finesine[an]) + FixedMul (th->momy, finecosine[an]);
		th->momx = newx;
		th->momy = newy;
	}

	th->angle = R_PointToAngle2 (0, 0, th->momx, th->momy);

	return P_CheckMissileSpawn (th) ? th : NULL;
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissileAngle
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

AActor *P_SpawnMissileAngle (AActor *source, const TypeInfo *type,
	angle_t angle, fixed_t momz)
{
	return P_SpawnMissileAngleZ (source, source->z + 32*FRACUNIT,
		type, angle, momz);
}

AActor *P_SpawnMissileAngleZ (AActor *source, fixed_t z,
	const TypeInfo *type, angle_t angle, fixed_t momz)
{
	AActor *mo;

	if (type->ActorInfo->flags3 & MF3_FLOORHUGGER)
	{
		z = ONFLOORZ;
	}
	else if (type->ActorInfo->flags3 & MF3_CEILINGHUGGER)
	{
		z = ONCEILINGZ;
	}
	if (z != ONFLOORZ)
	{
		z -= source->floorclip;
	}
	mo = Spawn (type, source->x, source->y, z);
	if (GetInfo (mo)->seesound)
	{
		S_Sound (mo, CHAN_VOICE, GetInfo (mo)->seesound, 1, ATTN_NORM);
	}
	mo->target = source; // Originator
	mo->angle = angle;
	angle >>= ANGLETOFINESHIFT;
	mo->momx = FixedMul (GetInfo (mo)->speed, finecosine[angle]);
	mo->momy = FixedMul (GetInfo (mo)->speed, finesine[angle]);
	mo->momz = momz;
	return P_CheckMissileSpawn(mo) ? mo : NULL;
}

/*
================
=
= P_SpawnPlayerMissile
=
= Tries to aim at a nearby monster
================
*/

AActor *P_SpawnPlayerMissile (AActor *source, const TypeInfo *type)
{
	return P_SpawnPlayerMissile (source, source->x, source->y, source->z, type, source->angle);
}

AActor *P_SpawnPlayerMissile (AActor *source, const TypeInfo *type, angle_t angle)
{
	return P_SpawnPlayerMissile (source, source->x, source->y, source->z, type, angle);
}

AActor *P_SpawnPlayerMissile (AActor *source, fixed_t x, fixed_t y, fixed_t z,
							  const TypeInfo *type, angle_t angle)
{
	angle_t an;
	fixed_t slope;
	fixed_t pitchslope = finetangent[FINEANGLES/4-(source->pitch>>ANGLETOFINESHIFT)];

	// see which target is to be aimed at
	an = angle;

	if (source->player &&
		source->player->userinfo.aimdist == 0 &&
		!(dmflags & DF_NO_FREELOOK))
	{
		slope = pitchslope;
	}
	else
	{
		slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
		
		if (!linetarget)
		{
			an += 1<<26;
			slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

			if (!linetarget)
			{
				an -= 2<<26;
				slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
			}

			if (!linetarget)
			{
				an = angle;
				// [RH] Use pitch to calculate slope instead of 0.
				slope = pitchslope;
			}
		}

		if (linetarget && source->player)
		{
			if (!(dmflags & DF_NO_FREELOOK)
				&& abs(slope - pitchslope) > source->player->userinfo.aimdist)
			{
				an = angle;
				slope = pitchslope;
			}
		}
	}

	if (type->ActorInfo->flags3 & MF3_FLOORHUGGER)
	{
		z = ONFLOORZ;
	}
	else if (type->ActorInfo->flags3 & MF3_CEILINGHUGGER)
	{
		z = ONCEILINGZ;
	}
	if (z != ONFLOORZ && z != ONCEILINGZ)
	{
		z += 4*8*FRACUNIT - source->floorclip;
	}
	MissileActor = Spawn (type, x, y, z);

	if (GetInfo (MissileActor)->seesound)
	{
		S_Sound (MissileActor, CHAN_VOICE, GetInfo (MissileActor)->seesound, 1, ATTN_NORM);
	}
	MissileActor->target = source;
	MissileActor->angle = an;

	vec3_t velocity;
	float speed = FIXED2FLOAT (GetInfo (MissileActor)->speed);

	velocity[0] = FIXED2FLOAT (finecosine[an>>ANGLETOFINESHIFT]);
	velocity[1] = FIXED2FLOAT (finesine[an>>ANGLETOFINESHIFT]);
	velocity[2] = FIXED2FLOAT (slope);

	VectorNormalize (velocity);

	MissileActor->momx = FLOAT2FIXED (velocity[0] * speed);
	MissileActor->momy = FLOAT2FIXED (velocity[1] * speed);
	MissileActor->momz = FLOAT2FIXED (velocity[2] * speed);

	if (P_CheckMissileSpawn (MissileActor))
	{
		return MissileActor;
	}
	return NULL;
}

// [RH] Throw gibs around (based on Q2 code)

#if 0	// Not used right now (Note that this code considers vectors to
		// be expressed as fixed_t's.)
void VelocityForDamage (int damage, vec3_t v)
{
	v[0] = 8 * crandom(pr_vel4dmg);
	v[1] = 8 * crandom(pr_vel4dmg);
	v[2] = 8 * FRACUNIT + (fixed_t)(P_Random(pr_vel4dmg) * (6553600/256));

	if (damage < 50)
		VectorScale (v, 0.7f, v);
	else 
		VectorScale (v, 1.2f, v);
}

void ClipGibVelocity (AActor *ent)
{
	if (ent->momx < -15 * FRACUNIT)
		ent->momx = -15 * FRACUNIT;
	else if (ent->momx > 15 * FRACUNIT)
		ent->momx = 15 * FRACUNIT;
	if (ent->momy < -15 * FRACUNIT)
		ent->momy = -15 * FRACUNIT;
	else if (ent->momy > 15 * FRACUNIT)
		ent->momy = 15 * FRACUNIT;
	if (ent->momz < 6 * FRACUNIT)
		ent->momz = 6 * FRACUNIT;	// always some upwards
	else if (ent->momz > 10 * FRACUNIT)
		ent->momz = 10 * FRACUNIT;
}

void ThrowGib (AActor *self, const TypeInfo *gibtype, int damage)
{
	AActor *gib;
	vec3_t	vd;
	vec3_t	selfvel;
	vec3_t	gibvel;

	gib = Spawn (gibtype,
		self->x + crandom(pr_throwgib) * (self->radius >> (FRACBITS + 1)),
		self->y + crandom(pr_throwgib) * (self->radius >> (FRACBITS + 1)),
		self->z + (self->height >> 1) + crandom(pr_throwgib) * (self->height >> (FRACBITS + 1)));

	VelocityForDamage (damage, vd);
	selfvel[0] = self->momx;
	selfvel[1] = self->momy;
	selfvel[2] = self->momz;
	VectorMA (selfvel, 1.0, vd, gibvel);
	gib->momx = gibvel[0];
	gib->momy = gibvel[1];
	gib->momz = gibvel[2];
	ClipGibVelocity (gib);
}
#else
void ThrowGib (AActor *self, const TypeInfo *gibtype, int damage)
{
}
#endif

bool AActor::IsTeammate (AActor *other)
{
	if (!player || !other || !other->player)
		return false;
	if (!deathmatch.value)
		return true;
	if (teamplay.value && other->player->userinfo.team[0] &&
		!stricmp (player->userinfo.team, other->player->userinfo.team))
	{
		return true;
	}
	return false;
}

int AActor::DoSpecialDamage (AActor *target, int damage)
{
	if ((target->flags2 & MF2_INVULNERABLE) && damage < 10000)
	{ // actor is invulnerable
		return -1;
	}
	else if (target->player && damage < 1000 &&
		((target->player->cheats & CF_GODMODE)
		|| target->player->powers[pw_invulnerability]))
	{
		return -1;
	}
	else
	{
		return damage;
	}
}
