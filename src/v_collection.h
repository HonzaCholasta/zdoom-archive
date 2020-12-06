/*
** v_collection.h
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
*/

#ifndef __V_COLLECTION_H__
#define __V_COLLECTION_H__

#include "doomtype.h"

struct patch_s;

struct RawImageHeader
{
	BYTE Magic[4];
	WORD Width;
	WORD Height;
	SWORD LeftOffset;
	SWORD TopOffset;
	BYTE Compression;
	BYTE Reserved[11];
};

class FImageCollection
{
public:
	FImageCollection ();
	FImageCollection (const char **patchNames, int numPatches);
	~FImageCollection ();

	void Init (const char **patchnames, int numPatches, int namespc=0);
	void Uninit ();

	byte *GetImage (int index, int *const width, int *const height, int *const xoffs, int *const yoffs) const;
	int GetImageWidth (int index) const;
	int GetImageHeight (int index) const;

protected:
	static patch_s *CachePatch (const char *name, int namespc);

	int NumImages;
	struct ImageData
	{
		WORD Width, Height;
		SWORD XOffs, YOffs;
		byte *Data;
	} *Images;
	byte *Bitmaps;
};

#endif //__V_COLLECTION_H__
