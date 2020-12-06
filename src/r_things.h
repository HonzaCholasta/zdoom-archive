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
// DESCRIPTION:
//		Rendering of moving objects, sprites.
//
//-----------------------------------------------------------------------------


#ifndef __R_THINGS__
#define __R_THINGS__

// [RH] Particle details
struct particle_s
{
	fixed_t	x,y,z;
	fixed_t velx,vely,velz;
	fixed_t accx,accy,accz;
	byte	ttl;
	byte	trans;
	byte	size;
	byte	fade;
	int		color;
	WORD	tnext;
	WORD	snext;
};
typedef struct particle_s particle_t;

extern int	NumParticles;
extern int	ActiveParticles;
extern int	InactiveParticles;
extern particle_t *Particles;

const WORD NO_PARTICLE = 0xffff;

inline particle_t *NewParticle (void)
{
	particle_t *result = NULL;
	if (InactiveParticles != NO_PARTICLE)
	{
		result = Particles + InactiveParticles;
		InactiveParticles = result->tnext;
		result->tnext = ActiveParticles;
		ActiveParticles = (int)(result - Particles);
	}
	return result;
}

void R_InitParticles ();
void R_ClearParticles ();
void R_DrawParticle (vissprite_t *);
void R_ProjectParticle (particle_t *, const sector_t *sector, int shade, int fakeside);
void R_FindParticleSubsectors ();

extern TArray<WORD>		ParticlesInSubsec;

extern int MaxVisSprites;

extern vissprite_t		*vissprites, *firstvissprite;
extern vissprite_t* 	vissprite_p;
extern vissprite_t		vsprsortedhead;

// Constant arrays used for psprite clipping
//	and initializing clipping.
extern short			negonearray[MAXWIDTH];
extern short			screenheightarray[MAXWIDTH];

// vars for R_DrawMaskedColumn
extern short*			mfloorclip;
extern short*			mceilingclip;
extern fixed_t			spryscale;
extern fixed_t			sprtopscreen;
extern bool				sprflipvert;

extern fixed_t			pspritexscale;
extern fixed_t			pspriteyscale;
extern fixed_t			pspritexiscale;


void R_DrawMaskedColumn (column_t* column);
void R_DrawMaskedColumn2 (column2_t* column);	// [RH]


void R_CacheSprite (spritedef_t *sprite);
void R_SortVisSprites (void);
void R_AddSprites (sector_t *sec, int lightlevel, int fakeside);
void R_AddPSprites ();
void R_DrawSprites ();
void R_InitSprites ();
void R_ClearSprites ();
void R_DrawMasked ();

void R_ClipVisSprite (vissprite_t *vis, int xl, int xh);


#endif
