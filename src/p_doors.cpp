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
// DESCRIPTION: Door animation code (opening/closing)
//		[RH] Removed sliding door code and simplified for Hexen-ish specials
//
//-----------------------------------------------------------------------------



#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "s_sndseq.h"
#include "doomstat.h"
#include "r_state.h"
#include "c_console.h"
#include "gi.h"
#include "a_keys.h"
#include "i_system.h"
#include "sc_man.h"

IMPLEMENT_CLASS (DDoor)

DDoor::DDoor ()
{
}

void DDoor::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << m_Type
		<< m_TopDist
		<< m_BotSpot << m_BotDist << m_OldFloorDist
		<< m_Speed
		<< m_Direction
		<< m_TopWait
		<< m_TopCountdown
		<< m_LightTag;
}


//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
void DDoor::Tick ()
{
	EResult res;

	if (m_Sector->floorplane.d != m_OldFloorDist)
	{
		m_OldFloorDist = m_Sector->floorplane.d;
		m_BotDist = m_Sector->ceilingplane.PointToDist (m_BotSpot,
			m_Sector->floorplane.ZatPoint (m_BotSpot));
	}

	switch (m_Direction)
	{
	case 0:
		// WAITING
		if (!--m_TopCountdown)
		{
			switch (m_Type)
			{
			case doorRaise:
				m_Direction = -1; // time to go back down
				DoorSound (false);
				break;
				
			case doorCloseWaitOpen:
				m_Direction = 1;
				DoorSound (true);
				break;
				
			default:
				break;
			}
		}
		break;
		
	case 2:
		//	INITIAL WAIT
		if (!--m_TopCountdown)
		{
			switch (m_Type)
			{
			case doorRaiseIn5Mins:
				m_Direction = 1;
				m_Type = doorRaise;
				DoorSound (true);
				break;
				
			default:
				break;
			}
		}
		break;
		
	case -1:
		// DOWN
		res = MoveCeiling (m_Speed, m_BotDist, -1, m_Direction);

		// killough 10/98: implement gradual lighting effects
		if (m_LightTag != 0 && m_TopDist != -m_Sector->floorplane.d)
		{
			EV_LightTurnOnPartway (m_LightTag, FixedDiv (m_Sector->ceilingplane.d + m_Sector->floorplane.d,
				m_TopDist + m_Sector->floorplane.d));
		}

		if (res == pastdest)
		{
			SN_StopSequence (m_Sector);
			switch (m_Type)
			{
			case doorRaise:
			case doorClose:
				m_Sector->ceilingdata = NULL;	//jff 2/22/98
				stopinterpolation (INTERP_SectorCeiling, m_Sector);
				Destroy ();						// unlink and free
				break;
				
			case doorCloseWaitOpen:
				m_Direction = 0;
				m_TopCountdown = m_TopWait;
				break;
				
			default:
				break;
			}
		}
		else if (res == crushed)
		{
			switch (m_Type)
			{
			case doorClose:				// DO NOT GO BACK UP!
				break;
				
			default:
				m_Direction = 1;
				DoorSound (true);
				break;
			}
		}
		break;
		
	case 1:
		// UP
		res = MoveCeiling (m_Speed, m_TopDist, -1, m_Direction);
		
		// killough 10/98: implement gradual lighting effects
		if (m_LightTag != 0 && m_TopDist != -m_Sector->floorplane.d)
		{
			EV_LightTurnOnPartway (m_LightTag, FixedDiv (m_Sector->ceilingplane.d + m_Sector->floorplane.d,
				m_TopDist + m_Sector->floorplane.d));
		}

		if (res == pastdest)
		{
			SN_StopSequence (m_Sector);
			switch (m_Type)
			{
			case doorRaise:
				m_Direction = 0; // wait at top
				m_TopCountdown = m_TopWait;
				break;
				
			case doorCloseWaitOpen:
			case doorOpen:
				m_Sector->ceilingdata = NULL;	//jff 2/22/98
				stopinterpolation (INTERP_SectorCeiling, m_Sector);
				Destroy ();						// unlink and free
				break;
				
			default:
				break;
			}
		}
		break;
	}
}

// [RH] DoorSound: Plays door sound depending on direction and speed
void DDoor::DoorSound (bool raise) const
{
	const char *snd;

	if (m_Sector->seqType >= 0)
	{
		SN_StartSequence (m_Sector, m_Sector->seqType, SEQ_DOOR);
	}
	else
	{
		switch (gameinfo.gametype)
		{
		default:
			snd = NULL;
			break;

		case GAME_Heretic:
			snd = raise ? "HereticDoorOpen" : "HereticDoorClose";
			break;

		case GAME_Doom:
			if (m_Speed >= 8*FRACUNIT)
			{
				snd = raise ? "DoorOpenBlazing" : "DoorCloseBlazing";
			}
			else
			{
				snd = raise ? "DoorOpenNormal" : "DoorCloseNormal";
			}
			break;

		case GAME_Strife:
			if (m_Speed >= FRACUNIT*8)
			{
				snd = raise ? "DoorOpenBlazing" : "DoorCloseBlazing";
			}
			else
			{
				snd = raise ? "DoorOpenSmallMetal" : "DoorCloseSmallMetal";

				// Search the front top textures of 2-sided lines on the door sector
				// for a door sound to use.
				for (int i = 0; i < m_Sector->linecount; ++i)
				{
					const char *texname;
					line_t *line = m_Sector->lines[i];

					if (line->backsector == NULL)
						continue;

					texname = TexMan[sides[line->sidenum[0]].toptexture]->Name;
					if (strncmp (texname, "DOR", 3) == 0)
					{
						switch (texname[3])
						{
						case 'S':
							snd = raise ? "DoorOpenStone" : "DoorCloseStone";
							break;

						case 'M':
							if (texname[4] == 'L')
							{
								snd = raise ? "DoorOpenLargeMetal" : "DoorCloseLargeMetal";
							}
							break;

						case 'W':
							if (texname[4] == 'L')
							{
								snd = raise ? "DoorOpenLargeWood" : "DoorCloseLargeWood";
							}
							else
							{
								snd = raise ? "DoorOpenSmallWood" : "DoorCloseSmallWood";
							}
							break;
						}
					}
				}
			}
			break;
		}
		if (snd != NULL)
		{
			SN_StartSequence (m_Sector, snd);
		}
	}
}

DDoor::DDoor (sector_t *sector)
	: DMovingCeiling (sector)
{
}

// [RH] Merged EV_VerticalDoor and EV_DoLockedDoor into EV_DoDoor
//		and made them more general to support the new specials.

// [RH] SpawnDoor: Helper function for EV_DoDoor
DDoor::DDoor (sector_t *sec, EVlDoor type, fixed_t speed, int delay, int lightTag)
	: DMovingCeiling (sec),
  	  m_Type (type), m_Speed (speed), m_TopWait (delay), m_LightTag (lightTag)
{
	vertex_t *spot;
	fixed_t height;

	if (compatflags & COMPATF_NODOORLIGHT)
	{
		m_LightTag = 0;
	}

	switch (type)
	{
	case doorClose:
		m_Direction = -1;
		height = sec->FindLowestCeilingSurrounding (&spot);
		m_TopDist = sec->ceilingplane.PointToDist (spot, height - 4*FRACUNIT);
		DoorSound (false);
		break;

	case doorOpen:
	case doorRaise:
		m_Direction = 1;
		height = sec->FindLowestCeilingSurrounding (&spot);
		m_TopDist = sec->ceilingplane.PointToDist (spot, height - 4*FRACUNIT);
		if (m_TopDist != sec->ceilingplane.d)
			DoorSound (true);
		break;

	case doorCloseWaitOpen:
		m_TopDist = sec->ceilingplane.d;
		m_Direction = -1;
		DoorSound (false);
		break;

	case doorRaiseIn5Mins:
		m_Direction = 2;
		height = sec->FindLowestCeilingSurrounding (&spot);
		m_TopDist = sec->ceilingplane.PointToDist (spot, height - 4*FRACUNIT);
		m_TopCountdown = 5 * 60 * TICRATE;
		break;
	}

	height = sec->FindHighestFloorPoint (&m_BotSpot);
	m_BotDist = sec->ceilingplane.PointToDist (m_BotSpot, height);
	m_OldFloorDist = sec->floorplane.d;
}

bool EV_DoDoor (DDoor::EVlDoor type, line_t *line, AActor *thing,
				int tag, int speed, int delay, int lock, int lightTag)
{
	bool		rtn = false;
	int 		secnum;
	sector_t*	sec;

	if (lock != 0 && !P_CheckKeys (thing, lock, tag != 0))
		return false;

	if (tag == 0)
	{		// [RH] manual door
		if (!line)
			return false;

		// if the wrong side of door is pushed, give oof sound
		if (line->sidenum[1] == NO_INDEX)			// killough
		{
			S_Sound (thing, CHAN_VOICE, "*usefail", 1, ATTN_NORM);
			return false;
		}

		// get the sector on the second side of activating linedef
		sec = sides[line->sidenum[1]].sector;
		secnum = sec-sectors;

		// if door already has a thinker, use it
		if (sec->ceilingdata)
		{
			if (sec->ceilingdata->IsKindOf (RUNTIME_CLASS(DDoor)))
			{
				DDoor *door = static_cast<DDoor *>(sec->ceilingdata);

				// ONLY FOR "RAISE" DOORS, NOT "OPEN"s
				if (door->m_Type == DDoor::doorRaise && type == DDoor::doorRaise)
				{
					if (door->m_Direction == -1)
					{
						door->m_Direction = 1;	// go back up
						door->DoorSound (true);	// [RH] Make noise
					}
					else if (GET_SPAC(line->flags) != SPAC_PUSH)
						// [RH] activate push doors don't go back down when you
						//		run into them (otherwise opening them would be
						//		a real pain).
					{
						if (!thing->player || thing->player->isbot)
							return false;	// JDC: bad guys never close doors
											//Added by MC: Neither do bots.

						door->m_Direction = -1;	// start going down immediately

						// [RH] If this sector doesn't have a specific sound
						// attached to it, start the door close sequence.
						// Otherwise, just let the current one continue.
						if (sec->seqType == -1)
						{
							door->DoorSound (false);
						}
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			return false;
		}
		if (new DDoor (sec, type, speed, delay, lightTag))
			rtn = true;
	}
	else
	{	// [RH] Remote door

		secnum = -1;
		while ((secnum = P_FindSectorFromTag (tag,secnum)) >= 0)
		{
			sec = &sectors[secnum];
			// if the ceiling already moving, don't start the door action
			if (sec->ceilingdata)
				continue;

			if (new DDoor (sec, type, speed, delay, lightTag))
				rtn = true;
		}
				
	}
	return rtn;
}


//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30 (sector_t *sec)
{
	fixed_t height;
	DDoor *door = new DDoor (sec);

	sec->special = 0;

	door->m_Sector = sec;
	door->m_Direction = 0;
	door->m_Type = DDoor::doorRaise;
	door->m_Speed = FRACUNIT*2;
	door->m_TopCountdown = 30 * TICRATE;
	height = sec->FindHighestFloorPoint (&door->m_BotSpot);
	door->m_BotDist = sec->ceilingplane.PointToDist (door->m_BotSpot, height);
	door->m_OldFloorDist = sec->floorplane.d;
	door->m_TopDist = sec->ceilingplane.d;
	door->m_LightTag = 0;
}

//
// Spawn a door that opens after 5 minutes
//
void P_SpawnDoorRaiseIn5Mins (sector_t *sec)
{
	sec->special = 0;
	new DDoor (sec, DDoor::doorRaiseIn5Mins, 2*FRACUNIT, TICRATE*30/7, 0);
}

// Strife's animated doors. Based on Doom's unused sliding doors, but slightly different.

TArray<FDoorAnimation> DoorAnimations;

// EV_SlidingDoor : slide a door horizontally
// (animate midtexture, then set noblocking line)
//

//
// Return index into "DoorAnimatinos" array for which door type to use
//
static int P_FindSlidingDoorType (int picnum)
{
	unsigned int i;

	for (i = 0; i < DoorAnimations.Size(); ++i)
	{
		if (picnum == DoorAnimations[i].BaseTexture)
			return i;
	}

	return -1;
}

void DAnimatedDoor::Tick ()
{
	FDoorAnimation &ani = DoorAnimations[m_WhichDoorIndex];

	switch (m_Status)
	{
	case Dead:
		m_Sector->ceilingdata = NULL;
		Destroy ();
		break;

	case Opening:
		if (!m_Timer--)
		{
			if (++m_Frame >= ani.NumTextureFrames)
			{
				// IF DOOR IS DONE OPENING...
				m_Line1->flags &= ~ML_BLOCKING;
				m_Line2->flags &= ~ML_BLOCKING;

				if (m_Delay == 0)
				{
					m_Sector->ceilingdata = NULL;
					Destroy ();
					break;
				}

				m_Timer = m_Delay;
				m_Status = Waiting;
			}
			else
			{
				// IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
				m_Timer = m_Speed;

				sides[m_Line1->sidenum[0]].midtexture =
				sides[m_Line1->sidenum[1]].midtexture =
				sides[m_Line2->sidenum[0]].midtexture =
				sides[m_Line2->sidenum[1]].midtexture =
					ani.TextureFrames[m_Frame];
			}
		}
		break;

	case Waiting:
		// IF DOOR IS DONE WAITING...
		if (!m_Timer--)
		{
			// CAN DOOR CLOSE?
			if (m_Sector->touching_thinglist != NULL)
			{
				m_Timer = m_Delay;
				break;
			}

			fixed_t topdist = m_Sector->ceilingplane.d;
			if (MoveCeiling (2048*FRACUNIT, m_BotDist, 0, -1) == crushed)
			{
				m_Timer = m_Delay;
				break;
			}

			MoveCeiling (2048*FRACUNIT, topdist, 1);

			m_Line1->flags |= ML_BLOCKING;
			m_Line2->flags |= ML_BLOCKING;
			if (ani.CloseSound != NULL)
			{
				SN_StartSequence (m_Sector, ani.CloseSound);
			}

			m_Status = Closing;
			m_Timer = m_Speed;
		}
		break;

	case Closing:
		if (!m_Timer--)
		{
			if (--m_Frame < 0)
			{
				// IF DOOR IS DONE CLOSING...
				MoveCeiling (2048*FRACUNIT, m_BotDist, -1);
				m_Sector->ceilingdata = NULL;
				break;
			}
			else
			{
				// IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
				m_Timer = m_Speed;

				sides[m_Line1->sidenum[0]].midtexture =
				sides[m_Line1->sidenum[1]].midtexture =
				sides[m_Line2->sidenum[0]].midtexture =
				sides[m_Line2->sidenum[1]].midtexture =
					ani.TextureFrames[m_Frame];
			}
		}
		break;
	}
}

IMPLEMENT_CLASS (DAnimatedDoor)

DAnimatedDoor::DAnimatedDoor ()
{
}

DAnimatedDoor::DAnimatedDoor (sector_t *sec)
	: DMovingCeiling (sec)
{
}

void DAnimatedDoor::Serialize (FArchive &arc)
{
	// If you have a pre-224 savegame with an active animated door, then you're out
	// of luck.
	if (SaveVersion < 224)
	{
		I_Error ("Can't load pre-2.0.93 savegames with active animated doors.\n");
	}
	Super::Serialize (arc);
	arc << m_Line1 << m_Line2
		<< m_Frame
		<< m_Timer
		<< m_BotDist
		<< m_Status
		<< m_Speed
		<< m_Delay;

	if (arc.IsStoring())
	{
		TexMan.WriteTexture (arc, DoorAnimations[m_WhichDoorIndex].BaseTexture);
	}
	else
	{
		int picnum = TexMan.ReadTexture (arc);
		m_WhichDoorIndex = P_FindSlidingDoorType (picnum);
		if (m_WhichDoorIndex == -1)
		{ // Oh no! The door animation doesn't exist anymore!
			m_WhichDoorIndex = 0;
		}
	}
}

DAnimatedDoor::DAnimatedDoor (sector_t *sec, line_t *line, int speed, int delay)
	: DMovingCeiling (sec)
{
	fixed_t topdist;
	int picnum;

	// The DMovingCeiling constructor automatically sets up an interpolation for us.
	// Stop it, since the ceiling is moving instantly here.
	stopinterpolation (INTERP_SectorCeiling, sec);
	m_WhichDoorIndex = P_FindSlidingDoorType (sides[line->sidenum[0]].toptexture);
	if (m_WhichDoorIndex < 0)
	{
		Printf ("EV_SlidingDoor: Textures are not defined for sliding door!");
		m_Status = Dead;
		return;
	}

	m_Line1 = line;
	m_Line2 = line;

	for (int i = 0; i < sec->linecount; ++i)
	{
		if (sec->lines[i] == line)
			continue;

		if (sides[sec->lines[i]->sidenum[0]].toptexture == sides[line->sidenum[0]].toptexture)
		{
			m_Line2 = sec->lines[i];
			break;
		}
	}

	picnum = sides[m_Line1->sidenum[0]].toptexture;
	sides[m_Line1->sidenum[0]].midtexture = picnum;
	sides[m_Line2->sidenum[0]].midtexture = picnum;

	topdist = TexMan[picnum]->GetHeight();
	topdist = m_Sector->ceilingplane.d - topdist * m_Sector->ceilingplane.c;

	m_Status = Opening;
	m_Speed = speed;
	m_Delay = delay;
	m_Timer = m_Speed;
	m_Frame = 0;
	m_Line1->flags |= ML_BLOCKING;
	m_Line2->flags |= ML_BLOCKING;
	m_BotDist = m_Sector->ceilingplane.d;
	MoveCeiling (2048*FRACUNIT, topdist, 1);
	if (DoorAnimations[m_WhichDoorIndex].OpenSound != NULL)
	{
		SN_StartSequence (m_Sector, DoorAnimations[m_WhichDoorIndex].OpenSound);
	}
}

//============================================================================
//
// EV_SlidingDoor
//
//============================================================================

bool EV_SlidingDoor (line_t *line, AActor *actor, int tag, int speed, int delay)
{
	sector_t *sec;
	int secnum;
	bool rtn;

	secnum = -1;
	rtn = false;

	if (tag == 0)
	{
		// Manual sliding door
		sec = line->backsector;

		// Make sure door isn't already being animated
		if (sec->ceilingdata != NULL)
		{
			if (actor->player == NULL)
				return false;

			if (sec->ceilingdata->IsA (RUNTIME_CLASS(DAnimatedDoor)))
			{
				DAnimatedDoor *door = static_cast<DAnimatedDoor *> (sec->ceilingdata);
				if (door->m_Status == DAnimatedDoor::Waiting)
				{
					door->m_Timer = door->m_Speed;
					door->m_Status = DAnimatedDoor::Closing;
					return true;
				}
			}
			return false;
		}
		if (P_FindSlidingDoorType (sides[line->sidenum[0]].toptexture) >= 0)
		{
			new DAnimatedDoor (sec, line, speed, delay);
			return true;
		}
		return false;
	}

	while ((secnum = P_FindSectorFromTag (tag, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->ceilingdata != NULL)
		{
			continue;
		}

		for (int i = 0; tag != 0 && i < sec->linecount; ++i)
		{
			line = sec->lines[i];
			if (line->backsector == NULL)
			{
				continue;
			}
			if (P_FindSlidingDoorType (sides[line->sidenum[0]].toptexture) >= 0)
			{
				rtn = true;
				new DAnimatedDoor (sec, line, speed, delay);
			}
		}
	}
	return rtn;
}

void P_ParseAnimatedDoor()
{
	const BITFIELD texflags = FTextureManager::TEXMAN_Overridable | FTextureManager::TEXMAN_TryAny;
	FDoorAnimation anim;
	TArray<int> frames;
	bool error = false;
	int v;

	SC_MustGetString();
	anim.BaseTexture = TexMan.CheckForTexture (sc_String, FTexture::TEX_Wall, texflags);
	anim.OpenSound = NULL;
	anim.CloseSound = NULL;

	if (anim.BaseTexture == -1)
	{
		error = true;
	}

	while (SC_GetString ())
	{
		if (SC_Compare ("opensound"))
		{
			SC_MustGetString ();
			anim.OpenSound = copystring (sc_String);
		}
		else if (SC_Compare ("closesound"))
		{
			SC_MustGetString ();
			anim.CloseSound = copystring (sc_String);
		}
		else if (SC_Compare ("pic"))
		{
			SC_MustGetString ();
			if (IsNum (sc_String))
			{
				v = atoi(sc_String) + anim.BaseTexture -1;
			}
			else
			{
				v = TexMan.CheckForTexture (sc_String, FTexture::TEX_Wall, texflags);
				if (v == -1 && anim.BaseTexture >= 0 && !error)
				{
					SC_ScriptError ("Unknown texture %s", sc_String);
				}
				frames.Push (v);
			}
		}
		else
		{
			SC_UnGet ();
			break;
		}
	}
	anim.TextureFrames = new int[frames.Size()];
	memcpy (anim.TextureFrames, &frames[0], sizeof(int) * frames.Size());
	anim.NumTextureFrames = frames.Size();
	if (!error)
	{
		DoorAnimations.Push (anim);
	}
}
