/*
** nodebuild.cpp
**
** The main logic for the internal node builder.
**
**---------------------------------------------------------------------------
** Copyright 2002-2005 Randy Heit
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
** 4. When not used as part of ZDoom or a ZDoom derivative, this code will be
**    covered by the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or (at
**    your option) any later version.
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
*/

#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "doomdata.h"
#include "nodebuild.h"
#include "templates.h"
#include "tarray.h"
#include "m_bbox.h"
#include "c_console.h"
#include "r_main.h"

const int MaxSegs = 64;
const int SplitCost = 8;
const int AAPreference = 16;

// Points within this distance of a line will be considered on the line.
// Units are in fixed_ts.
const double SIDE_EPSILON = 6.5536;

// Vertices within this distance of each other vertically and horizontally
// will be considered as the same vertex.
const fixed_t VERTEX_EPSILON = 6;

#if 0
#define D(x) x
#else
#define D(x) do{}while(0)
#endif

FNodeBuilder::FNodeBuilder (FLevel &level,
							TArray<FPolyStart> &polyspots, TArray<FPolyStart> &anchors,
							bool makeGLNodes)
	: Level (level), GLNodes (makeGLNodes), SegsStuffed (0)
{
	FindUsedVertices (Level.Vertices, Level.NumVertices);
	MakeSegsFromSides ();
	FindPolyContainers (polyspots, anchors);
	GroupSegPlanes ();
	BuildTree ();
}

void FNodeBuilder::BuildTree ()
{
	fixed_t bbox[4];

	C_InitTicker ("Building BSP", FRACUNIT);
	HackSeg = DWORD_MAX;
	CreateNode (0, bbox);
	CreateSubsectorsForReal ();
	C_InitTicker (NULL, 0);
}

int FNodeBuilder::CreateNode (DWORD set, fixed_t bbox[4])
{
	node_t node;
	int skip, count, selstat;
	DWORD splitseg;

	count = CountSegs (set);
	skip = count / MaxSegs;

	if ((selstat = SelectSplitter (set, node, splitseg, skip, true)) > 0 ||
		(skip > 0 && (selstat = SelectSplitter (set, node, splitseg, 1, true)) > 0) ||
		(selstat < 0 && (SelectSplitter (set, node, splitseg, skip, false) > 0) ||
						(skip > 0 && SelectSplitter (set, node, splitseg, 1, false))) ||
		CheckSubsector (set, node, splitseg, count))
	{
		// Create a normal node
		DWORD set1, set2;

		SplitSegs (set, node, splitseg, set1, set2);
		D(PrintSet (1, set1));
		D(Printf ("(%d,%d) delta (%d,%d) from seg %d\n", node.x>>16, node.y>>16, node.dx>>16, node.dy>>16, splitseg));
		D(PrintSet (2, set2));
		node.intchildren[0] = CreateNode (set1, node.bbox[0]);
		node.intchildren[1] = CreateNode (set2, node.bbox[1]);
		bbox[BOXTOP] = MAX (node.bbox[0][BOXTOP], node.bbox[1][BOXTOP]);
		bbox[BOXBOTTOM] = MIN (node.bbox[0][BOXBOTTOM], node.bbox[1][BOXBOTTOM]);
		bbox[BOXLEFT] = MIN (node.bbox[0][BOXLEFT], node.bbox[1][BOXLEFT]);
		bbox[BOXRIGHT] = MAX (node.bbox[0][BOXRIGHT], node.bbox[1][BOXRIGHT]);
		return (int)Nodes.Push (node);
	}
	else
	{
		return 0x80000000 | CreateSubsector (set, bbox);
	}
}

int FNodeBuilder::CreateSubsector (DWORD set, fixed_t bbox[4])
{
	int ssnum, count;

	bbox[BOXTOP] = bbox[BOXRIGHT] = INT_MIN;
	bbox[BOXBOTTOM] = bbox[BOXLEFT] = INT_MAX;

	D(Printf ("Subsector from set %d\n", set));

	assert (set != DWORD_MAX);

	// We cannot actually create the subsector now because the node building
	// process might split a seg in this subsector (because all partner segs
	// must use the same pair of vertices), adding a new seg that hasn't been
	// created yet. After all the nodes are built, then we can create the
	// actual subsectors using the CreateSubsectorsForReal function below.
	ssnum = (int)SubsectorSets.Push (set);

	count = 0;
	while (set != DWORD_MAX)
	{
		AddSegToBBox (bbox, &Segs[set]);
		set = Segs[set].next;
		count++;
	}

	SegsStuffed += count;
	if ((SegsStuffed & ~127) != ((SegsStuffed - count) & ~127))
	{
		C_SetTicker (MulScale16 (SegsStuffed, (SDWORD)Segs.Size()));
	}

	D(Printf ("bbox (%d,%d)-(%d,%d)\n", bbox[BOXLEFT]>>16, bbox[BOXBOTTOM]>>16, bbox[BOXRIGHT]>>16, bbox[BOXTOP]>>16));

	return ssnum;
}

void FNodeBuilder::CreateSubsectorsForReal ()
{
	subsector_t sub;
	unsigned int i;

	sub.poly = NULL;

	for (i = 0; i < SubsectorSets.Size(); ++i)
	{
		DWORD set = SubsectorSets[i];

		sub.firstline = (DWORD)SegList.Size();
		while (set != DWORD_MAX)
		{
			USegPtr ptr;

			ptr.SegPtr = &Segs[set];
			SegList.Push (ptr);
			set = ptr.SegPtr->next;
		}
		sub.numlines = (DWORD)(SegList.Size() - sub.firstline);

		// Sort segs by linedef for special effects
		qsort (&SegList[sub.firstline], sub.numlines, sizeof(int), SortSegs);

		// Convert seg pointers into indices
		for (unsigned int i = sub.firstline; i < SegList.Size(); ++i)
		{
			SegList[i].SegNum = SegList[i].SegPtr - &Segs[0];
		}
		Subsectors.Push (sub);
	}
}

int STACK_ARGS FNodeBuilder::SortSegs (const void *a, const void *b)
{
	const FPrivSeg *x = ((const USegPtr *)a)->SegPtr;
	const FPrivSeg *y = ((const USegPtr *)b)->SegPtr;

	// Segs are grouped into three categories in this order:
	//
	// 1. Segs with different front and back sectors (or no back at all).
	// 2. Segs with the same front and back sectors.
	// 3. Minisegs.
	//
	// Within the first two sets, segs are also sorted by linedef.
	//
	// Note that when GL subsectors are written, the segs will be reordered
	// so that they are in clockwise order, and extra minisegs will be added
	// as needed to close the subsector. But the first seg used will still be
	// the first seg chosen here.

	int xtype, ytype;

	if (x->linedef == -1)
	{
		xtype = 2;
	}
	else if (x->frontsector == x->backsector)
	{
		xtype = 1;
	}
	else
	{
		xtype = 0;
	}

	if (y->linedef == -1)
	{
		ytype = 2;
	}
	else if (y->frontsector == y->backsector)
	{
		ytype = 1;
	}
	else
	{
		ytype = 0;
	}

	if (xtype != ytype)
	{
		return xtype - ytype;
	}
	else if (xtype < 2)
	{
		return x->linedef - y->linedef;
	}
	else
	{
		return 0;
	}
}

int FNodeBuilder::CountSegs (DWORD set) const
{
	int count = 0;

	while (set != DWORD_MAX)
	{
		count++;
		set = Segs[set].next;
	}
	return count;
}

// Given a set of segs, checks to make sure they all belong to a single
// sector. If so, false is returned, and they become a subsector. If not,
// a splitter is synthesized, and true is returned to continue processing
// down this branch of the tree.

bool FNodeBuilder::CheckSubsector (DWORD set, node_t &node, DWORD &splitseg, int setsize)
{
	sector_t *sec;
	DWORD seg;

	sec = NULL;
	seg = set;

	do
	{
		D(Printf (" - seg %d(%d,%d)-(%d,%d) line %d front %d back %d\n", seg,
			Vertices[Segs[seg].v1].x>>16, Vertices[Segs[seg].v1].y>>16,
			Vertices[Segs[seg].v2].x>>16, Vertices[Segs[seg].v2].y>>16,
			Segs[seg].linedef, Segs[seg].frontsector, Segs[seg].backsector));
		if (Segs[seg].linedef != -1 &&
			Segs[seg].frontsector != sec
			// Segs with the same front and back sectors are allowed to reside
			// in a subsector with segs from a different sector, because the
			// only effect they can have on the display is to place masked
			// mid textures in the scene. Since minisegs only mark subsector
			// boundaries, their sector information is unimportant.
			//
			// Update: Lines with the same front and back sectors *can* affect
			// the display if their subsector does not match their front sector.
			/*&& Segs[seg].frontsector != Segs[seg].backsector*/)
		{
			if (sec == NULL)
			{
				sec = Segs[seg].frontsector;
			}
			else
			{
				break;
			}
		}
		seg = Segs[seg].next;
	} while (seg != DWORD_MAX);

	if (seg == DWORD_MAX)
	{ // It's a valid subsector
		return false;
	}

	D(Printf("Need to synthesize a splitter for set %d on seg %d\n", set, seg));
	splitseg = DWORD_MAX;

	// This is a very simple and cheap "fix" for subsectors with segs
	// from multiple sectors, and it seems ZenNode does something
	// similar. It is the only technique I could find that makes the
	// "transparent water" in nb_bmtrk.wad work properly.
	//
	// The seg is marked to indicate that it should be forced to the
	// back of the splitter. Because these segs already form a convex
	// set, all the other segs will be in front of the splitter. Since
	// the splitter is formed from this seg, the back of the splitter
	// will have a one-dimensional subsector. SplitSegs() will add two
	// new minisegs to close it: one seg replaces this one on the front
	// of the splitter, and the other is its partner on the back.
	//
	// Old code that will actually create valid two-dimensional sectors
	// is included below for reference but is not likely to be used again.

	SetNodeFromSeg (node, &Segs[seg]);
	HackSeg = seg;
	if (!Segs[seg].planefront)
	{
		node.x += node.dx;
		node.y += node.dy;
		node.dx = -node.dx;
		node.dy = -node.dy;
	}
	return Heuristic (node, set, false) != 0;

#if 0
	// If there are only two segs in the set, and they form two sides
	// of a triangle, the splitter should pass through their shared
	// point and the (imaginary) third side of the triangle
	if (setsize == 2)
	{
		FPrivVert *v1, *v2, *v3;

		if (Vertices[Segs[set].v2] == Vertices[Segs[seg].v1])
		{
			v1 = &Vertices[Segs[set].v1];
			v2 = &Vertices[Segs[seg].v2];
			v3 = &Vertices[Segs[set].v2];
		}
		else if (Vertices[Segs[set].v1] == Vertices[Segs[seg].v2])
		{
			v1 = &Vertices[Segs[seg].v1];
			v2 = &Vertices[Segs[set].v2];
			v3 = &Vertices[Segs[seg].v2];
		}
		else
		{
			v1 = v2 = v3 = NULL;
		}
		if (v1 != NULL)
		{
			node.x = v3->x;
			node.y = v3->y;
			node.dx = v1->x + (v2->x-v1->x)/2 - node.x;
			node.dy = v1->y + (v2->y-v1->y)/2 - node.y;
			return Heuristic (node, set, false) != 0;
		}
	}

	bool nosplit = true;
	int firsthit = seg;

	do
	{
		seg = firsthit;
		do
		{
			if (Segs[seg].linedef != -1 &&
				Segs[seg].frontsector != sec &&
				Segs[seg].frontsector == Segs[seg].backsector)
			{
				node.x = Vertices[Segs[set].v1].x;
				node.y = Vertices[Segs[set].v1].y;
				node.dx = Vertices[Segs[seg].v2].x - node.x;
				node.dy = Vertices[Segs[seg].v2].y - node.y;

				if (Heuristic (node, set, nosplit) != 0)
				{
					return true;
				}

				node.dx = Vertices[Segs[seg].v1].x - node.x;
				node.dy = Vertices[Segs[seg].v1].y - node.y;

				if (Heuristic (node, set, nosplit) != 0)
				{
					return true;
				}
			}

			seg = Segs[seg].next;
		} while (seg != DWORD_MAX);
	} while ((nosplit ^= 1) == 0);

	// Give up.
	return false;
#endif
}

// Splitters are chosen to coincide with segs in the given set. To reduce the
// number of segs that need to be considered as splitters, segs are grouped into
// according to the planes that they lie on. Because one seg on the plane is just
// as good as any other seg on the plane at defining a split, only one seg from
// each unique plane needs to be considered as a splitter. A result of 0 means
// this set is a convex region. A result of -1 means that there were possible
// splitters, but they all split segs we want to keep intact.
int FNodeBuilder::SelectSplitter (DWORD set, node_t &node, DWORD &splitseg, int step, bool nosplit)
{
	int stepleft;
	int bestvalue;
	DWORD bestseg;
	DWORD seg;
	bool nosplitters = false;

	bestvalue = 0;
	bestseg = DWORD_MAX;

	seg = set;
	stepleft = 0;

	memset (&PlaneChecked[0], 0, PlaneChecked.Size());

	while (seg != DWORD_MAX)
	{
		FPrivSeg *pseg = &Segs[seg];

		if (--stepleft <= 0)
		{
			int l = pseg->planenum >> 3;
			int r = 1 << (pseg->planenum & 7);

			if (l < 0 || (PlaneChecked[l] & r) == 0)
			{
				if (l >= 0)
				{
					PlaneChecked[l] |= r;
				}

				stepleft = step;
				SetNodeFromSeg (node, pseg);

				int value = Heuristic (node, set, nosplit);

				D(Printf ("Seg %5d (%5d,%5d)-(%5d,%5d) scores %d\n", seg, node.x>>16, node.y>>16,
					(node.x+node.dx)>>16, (node.y+node.dy)>>16, value));

				if (value > bestvalue)
				{
					bestvalue = value;
					bestseg = seg;
				}
				else if (value < 0)
				{
					nosplitters = true;
				}
			}
			else
			{
				pseg = pseg;
			}
		}

		seg = pseg->next;
	}

	if (bestseg == DWORD_MAX)
	{ // No lines split any others into two sets, so this is a convex region.
	D(Printf ("set %d, step %d, nosplit %d has no good splitter (%d)\n", set, step, nosplit, nosplitters));
		return nosplitters ? -1 : 0;
	}

	D(Printf ("split seg %lu in set %d, score %d, step %d, nosplit %d\n", bestseg, set, bestvalue, step, nosplit));

	splitseg = bestseg;
	SetNodeFromSeg (node, &Segs[bestseg]);
	return 1;
}

// Given a splitter (node), returns a score based on how "good" the resulting
// split in a set of segs is. Higher scores are better. -1 means this splitter
// splits something it shouldn't and will only be returned if honorNoSplit is
// true. A score of 0 means that the splitter does not split any of the segs
// in the set.

int FNodeBuilder::Heuristic (node_t &node, DWORD set, bool honorNoSplit)
{
	int score = 0;
	int segsInSet = 0;
	int counts[2] = { 0, 0 };
	int realSegs[2] = { 0, 0 };
	int specialSegs[2] = { 0, 0 };
	DWORD i = set;
	int sidev1, sidev2;
	int side;
	bool splitter = false;
	unsigned int max, m2, p, q;

	Touched.Clear ();
	Colinear.Clear ();

	while (i != DWORD_MAX)
	{
		const FPrivSeg *test = &Segs[i];

		if (HackSeg == i)
		{
			side = 1;
		}
		else
		{
			side = ClassifyLine (node, test, sidev1, sidev2);
		}
		switch (side)
		{
		case 0:	// Seg is on only one side of the partition
		case 1:
			// If we don't split this line, but it abuts the splitter, also reject it.
			// The "right" thing to do in this case is to only reject it if there is
			// another nosplit seg from the same sector at this vertex. Note that a line
			// that lies exactly on top of the splitter is okay.
			if (test->loopnum && honorNoSplit && (sidev1 == 0 || sidev2 == 0))
			{
				if ((sidev1 | sidev2) != 0)
				{
					max = Touched.Size();
					for (p = 0; p < max; ++p)
					{
						if (Touched[p] == test->loopnum)
						{
							break;
						}
					}
					if (p == max)
					{
						Touched.Push (test->loopnum);
					}
				}
				else
				{
					max = Colinear.Size();
					for (p = 0; p < max; ++p)
					{
						if (Colinear[p] == test->loopnum)
						{
							break;
						}
					}
					if (p == max)
					{
						Colinear.Push (test->loopnum);
					}
				}
			}

			counts[side]++;
			if (test->linedef != -1)
			{
				realSegs[side]++;
				if (test->frontsector == test->backsector)
				{
					specialSegs[side]++;
				}
				// Add some weight to the score for unsplit lines
				score += SplitCost;	
			}
			else
			{
				// Minisegs don't count quite as much for nosplitting
				score += SplitCost / 4;
			}
			break;

		default:	// Seg is cut by the partition
			// If we are not allowed to split this seg, reject this splitter
			if (test->loopnum)
			{
				if (honorNoSplit)
				{
					D(Printf ("Splits seg %d\n", i));
					return -1;
				}
				else
				{
					splitter = true;
				}
			}

			counts[0]++;
			counts[1]++;
			if (test->linedef != -1)
			{
				realSegs[0]++;
				realSegs[1]++;
				if (test->frontsector == test->backsector)
				{
					specialSegs[0]++;
					specialSegs[1]++;
				}
			}
			break;
		}

		segsInSet++;
		i = test->next;
	}

	// If this line is outside all the others, return a special score
	if (counts[0] == 0 || counts[1] == 0)
	{
		return 0;
	}

	// A splitter must have at least one real seg on each side.
	// Otherwise, a subsector could be left without any way to easily
	// determine which sector it lies inside.
	if (realSegs[0] == 0 || realSegs[1] == 0)
	{
		D(Printf ("Leaves a side with only mini segs\n"));
		return -1;
	}

	// Try to avoid splits that leave only "special" segs, so that the generated
	// subsectors have a better chance of choosing the correct sector. This situation
	// is not neccesarily bad, just undesirable.
	if (honorNoSplit && (specialSegs[0] == realSegs[0] || specialSegs[1] == realSegs[1]))
	{
		D(Printf ("Leaves a side with only special segs\n"));
		return -1;
	}

	// If this splitter intersects any vertices of segs that should not be split,
	// check if it is also colinear with another seg from the same sector. If it
	// is, the splitter is okay. If not, it should be rejected. Why? Assuming that
	// polyobject containers are convex (which they should be), a splitter that
	// is colinear with one of the sector's segs and crosses the vertex of another
	// seg of that sector must be crossing the container's corner and does not
	// actually split the container.

	max = Touched.Size ();
	m2 = Colinear.Size ();

	// If honorNoSplit is false, then both these lists will be empty.

	// If the splitter touches some vertices without being colinear to any, we
	// can skip further checks and reject this right away.
	if (m2 == 0 && max > 0)
	{
		return -1;
	}

	for (p = 0; p < max; ++p)
	{
		int look = Touched[p];
		for (q = 0; q < m2; ++q)
		{
			if (look == Colinear[q])
			{
				break;
			}
		}
		if (q == m2)
		{ // Not a good one
			return -1;
		}
	}

	// Doom maps are primarily axis-aligned lines, so it's usually a good
	// idea to prefer axis-aligned splitters over diagonal ones. Doom originally
	// had special-casing for orthogonal lines, so they performed better. ZDoom
	// does not care about the line's direction, so this is merely a choice to
	// try and improve the final tree.

	if ((node.dx == 0) || (node.dy == 0))
	{
		// If we have to split a seg we would prefer to keep unsplit, give
		// extra precedence to orthogonal lines so that the polyobjects
		// outside the entrance to MAP06 in Hexen MAP02 display properly.
		if (splitter)
		{
			score += segsInSet*8;
		}
		else
		{
			score += segsInSet/AAPreference;
		}
	}

	score += (counts[0] + counts[1]) - abs(counts[0] - counts[1]);

	return score;
}

int FNodeBuilder::ClassifyLine (node_t &node, const FPrivSeg *seg, int &sidev1, int &sidev2)
{
	const FPrivVert *v1 = &Vertices[seg->v1];
	const FPrivVert *v2 = &Vertices[seg->v2];
	sidev1 = PointOnSide (v1->x, v1->y, node.x, node.y, node.dx, node.dy);
	sidev2 = PointOnSide (v2->x, v2->y, node.x, node.y, node.dx, node.dy);

	if ((sidev1 | sidev2) == 0)
	{ // seg is coplanar with the splitter, so use its orientation to determine
	  // which child it ends up in. If it faces the same direction as the splitter,
	  // it goes in front. Otherwise, it goes in back.

		if (node.dx != 0)
		{
			if ((node.dx > 0 && v2->x > v1->x) || (node.dx < 0 && v2->x < v1->x))
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			if ((node.dy > 0 && v2->y > v1->y) || (node.dy < 0 && v2->y < v1->y))
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
	else if (sidev1 <= 0 && sidev2 <= 0)
	{
		return 0;
	}
	else if (sidev1 >= 0 && sidev2 >= 0)
	{
		return 1;
	}
	return -1;
}

void FNodeBuilder::SplitSegs (DWORD set, node_t &node, DWORD splitseg, DWORD &outset0, DWORD &outset1)
{
	outset0 = DWORD_MAX;
	outset1 = DWORD_MAX;

	Events.DeleteAll ();
	SplitSharers.Clear ();

	while (set != DWORD_MAX)
	{
		bool hack;
		FPrivSeg *seg = &Segs[set];
		int next = seg->next;

		int sidev1, sidev2, side;

		if (HackSeg == set)
		{
			HackSeg = DWORD_MAX;
			side = 1;
			sidev1 = sidev2 = 0;
			hack = true;
		}
		else
		{
			side = ClassifyLine (node, seg, sidev1, sidev2);
			hack = false;
		}

		switch (side)
		{
		case 0: // seg is entirely in front
			seg->next = outset0;
			outset0 = set;
			break;

		case 1: // seg is entirely in back
			seg->next = outset1;
			outset1 = set;
			break;

		default: // seg needs to be split
			double frac;
			FPrivVert newvert;
			unsigned int vertnum;
			int seg2;
			unsigned int i;

			if (seg->loopnum)
			{
				Printf ("   Split seg %lu (%ld,%ld)-(%ld,%ld) of sector %d in loop %d\n",
					set,
					Vertices[seg->v1].x>>16, Vertices[seg->v1].y>>16,
					Vertices[seg->v2].x>>16, Vertices[seg->v2].y>>16,
					seg->frontsector - sectors, seg->loopnum);
			}

			frac = InterceptVector (node, *seg);
			newvert.x = Vertices[seg->v1].x;
			newvert.y = Vertices[seg->v1].y;
			newvert.x += fixed_t(frac * double(Vertices[seg->v2].x - newvert.x));
			newvert.y += fixed_t(frac * double(Vertices[seg->v2].y - newvert.y));
			for (i = 0; i < Vertices.Size(); ++i)
			{
				if (abs(Vertices[i].x - newvert.x) < VERTEX_EPSILON &&
					abs(Vertices[i].y - newvert.y) < VERTEX_EPSILON)
				{
					break;
				}
			}
			if (i < Vertices.Size())
			{
				vertnum = i;
			}
			else
			{
				newvert.segs = DWORD_MAX;
				newvert.segs2 = DWORD_MAX;
				vertnum = Vertices.Push (newvert);
			}

			seg2 = SplitSeg (set, vertnum, sidev1);

			Segs[seg2].next = outset0;
			outset0 = seg2;
			Segs[set].next = outset1;
			outset1 = set;

			// Also split the seg on the back side
			if (Segs[set].partner != DWORD_MAX)
			{
				int partner1 = Segs[set].partner;
				int partner2 = SplitSeg (partner1, vertnum, sidev2);
				// The newly created seg stays in the same set as the
				// back seg because it has not been considered for splitting
				// yet. If it had been, then the front seg would have already
				// been split, and we would not be in this default case.
				// Moreover, the back seg may not even be in the set being
				// split, so we must not move its pieces into the out sets.
				Segs[partner1].next = partner2;
				Segs[partner2].partner = seg2;
				Segs[seg2].partner = partner2;
			}

			if (GLNodes)
			{
				AddIntersection (node, vertnum);
			}

			break;
		}
		if (side >= 0 && GLNodes)
		{
			if (sidev1 == 0)
			{
				double dist1 = AddIntersection (node, seg->v1);
				if (sidev2 == 0)
				{
					double dist2 = AddIntersection (node, seg->v2);
					FSplitSharer share = { dist1, set, dist2 > dist1 };
					SplitSharers.Push (share);
				}
			}
			else if (sidev2 == 0)
			{
				AddIntersection (node, seg->v2);
			}
		}
		if (hack && GLNodes)
		{
			DWORD newback, newfront;

			newback = AddMiniseg (seg->v2, seg->v1, DWORD_MAX, set, splitseg);
			newfront = AddMiniseg (Segs[set].v1, Segs[set].v2, newback, set, splitseg);
			Segs[newback].frontsector = Segs[newback].backsector =
				Segs[newfront].frontsector = Segs[newfront].backsector =
				Segs[set].frontsector;

			Segs[newback].next = outset1;
			outset1 = newback;
			Segs[newfront].next = outset0;
			outset0 = newfront;
		}
		set = next;
	}
	FixSplitSharers (node);
	if (GLNodes)
	{
		AddMinisegs (node, splitseg, outset0, outset1);
	}
}

void FNodeBuilder::SetNodeFromSeg (node_t &node, const FPrivSeg *pseg) const
{
	if (pseg->planenum >= 0)
	{
		FSimpleLine *pline = &Planes[pseg->planenum];
		node.x = pline->x;
		node.y = pline->y;
		node.dx = pline->dx;
		node.dy = pline->dy;
	}
	else
	{
		node.x = Vertices[pseg->v1].x;
		node.y = Vertices[pseg->v1].y;
		node.dx = Vertices[pseg->v2].x - node.x;
		node.dy = Vertices[pseg->v2].y - node.y;
	}
}

DWORD FNodeBuilder::SplitSeg (DWORD segnum, int splitvert, int v1InFront)
{
	double dx, dy;
	FPrivSeg newseg;
	int newnum = (int)Segs.Size();

	newseg = Segs[segnum];
	dx = double(Vertices[splitvert].x - Vertices[newseg.v1].x);
	dy = double(Vertices[splitvert].y - Vertices[newseg.v1].y);
	if (v1InFront > 0)
	{
		newseg.v1 = splitvert;
		Segs[segnum].v2 = splitvert;

		RemoveSegFromVert2 (segnum, newseg.v2);

		newseg.nextforvert = Vertices[splitvert].segs;
		Vertices[splitvert].segs = newnum;

		newseg.nextforvert2 = Vertices[newseg.v2].segs2;
		Vertices[newseg.v2].segs2 = newnum;

		Segs[segnum].nextforvert2 = Vertices[splitvert].segs2;
		Vertices[splitvert].segs2 = segnum;
	}
	else
	{
		Segs[segnum].v1 = splitvert;
		newseg.v2 = splitvert;

		RemoveSegFromVert1 (segnum, newseg.v1);

		newseg.nextforvert = Vertices[newseg.v1].segs;
		Vertices[newseg.v1].segs = newnum;

		newseg.nextforvert2 = Vertices[splitvert].segs2;
		Vertices[splitvert].segs2 = newnum;

		Segs[segnum].nextforvert = Vertices[splitvert].segs;
		Vertices[splitvert].segs = segnum;
	}

	Segs.Push (newseg);

	D(Printf ("Split seg %d to get seg %d\n", segnum, newnum));

	return newnum;
}

void FNodeBuilder::RemoveSegFromVert1 (DWORD segnum, int vertnum)
{
	FPrivVert *v = &Vertices[vertnum];

	if (v->segs == segnum)
	{
		v->segs = Segs[segnum].nextforvert;
	}
	else
	{
		DWORD prev, curr;
		prev = 0;
		curr = v->segs;
		while (curr != DWORD_MAX && curr != segnum)
		{
			prev = curr;
			curr = Segs[curr].nextforvert;
		}
		if (curr == segnum)
		{
			Segs[prev].nextforvert = Segs[curr].nextforvert;
		}
	}
}

void FNodeBuilder::RemoveSegFromVert2 (DWORD segnum, int vertnum)
{
	FPrivVert *v = &Vertices[vertnum];

	if (v->segs2 == segnum)
	{
		v->segs2 = Segs[segnum].nextforvert2;
	}
	else
	{
		DWORD prev, curr;
		prev = 0;
		curr = v->segs2;
		while (curr != DWORD_MAX && curr != segnum)
		{
			prev = curr;
			curr = Segs[curr].nextforvert2;
		}
		if (curr == segnum)
		{
			Segs[prev].nextforvert2 = Segs[curr].nextforvert2;
		}
	}
}

double FNodeBuilder::InterceptVector (const node_t &splitter, const FPrivSeg &seg)
{
	double v2x = (double)Vertices[seg.v1].x;
	double v2y = (double)Vertices[seg.v1].y;
	double v2dx = (double)Vertices[seg.v2].x - v2x;
	double v2dy = (double)Vertices[seg.v2].y - v2y;
	double v1dx = (double)splitter.dx;
	double v1dy = (double)splitter.dy;

	double den = v1dy*v2dx - v1dx*v2dy;

	if (den == 0.0)
		return 0;		// parallel

	double v1x = (double)splitter.x;
	double v1y = (double)splitter.y;

	double num = (v1x - v2x)*v1dy + (v2y - v1y)*v1dx;
	double frac = num / den;

	return frac;
}

int FNodeBuilder::PointOnSide (int x, int y, int x1, int y1, int dx, int dy)
{
	// For most cases, a simple dot product is enough.
	double d_dx = double(dx);
	double d_dy = double(dy);
	double d_x = double(x);
	double d_y = double(y);
	double d_x1 = double(x1);
	double d_y1 = double(y1);

	double s_num = (d_y1-d_y)*d_dx - (d_x1-d_x)*d_dy;

	if (fabs(s_num) < 17179869184.0)	// 4<<32
	{
		// Either the point is very near the line, or the segment defining
		// the line is very short: Do a more expensive test to determine
		// just how far from the line the point is.
		double l = sqrt(d_dx*d_dx+d_dy*d_dy);
		double dist = fabs(s_num)/l;
		if (dist < SIDE_EPSILON)
		{
			return 0;
		}
	}
	return s_num > 0.0 ? -1 : 1;
}

void FNodeBuilder::PrintSet (int l, DWORD set)
{
	Printf ("set %d:\n", l);
	for (; set != DWORD_MAX; set = Segs[set].next)
	{
		Printf ("\t%lu(%d):%d(%ld,%ld)-%d(%ld,%ld) ", set, Segs[set].frontsector-sectors,
			Segs[set].v1,
			Vertices[Segs[set].v1].x>>16, Vertices[Segs[set].v1].y>>16,
			Segs[set].v2,
			Vertices[Segs[set].v2].x>>16, Vertices[Segs[set].v2].y>>16);
	}
	Printf ("*\n");
}
