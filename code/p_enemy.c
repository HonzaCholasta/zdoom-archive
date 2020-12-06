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
//		Enemy thinking, AI.
//		Action Pointer Functions
//		that are associated with states/frames. 
//
//-----------------------------------------------------------------------------


#include <stdlib.h>

#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "p_local.h"
#include "p_lnspec.h"

#include "s_sound.h"

#include "g_game.h"

// State.
#include "doomstat.h"
#include "r_state.h"

#include "c_cvars.h"

cvar_t *testgibs;



typedef enum
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
	
} dirtype_t;


//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
	DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};





void A_Fall (mobj_t *actor);


//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

mobj_t *soundtarget;

void P_RecursiveSound (sector_t *sec, int soundblocks)
{
	int 		i;
	line_t* 	check;
	sector_t*	other;
		
	// wake up all monsters in this sector
	if (sec->validcount == validcount
		&& sec->soundtraversed <= soundblocks+1)
	{
		return; 		// already flooded
	}
	
	sec->validcount = validcount;
	sec->soundtraversed = soundblocks+1;
	sec->soundtarget = soundtarget;
		
	for (i=0 ;i<sec->linecount ; i++)
	{
		check = sec->lines[i];
		if (! (check->flags & ML_TWOSIDED) )
			continue;
		
		P_LineOpening (check);

		if (openrange <= 0)
			continue;	// closed door
		
		if ( sides[ check->sidenum[0] ].sector == sec)
			other = sides[ check->sidenum[1] ] .sector;
		else
			other = sides[ check->sidenum[0] ].sector;
		
		if (check->flags & ML_SOUNDBLOCK)
		{
			if (!soundblocks)
				P_RecursiveSound (other, 1);
		}
		else
			P_RecursiveSound (other, soundblocks);
	}
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert (mobj_t *target, mobj_t *emmiter)
{
	if (target->player && (target->player->cheats & CF_NOTARGET))
		return;

	soundtarget = target;
	validcount++;
	P_RecursiveSound (emmiter->subsector->sector, 0);
}




//
// P_CheckMeleeRange
//
BOOL P_CheckMeleeRange (mobj_t *actor)
{
	mobj_t *pl;
	fixed_t dist;
		
	if (!actor->target)
		return false;
				
	pl = actor->target;
	dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);

	if (dist >= MELEERANGE-20*FRACUNIT+pl->info->radius)
		return false;

	// [RH] If moving toward goal, then we've reached it.
	if (actor->target == actor->goal)
		return true;

	// [RH] Don't melee things too far above or below actor.
	if (!olddemo) {
		if (pl->z > actor->z + actor->height + 20*FRACUNIT)
			return false;
		if (pl->z + pl->height < actor->z - 20*FRACUNIT)
			return false;
	}
		
	if (! P_CheckSight (actor, pl, false) )
		return false;
														
	return true;				
}

//
// P_CheckMissileRange
//
BOOL P_CheckMissileRange (mobj_t *actor)
{
	fixed_t dist;
		
	if (! P_CheckSight (actor, actor->target, false) )
		return false;
		
	if ( actor->flags & MF_JUSTHIT )
	{
		// the target just hit the enemy,
		// so fight back!
		actor->flags &= ~MF_JUSTHIT;
		return true;
	}
		
	if (actor->reactiontime)
		return false;	// do not attack yet
				
	// OPTIMIZE: get this from a global checksight
	dist = P_AproxDistance ( actor->x-actor->target->x,
							 actor->y-actor->target->y) - 64*FRACUNIT;
	
	if (!actor->info->meleestate)
		dist -= 128*FRACUNIT;	// no melee attack, so fire more

	dist >>= 16;

	if (actor->type == MT_VILE)
	{
		if (dist > 14*64)		
			return false;		// too far away
	}
		

	if (actor->type == MT_UNDEAD)
	{
		if (dist < 196) 
			return false;		// close for fist attack
		dist >>= 1;
	}
		

	if (actor->type == MT_CYBORG
		|| actor->type == MT_SPIDER
		|| actor->type == MT_SKULL)
	{
		dist >>= 1;
	}
	
	if (dist > 200)
		dist = 200;
				
	if (actor->type == MT_CYBORG && dist > 160)
		dist = 160;
				
	if (P_Random (pr_checkmissilerange) < dist)
		return false;
				
	return true;
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

extern	line_t	**spechit;
extern	int 	numspechit;

BOOL P_Move (mobj_t *actor)
{
	fixed_t	tryx;
	fixed_t	tryy;
	
	line_t* ld;
	
	// warning: 'catch', 'throw', and 'try'
	// are all C++ reserved words
	BOOL 	try_ok;
	BOOL 	good;
				
	if (actor->movedir == DI_NODIR)
		return false;
				
	if ((unsigned)actor->movedir >= 8)
		I_Error ("Weird actor->movedir!");
				
	tryx = actor->x + actor->info->speed * xspeed[actor->movedir];
	tryy = actor->y + actor->info->speed * yspeed[actor->movedir];

	// killough 3/15/98: don't jump over dropoffs:
	try_ok = P_TryMove (actor, tryx, tryy, false);

	if (!try_ok)
	{
		// open any specials
		if (actor->flags & MF_FLOAT && floatok)
		{
			// must adjust height
			if (actor->z < tmfloorz)
				actor->z += FLOATSPEED;
			else
				actor->z -= FLOATSPEED;

			actor->flags |= MF_INFLOAT;
			return true;
		}
				
		if (!numspechit)
			return false;
						
		actor->movedir = DI_NODIR;
		good = false;
		while (numspechit--)
		{
			ld = spechit[numspechit];
			// if the special is not a door
			// that can be opened, return false
			if ((ld->flags & ML_ACTIVATIONMASK) == ML_ACTIVATEPUSH)
				good = P_PushSpecialLine (actor,
					P_PointOnLineSide (actor->x, actor->y, ld),
					ld);	// [RH] Try to push the line
			else
				good = P_UseSpecialLine (actor, ld, 0);
		}
		return good && (olddemo || (P_Random (pr_trywalk) & 3));	//jff 8/13/98
	}													// 1 in 4 try a different dir
	else												// avoid stuck in doorway
	{
		actor->flags &= ~MF_INFLOAT;
	}
		
		
	if (! (actor->flags & MF_FLOAT) )
		P_StandOnThing (actor, NULL);
//		actor->z = actor->floorz;
	return true; 
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
BOOL P_TryWalk (mobj_t *actor)
{		
	if (!P_Move (actor))
	{
		return false;
	}

	actor->movecount = P_Random (pr_trywalk) & 15;
	return true;
}




void P_NewChaseDir (mobj_t *actor)
{
	fixed_t 	deltax;
	fixed_t 	deltay;
	
	dirtype_t	d[3];
	
	int 		tdir;
	dirtype_t	olddir;
	
	dirtype_t	turnaround;

	if (!actor->target) {
		Printf ("P_NewChaseDir: called with no target");
		P_SetMobjState (actor, actor->info->spawnstate);
		return;
	}
				
	olddir = actor->movedir;
	turnaround = opposite[olddir];

	deltax = actor->target->x - actor->x;
	deltay = actor->target->y - actor->y;

	if (deltax>10*FRACUNIT)
		d[1]= DI_EAST;
	else if (deltax<-10*FRACUNIT)
		d[1]= DI_WEST;
	else
		d[1]=DI_NODIR;

	if (deltay<-10*FRACUNIT)
		d[2]= DI_SOUTH;
	else if (deltay>10*FRACUNIT)
		d[2]= DI_NORTH;
	else
		d[2]=DI_NODIR;

	// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		actor->movedir = diags[((deltay<0)<<1) + (deltax>0)];
		if (actor->movedir != turnaround && P_TryWalk(actor))
			return;
	}

	// try other directions
	if (P_Random (pr_newchasedir) > 200 || abs(deltay) > abs(deltax))
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}

	if (d[1] == turnaround)
		d[1] = DI_NODIR;
	if (d[2] == turnaround)
		d[2] = DI_NODIR;
		
	if (d[1] != DI_NODIR)
	{
		actor->movedir = d[1];
		if (P_TryWalk (actor))
		{
			// either moved forward or attacked
			return;
		}
	}

	if (d[2] != DI_NODIR)
	{
		actor->movedir = d[2];

		if (P_TryWalk (actor))
			return;
	}

	// there is no direct path to the player,
	// so pick another direction.
	if (olddir != DI_NODIR)
	{
		actor->movedir = olddir;

		if (P_TryWalk (actor))
			return;
	}

	// randomly determine direction of search
	if (P_Random (pr_newchasedir) & 1)	
	{
		for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
		{
			if (tdir != turnaround)
			{
				actor->movedir =tdir;
				
				if ( P_TryWalk(actor) )
					return;
			}
		}
	}
	else
	{
		for (tdir = DI_SOUTHEAST; tdir != (DI_EAST-1); tdir--)
		{
			if (tdir != turnaround)
			{
				actor->movedir = tdir;
				
				if ( P_TryWalk(actor) )
					return;
			}
		}
	}

	if (turnaround != DI_NODIR)
	{
		actor->movedir =turnaround;
		if ( P_TryWalk(actor) )
			return;
	}

	actor->movedir = DI_NODIR;	// can not move
}



//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
BOOL P_LookForPlayers (mobj_t *actor, BOOL allaround)
{
	int 		c;
	int 		stop;
	player_t*	player;
	sector_t*	sector;
	angle_t 	an;
	fixed_t 	dist;
				
	sector = actor->subsector->sector;
		
	c = 0;
	stop = (actor->lastlook-1) & (MAXPLAYERS-1);
		
	for ( ; ; actor->lastlook = (actor->lastlook+1)&(MAXPLAYERS-1) )
	{
		if (!playeringame[actor->lastlook])
			continue;
						
		if (c++ == 2 || actor->lastlook == stop)
		{
			// done looking		// [RH] use goal as target
			return (actor->target = actor->goal) ? true : false;
		}
		
		player = &players[actor->lastlook];

		if (player->cheats & CF_NOTARGET)
			continue;			// no target

		if (player->health <= 0)
			continue;			// dead

		if (!P_CheckSight (actor, player->mo, false))
			continue;			// out of sight
						
		if (!allaround)
		{
			an = R_PointToAngle2 (actor->x,
								  actor->y, 
								  player->mo->x,
								  player->mo->y)
				- actor->angle;
			
			if (an > ANG90 && an < ANG270)
			{
				dist = P_AproxDistance (player->mo->x - actor->x,
										player->mo->y - actor->y);
				// if real close, react anyway
				if (dist > MELEERANGE)
					continue;	// behind back
			}
		}
		
		// [RH] Need to be sure the reactiontime is 0 if the monster is
		//		leaving its goal to go after a player.
		if (actor->goal && actor->target == actor->goal)
			actor->reactiontime = 0;

		actor->target = player->mo;
		return true;
	}

	// Use last known enemy if no players sighted -- killough 2/15/98:

	if (/*smartypants && */ !olddemo)
		if (actor->lastenemy && actor->lastenemy->health > 0)
		{
			actor->target = actor->lastenemy;
			actor->lastenemy = NULL;
			return true;
		}

	// [RH] Use goal as a last resort
	return (actor->target = actor->goal) ? true : false;

}


//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie (mobj_t *mo)
{
	thinker_t*	th;
	mobj_t* 	mo2;

	A_Fall (mo);
	
	// scan the remaining thinkers
	// to see if all Keens are dead
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;
		if (mo2 != mo
			&& mo2->type == mo->type
			&& mo2->health > 0)
		{
			// other Keen not dead
			return; 			
		}
	}

	EV_DoDoor (doorOpen, NULL, NULL, 666, 2*FRACUNIT, 0, 0);
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
// [RH] Will also leave state to move to goal.
//
void A_Look (mobj_t *actor)
{
	mobj_t *targ;

	// [RH] Set goal now if appropriate
	if (actor->special == Thing_SetGoal && actor->args[0] == 0) {
		actor->special = 0;
		actor->goal = P_FindGoal (NULL, actor->args[1], MT_PATHNODE);
		actor->reactiontime = actor->args[2] * TICRATE + level.time;
	}

	actor->threshold = 0;		// any shot will wake up
	targ = actor->subsector->sector->soundtarget;

	if (targ && targ->player && (targ->player->cheats & CF_NOTARGET))
		return;

	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		P_IncreaseVisibility(actor);
	}

	if (targ && (targ->flags & MF_SHOOTABLE) )
	{
		actor->target = targ;

		if ( actor->flags & MF_AMBUSH )
		{
			if (P_CheckSight (actor, actor->target, false))
				goto seeyou;
		}
		else
			goto seeyou;
	}
		
		
	if (!P_LookForPlayers (actor, false) )
		return;
				
	// go into chase state
  seeyou:
	// [RH] Don't start chasing after a goal if it isn't time yet.
	if (actor->target == actor->goal) {
		if (actor->reactiontime > level.time)
			actor->target = NULL;
	} else if (actor->info->seesound) {
		char sound[MAX_SNDNAME];

		strcpy (sound, actor->info->seesound);

		if (sound[strlen(sound)-1] == '1') {
			sound[strlen(sound)-1] = P_Random(pr_look)%3 + '1';
			if (S_FindSound (sound) == -1)
				sound[strlen(sound)-1] = '1';
		}

		if (actor->type==MT_SPIDER || actor->type == MT_CYBORG)
		{
			// full volume
			S_StartSound (ORIGIN_SURROUND, sound, 90);
		}
		else
			S_StartSound (actor, sound, 90);
	}

	if (actor->target)
		P_SetMobjState (actor, actor->info->seestate);
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase (mobj_t *actor)
{
	int delta;

	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		P_DecreaseVisibility(actor);
	}

	if (actor->reactiontime)
		actor->reactiontime--;
								
	// modify target threshold
	if (actor->threshold)
	{
		if (!actor->target || actor->target->health <= 0)
		{
			actor->threshold = 0;
		}
		else
			actor->threshold--;
	}
	
	// turn towards movement direction if not there yet
	if (actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);
		
		if (delta > 0)
			actor->angle -= ANG90/2;
		else if (delta < 0)
			actor->angle += ANG90/2;
	}

	if (!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{
		// look for a new target
		if (P_LookForPlayers (actor, true) && actor->target != actor->goal)
			return; 	// got a new target
		
		if (actor->target == NULL) {
			P_SetMobjState (actor, actor->info->spawnstate);
			return;
		}
	}
	
	// do not attack twice in a row
	if (actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		if (gameskill->value != sk_nightmare && !(dmflags & DF_FAST_MONSTERS))
			P_NewChaseDir (actor);
		return;
	}
	
	// [RH] Don't attack if just moving toward goal
	if (actor->target == actor->goal) {
		if (P_CheckMeleeRange (actor)) {
			// reached the goal
			actor->reactiontime = actor->goal->args[1] * TICRATE + level.time;
			actor->goal = P_FindGoal (NULL, actor->goal->args[0], MT_PATHNODE);
			actor->target = NULL;
			P_SetMobjState (actor, actor->info->spawnstate);
			return;
		}
		goto nomissile;
	}

	// check for melee attack
	if (actor->info->meleestate && P_CheckMeleeRange (actor))
	{
		if (actor->info->attacksound)
			S_StartSound (actor, actor->info->attacksound, 70);

		P_SetMobjState (actor, actor->info->meleestate);
		return;
	}
	
	// check for missile attack
	if (actor->info->missilestate)
	{
		if (gameskill->value < sk_nightmare
			&& actor->movecount && !(dmflags & DF_FAST_MONSTERS))
		{
			goto nomissile;
		}
		
		if (!P_CheckMissileRange (actor))
			goto nomissile;
		
		P_SetMobjState (actor, actor->info->missilestate);
		actor->flags |= MF_JUSTATTACKED;
		return;
	}

	// ?
  nomissile:
	// possibly choose another target
	if (netgame
		&& !actor->threshold
		&& !P_CheckSight (actor, actor->target, false) )
	{
		if (P_LookForPlayers(actor,true))
			return; 	// got a new target
	}
	
	// chase towards player
	if (--actor->movecount < 0 || !P_Move (actor))
	{
		P_NewChaseDir (actor);
	}
	
	// make active sound
	if (actor->info->activesound && P_Random (pr_chase) < 3)
	{
		S_StartSound (actor, actor->info->activesound, 120);
	}
}


//
// A_FaceTarget
//
void A_FaceTarget (mobj_t *actor)
{
	if (!actor->target)
		return;

	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		P_IncreaseVisibility(actor);
	}

	actor->flags &= ~MF_AMBUSH;
		
	actor->angle = R_PointToAngle2 (actor->x,
									actor->y,
									actor->target->x,
									actor->target->y);
	
	if (actor->target->flags & MF_SHADOW)
    {
      int t = P_Random(pr_facetarget);
      actor->angle += (t-P_Random(pr_facetarget))<<21;
    }
}


//
// A_PosAttack
//
void A_PosAttack (mobj_t *actor)
{
	int angle;
	int damage;
	int slope;
	int t;
		
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
	angle = actor->angle;
	slope = P_AimLineAttack (actor, angle, MISSILERANGE);

	S_StartSound (actor, "grunt/attack", 64);
	t = P_Random(pr_posattack);
	angle += (t - P_Random (pr_posattack))<<20;
	damage = ((P_Random (pr_posattack)%5)+1)*3;
	P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack (mobj_t *actor)
{
	int i;
	int bangle;
	int slope;
		
	if (!actor->target)
		return;

	S_StartSound (actor, "shotguy/attack", 64);
	A_FaceTarget (actor);
	bangle = actor->angle;
	slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

	for (i=0 ; i<3 ; i++)
    {
		int t = P_Random (pr_sposattack);
		int angle = bangle + ((t - P_Random (pr_sposattack))<<20);
		int damage = ((P_Random (pr_sposattack)%5)+1)*3;
		P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack (mobj_t *actor)
{
	int angle;
	int bangle;
	int damage;
	int slope;
	int t;
		
	if (!actor->target)
		return;

	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		P_IncreaseVisibility(actor);
	}

	S_StartSound (actor, "chainguy/attack", 64);
	A_FaceTarget (actor);
	bangle = actor->angle;
	slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

	t = P_Random (pr_cposattack);
	angle = bangle + ((t - P_Random (pr_cposattack))<<20);
	damage = ((P_Random (pr_cposattack)%5)+1)*3;
	P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire (mobj_t *actor)
{		
	// keep firing unless target got out of sight
	A_FaceTarget (actor);

	if (P_Random (pr_cposrefire) < 40)
		return;

	if (!actor->target
		|| actor->target->health <= 0
		|| !P_CheckSight (actor, actor->target, false) )
	{
		P_SetMobjState (actor, actor->info->seestate);
	}
}


void A_SpidRefire (mobj_t *actor)
{		
	// keep firing unless target got out of sight
	A_FaceTarget (actor);

	if (P_Random (pr_spidrefire) < 10)
		return;

	if (!actor->target
		|| actor->target->health <= 0
		|| !P_CheckSight (actor, actor->target, false) )
	{
		P_SetMobjState (actor, actor->info->seestate);
	}
}

void A_BspiAttack (mobj_t *actor)
{		
	if (!actor->target)
		return;

	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		P_IncreaseVisibility(actor);
	}

	A_FaceTarget (actor);

	// launch a missile
	P_SpawnMissile (actor, actor->target, MT_ARACHPLAZ);
}


//
// A_TroopAttack
//
void A_TroopAttack (mobj_t *actor)
{
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		int damage = (P_Random (pr_troopattack)%8+1)*3;
		S_StartSound (actor, "imp/melee", 70);
		P_DamageMobj (actor->target, actor, actor, damage, MOD_HIT);
		return;
	}
	
	// launch a missile
	P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}


void A_SargAttack (mobj_t *actor)
{
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		int damage = ((P_Random (pr_sargattack)%10)+1)*4;
		P_DamageMobj (actor->target, actor, actor, damage, MOD_HIT);
	}
}

void A_HeadAttack (mobj_t *actor)
{
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		int damage = (P_Random (pr_headattack)%6+1)*10;
		P_DamageMobj (actor->target, actor, actor, damage, MOD_HIT);
		return;
	}
	
	// launch a missile
	P_SpawnMissile (actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack (mobj_t *actor)
{		
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
	P_SpawnMissile (actor, actor->target, MT_ROCKET);
}


void A_BruisAttack (mobj_t *actor)
{
	if (!actor->target)
		return;
				
	if (P_CheckMeleeRange (actor))
	{
		int damage = (P_Random (pr_bruisattack)%8+1)*10;
		S_StartSound (actor, "baron/melee", 70);
		P_DamageMobj (actor->target, actor, actor, damage, MOD_HIT);
		return;
	}
	
	// launch a missile
	P_SpawnMissile (actor, actor->target, MT_BRUISERSHOT);
}


//
// A_SkelMissile
//
void A_SkelMissile (mobj_t *actor)
{		
	mobj_t *mo;
		
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
	actor->z += 16*FRACUNIT;	// so missile spawns higher
	mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
	actor->z -= 16*FRACUNIT;	// back to normal

	mo->x += mo->momx;
	mo->y += mo->momy;
	mo->tracer = actor->target;
}

int 	TRACEANGLE = 0xc000000;

void A_Tracer (mobj_t *actor)
{
	angle_t 	exact;
	fixed_t 	dist;
	fixed_t 	slope;
	mobj_t* 	dest;
	mobj_t* 	th;
				
	// killough 1/18/98: this is why some missiles do not have smoke
	// and some do. Also, internal demos start at random gametics, thus
	// the bug in which revenants cause internal demos to go out of sync.
	//
	// killough 3/6/98: fix revenant internal demo bug by subtracting
	// levelstarttic from gametic:
	//
	// [RH] level.time is always 0-based, so nothing special to do here.

	if (level.time & 3)
		return;
	
	// spawn a puff of smoke behind the rocket			
	P_SpawnPuff (actor->x, actor->y, actor->z);
		
	th = P_SpawnMobj (actor->x-actor->momx,
					  actor->y-actor->momy,
					  actor->z, MT_SMOKE, 0);
	
	th->momz = FRACUNIT;
	th->tics -= P_Random (pr_tracer)&3;
	if (th->tics < 1)
		th->tics = 1;
	
	// adjust direction
	dest = actor->tracer;
		
	if (!dest || dest->health <= 0)
		return;
	
	// change angle 	
	exact = R_PointToAngle2 (actor->x,
							 actor->y,
							 dest->x,
							 dest->y);

	if (exact != actor->angle)
	{
		if (exact - actor->angle > 0x80000000)
		{
			actor->angle -= TRACEANGLE;
			if (exact - actor->angle < 0x80000000)
				actor->angle = exact;
		}
		else
		{
			actor->angle += TRACEANGLE;
			if (exact - actor->angle > 0x80000000)
				actor->angle = exact;
		}
	}
		
	exact = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul (actor->info->speed, finecosine[exact]);
	actor->momy = FixedMul (actor->info->speed, finesine[exact]);
	
	// change slope
	dist = P_AproxDistance (dest->x - actor->x,
							dest->y - actor->y);
	
	dist = dist / actor->info->speed;

	if (dist < 1)
		dist = 1;
	slope = (dest->z+40*FRACUNIT - actor->z) / dist;

	if (slope < actor->momz)
		actor->momz -= FRACUNIT/8;
	else
		actor->momz += FRACUNIT/8;
}


void A_SkelWhoosh (mobj_t *actor)
{
	if (!actor->target)
		return;
	A_FaceTarget (actor);
	S_StartSound (actor, "skeleton/swing", 70);
}

void A_SkelFist (mobj_t *actor)
{
	if (!actor->target)
		return;
				
	A_FaceTarget (actor);
		
	if (P_CheckMeleeRange (actor))
	{
		int damage = ((P_Random (pr_skelfist)%10)+1)*6;
		S_StartSound (actor, "skeleton/melee", 70);
		P_DamageMobj (actor->target, actor, actor, damage, MOD_HIT);
	}
}



//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t* 		corpsehit;
mobj_t* 		vileobj;
fixed_t 		viletryx;
fixed_t 		viletryy;

BOOL PIT_VileCheck (mobj_t *thing)
{
	int 	maxdist;
	BOOL 	check;
		
	if (!(thing->flags & MF_CORPSE) )
		return true;	// not a monster
	
	if (thing->tics != -1)
		return true;	// not lying still yet
	
	if (thing->info->raisestate == S_NULL)
		return true;	// monster doesn't have a raise state
	
	maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;
		
	if ( abs(thing->x - viletryx) > maxdist
		 || abs(thing->y - viletryy) > maxdist )
		return true;			// not actually touching
				
	corpsehit = thing;
	corpsehit->momx = corpsehit->momy = 0;
	if (olddemo) {
		corpsehit->height <<= 2;
		check = P_CheckPosition (corpsehit, corpsehit->x, corpsehit->y);
		corpsehit->height >>= 2;
	} else {
		// [RH] Check against real height and radius
		fixed_t oldheight = corpsehit->height;
		fixed_t oldradius = corpsehit->radius;
		int oldflags = corpsehit->flags;
		corpsehit->flags |= MF_SOLID;
		corpsehit->height = corpsehit->info->height;
		check = P_CheckPosition (corpsehit, corpsehit->x, corpsehit->y);
		corpsehit->flags = oldflags;
		corpsehit->radius = oldradius;
		corpsehit->height = oldheight;
	}

	return !check;
}



//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase (mobj_t *actor)
{
	int 				xl;
	int 				xh;
	int 				yl;
	int 				yh;
	
	int 				bx;
	int 				by;

	mobjinfo_t* 		info;
	mobj_t* 			temp;
		
	if (actor->movedir != DI_NODIR)
	{
		// check for corpses to raise
		viletryx =
			actor->x + actor->info->speed*xspeed[actor->movedir];
		viletryy =
			actor->y + actor->info->speed*yspeed[actor->movedir];

		xl = (viletryx - bmaporgx - MAXRADIUS*2)>>MAPBLOCKSHIFT;
		xh = (viletryx - bmaporgx + MAXRADIUS*2)>>MAPBLOCKSHIFT;
		yl = (viletryy - bmaporgy - MAXRADIUS*2)>>MAPBLOCKSHIFT;
		yh = (viletryy - bmaporgy + MAXRADIUS*2)>>MAPBLOCKSHIFT;
		
		vileobj = actor;
		for (bx=xl ; bx<=xh ; bx++)
		{
			for (by=yl ; by<=yh ; by++)
			{
				// Call PIT_VileCheck to check
				// whether object is a corpse
				// that canbe raised.
				if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
				{
					// got one!
					temp = actor->target;
					actor->target = corpsehit;
					A_FaceTarget (actor);
					actor->target = temp;
										
					P_SetMobjState (actor, S_VILE_HEAL1);
					S_StartSound (corpsehit, "vile/raise", 78);
					info = corpsehit->info;
					
					P_SetMobjState (corpsehit,info->raisestate);
					corpsehit->height = info->height;	// [RH] Use real mobj height
					corpsehit->radius = info->radius;	// [RH] Use real radius
					corpsehit->flags = (info->flags & ~MF_TRANSLUCBITS) | MF_TRANSLUC50;
					corpsehit->health = info->spawnhealth;
					corpsehit->target = NULL;

					return;
				}
			}
		}
	}

	// Return to normal attack.
	A_Chase (actor);
}


//
// A_VileStart
//
void A_VileStart (mobj_t *actor)
{
	S_StartSound (actor, "vile/start", 70);
}


//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire (mobj_t *actor);

void A_StartFire (mobj_t *actor)
{
	S_StartSound(actor, "vile/firestrt", 32);
	A_Fire(actor);
}

void A_FireCrackle (mobj_t *actor)
{
	S_StartSound(actor, "vile/firecrkl", 32);
	A_Fire(actor);
}

void A_Fire (mobj_t *actor)
{
	mobj_t* 	dest;
	unsigned	an;
				
	dest = actor->tracer;
	if (!dest)
		return;
				
	// don't move it if the vile lost sight
	if (!P_CheckSight (actor->target, dest, false) )
		return;

	an = dest->angle >> ANGLETOFINESHIFT;

	P_UnsetThingPosition (actor);
	actor->x = dest->x + FixedMul (24*FRACUNIT, finecosine[an]);
	actor->y = dest->y + FixedMul (24*FRACUNIT, finesine[an]);
	actor->z = dest->z;
	P_SetThingPosition (actor);
}



//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget (mobj_t *actor)
{
	mobj_t *fog;
		
	if (!actor->target)
		return;

	A_FaceTarget (actor);

	fog = P_SpawnMobj (actor->target->x,
					   actor->target->x,
					   actor->target->z, MT_FIRE, 0);
	
	actor->tracer = fog;
	fog->target = actor;
	fog->tracer = actor->target;
	A_Fire (fog);
}




//
// A_VileAttack
//
void A_VileAttack (mobj_t *actor)
{		
	mobj_t *fire;
	int an;
		
	if (!actor->target)
		return;
	
	A_FaceTarget (actor);

	if (!P_CheckSight (actor, actor->target, false) )
		return;

	S_StartSound (actor, "vile/stop", 60);
	P_DamageMobj (actor->target, actor, actor, 20, MOD_UNKNOWN);
	actor->target->momz = 1000*FRACUNIT/actor->target->info->mass;
		
	an = actor->angle >> ANGLETOFINESHIFT;

	fire = actor->tracer;

	if (!fire)
		return;
				
	// move the fire between the vile and the player
	fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[an]);
	fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[an]);	
	P_RadiusAttack (fire, actor, 70, MOD_UNKNOWN);
}




//
// Mancubus attack,
// firing three missiles (bruisers) in three different directions?
// Doesn't look like it. 
//
#define FATSPREAD		(ANG90/8)

void A_FatRaise (mobj_t *actor)
{
	A_FaceTarget (actor);
	S_StartSound (actor, "fatso/attack", 64);
}


void A_FatAttack1 (mobj_t *actor)
{
	mobj_t* 	mo;
	int 		an;
		
	A_FaceTarget (actor);
	// Change direction  to ...
	actor->angle += FATSPREAD;
	P_SpawnMissile (actor, actor->target, MT_FATSHOT);

	mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
	mo->angle += FATSPREAD;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul (mo->info->speed, finecosine[an]);
	mo->momy = FixedMul (mo->info->speed, finesine[an]);
}

void A_FatAttack2 (mobj_t *actor)
{
	mobj_t* 	mo;
	int 		an;

	A_FaceTarget (actor);
	// Now here choose opposite deviation.
	actor->angle -= FATSPREAD;
	P_SpawnMissile (actor, actor->target, MT_FATSHOT);

	mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
	mo->angle -= FATSPREAD*2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul (mo->info->speed, finecosine[an]);
	mo->momy = FixedMul (mo->info->speed, finesine[an]);
}

void A_FatAttack3 (mobj_t *actor)
{
	mobj_t* 	mo;
	int 		an;

	A_FaceTarget (actor);
	
	mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
	mo->angle -= FATSPREAD/2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul (mo->info->speed, finecosine[an]);
	mo->momy = FixedMul (mo->info->speed, finesine[an]);

	mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
	mo->angle += FATSPREAD/2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul (mo->info->speed, finecosine[an]);
	mo->momy = FixedMul (mo->info->speed, finesine[an]);
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED (20*FRACUNIT)

void A_SkullAttack (mobj_t *actor)
{
	mobj_t* 			dest;
	angle_t 			an;
	int 				dist;

	if (!actor->target)
		return;
				
	dest = actor->target;		
	actor->flags |= MF_SKULLFLY;

	S_StartSound (actor, actor->info->attacksound, 70);
	A_FaceTarget (actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul (SKULLSPEED, finecosine[an]);
	actor->momy = FixedMul (SKULLSPEED, finesine[an]);
	dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
	dist = dist / SKULLSPEED;
	
	if (dist < 1)
		dist = 1;
	actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}


//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void A_PainShootSkull (mobj_t *actor, angle_t angle)
{
	fixed_t 	x;
	fixed_t 	y;
	fixed_t 	z;
	
	mobj_t* 	newmobj;
	angle_t 	an;
	int 		prestep;
	int 		count;
	thinker_t*	currentthinker;

	// count total number of skull currently on the level
	count = 0;

	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		if (   (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
			&& ((mobj_t *)currentthinker)->type == MT_SKULL)
			count++;
		currentthinker = currentthinker->next;
	}

	// if there are allready 20 skulls on the level,
	// don't spit another one
	if (count > 20)
		return;

	// okay, there's room for another one
	an = angle >> ANGLETOFINESHIFT;
	
	prestep = 4*FRACUNIT + 3*(actor->info->radius + mobjinfo[MT_SKULL].radius)/2;
	
	x = actor->x + FixedMul (prestep, finecosine[an]);
	y = actor->y + FixedMul (prestep, finesine[an]);
	z = actor->z + 8*FRACUNIT;
				
	newmobj = P_SpawnMobj (x , y, z, MT_SKULL, 0);
	if (olddemo)													// phares
		newmobj = P_SpawnMobj(x, y, z, MT_SKULL, 0);				//   |
	else															//   V
	{
		// Check whether the Lost Soul is being fired through a 1-sided
		// wall or an impassible line, or a "monsters can't cross" line.
		// If it is, then we don't allow the spawn. This is a bug fix, but
		// it should be considered an enhancement, since it may disturb
		// existing demos, so don't do it in compatibility mode.

		if (Check_Sides(actor,x,y))
			return;

		newmobj = P_SpawnMobj(x, y, z, MT_SKULL, 0);

		// Check to see if the new Lost Soul's z value is above the
		// ceiling of its new sector, or below the floor. If so, kill it.

		if ((newmobj->z >
             (newmobj->subsector->sector->ceilingheight - newmobj->height)) ||
            (newmobj->z < newmobj->subsector->sector->floorheight))
		{
			// kill it immediately
			P_DamageMobj(newmobj,actor,actor,10000,MOD_UNKNOWN);
			return;													//   ^
		}															//   |
	}																// phares

	// Check for movements.
	// killough 3/15/98: don't jump over dropoffs:

	if (!P_TryMove (newmobj, newmobj->x, newmobj->y, false))
	{
		// kill it immediately
		P_DamageMobj (newmobj,actor,actor,10000,MOD_UNKNOWN);		
		return;
	}
				
	newmobj->target = actor->target;
	A_SkullAttack (newmobj);
}


//
// A_PainAttack
// Spawn a lost soul and launch it at the target
// 
void A_PainAttack (mobj_t *actor)
{
	if (!actor->target)
		return;

	A_FaceTarget (actor);
	A_PainShootSkull (actor, actor->angle);
}

void A_PainDie (mobj_t *actor)
{
	A_Fall (actor);
	A_PainShootSkull (actor, actor->angle+ANG90);
	A_PainShootSkull (actor, actor->angle+ANG180);
	A_PainShootSkull (actor, actor->angle+ANG270);
}


void A_Scream (mobj_t *actor)
{
	char sound[MAX_SNDNAME];

	strcpy (sound, actor->info->deathsound);

	if (sound[strlen(sound)-1] == '1') {
		sound[strlen(sound)-1] = P_Random(pr_look)%3 + '1';
		if (S_FindSound (sound) == -1)
			sound[strlen(sound)-1] = '1';
	}

	// Check for bosses.
	if (actor->type==MT_SPIDER || actor->type == MT_CYBORG)
	{
		// full volume
		S_StartSound (ORIGIN_SURROUND, sound, 70);
	}
	else
		S_StartSound (actor, sound, 70);
}


void A_XScream (mobj_t *actor)
{
	if (actor->player)
		S_StartSound (actor, "*gibbed", 78);
	else
		S_StartSound (actor, "misc/gibbed", 78);
}

void A_Pain (mobj_t *actor)
{
	// [RH] Have some variety in player pain sounds
	if (actor->player) {
		// [RH] Allow for a little more variety in pain sounds
		int l, r = (P_Random (pr_playerpain) & 1) + 1;
		char nametemp[128];

		if (actor->health < 25)
			l = 25;
		else if (actor->health < 50)
			l = 50;
		else if (actor->health < 75)
			l = 75;
		else
			l = 100;

		sprintf (nametemp, "*pain%d_%d", l, r);
		S_StartSound (actor, nametemp, 96);
	} else if (actor->info->painsound)
		S_StartSound (actor, actor->info->painsound, 96);	
}



void A_Fall (mobj_t *actor)
{
	int n;

	// [RH] Andy Baker's stealth monsters
	if (actor->flags & MF_STEALTH)
	{
		P_BecomeVisible(actor);
	}

	// actor is on ground, it can be walked over
	actor->flags &= ~MF_SOLID;

	// So change this if corpse objects
	// are meant to be obstacles.

	// [RH] Toss some gibs
	//if (actor->health < -80)
	if (testgibs->value)
		for (n = 0; n < 6; n++)
			ThrowGib (actor, MT_GIB0 + (int)(P_Random(pr_gengib) >> 5), -actor->health);
}


//
// A_Explode
//
void A_Explode (mobj_t *thingy)
{
	// [RH] figure out means of death;
	int mod;

	switch (thingy->type) {
		case MT_BARREL:
			mod = MOD_BARREL;
			break;
		case MT_ROCKET:
			mod = MOD_ROCKET;
			break;
		default:
			mod = MOD_UNKNOWN;
			break;
	}

	P_RadiusAttack ( thingy, thingy->target, 128, mod );
}


//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath (mobj_t *mo)
{
	thinker_t*	th;
	mobj_t* 	mo2;
	int 		i;

	// [RH] These all depend on the presence of level flags now
	//		rather than being hard-coded to specific levels.

	if ((level.flags & (LEVEL_MAP07SPECIAL|
						LEVEL_BRUISERSPECIAL|
						LEVEL_CYBORGSPECIAL|
						LEVEL_SPIDERSPECIAL)) == 0)
		return;

	if (
		((level.flags & LEVEL_MAP07SPECIAL) && (mo->type == MT_FATSO || mo->type == MT_BABY)) ||
		((level.flags & LEVEL_BRUISERSPECIAL) && (mo->type == MT_BRUISER)) ||
		((level.flags & LEVEL_CYBORGSPECIAL) && (mo->type == MT_CYBORG)) ||
		((level.flags & LEVEL_SPIDERSPECIAL) && (mo->type == MT_SPIDER))
	   )
		;
	else return;

	// make sure there is a player alive for victory
	for (i=0 ; i<MAXPLAYERS ; i++)
		if (playeringame[i] && players[i].health > 0)
			break;
	
	if (i==MAXPLAYERS)
		return; // no one left alive, so do not end game
	
	// scan the remaining thinkers to see
	// if all bosses are dead
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;
		
		mo2 = (mobj_t *)th;
		if (mo2 != mo
			&& mo2->type == mo->type
			&& mo2->health > 0)
		{
			// other boss not dead
			return;
		}
	}
		
	// victory!
	if (level.flags & LEVEL_MAP07SPECIAL)
	{
		if (mo->type == MT_FATSO)
		{
			EV_DoFloor (floorLowerToLowest, NULL, 666, FRACUNIT, 0, 0, 0);
			return;
		}
		
		if (mo->type == MT_BABY)
		{
			EV_DoFloor (floorRaiseByTexture, NULL, 667, FRACUNIT, 0, 0, 0);
			return;
		}
	}
	else
	{
		switch (level.flags & LEVEL_SPECACTIONSMASK) {
			case LEVEL_SPECLOWERFLOOR:
				EV_DoFloor (floorLowerToLowest, NULL, 666, FRACUNIT, 0, 0, 0);
				return;
				break;
			
			case LEVEL_SPECOPENDOOR:
				EV_DoDoor (doorOpen, NULL, NULL, 666, 8*TICRATE, 0, 0);
				return;
				break;
		}
	}

	// [RH] If noexit, then don't end the level.
	if (deathmatch->value && (dmflags & DF_NO_EXIT))
		return;

	G_ExitLevel (0);
}


void A_Hoof (mobj_t *mo)
{
	S_StartSound (mo, "cyber/hoof", 70);
	A_Chase (mo);
}

void A_Metal (mobj_t *mo)
{
	S_StartSound (mo, "spider/walk", 70);
	A_Chase (mo);
}

void A_BabyMetal (mobj_t *mo)
{
	S_StartSound (mo, "baby/walk", 100);
	A_Chase (mo);
}

void A_OpenShotgun2 (player_t *player, pspdef_t *psp)
{
	S_StartSound (player->mo, "weapons/sshoto", 64);
}

void A_LoadShotgun2 (player_t *player, pspdef_t *psp)
{
	S_StartSound (player->mo, "weapons/sshotl", 64);
}

void A_ReFire (player_t *player, pspdef_t *psp);

void A_CloseShotgun2 (player_t *player, pspdef_t *psp)
{
	S_StartSound (player->mo, "weapons/sshotc", 64);
	A_ReFire(player,psp);
}



// killough 2/7/98: Remove limit on icon landings:
mobj_t **braintargets;
int    numbraintargets_alloc;
int    numbraintargets;

struct brain_s brain;   // killough 3/26/98: global state of boss brain

// killough 3/26/98: initialize icon landings at level startup,
// rather than at boss wakeup, to prevent savegame-related crashes

void P_SpawnBrainTargets (void)	// killough 3/26/98: renamed old function
{
	thinker_t *thinker;

	// find all the target spots
	numbraintargets = 0;
	brain.targeton = 0;
	brain.easy = 0;				// killough 3/26/98: always init easy to 0

	for (thinker = thinkercap.next ;
		 thinker != &thinkercap ;
		 thinker = thinker->next)
		if (thinker->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj_t *m = (mobj_t *) thinker;

			if (m->type == MT_BOSSTARGET )
			{		// killough 2/7/98: remove limit on icon landings:
				if (numbraintargets >= numbraintargets_alloc)
					braintargets = realloc(braintargets,
						(numbraintargets_alloc = numbraintargets_alloc ?
						 numbraintargets_alloc*2 : 32) *sizeof *braintargets);
				braintargets[numbraintargets++] = m;
			}
		}
}

void A_BrainAwake (mobj_t *mo)
{
	// killough 3/26/98: only generates sound now
	S_StartSound (ORIGIN_SURROUND, "brain/sight", 70);
}


void A_BrainPain (mobj_t *mo)
{
	S_StartSound (ORIGIN_SURROUND2, "brain/pain", 70);
}


void A_BrainScream (mobj_t *mo)
{
	int 		x;
	int 		y;
	int 		z;
	mobj_t* 	th;
		
	for (x=mo->x - 196*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
	{
		y = mo->y - 320*FRACUNIT;
		z = 128 + (P_Random (pr_brainscream) << (FRACBITS + 1));
		th = P_SpawnMobj (x,y,z, MT_ROCKET, 0);
		th->momz = P_Random (pr_brainscream) << 9;

		P_SetMobjState (th, S_BRAINEXPLODE1);

		th->tics -= P_Random (pr_brainscream) & 7;
		if (th->tics < 1)
			th->tics = 1;
	}
		
	S_StartSound (ORIGIN_SURROUND, "brain/death", 70);
}



void A_BrainExplode (mobj_t *mo)
{
	int t = P_Random (pr_brainexplode);
	int x = mo->x + (t - P_Random (pr_brainexplode))*2048;
	int y = mo->y;
	int z = 128 + P_Random (pr_brainexplode)*2*FRACUNIT;
	mobj_t *th = P_SpawnMobj (x,y,z, MT_ROCKET, 0);
	th->momz = P_Random (pr_brainexplode) << 9;

	P_SetMobjState (th, S_BRAINEXPLODE1);

	th->tics -= P_Random (pr_brainexplode) & 7;
	if (th->tics < 1)
		th->tics = 1;
}


void A_BrainDie (mobj_t *mo)
{
	// [RH] If noexit, then don't end the level.
	if (deathmatch->value && (dmflags & DF_NO_EXIT))
		return;

	G_ExitLevel (0);
}

void A_BrainSpit (mobj_t *mo)
{
	mobj_t* 	targ;
	mobj_t* 	newmobj;
	
	// [RH] Do nothing if there are no brain targets.
	if (numbraintargets == 0)
		return;

	brain.easy ^= 1;		// killough 3/26/98: use brain struct
	if (gameskill->value <= sk_easy && (!brain.easy))
		return;
				
	// shoot a cube at current target
	targ = braintargets[brain.targeton++];	// killough 3/26/98:
	brain.targeton %= numbraintargets;		// Use brain struct for targets

	// spawn brain missile
	newmobj = P_SpawnMissile (mo, targ, MT_SPAWNSHOT);
	newmobj->target = targ;
	newmobj->reactiontime =
		((targ->y - mo->y)/newmobj->momy) / newmobj->state->tics;

	S_StartSound (ORIGIN_SURROUND, "brain/spit", 70);
}



void A_SpawnFly (mobj_t *mo);

// travelling cube sound
void A_SpawnSound (mobj_t *mo)	
{
	S_StartSound (mo, "brain/cube", 70);
	A_SpawnFly(mo);
}

void A_SpawnFly (mobj_t *mo)
{
	mobj_t* 	newmobj;
	mobj_t* 	fog;
	mobj_t* 	targ;
	int 		r;
	mobjtype_t	type;
		
	if (--mo->reactiontime)
		return; // still flying
		
	targ = mo->target;

	// First spawn teleport fog.
	fog = P_SpawnMobj (targ->x, targ->y, targ->z, MT_SPAWNFIRE, 0);
	S_StartSound (fog, "misc/teleport", 32);

	// Randomly select monster to spawn.
	r = P_Random (pr_spawnfly);

	// Probability distribution (kind of :),
	// decreasing likelihood.
	if ( r<50 )
		type = MT_TROOP;
	else if (r<90)
		type = MT_SERGEANT;
	else if (r<120)
		type = MT_SHADOWS;
	else if (r<130)
		type = MT_PAIN;
	else if (r<160)
		type = MT_HEAD;
	else if (r<162)
		type = MT_VILE;
	else if (r<172)
		type = MT_UNDEAD;
	else if (r<192)
		type = MT_BABY;
	else if (r<222)
		type = MT_FATSO;
	else if (r<246)
		type = MT_KNIGHT;
	else
		type = MT_BRUISER;

	newmobj = P_SpawnMobj (targ->x, targ->y, targ->z, type, 0);
	if (P_LookForPlayers (newmobj, true))
		P_SetMobjState (newmobj, newmobj->info->seestate);
		
	// telefrag anything in this spot
	P_TeleportMove (newmobj, newmobj->x, newmobj->y, newmobj->z, true);

	// remove self (i.e., cube).
	P_RemoveMobj (mo);
}



void A_PlayerScream (mobj_t *mo)
{
	char nametemp[128];
	char *sound;
		
	if ( (gamemode == commercial) && (mo->health < -50))
	{
		// IF THE PLAYER DIES LESS THAN -50% WITHOUT GIBBING
		sound = "*gibbed";
	} else {
		// [RH] More variety in death sounds
		sprintf (nametemp, "*death%d", (P_Random (pr_playerscream)&3) + 1);
		sound = nametemp;
	}
	
	S_StartSound (mo, sound, 32);
}


/***** Start of new functions for Andy Baker's stealth monsters ******/

void P_BecomeVisible (mobj_t *actor)
{
	actor->flags2 &= ~MF2_INVISIBLE;
	actor->flags &= ~MF_TRANSLUCBITS;
};

void P_IncreaseVisibility (mobj_t *actor)
{
	if (actor->flags2 & MF2_INVISIBLE) {
		actor->flags2 &= ~MF2_INVISIBLE;
		actor->flags |= MF_TRANSLUC25;
	} else switch (actor->flags & MF_TRANSLUCBITS) {
		case MF_TRANSLUC25:
			actor->flags ^= MF_TRANSLUCBITS;
			break;
		case MF_TRANSLUC50:
			actor->flags |= MF_TRANSLUC25;
			break;
		case MF_TRANSLUC75:
			actor->flags &= ~MF_TRANSLUCBITS;
			break;
	}
}

void P_DecreaseVisibility (mobj_t *actor)
{
	if (actor->flags2 & MF2_INVISIBLE)
		return;			// already invisible

	switch (actor->flags & MF_TRANSLUCBITS) {
		case 0:
			actor->flags |= MF_TRANSLUC75;
			break;
		case MF_TRANSLUC75:
			actor->flags &= ~MF_TRANSLUC25;
			break;
		case MF_TRANSLUC50:
			actor->flags ^= MF_TRANSLUCBITS;
			break;
		case MF_TRANSLUC25:
			actor->flags &= ~MF_TRANSLUCBITS;
			actor->flags2 |= MF2_INVISIBLE;
	}
}
/***** End of new functions for Andy Baker's stealth monsters ******/