#pragma once

#include "../../ExtractBase.h"

class CCpz : public CExtractBase
{
public:

    struct SCPZ5Header
    {
        char            szIdentifier[4];            // Identifier
        DWORD           dwDirs;                     // Number of directories
        DWORD           dwTotalDirIndexSize;        // Total size of the directory index
        DWORD           dwTotalFileIndexSize;       // Total size of the file index
        DWORD           adwUnknown1[4];             // Unknown
        DWORD           adwMD5[4];                  // MD5
        DWORD           dwIndexKey;                 // Index Key
        DWORD           adwUnKnown2[3];             // Unknown
    };


    BOOL                Mount( CArcFile* pclArc );
    BOOL                Decode( CArcFile* pclArc );


protected:

    BOOL                MountCpz1( CArcFile* pclArc );
    BOOL                MountCpz2( CArcFile* pclArc );
    BOOL                MountCpz3( CArcFile* pclArc );
    BOOL                MountCpz5( CArcFile* pclArc );

    BOOL                DecodeCpz1( CArcFile* pclArc );
    BOOL                DecodeCpz2( CArcFile* pclArc );
    BOOL                DecodeCpz3( CArcFile* pclArc );
    BOOL                DecodeCpz5( CArcFile* pclArc );

    static void         Decrypt1( BYTE* pbtTarget, DWORD dwSize );
    static void         Decrypt2( BYTE* pbtTarget, DWORD dwSize, DWORD dwKey );
    static void         Decrypt3( BYTE* pbtTarget, DWORD dwSize, DWORD dwKey );
    static void         Decrypt5( BYTE* pbtTarget, DWORD dwSize, DWORD dwKey );

    static const BYTE*  InitDecryptWithTable5( DWORD dwKey, DWORD dwSeed );
    static void	        DecryptWithTable5( BYTE* pbtTarget, DWORD dwSize, const BYTE* pbtDecryptTable, DWORD dwKey );
    static void         DecryptOfData5( BYTE* pbtTarget, DWORD dwSize, const BYTE* pbtDecryptTable, const DWORD* pdwKey, DWORD dwSeed );

    // Callback function from PB3B

    static void         OnDecrypt3FromPB3B( BYTE* pbtTarget, DWORD dwTargetSize, CArcFile* pclArc, const SFileInfo& rfstFileInfo );
    static void         OnDecrypt5FromPB3B( BYTE* pbtTarget, DWORD dwTargetSize, CArcFile* pclArc, const SFileInfo& rfstFileInfo );
};
