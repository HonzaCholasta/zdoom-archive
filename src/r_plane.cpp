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
//		Here is a core component: drawing the floors and ceilings,
//		 while maintaining a per column clipping list only.
//		Moreover, the sky areas have to be determined.
//
// MAXVISPLANES is no longer a limit on the number of visplanes,
// but a limit on the number of hash slots; larger numbers mean
// better performance usually but after a point they are wasted,
// and memory and time overheads creep in.
//
// Lee Killough
//
// [RH] Further modified to significantly increase accuracy and add slopes.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "templates.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"

#include "m_alloc.h"
#include "v_video.h"
#include "vectors.h"
#include "a_sharedglobal.h"

static void R_DrawSkyStriped (visplane_t *pl);

EXTERN_CVAR (Bool, r_particles);

planefunction_t 		floorfunc;
planefunction_t 		ceilingfunc;

// Here comes the obnoxious "visplane".
#define MAXVISPLANES 128    /* must be a power of 2 */

// [RH] Allocate one extra for sky box planes.
static visplane_t		*visplanes[MAXVISPLANES+1];	// killough
static visplane_t		*freetail;					// killough
static visplane_t		**freehead = &freetail;		// killough

visplane_t 				*floorplane;
visplane_t 				*ceilingplane;

static bool				r_InSkyBox;

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+((height).d)*7) & (MAXVISPLANES-1))

//
// opening
//

size_t					maxopenings;
short					*openings;
ptrdiff_t				lastopening;


//
// Clip values are the solid pixel bounding the range.
//	floorclip starts out SCREENHEIGHT and is just outside the range
//	ceilingclip starts out 0 and is just inside the range
//
short					floorclip[MAXWIDTH];
short					ceilingclip[MAXWIDTH];

//
// texture mapping
//

static fixed_t			planeheight;

extern "C" {
//
// spanend holds the end of a plane span in each screen row
//
short					spanend[MAXHEIGHT];
byte					*tiltlighting[MAXWIDTH];

int						planeshade;
vec3_t					plane_sz, plane_su, plane_sv;
float					planelightfloat;
bool					plane_shade;
fixed_t					pviewx, pviewy;

void R_DrawTiltedPlane_ASM (int y, int x1);
}

fixed_t 				yslope[MAXHEIGHT];
static fixed_t			xscale, yscale;
static DWORD			xstepscale, ystepscale;
static DWORD			basexfrac, baseyfrac;

#ifdef USEASM
extern "C" void R_SetSpanSource_ASM (byte *flat);
extern "C" void R_SetSpanColormap_ASM (byte *colormap);
extern "C" void R_SetTiltedSpanSource_ASM (byte *flat);
extern "C" byte *ds_curcolormap, *ds_cursource, *ds_curtiltedsource;
#endif

//==========================================================================
//
// R_InitPlanes
//
// Called at game startup.
//
//==========================================================================

void R_InitPlanes ()
{
}

//==========================================================================
//
// R_MapPlane
//
// Globals used: planeheight, ds_source, basexscale, baseyscale,
// pviewx, pviewy, xoffs, yoffs, basecolormap, xscale, yscale.
//
//==========================================================================

void R_MapPlane (int y, int x1)
{
	int x2 = spanend[y];
	fixed_t distance;

#ifdef RANGECHECK
	if (x2 < x1 || x1<0 || x2>=viewwidth || (unsigned)y>=(unsigned)viewheight)
	{
		I_FatalError ("R_MapPlane: %i, %i at %i", x1, x2, y);
	}
#endif

	// [RH] Notice that I dumped the caching scheme used by Doom.
	// It did not offer any appreciable speedup.

	distance = FixedMul (planeheight, yslope[y]);

	ds_xstep = FixedMul (distance, xstepscale);
	ds_ystep = FixedMul (distance, ystepscale);
	ds_xfrac = FixedMul (distance, basexfrac) + pviewx;
	ds_yfrac = FixedMul (distance, baseyfrac) + pviewy;

	if (plane_shade)
	{
		// Determine lighting based on the span's distance from the viewer.
		ds_colormap = basecolormap + (GETPALOOKUP (
			FixedMul (GlobVis, abs (centeryfrac - (y << FRACBITS))), planeshade) << COLORMAPSHIFT);
	}

#ifdef USEASM
	if (ds_colormap != ds_curcolormap)
		R_SetSpanColormap_ASM (ds_colormap);
#endif

	ds_y = y;
	ds_x1 = x1;
	ds_x2 = x2;

	spanfunc ();
}

//==========================================================================
//
// R_CalcTiltedLighting
//
// Calculates the lighting for one row of a tilted plane. If the definition
// of GETPALOOKUP changes, this needs to change, too.
//
//==========================================================================

extern "C" {
void STACK_ARGS R_CalcTiltedLighting (fixed_t lval, fixed_t lend, int width)
{
	fixed_t lstep;
	byte *lightfiller;
	int i = 0;

	lval = planeshade - lval;
	lend = planeshade - lend;

	if (width == 0 || lval == lend)
	{ // Constant lighting
		lightfiller = basecolormap + (GETPALOOKUP (-lval, 0) << COLORMAPSHIFT);
	}
	else if ((lstep = (lend - lval) / width) < 0)
	{ // Going from dark to light
		if (lval < FRACUNIT)
		{ // All bright
			lightfiller = basecolormap;
		}
		else
		{
			if (lval >= NUMCOLORMAPS*FRACUNIT)
			{ // Starts beyond the dark end
				byte *clight = basecolormap + ((NUMCOLORMAPS-1) << COLORMAPSHIFT);
				while (lval >= NUMCOLORMAPS*FRACUNIT && i <= width)
				{
					tiltlighting[i++] = clight;
					lval += lstep;
				}
				if (i > width)
					return;
			}
			while (i <= width && lval >= 0)
			{
				tiltlighting[i++] = basecolormap + ((lval >> FRACBITS) << COLORMAPSHIFT);
				lval += lstep;
			}
			lightfiller = basecolormap;
		}
	}
	else
	{ // Going from light to dark
		if (lval >= (NUMCOLORMAPS-1)*FRACUNIT)
		{ // All dark
			lightfiller = basecolormap + ((NUMCOLORMAPS-1) << COLORMAPSHIFT);
		}
		else
		{
			while (lval < 0 && i <= width)
			{
				tiltlighting[i++] = basecolormap;
				lval += lstep;
			}
			if (i > width)
				return;
			while (i <= width && lval < (NUMCOLORMAPS-1)*FRACUNIT)
			{
				tiltlighting[i++] = basecolormap + ((lval >> FRACBITS) << COLORMAPSHIFT);
				lval += lstep;
			}
			lightfiller = basecolormap + ((NUMCOLORMAPS-1) << COLORMAPSHIFT);
		}
	}

	for (; i <= width; i++)
	{
		tiltlighting[i] = lightfiller;
	}
}
}

//==========================================================================
//
// R_MapTiltedPlane
//
//==========================================================================

void R_MapTiltedPlane (int y, int x1)
{
	int x2 = spanend[y];
	int width = x2 - x1;
	float iz, uz, vz;
	byte *fb;
	DWORD u, v;
	int i;

	iz = plane_sz[2] + plane_sz[1]*(centery-y) + plane_sz[0]*(x1-centerx);

	// Lighting is simple. It's just linear interpolation from start to end
	if (plane_shade)
	{
		uz = (iz + plane_sz[0]*width) * planelightfloat;
		vz = iz * planelightfloat;
		R_CalcTiltedLighting (toint (vz), toint (uz), width);
	}

	uz = plane_su[2] + plane_su[1]*(centery-y) + plane_su[0]*(x1-centerx);
	vz = plane_sv[2] + plane_sv[1]*(centery-y) + plane_sv[0]*(x1-centerx);

	fb = ylookup[y] + x1;

#if 0		// The "perfect" reference version of this routine. Pretty slow.
			// Use it only to see how things are supposed to look.
	i = 0;
	do
	{
		float z = 1.f/iz;
		u = toint (uz*z) + pviewx;
		v = toint (vz*z) + pviewy;
		ds_colormap = tiltlighting[i];
		fb[i++] = ds_colormap[ds_source[(u>>26)|((v>>20)&(64*63))]];
		iz += plane_sz[0];
		uz += plane_su[0];
		vz += plane_sv[0];
	} while (--width >= 0);
#else
//#define SPANSIZE 32
//#define INVSPAN 0.03125f
//#define SPANSIZE 8
//#define INVSPAN 0.125f
#define SPANSIZE 16
#define INVSPAN	0.0625f

	float startz = 1.f/iz;
	float startu = uz*startz;
	float startv = vz*startz;
	float izstep, uzstep, vzstep;

	izstep = plane_sz[0] * SPANSIZE;
	uzstep = plane_su[0] * SPANSIZE;
	vzstep = plane_sv[0] * SPANSIZE;
	x1 = 0;
	width++;

	while (width >= SPANSIZE)
	{
		iz += izstep;
		uz += uzstep;
		vz += vzstep;

		float endz = 1.f/iz;
		float endu = uz*endz;
		float endv = vz*endz;
		DWORD stepu = toint ((endu - startu) * INVSPAN);
		DWORD stepv = toint ((endv - startv) * INVSPAN);
		u = toint (startu) + pviewx;
		v = toint (startv) + pviewy;

		for (i = SPANSIZE-1; i >= 0; i--)
		{
			fb[x1] = *(tiltlighting[x1] + ds_source[(u>>26)|((v>>20)&(64*63))]);
			x1++;
			u += stepu;
			v += stepv;
		}
		startu = endu;
		startv = endv;
		startz = endz;
		width -= SPANSIZE;
	}
	if (width > 0)
	{
		if (width == 1)
		{
			u = toint (startu);
			v = toint (startv);
			fb[x1] = *(tiltlighting[x1] + ds_source[(u>>26)|((v>>20)&(64*63))]);
		}
		else
		{
			float left = (float)width;
			iz += plane_sz[0] * left;
			uz += plane_su[0] * left;
			vz += plane_sv[0] * left;

			float endz = 1.f/iz;
			float endu = uz*endz;
			float endv = vz*endz;
			left = 1.f/left;
			DWORD stepu = toint ((endu - startu) * left);
			DWORD stepv = toint ((endv - startv) * left);
			u = toint (startu) + pviewx;
			v = toint (startv) + pviewy;

			for (; width != 0; width--)
			{
				fb[x1] = *(tiltlighting[x1] + ds_source[(u>>26)|((v>>20)&(64*63))]);
				x1++;
				u += stepu;
				v += stepv;
			}
		}
	}
#endif
}

//==========================================================================
//
// R_MapColoredPlane
//
//==========================================================================

void R_MapColoredPlane (int y, int x1)
{
	memset (ylookup[y] + x1, ds_color, spanend[y] - x1 + 1);
}

//==========================================================================
//
// R_ClearPlanes
//
// Called at the beginning of each frame.
//
//==========================================================================

extern int ConBottom;

void R_ClearPlanes (bool fullclear)
{
	int i;
	
	for (i = 0; i < MAXVISPLANES; i++)	// new code -- killough
		for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
			freehead = &(*freehead)->next;

	if (fullclear)
	{
		// opening / clipping determination
		clearbufshort (floorclip, viewwidth, viewheight);
		// [RH] clip ceiling to console bottom
		clearbufshort (ceilingclip, viewwidth,
			ConBottom > viewwindowy && !bRenderingToCanvas
			? ((ConBottom - viewwindowy) >> detailyshift) : 0);

		lastopening = 0;
	}
}

//==========================================================================
//
// new_visplane
//
// New function, by Lee Killough
// [RH] top and bottom buffers get allocated immediately after the visplane.
//
//==========================================================================

static visplane_t *new_visplane (unsigned hash)
{
	visplane_t *check = freetail;

	if (check == NULL)
	{
		check = (visplane_t *)Calloc (1, sizeof(*check) + sizeof(*check->top)*(SCREENWIDTH*2));
		check->bottom = &check->top[SCREENWIDTH+2];
	}
	else if (NULL == (freetail = freetail->next))
	{
		freehead = &freetail;
	}

	check->next = visplanes[hash];
	visplanes[hash] = check;
	return check;
}


//==========================================================================
//
// R_FindPlane
//
// killough 2/28/98: Add offsets
//==========================================================================

visplane_t *R_FindPlane (const secplane_t &height, int picnum, int lightlevel,
						 fixed_t xoffs, fixed_t yoffs,
						 fixed_t xscale, fixed_t yscale, angle_t angle,
						 ASkyViewpoint *skybox)
{
	secplane_t plane;
	visplane_t *check;
	unsigned hash;						// killough
	bool isskybox;

	if (picnum == skyflatnum || picnum & PL_SKYFLAT)	// killough 10/98
	{ // most skies map together
		lightlevel = 0;
		plane.a = plane.b = plane.d = 0;
		plane.c = height.c;
		plane.ic = height.ic;
		isskybox = (picnum == skyflatnum) && (skybox != NULL) && !r_InSkyBox;
	}
	else
	{
		plane = height;
		isskybox = false;
	}
		
	// New visplane algorithm uses hash table -- killough
	hash = isskybox ? MAXVISPLANES : visplane_hash (picnum, lightlevel, height);

	for (check = visplanes[hash]; check; check = check->next)	// killough
	{
		if (isskybox)
		{
			if (skybox == check->skybox)
			{
				return check;
			}
		}
		else
		if (plane == check->height &&
			picnum == check->picnum &&
			lightlevel == check->lightlevel &&
			xoffs == check->xoffs &&	// killough 2/28/98: Add offset checks
			yoffs == check->yoffs &&
			basecolormap == check->colormap &&	// [RH] Add more checks
			xscale == check->xscale &&
			yscale == check->yscale &&
			angle == check->angle
			)
		{
		  return check;
		}
	}

	check = new_visplane (hash);		// killough

	check->height = plane;
	check->picnum = picnum;
	check->lightlevel = lightlevel;
	check->xoffs = xoffs;				// killough 2/28/98: Save offsets
	check->yoffs = yoffs;
	check->xscale = xscale;
	check->yscale = yscale;
	check->angle = angle;
	check->colormap = basecolormap;		// [RH] Save colormap
	check->skybox = skybox;
	check->minx = viewwidth;			// Was SCREENWIDTH -- killough 11/98
	check->maxx = -1;

	clearbufshort (check->top, viewwidth, 0x7fff);

	return check;
}

//==========================================================================
//
// R_CheckPlane
//
//==========================================================================

visplane_t *R_CheckPlane (visplane_t *pl, int start, int stop)
{
	int intrl, intrh;
	int unionl, unionh;
	int x;

	if (start < pl->minx)
	{
		intrl = pl->minx;
		unionl = start;
	}
	else
	{
		unionl = pl->minx;
		intrl = start;
	}
		
	if (stop > pl->maxx)
	{
		intrh = pl->maxx;
		unionh = stop;
	}
	else
	{
		unionh = pl->maxx;
		intrh = stop;
	}

	for (x = intrl; x <= intrh && pl->top[x] == 0x7fff; x++)
		;

	if (x > intrh)
	{
		// use the same visplane
		pl->minx = unionl;
		pl->maxx = unionh;
	}
	else
	{
		// make a new visplane
		unsigned hash;

		if (pl->picnum == skyflatnum && pl->skybox != NULL && !r_InSkyBox)
		{
			hash = MAXVISPLANES;
		}
		else
		{
			hash = visplane_hash (pl->picnum, pl->lightlevel, pl->height);
		}
		visplane_t *new_pl = new_visplane (hash);

		new_pl->height = pl->height;
		new_pl->picnum = pl->picnum;
		new_pl->lightlevel = pl->lightlevel;
		new_pl->xoffs = pl->xoffs;			// killough 2/28/98
		new_pl->yoffs = pl->yoffs;
		new_pl->xscale = pl->xscale;		// [RH] copy these, too
		new_pl->yscale = pl->yscale;
		new_pl->angle = pl->angle;
		new_pl->colormap = pl->colormap;
		new_pl->skybox = pl->skybox;
		pl = new_pl;
		pl->minx = start;
		pl->maxx = stop;
		clearbufshort (pl->top, viewwidth, 0x7fff);
	}
	return pl;
}


//==========================================================================
//
// R_MakeSpans
//
//
//==========================================================================

inline void R_MakeSpans (int x, int t1, int b1, int t2, int b2, void (*mapfunc)(int y, int x1))
{
}

//==========================================================================
//
// R_DrawSky
//
// Can handle overlapped skies. Note that the front sky is *not* masked in
// in the normal convention for patches, but uses color 0 as a transparent
// color instead.
//
//==========================================================================

static int frontskytex, backskytex;
static angle_t skyflip;
static int frontpos, backpos;

extern fixed_t swall[MAXWIDTH];
extern fixed_t lwall[MAXWIDTH];
extern fixed_t rw_offset;
extern int rw_picnum;

// Allow for layer skies up to 512 pixels tall. This is overkill,
// since the most anyone can ever see of the sky is 500 pixels.
// We need 4 skybufs because wallscan can draw up to 4 columns at a time.
static BYTE skybuf[4][512];
static DWORD lastskycol[4];
static int skycolplace;

// Get a column of sky when there is only one sky texture.
static BYTE *R_GetOneSkyColumn (int fronttex, int x)
{
	return R_GetColumn (fronttex,
		((((viewangle + xtoviewangle[x])^skyflip) >> sky1shift) + frontpos) >> FRACBITS);
}

// Get a columrn of sky when there are two overlapping sky textures
static BYTE *R_GetTwoSkyColumns (int fronttex, int x)
{
	DWORD ang = (viewangle + xtoviewangle[x])^skyflip;
	DWORD angle1 = ((ang >> sky1shift) + frontpos) >> FRACBITS;
	DWORD angle2 = ((ang >> sky2shift) + backpos) >> FRACBITS;

	// Check if this column has already been built. If so, there's
	// no reason to waste time building it again.
	DWORD skycol = (angle1 << 16) | angle2;
	int i;

	for (i = 0; i < 4; ++i)
	{
		if (lastskycol[i] == skycol)
		{
			return skybuf[i];
		}
	}

	lastskycol[skycolplace] = skycol;
	BYTE *composite = skybuf[skycolplace];
	skycolplace = (skycolplace + 1) & 3;

	// The ordering of the following code has been tuned to allow VC++ to optimize
	// it well. In particular, this arrangement lets it keep count in a register
	// instead of on the stack.
	BYTE *front = R_GetColumn (fronttex, angle1);
	BYTE *back = R_GetColumn (backskytex, angle2);

	int count = MIN<int> (512, MIN (textureheight[backskytex], textureheight[fronttex]) >> FRACBITS);
	i = 0;
	do
	{
		if (front[i])
		{
			composite[i] = front[i];
		}
		else
		{
			composite[i] = back[i];
		}
	} while (++i, --count);
	return composite;
}

static void R_DrawSky (visplane_t *pl)
{
	int x;

	if (pl->minx > pl->maxx)
		return;

	dc_iscale = skyiscale >> skystretch;
	dc_texturemid = skytexturemid;

	clearbuf (swall+pl->minx, pl->maxx-pl->minx+1, dc_iscale<<2);
	rw_offset = frontpos;

	if (MirrorFlags & RF_XFLIP)
	{
		for (x = pl->minx; x <= pl->maxx; ++x)
		{
			lwall[x] = (viewwidth - x) << FRACBITS;
		}
	}
	else
	{
		for (x = pl->minx; x <= pl->maxx; ++x)
		{
			lwall[x] = x << FRACBITS;
		}
	}

	for (x = 0; x < 4; ++x)
	{
		lastskycol[x] = 0xffffffff;
	}

	rw_picnum = frontskytex;
	rw_offset = 0;

	if (1 << (textureheightlog2[frontskytex]+FRACBITS) == textureheight[frontskytex])
	{ // The texture tiles nicely
		for (x = 0; x < 4; ++x)
		{
			lastskycol[x] = 0xffffffff;
		}
		wallscan (pl->minx, pl->maxx, (short *)pl->top, (short *)pl->bottom, swall, lwall, 
			backskytex == -1 ? R_GetOneSkyColumn : R_GetTwoSkyColumns);
	}
	else
	{ // The texture does not tile nicely
		R_DrawSkyStriped (pl);
	}
}

static void R_DrawSkyStriped (visplane_t *pl)
{
	fixed_t centerysave = centeryfrac;
	short drawheight = (short)MulScale32 (textureheight[frontskytex], skyscale << skystretch);
	fixed_t topfrac;
	fixed_t iscale = skyiscale >> skystretch;
	short top[MAXWIDTH], bot[MAXWIDTH];
	short yl, yh;
	int x;

	// So that I don't have to worry about fractional precision, chop off the
	// fractional part of centeryfrac.
	centeryfrac = centery << FRACBITS;
	topfrac = (skytexturemid + iscale * (1-centery)) % textureheight[frontskytex];
	if (topfrac < 0) topfrac += textureheight[frontskytex];
	yl = 0;
	yh = (short)MulScale32 (textureheight[frontskytex] - topfrac, skyscale<<skystretch);
	dc_texturemid = topfrac - iscale * (1-centery);

	while (yl < viewheight)
	{
		for (x = pl->minx; x <= pl->maxx; ++x)
		{
			top[x] = MAX (yl, (short)pl->top[x]);
			bot[x] = MIN (yh, (short)pl->bottom[x]);
		}
		for (x = 0; x < 4; ++x)
		{
			lastskycol[x] = 0xffffffff;
		}
		wallscan (pl->minx, pl->maxx, top, bot, swall, lwall, 
			backskytex == -1 ? R_GetOneSkyColumn : R_GetTwoSkyColumns);
		yl = yh;
		yh += drawheight;
		dc_texturemid = iscale * (centery-yl-1);
	}
	centeryfrac = centerysave;
}

//==========================================================================
//
// R_DrawPlanes
//
// At the end of each frame.
//
//==========================================================================

CVAR (Bool, tilt, false, 0);

void R_DrawPlanes ()
{
	visplane_t *pl;
	int i, x;
	int vpcount;

	ds_color = 3;

	for (i = vpcount = 0; i < MAXVISPLANES; i++)
	{
		for (pl = visplanes[i]; pl; pl = pl->next)
		{
			vpcount++;

			if (pl->minx > pl->maxx)
				continue;

			if (r_drawflat)
			{ // [RH] no texture mapping
				ds_color += 4;
				R_MapVisPlane (pl, R_MapColoredPlane);
			}
			else if (pl->picnum == skyflatnum || pl->picnum & PL_SKYFLAT)
			{ // sky flat
				R_DrawSkyPlane (pl);
			}
			else
			{ // regular flat
				int useflatnum = flattranslation[pl->picnum];

				ds_source = (byte *)W_CacheLumpNum (firstflat + useflatnum,
					PU_STATIC);

				// [RH] warp a flat if desired
				if (flatwarp[useflatnum])
				{
					if ((!warpedflats[useflatnum]
						 && Z_Malloc (64*64, PU_STATIC, &warpedflats[useflatnum]))
						|| flatwarpedwhen[useflatnum] != level.time)
					{
						static byte buffer[64];
						int timebase = level.time*23;

						flatwarpedwhen[useflatnum] = level.time;
						byte *warped = warpedflats[useflatnum];

						for (x = 63; x >= 0; x--)
						{
							int yt, yf = (finesine[(timebase+(x+17)*128)&FINEMASK]>>13) & 63;
							byte *source = ds_source + x;
							byte *dest = warped + x;
							for (yt = 64; yt; yt--, yf = (yf+1)&63, dest += 64)
								*dest = *(source+yf*64);
						}
						timebase = level.time*32;
						int y;
						for (y = 63; y >= 0; y--)
						{
							int xt, xf = (finesine[(timebase+y*128)&FINEMASK]>>13) & 63;
							byte *source = warped + y*64;
							byte *dest = buffer;
							for (xt = 64; xt; xt--, xf = (xf+1)&63)
								*dest++ = *(source+xf);
							memcpy (warped+y*64, buffer, 64);
						}
						Z_ChangeTag (ds_source, PU_CACHE);
						ds_source = warped;
					}
					else
					{
						Z_ChangeTag (ds_source, PU_CACHE);
						ds_source = warpedflats[useflatnum];
						Z_ChangeTag (ds_source, PU_STATIC);
					}
				}

				basecolormap = pl->colormap;
				planeshade = LIGHT2SHADE(pl->lightlevel + r_actualextralight);

				if (r_drawflat || (pl->height.a == 0 && pl->height.b == 0) && !tilt)
				{
					R_DrawNormalPlane (pl);
				}
				else
				{
					R_DrawTiltedPlane (pl);
				}

				Z_ChangeTag (ds_source, PU_CACHE);
			}
		}
	}
}

//==========================================================================
//
// R_DrawSkyBoxes
//
// Draws any recorded sky boxes and then frees them.
//
// The process:
//   1. Move the camera to coincide with the SkyViewpoint.
//   2. Clear out the old planes. (They have already been drawn.)
//   3. Clear a window out of the ClipSegs just large enough for the plane.
//   4. Pretend the existing vissprites and drawsegs aren't there.
//   5. Create a drawseg at 0 distance to clip sprites to the visplane. It
//      doesn't need to be associated with a line in the map, since there
//      will never be any sprites in front of it.
//   6. Render the BSP, then planes, then masked stuff.
//   7. Restore the previous vissprites and drawsegs.
//   8. Repeat for any other sky boxes.
//   9. Put the camera back where it was to begin with.
//
//==========================================================================

void R_DrawSkyBoxes ()
{
	if (visplanes[MAXVISPLANES] == NULL)
		return;

	int savedextralight = extralight;
	fixed_t savedx = viewx;
	fixed_t savedy = viewy;
	fixed_t savedz = viewz;
	angle_t savedangle = viewangle;
	ptrdiff_t savedvissprite_p = vissprite_p - vissprites;
	ptrdiff_t savedds_p = ds_p - drawsegs;
	ptrdiff_t savedlastopening = lastopening;
	int savedinteresting = FirstInterestingDrawseg;
	float savedvisibility = R_GetVisibility ();
	AActor *savedcamera = camera;
	sector_t *savedsector = viewsector;

	int i;
	visplane_t *pl;

	// Don't draw sky boxes inside sky boxes.
	r_InSkyBox = true;

	// Don't let gun flashes brighten the sky box
	extralight = 0;

	for (pl = visplanes[MAXVISPLANES]; pl != NULL; pl = pl->next)
	{
		if (pl->maxx < pl->minx)
			continue;

		ASkyViewpoint *sky = pl->skybox;

		viewx = sky->x;
		viewy = sky->y;
		viewz = sky->z;
		camera = sky;
		viewsector = sky->Sector;
		R_SetViewAngle (savedangle + sky->angle);
		validcount++;	// Make sure we see all sprites

		R_SetVisibility (sky->args[0] * 0.25f);

		R_ClearPlanes (false);
		R_ClearClipSegs (pl->minx, pl->maxx + 1);
		WindowLeft = pl->minx;
		WindowRight = pl->maxx;

		for (i = pl->minx; i <= pl->maxx; i++)
		{
			if (pl->top[i] == 0x7fff)
			{
				ceilingclip[i] = viewheight;
				floorclip[i] = -1;
			}
			else
			{
				ceilingclip[i] = pl->top[i];
				floorclip[i] = pl->bottom[i];
			}
		}

		// Create a drawseg to clip sprites to the sky plane
		R_CheckDrawSegs ();
		R_CheckOpenings ((pl->maxx - pl->minx + 1)*2);
		ds_p->fardepth = 0;
		ds_p->neardepth = 0;
		ds_p->x1 = pl->minx;
		ds_p->x2 = pl->maxx;
		ds_p->silhouette = SIL_BOTH;
		ds_p->sprbottomclip = R_NewOpening (pl->maxx - pl->minx + 1);
		ds_p->sprtopclip = R_NewOpening (pl->maxx - pl->minx + 1);
		ds_p->maskedtexturecol = ds_p->swall = -1;
		ds_p->bFogBoundary = false;
		memcpy (openings + ds_p->sprbottomclip, floorclip + pl->minx, (pl->maxx - pl->minx + 1)*sizeof(short));
		memcpy (openings + ds_p->sprtopclip, ceilingclip + pl->minx, (pl->maxx - pl->minx + 1)*sizeof(short));

		firstvissprite = vissprite_p;
		firstdrawseg = ds_p++;
		FirstInterestingDrawseg = InterestingDrawsegs.Size();

		R_RenderBSPNode (numnodes - 1);
		R_DrawPlanes ();
		R_DrawMasked ();

		firstvissprite = vissprites;
		vissprite_p = vissprites + savedvissprite_p;
		firstdrawseg = drawsegs;
		ds_p = drawsegs + savedds_p;
		InterestingDrawsegs.Resize (FirstInterestingDrawseg);
		FirstInterestingDrawseg = savedinteresting;
		lastopening = savedlastopening;
	}

	camera = savedcamera;
	viewsector = savedsector;
	viewx = savedx;
	viewy = savedy;
	viewz = savedz;
	R_SetVisibility (savedvisibility);
	extralight = savedextralight;
	R_SetViewAngle (savedangle);

	r_InSkyBox = false;

	for (*freehead = visplanes[MAXVISPLANES], visplanes[MAXVISPLANES] = NULL; *freehead; )
		freehead = &(*freehead)->next;
}

//==========================================================================
//
// R_DrawSkyPlane
//
//==========================================================================

void R_DrawSkyPlane (visplane_t *pl)
{
	int sky1tex, sky2tex;

	if (level.flags & LEVEL_SWAPSKIES)
	{
		sky1tex = sky2texture;
		sky2tex = sky1texture;
	}
	else
	{
		sky1tex = sky1texture;
		sky2tex = sky2texture;
	}

	if (pl->picnum == skyflatnum)
	{	// use sky1
		frontskytex = texturetranslation[sky1tex];
		if (level.flags & LEVEL_DOUBLESKY)
			backskytex = texturetranslation[sky2tex];
		else
			backskytex = -1;
		skyflip = 0;
		frontpos = sky1pos;
		backpos = sky2pos;
	}
	else if (pl->picnum == PL_SKYFLAT)
	{	// use sky2
		frontskytex = texturetranslation[sky2tex];
		backskytex = -1;
		skyflip = 0;
		frontpos = sky2pos;
	}
	else
	{	// MBF's linedef-controlled skies
		// Sky Linedef
		const line_t *l = &lines[(pl->picnum & ~PL_SKYFLAT)-1];

		// Sky transferred from first sidedef
		const side_t *s = *l->sidenum + sides;

		// Texture comes from upper texture of reference sidedef
		// [RH] If swapping skies, then use the lower sidedef
		if (level.flags & LEVEL_SWAPSKIES)
		{
			frontskytex = texturetranslation[s->bottomtexture];
		}
		else
		{
			frontskytex = texturetranslation[s->toptexture];
		}
		backskytex = -1;

		// Horizontal offset is turned into an angle offset,
		// to allow sky rotation as well as careful positioning.
		// However, the offset is scaled very small, so that it
		// allows a long-period of sky rotation.
		frontpos = (-s->textureoffset) >> 6;

		// Vertical offset allows careful sky positioning.
		dc_texturemid = s->rowoffset - 28*FRACUNIT;

		// We sometimes flip the picture horizontally.
		//
		// Doom always flipped the picture, so we make it optional,
		// to make it easier to use the new feature, while to still
		// allow old sky textures to be used.
		skyflip = l->args[2] ? 0u : ~0u;
	}

	bool fakefixed = false;
	if (fixedcolormap)
	{
		dc_colormap = fixedcolormap;
	}
	else if (!fixedcolormap)
	{
		fakefixed = true;
		fixedcolormap = dc_colormap = NormalLight.Maps;
	}

	R_DrawSky (pl);

	if (fakefixed)
		fixedcolormap = NULL;
}

//==========================================================================
//
// R_DrawNormalPlane
//
//==========================================================================

void R_DrawNormalPlane (visplane_t *pl)
{
#ifdef USEASM
	if (ds_source != ds_cursource)
		R_SetSpanSource_ASM (ds_source);
#endif

	angle_t planeang = pl->angle;
	xscale = pl->xscale << 10;
	yscale = pl->yscale << 10;

	if (planeang != 0)
	{
		fixed_t cosine = finecosine[planeang >> ANGLETOFINESHIFT];
		fixed_t sine = finesine[planeang >> ANGLETOFINESHIFT];

		pviewx = pl->xoffs + FixedMul (viewx, cosine) - FixedMul (viewy, sine);
		pviewy = pl->yoffs - FixedMul (viewx, sine) - FixedMul (viewy, cosine);
	}
	else
	{
		pviewx = pl->xoffs + viewx;
		pviewy = pl->yoffs - viewy;
	}

	pviewx = FixedMul (xscale, pviewx);
	pviewy = FixedMul (yscale, pviewy);
	
	// left to right mapping
	planeang = (viewangle - ANG90 + planeang) >> ANGLETOFINESHIFT;
	// Scale will be unit scale at FocalLengthX (normally SCREENWIDTH/2) distance
	xstepscale = Scale (xscale, finecosine[planeang], FocalLengthX);
	ystepscale = Scale (yscale, -finesine[planeang], FocalLengthX);

	// [RH] flip for mirrors
	if (MirrorFlags & RF_XFLIP)
	{
		xstepscale = (DWORD)(-(SDWORD)xstepscale);
		ystepscale = (DWORD)(-(SDWORD)ystepscale);
	}

	int x = pl->maxx - halfviewwidth;
	planeang = (planeang + (ANG90 >> ANGLETOFINESHIFT)) & FINEMASK;
	basexfrac = FixedMul (xscale, finecosine[planeang]) + x*xstepscale;
	baseyfrac = FixedMul (yscale, -finesine[planeang]) + x*ystepscale;

	planeheight = abs (FixedMul (pl->height.d, -pl->height.ic) - viewz);

	GlobVis = FixedDiv (r_FloorVisibility, planeheight);
	if (fixedlightlev)
		ds_colormap = basecolormap + fixedlightlev, plane_shade = false;
	else if (fixedcolormap)
		ds_colormap = fixedcolormap, plane_shade = false;
	else
		plane_shade = true;

	R_MapVisPlane (pl, R_MapPlane);
}

//==========================================================================
//
// R_DrawTiltedPlane
//
//==========================================================================

void R_DrawTiltedPlane (visplane_t *pl)
{
	float lxscale, lyscale;
	float xscale, yscale;
	fixed_t ixscale, iyscale;
	angle_t ang;
	vec3_t p, m, n;
	fixed_t zeroheight;

	// p is the texture origin in view space
	// Don't add in the offsets at this stage, because doing so can result in
	// errors if the flat is rotated.

	lxscale = FIXED2FLOAT(pl->xscale);
	lyscale = FIXED2FLOAT(pl->yscale);
	xscale = 64.f/lxscale;
	yscale = 64.f/lyscale;
	ixscale = quickertoint(xscale*65536.f);
	iyscale = quickertoint(yscale*65536.f);
	zeroheight = pl->height.ZatPoint (0, 0);

	pviewx = MulScale6 (pl->xoffs, pl->xscale);
	pviewy = MulScale6 (pl->yoffs, pl->yscale);

	ang = (ANG270 - viewangle) >> ANGLETOFINESHIFT;
	p[0] = FIXED2FLOAT(DMulScale16 (viewx, finecosine[ang], -viewy, finesine[ang]));
	p[2] = FIXED2FLOAT(DMulScale16 (viewx, finesine[ang], viewy, finecosine[ang]));
	p[1] = FIXED2FLOAT(pl->height.ZatPoint (0, 0) - viewz);

	// m is the v direction vector in view space
	ang = (ANG180 - viewangle - pl->angle) >> ANGLETOFINESHIFT;
	m[0] = yscale * FIXED2FLOAT(finecosine[ang]);
	m[2] = yscale * FIXED2FLOAT(finesine[ang]);
//	m[1] = FIXED2FLOAT(pl->height.ZatPoint (0, iyscale) - pl->height.ZatPoint (0,0));
//	VectorScale2 (m, 64.f/VectorLength(m));

	// n is the u direction vector in view space
	ang = (ang + (ANG90>>ANGLETOFINESHIFT)) & FINEMASK;
	n[0] = -xscale * FIXED2FLOAT(finecosine[ang]);
	n[2] = -xscale * FIXED2FLOAT(finesine[ang]);
//	n[1] = FIXED2FLOAT(pl->height.ZatPoint (ixscale, 0) - pl->height.ZatPoint (0,0));
//	VectorScale2 (n, 64.f/VectorLength(n));

	ang = pl->angle >> ANGLETOFINESHIFT;
	m[1] = FIXED2FLOAT(pl->height.ZatPoint (
		MulScale16 (iyscale, finesine[ang]),
		MulScale16 (iyscale, finecosine[ang])) - zeroheight);
	ang = (pl->angle + ANGLE_90) >> ANGLETOFINESHIFT;
	n[1] = FIXED2FLOAT(pl->height.ZatPoint (
		MulScale16 (ixscale, finesine[ang]),
		MulScale16 (ixscale, finecosine[ang])) - zeroheight);

	CrossProduct (p, m, plane_su);
	CrossProduct (p, n, plane_sv);
	CrossProduct (m, n, plane_sz);

	plane_su[2] *= FocalLengthXfloat;
	plane_sv[2] *= FocalLengthXfloat;
	plane_sz[2] *= FocalLengthXfloat;

	plane_su[1] *= iyaspectmulfloat;
	plane_sv[1] *= iyaspectmulfloat;
	plane_sz[1] *= iyaspectmulfloat;

	// Premultiply the texture vectors with the scale factors
	VectorScale2 (plane_su, 4294967296.f);
	VectorScale2 (plane_sv, 4294967296.f);

	if (MirrorFlags & RF_XFLIP)
	{
		plane_su[0] = -plane_su[0];
		plane_sv[0] = -plane_sv[0];
		plane_sz[0] = -plane_sz[0];
	}

	planelightfloat = (r_TiltVisibility * lxscale * lyscale) / (float)(abs(pl->height.ZatPoint (viewx, viewy) - viewz));

	if (pl->height.c > 0)
		planelightfloat = -planelightfloat;

	if (fixedlightlev)
		ds_colormap = basecolormap + fixedlightlev, plane_shade = false;
	else if (fixedcolormap)
		ds_colormap = fixedcolormap, plane_shade = false;
	else
		ds_colormap = basecolormap, plane_shade = true;

	if (!plane_shade)
	{
		for (int i = 0; i < viewwidth; i++)
		{
			tiltlighting[i] = ds_colormap;
		}
	}

#if defined(USEASM)
	if (ds_source != ds_curtiltedsource)
		R_SetTiltedSpanSource_ASM (ds_source);
	R_MapVisPlane (pl, R_DrawTiltedPlane_ASM);
#else
	R_MapVisPlane (pl, R_MapTiltedPlane);
#endif
}

//==========================================================================
//
// R_MapVisPlane
//
// t1/b1 are at x
// t2/b2 are at x+1
// spanend[y] is at the right edge
//
//==========================================================================

void R_MapVisPlane (visplane_t *pl, void (*mapfunc)(int y, int x1))
{
	int x = pl->maxx;
	int t2 = pl->top[x];
	int b2 = pl->bottom[x];

	if (b2 > t2)
	{
		clearbufshort (spanend+t2, b2-t2, x);
	}

	for (--x; x >= pl->minx; --x)
	{
		int t1 = pl->top[x];
		int b1 = pl->bottom[x];
		const int xr = x+1;
		int stop;

		// Draw any spans that have just closed
		stop = MIN (t1, b2);
		while (t2 < stop)
		{
			mapfunc (t2++, xr);
		}
		stop = MAX (b1, t2);
		while (b2 > stop)
		{
			mapfunc (--b2, xr);
		}

		// Mark any spans that have just opened
		stop = MIN (t2, b1);
		while (t1 < stop)
		{
			spanend[t1++] = x;
		}
		stop = MAX (b2, t2);
		while (b1 > stop)
		{
			spanend[--b1] = x;
		}

		t2 = pl->top[x];
		b2 = pl->bottom[x];
		basexfrac -= xstepscale;
		baseyfrac -= ystepscale;
	}
	// Draw any spans that are still open
	while (t2 < b2)
	{
		mapfunc (--b2, pl->minx);
	}
}

//==========================================================================
//
// R_PlaneInitData
//
//==========================================================================

BOOL R_PlaneInitData ()
{
	int i;
	visplane_t *pl;

	// Free all visplanes and let them be re-allocated as needed.
	pl = freetail;

	while (pl)
	{
		visplane_t *next = pl->next;
		free (pl);
		pl = next;
	}
	freetail = NULL;
	freehead = &freetail;

	for (i = 0; i < MAXVISPLANES; i++)
	{
		pl = visplanes[i];
		visplanes[i] = NULL;
		while (pl)
		{
			visplane_t *next = pl->next;
			free (pl);
			pl = next;
		}
	}

	return true;
}

//==========================================================================
//
// R_AlignFlat
//
//==========================================================================

bool R_AlignFlat (int linenum, int side, int fc)
{
	line_t *line = lines + linenum;
	sector_t *sec = side ? line->backsector : line->frontsector;

	if (!sec)
		return false;

	fixed_t x = line->v1->x;
	fixed_t y = line->v1->y;

	angle_t angle = R_PointToAngle2 (x, y, line->v2->x, line->v2->y);
	angle_t norm = (angle-ANGLE_90) >> ANGLETOFINESHIFT;

	fixed_t dist = -FixedMul (finecosine[norm], x) - FixedMul (finesine[norm], y);

	if (side)
	{
		angle = angle + ANGLE_180;
		dist = -dist;
	}

	if (fc)
	{
		sec->base_ceiling_angle = 0-angle;
		sec->base_ceiling_yoffs = dist & ((1<<(FRACBITS+8))-1);
	}
	else
	{
		sec->base_floor_angle = 0-angle;
		sec->base_floor_yoffs = dist & ((1<<(FRACBITS+8))-1);
	}

	return true;
}