/*
** emptytexture.cpp
** Texture class for empty placeholder textures
** (essentially patches with dimensions and offsets of (0,0) )
** These need special treatment because a texture size of 0 is illegal
**
**---------------------------------------------------------------------------
** Copyright 2009 Christoph Oelckers
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
**
*/

#include "files.h"
#include "filesystem.h"
#include "image.h"

//==========================================================================
//
// 
//
//==========================================================================

class FEmptyTexture : public FImageSource
{
public:
	FEmptyTexture (int lumpnum);
	TArray<uint8_t> CreatePalettedPixels(int conversion) override;
};

//==========================================================================
//
// 
//
//==========================================================================

FImageSource *EmptyImage_TryCreate(FileReader & file, int lumpnum)
{
	char check[8];
	if (file.GetLength() != 8) return NULL;
	file.Seek(0, FileReader::SeekSet);
	if (file.Read(check, 8) != 8) return NULL;
	if (memcmp(check, "\0\0\0\0\0\0\0\0", 8)) return NULL;

	return new FEmptyTexture(lumpnum);
}

FImageSource* CreateEmptyTexture()
{
	return new FEmptyTexture(0);
}

//==========================================================================
//
//
//
//==========================================================================

FEmptyTexture::FEmptyTexture (int lumpnum)
: FImageSource(lumpnum)
{
	bMasked = true;
	Width = Height = 1;
	bUseGamePalette = true;
}

//==========================================================================
//
//
//
//==========================================================================

TArray<uint8_t> FEmptyTexture::CreatePalettedPixels(int conversion)
{
	TArray<uint8_t> Pixel(1, true);
	Pixel[0] = 0;
	return Pixel;
}

