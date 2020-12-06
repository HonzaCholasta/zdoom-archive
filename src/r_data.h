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
//	Refresh module, data I/O, caching, retrieval of graphics
//	by name.
//
//-----------------------------------------------------------------------------


#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"


// A texture that doesn't really exist
class FDummyTexture : public FTexture
{
public:
	FDummyTexture ();
	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();
	void SetSize (int width, int height);
};


// A texture that is just a single patch
class FPatchTexture : public FTexture
{
public:
	FPatchTexture (int lumpnum, int usetype);
	~FPatchTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	int SourceLump;
	BYTE *Pixels;
	Span **Spans;

	virtual void MakeTexture ();
	void HackHack (int newheight);
	void GetDimensions ();

	friend class FMultiPatchTexture;
};

// A texture defined in a TEXTURE1 or TEXTURE2 lump
class FMultiPatchTexture : public FTexture
{
public:
	FMultiPatchTexture (const void *texdef, FTexture **patchlookup, bool strife);
	~FMultiPatchTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	BYTE *Pixels;
	Span **Spans;

	struct TexPart
	{
		SWORD OriginX, OriginY;
		FTexture *Texture;
	};

	int NumParts;
	TexPart *Parts;
	bool bRedirect;

	void MakeTexture ();

private:
	void CheckForHacks ();
};

// A texture defined between F_START and F_END markers
class FFlatTexture : public FTexture
{
public:
	FFlatTexture (int lumpnum);
	~FFlatTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	int SourceLump;
	BYTE *Pixels;
	Span DummySpans[2];

	void MakeTexture ();
};

// A texture defined in a Build TILESxxx.ART file
class FBuildTexture : public FTexture
{
public:
	FBuildTexture (int tilenum, const BYTE *pixels, int width, int height, int left, int top);
	~FBuildTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	const BYTE *Pixels;
	Span **Spans;
};


// A raw 320x200 graphic used by Heretic and Hexen fullscreen images
class FRawPageTexture : public FTexture
{
public:
	FRawPageTexture (int lumpnum);
	~FRawPageTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	int SourceLump;
	BYTE *Pixels;
	static const Span DummySpans[2];

	void MakeTexture ();
};


// An IMGZ image (mostly just crosshairs)
class FIMGZTexture : public FTexture
{
public:
	FIMGZTexture (int lumpnum);
	~FIMGZTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	int SourceLump;
	BYTE *Pixels;
	Span **Spans;

	void GetDimensions ();
	void MakeTexture ();

	struct ImageHeader;
};

// A texture that returns a wiggly version of another texture.
class FWarpTexture : public FTexture
{
public:
	FWarpTexture (FTexture *source);
	~FWarpTexture ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();

protected:
	FTexture *SourcePic;
	BYTE *Pixels;
	Span **Spans;
	DWORD GenTime;

	void MakeTexture (DWORD time);
};



// I/O, setting up the stuff.
void R_InitData (void);
void R_PrecacheLevel (void);


// Retrieval.


DWORD R_ColormapNumForName(const char *name);	// killough 4/4/98
void R_SetDefaultColormap (const char *name);	// [RH] change normal fadetable
DWORD R_BlendForColormap (DWORD map);		// [RH] return calculated blend for a colormap
extern byte *realcolormaps;						// [RH] make the colormaps externally visible
extern size_t numfakecmaps;

int R_FindSkin (const char *name);	// [RH] Find a skin

#endif
