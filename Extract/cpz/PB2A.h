#pragma once

#include "PB.h"
#include "../../ArcFile.h"

class CPB2A : public CPB
{
public:

    BOOL    Decode( CArcFile* pclArc, void* pbtSrc, DWORD dwSrcSize );

protected:

    void    Decrypt( BYTE* pbtTarget, DWORD dwSize );

    BOOL    Decode1( CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decode2( CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decode4( CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decode5( CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decode6( CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );

    BOOL    Decomp1( BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decomp2( BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decomp4( BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp );
    BOOL    Decomp5( BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, const BYTE* pbtBase, DWORD dwFrameNumber );
    BOOL    Decomp6( BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, const BYTE* pbtBase, DWORD dwFrameNumber );
};
