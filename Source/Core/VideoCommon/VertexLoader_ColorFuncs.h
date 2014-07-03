#include "Common/Common.h"
#include "VideoCommon/VertexLoader.h"

#pragma once
#define ASHIFT 24
#define AMASK 0xFF000000

extern int colIndex;
extern int colElements[2];


__forceinline void _SetCol(u32 val)
{
	DataWrite(val);
	colIndex++;
}

//color comes in format BARG in 16 bits
//BARG -> AABBGGRR
__forceinline void _SetCol4444(u16 val)
{
	u32 col = (val & 0xF0);				// col  = 000000R0;
	col |= (val & 0xF) << 12;		// col |= 0000G000;
	col |= (((u32)val) & 0xF000) << 8;	// col |= 00B00000;
	col |= (((u32)val) & 0x0F00) << 20;	// col |= A0000000;
	col |= col >> 4;					// col =  A0B0G0R0 | 0A0B0G0R;
	_SetCol(col);
}

//color comes in format RGBA
//RRRRRRGG GGGGBBBB BBAAAAAA
__forceinline void _SetCol6666(u32 val)
{
	u32 col = (val >> 16) & 0xFC;
	col |= (val >> 2) & 0xFC00;
	col |= (val << 12) & 0xFC0000;
	col |= (val << 26) & 0xFC000000;
	col |= (col >> 6) & 0x03030303;
	_SetCol(col);
}

//color comes in RGB
//RRRRRGGG GGGBBBBB
__forceinline void _SetCol565(u16 val)
{
	u32 col = (val >> 8) & 0xF8;
	col |= (val << 5) & 0xFC00;
	col |= (((u32)val) << 19) & 0xF80000;
	col |= (col >> 5) & 0x070007;
	col |= (col >> 6) & 0x000300;
	_SetCol(col | AMASK);
}

__forceinline u32 _Read24(const u8 *addr)
{
	return (*(const u32 *)addr) | AMASK;
}

__forceinline u32 _Read32(const u8 *addr)
{
	return *(const u32 *)addr;
}

__forceinline void _Color_ReadDirect_24b_888()
{
	_SetCol(_Read24(DataGetPosition()));
	DataSkip(3);
}

__forceinline void _Color_ReadDirect_32b_888x()
{
	_SetCol(_Read24(DataGetPosition()));
	DataSkip(4);
}

__forceinline void _Color_ReadDirect_16b_565()
{
	_SetCol565(DataReadU16());
}

__forceinline void _Color_ReadDirect_16b_4444()
{
	_SetCol4444(*(u16*)DataGetPosition());
	DataSkip(2);
}

__forceinline void _Color_ReadDirect_24b_6666()
{
	_SetCol6666(Common::swap32(DataGetPosition() - 1));
	DataSkip(3);
}
// F|RES: i am not 100 percent sure, but the colElements seems to be important for rendering only
// at least it fixes mario party 4
//
//	if (colElements[colIndex])	
//	else
//		col |= 0xFF<<ASHIFT;
//
__forceinline void _Color_ReadDirect_32b_8888()
{
	// TODO (mb2): check this
	u32 col = DataReadU32Unswapped();

	// "kill" the alpha
	if (!colElements[colIndex])
		col |= 0xFF << ASHIFT;

	_SetCol(col);
}

template <typename T>
__forceinline u8* IndexedColorPosition()
{
	auto const index = DataRead<T>();
	return cached_arraybases[ARRAY_COLOR + colIndex] + (index * arraystrides[ARRAY_COLOR + colIndex]);
}

template <typename I>
__forceinline void Color_ReadIndex_16b_565()
{
	u16 val = Common::swap16(*(const u16 *)IndexedColorPosition<I>());
	_SetCol565(val);
}

template <typename I>
__forceinline void Color_ReadIndex_24b_888()
{
	const u8 *iAddress = IndexedColorPosition<I>();
	_SetCol(_Read24(iAddress));
}

template <typename I>
__forceinline void Color_ReadIndex_32b_888x()
{
	const u8 *iAddress = IndexedColorPosition<I>();
	_SetCol(_Read24(iAddress));
}

template <typename I>
__forceinline void Color_ReadIndex_16b_4444()
{
	u16 val = *(const u16 *)IndexedColorPosition<I>();
	_SetCol4444(val);
}

template <typename I>
__forceinline void Color_ReadIndex_24b_6666()
{
	const u8* pData = IndexedColorPosition<I>() - 1;
	u32 val = Common::swap32(pData);
	_SetCol6666(val);
}

template <typename I>
__forceinline void Color_ReadIndex_32b_8888()
{
	const u8 *iAddress = IndexedColorPosition<I>();
	_SetCol(_Read32(iAddress));
}