#ifndef __P_GRUBBER_H__
#define __P_GRUBBER_H__

#include "c_cvars.h"

// Shell ejecting
void P_EjectShell (AActor *start, int type, fixed_t dist);
void A_EjectShell (AActor *self);
void A_EjectBullet (AActor *self);

EXTERN_CVAR (Bool, cl_shellejecting);

// Weapon options
EXTERN_CVAR (Bool, cl_wpn_fist_thrust);
EXTERN_CVAR (Bool, cl_wpn_chainsaw_throw);
EXTERN_CVAR (Bool, cl_wpn_pistol_burst);
EXTERN_CVAR (Bool, cl_wpn_pistol_expuffs);
EXTERN_CVAR (Int, cl_wpn_pistol_spread);
EXTERN_CVAR (Bool, cl_wpn_shotgun_auto);
EXTERN_CVAR (Bool, cl_wpn_shotgun_expuffs);
EXTERN_CVAR (Int, cl_wpn_shotgun_spread);
EXTERN_CVAR (Bool, cl_wpn_sshotgun_fireone);
EXTERN_CVAR (Bool, cl_wpn_sshotgun_auto);
EXTERN_CVAR (Bool, cl_wpn_sshotgun_expuffs);
EXTERN_CVAR (Int, cl_wpn_sshotgun_spread);
EXTERN_CVAR (Bool, cl_wpn_chaingun_shell);
EXTERN_CVAR (Bool, cl_wpn_chaingun_expuffs);
EXTERN_CVAR (Int, cl_wpn_chaingun_spread);
EXTERN_CVAR (Int, cl_wpn_rocket_mode);
EXTERN_CVAR (Bool, cl_wpn_rocket_spreadfire);
EXTERN_CVAR (Bool, cl_wpn_rocket_bounce);
EXTERN_CVAR (Bool, cl_wpn_rocket_guide);
EXTERN_CVAR (Bool, cl_wpn_rocket_killable);
EXTERN_CVAR (Bool, cl_wpn_rocket_bobbing);
EXTERN_CVAR (Int, cl_wpn_plasma_mode);
EXTERN_CVAR (Bool, cl_wpn_plasma_spreadfire);
EXTERN_CVAR (Bool, cl_wpn_plasma_green);
EXTERN_CVAR (Bool, cl_wpn_plasma_bounce);
EXTERN_CVAR (Bool, cl_wpn_plasma_bobbing);
EXTERN_CVAR (Bool, cl_wpn_plasma_killable);
EXTERN_CVAR (Bool, cl_wpn_bfg_spreadfire);
EXTERN_CVAR (Bool, cl_wpn_bfg_mini);
EXTERN_CVAR (Bool, cl_wpn_bfg_bounce);
EXTERN_CVAR (Bool, cl_wpn_bfg_bobbing);
EXTERN_CVAR (Bool, cl_wpn_bfg_killable);

void A_Guide (AActor *);
void P_SpawnExplosion (AActor *self);

// Monster options
EXTERN_CVAR (Int, cl_mon_zombieman_fire);
EXTERN_CVAR (Int, cl_mon_shotgunguy_fire);
EXTERN_CVAR (Int, cl_mon_heavyweapondude_fire);
EXTERN_CVAR (Int, cl_mon_imp_fire);
//EXTERN_CVAR (Int, cl_mon_demon_fire);
EXTERN_CVAR (Int, cl_mon_lostsoul_fire);
EXTERN_CVAR (Int, cl_mon_cacodemon_fire);
EXTERN_CVAR (Int, cl_mon_hellknight_fire);
EXTERN_CVAR (Int, cl_mon_baronofhell_fire);
EXTERN_CVAR (Int, cl_mon_arachnotron_fire);
EXTERN_CVAR (Int, cl_mon_painelemental_fire);
EXTERN_CVAR (Int, cl_mon_revenant_fire);
EXTERN_CVAR (Int, cl_mon_mancubus_fire);
//EXTERN_CVAR (Int, cl_mon_archvile_fire);
EXTERN_CVAR (Int, cl_mon_spiderboss_fire);
EXTERN_CVAR (Int, cl_mon_cyberdemon_fire);
EXTERN_CVAR (Int, cl_mon_ss_fire);

AActor *P_MonsterFire (int cvar, AActor *self, int angle, int slope);

// Linear map teleporter stuff
extern bool PosSaved;
extern angle_t EndAngle;
extern fixed_t EndPitch;
extern fixed_t EndDist;
extern angle_t PlrAngle;
extern fixed_t PlrPitch;
extern angle_t PlrMomAng;
extern fixed_t PlrMomDist;
extern fixed_t PlrMomZ;
extern pspdef_t PlrPsp[NUMPSPRITES];

void P_SavePosData (WORD tid);
//void P_LoadPosData ();

#endif	// __P_GRUBBER_H__