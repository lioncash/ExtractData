#pragma once

#include "../ArcFile.h"

class CLZSS
{
public:

    BOOL Decode( CArcFile* pclArc );

    BOOL Decomp( CArcFile* pclArc, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset );
    BOOL Decomp( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset );
};
