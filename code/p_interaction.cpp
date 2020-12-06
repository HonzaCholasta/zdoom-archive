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
//		Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------




// Data.
#include "doomdef.h"
#include "gstrings.h"

#include "doomstat.h"

#include "m_random.h"
#include "i_system.h"

#include "am_map.h"

#include "c_console.h"
#include "c_dispatch.h"

#include "p_local.h"

#include "p_inter.h"
#include "p_lnspec.h"
#include "p_effect.h"
#include "p_acs.h"

#include "b_bot.h"	//Added by MC:

#include "a_doomglobal.h"
#include "a_hereticglobal.h"
#include "a_hexenglobal.h"
#include "a_sharedglobal.h"
#include "a_pickups.h"
#include "gi.h"

#include "sbar.h"

CVAR (Bool, cl_showsprees, true, CVAR_ARCHIVE)
CVAR (Bool, cl_showmultikills, true, CVAR_ARCHIVE)

//
// GET STUFF
//

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing (AActor *special, AActor *toucher)
{
	fixed_t delta = special->z - toucher->z;

	if (delta > toucher->height || delta < -32*FRACUNIT)
	{
		// out of reach
		return;
	}

	// Dead thing touching.
	// Can happen with a sliding player corpse.
	if (toucher->health <= 0)
		return;

	//Added by MC: Finished with this destination.
	if (toucher->player->isbot && special == toucher->player->dest)
	{
		toucher->player->prev = toucher->player->dest;
    	toucher->player->dest = NULL;
	}

	if (special->IsKindOf (RUNTIME_CLASS (AInventory)))
	{
		static_cast<AInventory *>(special)->Touch (toucher);
	}
	else
	{
		I_Error ("P_SpecialThing: Unknown gettable thing (%s)", RUNTIME_TYPE (special)->Name);
	}
}


// [RH]
// SexMessage: Replace parts of strings with gender-specific pronouns
//
// The following expansions are performed:
//		%g -> he/she/it
//		%h -> him/her/it
//		%p -> his/her/its
//		%o -> other (victim)
//		%k -> killer
//
void SexMessage (const char *from, char *to, int gender, const char *victim, const char *killer)
{
	static const char *genderstuff[3][3] =
	{
		{ "he",  "him", "his" },
		{ "she", "her", "her" },
		{ "it",  "it",  "its" }
	};
	static const int gendershift[3][3] =
	{
		{ 2, 3, 3 },
		{ 3, 3, 3 },
		{ 2, 2, 3 }
	};
	int gendermsg;
	const char *subst = NULL;

	do
	{
		if (*from != '%')
		{
			*to++ = *from;
		}
		else
		{
			switch (from[1])
			{
			default:	gendermsg = -1;	break;
			case 'g':	gendermsg = 0;	break;
			case 'h':	gendermsg = 1;	break;
			case 'p':	gendermsg = 2;	break;
			case 'o':	subst = victim;	break;
			case 'k':	subst = killer;	break;
			}
			if (subst != NULL)
			{
				int len = strlen (subst);
				memcpy (to, subst, len);
				to += len;
				from++;
				subst = NULL;
			}
			else if (gendermsg < 0)
			{
				*to++ = '%';
			}
			else
			{
				strcpy (to, genderstuff[gender][gendermsg]);
				to += gendershift[gender][gendermsg];
				from++;
			}
		}
	} while (*from++);
}

// [RH]
// ClientObituary: Show a message when a player dies
//
void ClientObituary (AActor *self, AActor *inflictor, AActor *attacker)
{
	int	 mod;
	const char *message;
	int messagenum;
	char gendermessage[1024];
	BOOL friendly;
	int  gender;

	if (self->player == NULL)
		return;

	gender = self->player->userinfo.gender;

	// Treat voodoo dolls as unknown deaths
	if (inflictor && inflictor->player == self->player)
		MeansOfDeath = MOD_UNKNOWN;

	if (multiplayer && !*deathmatch)
		MeansOfDeath |= MOD_FRIENDLY_FIRE;

	friendly = MeansOfDeath & MOD_FRIENDLY_FIRE;
	mod = MeansOfDeath & ~MOD_FRIENDLY_FIRE;
	message = NULL;
	messagenum = 0;

	switch (mod)
	{
	case MOD_SUICIDE:		messagenum = OB_SUICIDE;	break;
	case MOD_FALLING:		messagenum = OB_FALLING;	break;
	case MOD_CRUSH:			messagenum = OB_CRUSH;		break;
	case MOD_EXIT:			messagenum = OB_EXIT;		break;
	case MOD_WATER:			messagenum = OB_WATER;		break;
	case MOD_SLIME:			messagenum = OB_SLIME;		break;
	case MOD_LAVA:			messagenum = OB_LAVA;		break;
	case MOD_BARREL:		messagenum = OB_BARREL;		break;
	case MOD_SPLASH:		messagenum = OB_SPLASH;		break;
	}

	if (messagenum)
		message = GStrings(messagenum);

	if (attacker != NULL && message == NULL)
	{
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_R_SPLASH:	messagenum = OB_R_SPLASH;		break;
			case MOD_ROCKET:	messagenum = OB_ROCKET;			break;
			default:			messagenum = OB_KILLEDSELF;		break;
			}
			message = GStrings(messagenum);
		}
		else if (attacker->player == NULL)
		{
			if (mod == MOD_HIT)
				message = attacker->GetHitObituary ();
			else
				message = attacker->GetObituary ();
		}
	}

	if (message)
	{
		SexMessage (message, gendermessage, gender,
			self->player->userinfo.netname, self->player->userinfo.netname);
		Printf (PRINT_MEDIUM, "%s\n", gendermessage);
		return;
	}

	if (attacker != NULL && attacker->player != NULL)
	{
		if (friendly)
		{
			int rnum = P_Random (pr_obituary);

			attacker->player->fragcount -= 2;
			attacker->player->frags[attacker->player - players]++;
			self = attacker;
			gender = self->player->userinfo.gender;

			messagenum = OB_FRIENDLY1 + (rnum & 3);
		}
		else
		{
			switch (mod)
			{
			case MOD_FIST:			messagenum = OB_MPFIST;			break;
			case MOD_CHAINSAW:		messagenum = OB_MPCHAINSAW;		break;
			case MOD_PISTOL:		messagenum = OB_MPPISTOL;		break;
			case MOD_SHOTGUN:		messagenum = OB_MPSHOTGUN;		break;
			case MOD_SSHOTGUN:		messagenum = OB_MPSSHOTGUN;		break;
			case MOD_CHAINGUN:		messagenum = OB_MPCHAINGUN;		break;
			case MOD_ROCKET:		messagenum = OB_MPROCKET;		break;
			case MOD_R_SPLASH:		messagenum = OB_MPR_SPLASH;		break;
			case MOD_PLASMARIFLE:	messagenum = OB_MPPLASMARIFLE;	break;
			case MOD_BFG_BOOM:		messagenum = OB_MPBFG_BOOM;		break;
			case MOD_BFG_SPLASH:	messagenum = OB_MPBFG_SPLASH;	break;
			case MOD_TELEFRAG:		messagenum = OB_MPTELEFRAG;		break;
			case MOD_RAILGUN:		messagenum = OB_RAILGUN;		break;
			}
		}
		if (messagenum)
			message = GStrings(messagenum);
	}

	if (message)
	{
		SexMessage (message, gendermessage, gender,
			self->player->userinfo.netname, attacker->player->userinfo.netname);
		Printf (PRINT_MEDIUM, "%s\n", gendermessage);
		return;
	}

	SexMessage (GStrings(OB_DEFAULT), gendermessage, gender,
		self->player->userinfo.netname, self->player->userinfo.netname);
	Printf (PRINT_MEDIUM, "%s\n", gendermessage);
}


//
// KillMobj
//
EXTERN_CVAR (Int, fraglimit)
extern void P_ExplodeMissile (AActor *);

void AActor::Die (AActor *source, AActor *inflictor)
{
	if (flags & MF_MISSILE)
	{ // [RH] When missiles die, they just explode
		P_ExplodeMissile (this);
		return;
	}
	flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY|MF_NOGRAVITY);
	flags |= MF_CORPSE|MF_DROPOFF;
	flags2 &= ~MF2_PASSMOBJ;
	height >>= 2;

	// [RH] If the thing has a special, execute and remove it
	//		Note that the thing that killed it is considered
	//		the activator of the script.
	if ((flags & MF_COUNTKILL) && special)
	{
		LineSpecials[special] (NULL, source, args[0], args[1], args[2], args[3], args[4]);
		special = 0;
	}

	if (source && source->player)
	{
		if (flags & MF_COUNTKILL)
		{ // count for intermission
			source->player->killcount++;
			level.killed_monsters++;
		}

		// Don't count any frags at level start, because they're just telefrags
		// resulting from insufficient deathmatch starts, and it wouldn't be
		// fair to count them toward a player's score.
		if (player && level.time)
		{
			source->player->frags[player - players]++;
			if (player == source->player)	// [RH] Cumulative frag count
			{
				char buff[256];

				player->fragcount--;
				if (*deathmatch && player->spreecount >= 5 && *cl_showsprees)
				{
					SexMessage (GStrings(SPREEKILLSELF), buff,
						player->userinfo.gender, player->userinfo.netname,
						player->userinfo.netname);
					StatusBar->AttachMessage (new FHUDMessageFadeOut (buff,
							1.5f, 0.2f, CR_WHITE, 3.f, 0.5f), 'KSPR');
				}
			}
			else
			{
				++source->player->fragcount;
				++source->player->spreecount;
				if (*deathmatch && *cl_showsprees)
				{
					const char *spreemsg;
					char buff[256];

					switch (source->player->spreecount)
					{
					case 5:
						spreemsg = GStrings(SPREE5);
						break;
					case 10:
						spreemsg = GStrings(SPREE10);
						break;
					case 15:
						spreemsg = GStrings(SPREE15);
						break;
					case 20:
						spreemsg = GStrings(SPREE20);
						break;
					case 25:
						spreemsg = GStrings(SPREE25);
						break;
					default:
						spreemsg = NULL;
						break;
					}

					if (spreemsg == NULL && player->spreecount >= 5)
					{
						SexMessage (GStrings(SPREEOVER), buff, player->userinfo.gender,
							player->userinfo.netname, source->player->userinfo.netname);
						StatusBar->AttachMessage (new FHUDMessageFadeOut (buff,
							1.5f, 0.2f, CR_WHITE, 3.f, 0.5f), 'KSPR');
					}
					else if (spreemsg != NULL)
					{
						SexMessage (spreemsg, buff, player->userinfo.gender,
							player->userinfo.netname, source->player->userinfo.netname);
						StatusBar->AttachMessage (new FHUDMessageFadeOut (buff,
							1.5f, 0.2f, CR_WHITE, 3.f, 0.5f), 'KSPR');
					}
				}
			}

			// [RH] Multikills
			source->player->multicount++;
			if (source->player->lastkilltime > 0)
			{
				if (source->player->lastkilltime < level.time - 3*TICRATE)
				{
					source->player->multicount = 1;
				}

				if (*deathmatch &&
					source->player->mo == players[consoleplayer].camera &&
					*cl_showmultikills)
				{
					const char *multimsg;

					switch (source->player->multicount)
					{
					case 1:
						multimsg = NULL;
						break;
					case 2:
						multimsg = GStrings(MULTI2);
						break;
					case 3:
						multimsg = GStrings(MULTI3);
						break;
					case 4:
						multimsg = GStrings(MULTI4);
						break;
					default:
						multimsg = GStrings(MULTI5);
						break;
					}
					if (multimsg != NULL)
					{
						char buff[256];

						SexMessage (multimsg, buff, player->userinfo.gender,
							player->userinfo.netname, source->player->userinfo.netname);
						StatusBar->AttachMessage (new FHUDMessageFadeOut (buff,
							1.5f, 0.8f, CR_RED, 3.f, 0.5f), 'MKIL');
					}
				}
			}
			source->player->lastkilltime = level.time;

			// [RH] Implement fraglimit
			if (*deathmatch && *fraglimit &&
				*fraglimit == source->player->fragcount)
			{
				Printf ("%s\n", GStrings(TXT_FRAGLIMIT));
				G_ExitLevel (0);
			}
		}
	}
	else if (!multiplayer && (flags & MF_COUNTKILL) )
	{
		// count all monster deaths,
		// even those caused by other monsters
		players[0].killcount++;
		level.killed_monsters++;
	}
	
	if (player)
	{
		// [RH] Death messages
		ClientObituary (this, inflictor, source);

		// Death script execution, care of Skull Tag
		if (level.behavior != NULL)
		{
			level.behavior->StartTypedScripts (SCRIPT_Death, this);
		}

		// [RH] Force a delay between death and respawn
		player->respawn_time = level.time + TICRATE;

		//Added by MC: Respawn bots
		if (bglobal.botnum && consoleplayer == Net_Arbitrator && !demoplayback)
		{
			if (player->isbot)
				player->t_respawn = (P_Random(pr_botrespawn)%15)+((bglobal.botnum-1)*2)+TICRATE+1;

			//Added by MC: Discard enemies.
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				if (players[i].isbot && this == players[i].enemy)
				{
					if (players[i].dest ==  players[i].enemy)
						players[i].dest = NULL;
					players[i].enemy = NULL;
				}
			}

			player->spreecount = 0;
			player->multicount = 0;
		}

		// count environment kills against you
		if (!source)
		{
			player->frags[player - players]++;
			player->fragcount--;	// [RH] Cumulative frag count
		}
						
		flags &= ~MF_SOLID;
		player->playerstate = PST_DEAD;
		P_DropWeapon (player);
		if (this == players[consoleplayer].camera && automapactive)
		{
			// don't die in auto map, switch view prior to dying
			AM_Stop ();
		}
	}

	if (flags2 & MF2_FIREDAMAGE && BDeathState)
	{ // Burn death
		SetState (BDeathState);
	}
	else if (flags2 & MF2_ICEDAMAGE && IDeathState)
	{ // Ice death
		SetState (IDeathState);
	}
	else if (health < -GetDefault()->health && XDeathState)
	{ // Extreme death
		SetState (XDeathState);
	}
	else
	{ // Normal death
		SetState (DeathState);
	}

	tics -= P_Random (pr_killmobj) & 3;
	if (tics < 1)
		tics = 1;
}




//---------------------------------------------------------------------------
//
// PROC P_AutoUseHealth
//
//---------------------------------------------------------------------------

void P_AutoUseHealth(player_t *player, int saveHealth)
{
	int i;
	int count;
	int normalCount;
	int superCount;

	normalCount = player->inventory[arti_health];
	superCount = player->inventory[arti_superhealth];
	if ((*gameskill == sk_baby) && (normalCount*25 >= saveHealth))
	{ // Use quartz flasks
		count = (saveHealth+24)/25;
		for(i = 0; i < count; i++)
		{
			player->health += 25;
			P_PlayerRemoveArtifact (player, arti_health);
		}
	}
	else if (superCount*100 >= saveHealth)
	{ // Use mystic urns
		count = (saveHealth+99)/100;
		for(i = 0; i < count; i++)
		{
			player->health += 100;
			P_PlayerRemoveArtifact (player, arti_superhealth);
		}
	}
	else if ((*gameskill == sk_baby)
		&& (superCount*100+normalCount*25 >= saveHealth))
	{ // Use mystic urns and quartz flasks
		count = (saveHealth+24)/25;
		saveHealth -= count*25;
		for(i = 0; i < count; i++)
		{
			player->health += 25;
			P_PlayerRemoveArtifact (player, arti_health);
		}
		count = (saveHealth+99)/100;
		for(i = 0; i < count; i++)
		{
			player->health += 100;
			P_PlayerRemoveArtifact (player, arti_superhealth);
		}
	}
	player->mo->health = player->health;
}

/*
=================
=
= P_DamageMobj
=
= Damages both enemies and players
= inflictor is the thing that caused the damage
= 		creature or missile, can be NULL (slime, etc)
= source is the thing to target after taking damage
=		creature or NULL
= Source and inflictor are the same for melee attacks
= source can be null for barrel explosions and other environmental stuff
==================
*/

int MeansOfDeath;

void P_DamageMobj (AActor *target, AActor *inflictor, AActor *source, int damage, int mod, int flags)
{
	unsigned ang;
	int saved;
	fixed_t savedPercent;
	player_t *player;
	AActor *master;
	fixed_t thrust;
	int temp;
	int i;

	if (target == NULL || !(target->flags & MF_SHOOTABLE))
	{ // Shouldn't happen
		return;
	}
	if (target->health <= 0)
	{
		if (inflictor && inflictor->flags2 & MF2_ICEDAMAGE)
		{
			return;
		}
		else if (target->flags & MF_ICECORPSE) // frozen
		{
			target->tics = 1;
			target->momx = target->momy = 0;
		}
		return;
	}
	MeansOfDeath = mod;
	// [RH] Andy Baker's Stealth monsters
	if (target->flags & MF_STEALTH)
	{
		target->alpha = OPAQUE;
		target->visdir = -1;
	}
	if (target->flags & MF_SKULLFLY)
	{
		if (gameinfo.gametype == GAME_Heretic &&
			target->IsKindOf (RUNTIME_CLASS(AMinotaur)))
		{ // Minotaur is invulnerable during charge attack
			return;
		}
		target->momx = target->momy = target->momz = 0;
	}
	if (target->flags2 & MF2_DORMANT)
	{
		// Invulnerable, and won't wake up
		return;
	}
	player = target->player;
	if (player && *gameskill == sk_baby)
	{
		// Take half damage in trainer mode
		damage >>= 1;
	}
	// Special damage types
	if (inflictor)
	{
		damage = inflictor->DoSpecialDamage (target, damage);
		if (damage == -1)
			return;
	}
	// Push the target unless the source's weapon's kickback is 0.
	// (i.e. Guantets/Chainsaw)
	if (inflictor
		&& !(target->flags & MF_NOCLIP)
		&& (!source || !source->player || !(inflictor->flags2 & MF2_NODMGTHRUST)))
	{
		int kickback;

		if (!source || !source->player)
			kickback = gameinfo.defKickback;
		else if (source->player->powers[pw_weaponlevel2])
			kickback = wpnlev2info[source->player->readyweapon]->kickback;
		else
			kickback = wpnlev1info[source->player->readyweapon]->kickback;

		if (kickback)
		{
			ang = R_PointToAngle2 (inflictor->x, inflictor->y,
				target->x, target->y);
			thrust = damage*(FRACUNIT>>3)*kickback / target->Mass;
			// make fall forwards sometimes
			if ((damage < 40) && (damage > target->health)
				 && (target->z - inflictor->z > 64*FRACUNIT)
				 && (P_Random (pr_damagemobj)&1))
			{
				ang += ANG180;
				thrust *= 4;
			}
			ang >>= ANGLETOFINESHIFT;
			if (source && source->player && (source == inflictor)
				&& source->player->powers[pw_weaponlevel2]
				&& source->player->readyweapon == wp_staff)
			{
				// Staff power level 2
				target->momx += FixedMul (10*FRACUNIT, finecosine[ang]);
				target->momy += FixedMul (10*FRACUNIT, finesine[ang]);
				if (!(target->flags & MF_NOGRAVITY))
				{
					target->momz += 5*FRACUNIT;
				}
			}
			else
			{
				target->momx += FixedMul (thrust, finecosine[ang]);
				target->momy += FixedMul (thrust, finesine[ang]);
			}
			// killough 11/98: thrust objects hanging off ledges
			if (target->flags & MF_FALLING && target->gear >= MAXGEAR)
			{
				target->gear = 0;
			}
		}
	}

	//
	// player specific
	//
	if (player)
	{
        //Added by MC: Lets bots look allround for enemies if they survive an ambush.
        if (player->isbot)
		{
            player->allround = true;
		}

		// end of game hell hack
		if ((target->subsector->sector->special & 255) == dDamage_End
			&& damage >= target->health)
		{
			damage = target->health - 1;
		}

		if (damage < 1000 && ((target->player->cheats & CF_GODMODE)
			|| target->player->powers[pw_invulnerability]))
		{
			return;
		}

		// [RH] Avoid friendly fire if enabled
		if (target != source && target->IsTeammate (source))
		{
			MeansOfDeath |= MOD_FRIENDLY_FIRE;
			if (damage < 10000)
			{ // Still allow telefragging :-(
				damage = (int)((float)damage * *teamdamage);
				if (damage <= 0)
					return;
			}
		}

		if (gameinfo.gametype != GAME_Hexen)
		{
			if (player->armortype && !(flags & DMG_NO_ARMOR))
			{
				if (gameinfo.gametype == GAME_Doom)
				{
					saved = (player->armortype == deh.GreenAC) ?
						damage/3 : damage/2;
				}
				else
				{
					saved = (player->armortype == 1) ?
						damage >> 1 : (damage >> 1) + (damage >> 2);
				}
				if (player->armorpoints[0] <= saved)
				{
					// armor is used up
					saved = player->armorpoints[0];
					player->armortype = 0;
				}
				player->armorpoints[0] -= saved;
				damage -= saved;
			}
		}
		else if (!(flags & DMG_NO_ARMOR))	// && GAME_Hexen
		{
			savedPercent = player->mo->GetAutoArmorSave ()
				+player->armorpoints[(int)ARMOR_ARMOR]
				+player->armorpoints[(int)ARMOR_SHIELD]
				+player->armorpoints[(int)ARMOR_HELMET]
				+player->armorpoints[(int)ARMOR_AMULET];
			if (savedPercent)
			{ // armor absorbed some damage
				if (savedPercent > 100*FRACUNIT)
				{
					savedPercent = 100*FRACUNIT;
				}
				for (i = 0; i < NUMARMOR; i++)
				{
					if (player->armorpoints[i])
					{
						player->armorpoints[i] -= 
							Scale(damage<<FRACBITS,
							player->mo->GetArmorIncrement (i), 300*FRACUNIT);
						if (player->armorpoints[i] < 2*FRACUNIT)
						{
							player->armorpoints[i] = 0;
						}
					}
				}
				saved = Scale (damage<<FRACBITS, savedPercent,
					100*FRACUNIT);
				if (saved > savedPercent*2)
				{	
					saved = savedPercent*2;
				}
				damage -= saved>>FRACBITS;
			}
		}
		if (damage >= player->health
			&& ((*gameskill == sk_baby) || *deathmatch)
			&& !player->morphTics)
		{ // Try to use some inventory health
			P_AutoUseHealth (player, damage - player->health + 1);
		}
		player->health -= damage;		// mirror mobj health here for Dave
		if (player->health < 0)
		{
			player->health = 0;
		}
		player->attacker = source;
		player->damagecount += damage;	// add damage after armor / invuln
		if (player->damagecount > 100)
		{
			player->damagecount = 100;	// teleport stomp does 10k points...
		}
		temp = damage < 100 ? damage : 100;
		if (player == &players[consoleplayer])
		{
			I_Tactile (40,10,40+temp*2);
		}
	}
	
	//
	// do the damage
	//
	target->health -= damage;	
	if (target->health <= 0)
	{ // Death
		target->special1 = damage;
		if (source && target->IsKindOf (RUNTIME_CLASS(AExplosiveBarrel))
			&& !source->IsKindOf (RUNTIME_CLASS(AExplosiveBarrel)))
		{ // Make sure players get frags for chain-reaction kills
			target->target = source;
		}
		// check for special fire damage or ice damage deaths
		if ((inflictor && (inflictor->flags2 & MF2_FIREDAMAGE)) ||
			(flags & DMG_FIRE_DAMAGE))
		{
			if (player && !player->morphTics)
			{ // Check for flame death
				if (!inflictor ||
					((target->health > -50) && (damage > 25)) ||
					!inflictor->IsKindOf (RUNTIME_CLASS(APhoenixFX1)))
				{
					target->flags2 |= MF2_FIREDAMAGE;
				}
			}
			else
			{
				target->flags2 |= MF2_FIREDAMAGE;
			}
		}
		else if ((inflictor && inflictor->flags2 & MF2_ICEDAMAGE) ||
			(flags & DMG_ICE_DAMAGE))
		{
			target->flags2 |= MF2_ICEDAMAGE;
		}
		if (source && source->IsKindOf (RUNTIME_CLASS (ADarkServant)))
		{ // Minotaur's kills go to his master
			master = static_cast<ADarkServant *>(source)->master;
			// Make sure still alive and not a pointer to fighter head
			if (master->player && (master->player->mo == master))
			{
				source = master;
			}
		}
		if (source && (source->player) &&
			(source->player->readyweapon == wp_fsword ||
			 source->player->readyweapon == wp_choly ||
			 source->player->readyweapon == wp_mstaff))
		{
			// Always extreme death from fourth weapon
			target->health = -5000;
		}
		target->Die (source, inflictor);
		return;
	}
	if ((P_Random (pr_damagemobj) < target->PainChance)
		 && !(target->flags & MF_SKULLFLY))
	{
		if (inflictor && inflictor->IsKindOf (RUNTIME_CLASS(ALightning)))
		{
			if (P_Random() < 96)
			{
				target->flags |= MF_JUSTHIT; // fight back!
				target->SetState (target->PainState);
			}
			else
			{ // "electrocute" the target
				target->renderflags |= RF_FULLBRIGHT;
				if (target->flags & MF_COUNTKILL && P_Random() < 128)
				{
					target->Howl ();
				}
			}
		}
		else
		{
			target->flags |= MF_JUSTHIT; // fight back!
			target->SetState (target->PainState);	
			if (inflictor && inflictor->IsKindOf (RUNTIME_CLASS(APoisonCloud)))
			{
				if (target->flags & MF_COUNTKILL && P_Random() < 128)
				{
					target->Howl ();
				}
			}
		}
	}
	target->reactiontime = 0;			// we're awake now...	
	if (source && source != target && !(source->flags3 & MF3_NOTARGET)
		&& !source->IsKindOf (RUNTIME_CLASS(AArchvile))
		&& (!target->threshold || target->IsKindOf (RUNTIME_CLASS(AArchvile)))
		&& target->NewTarget (source))
	{
		// Target actor is not intent on another actor,
		// so make him chase after source

		// killough 2/15/98: remember last enemy, to prevent
		// sleeping early; 2/21/98: Place priority on players

		if (!target->lastenemy || !target->lastenemy->player ||
			 target->lastenemy->health <= 0)
			target->lastenemy = target->target; // remember last enemy - killough

		target->target = source;
		target->threshold = BASETHRESHOLD;
		if (target->state == target->SpawnState
			&& target->SeeState != NULL)
		{
			target->SetState (target->SeeState);
		}
	}
}

//==========================================================================
//
// P_PoisonDamage - Similar to P_DamageMobj
//
//==========================================================================

void P_PoisonDamage (player_t *player, AActor *source, int damage,
	bool playPainSound)
{
	AActor *target;
	AActor *inflictor;

	target = player->mo;
	inflictor = source;
	if (target->health <= 0)
	{
		return;
	}
	if (target->flags2&MF2_INVULNERABLE && damage < 10000)
	{ // target is invulnerable
		return;
	}
	if (player && *gameskill == sk_baby)
	{
		// Take half damage in trainer mode
		damage >>= 1;
	}
	if(damage < 1000 && ((player->cheats&CF_GODMODE)
		|| player->powers[pw_invulnerability]))
	{
		return;
	}
	if (damage >= player->health
		&& ((*gameskill == sk_baby) || *deathmatch)
		&& !player->morphTics)
	{ // Try to use some inventory health
		P_AutoUseHealth (player, damage - player->health+1);
	}
	player->health -= damage; // mirror mobj health here for Dave
	if (player->health < 0)
	{
		player->health = 0;
	}
	player->attacker = source;

	//
	// do the damage
	//
	target->health -= damage;
	if (target->health <= 0)
	{ // Death
		target->special1 = damage;
		if (player && inflictor && !player->morphTics)
		{ // Check for flame death
			if ((inflictor->flags2&MF2_FIREDAMAGE)
				&& (target->health > -50) && (damage > 25))
			{
				target->flags2 |= MF2_FIREDAMAGE;
			}
			if (inflictor->flags2&MF2_ICEDAMAGE)
			{
				target->flags2 |= MF2_ICEDAMAGE;
			}
		}
		target->Die (source, source);
		return;
	}
	if (!(level.time&63) && playPainSound)
	{
		target->SetState (target->PainState);
	}
/*
	if((P_Random() < target->info->painchance)
		&& !(target->flags&MF_SKULLFLY))
	{
		target->flags |= MF_JUSTHIT; // fight back!
		P_SetMobjState(target, target->info->painstate);
	}
*/
}

BOOL CheckCheatmode ();

CCMD (kill)
{
	if (argc > 1 && !stricmp (argv[1], "monsters"))
	{
		// Kill all the monsters
		if (CheckCheatmode ())
			return;

		Net_WriteByte (DEM_GENERICCHEAT);
		Net_WriteByte (CHT_MASSACRE);
	}
	else
	{
		// Kill the player
		Net_WriteByte (DEM_SUICIDE);
	}
	C_HideConsole ();
}
