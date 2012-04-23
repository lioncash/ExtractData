#include	"stdafx.h"
#include	"Specialite.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Check if it can be decrypted

BOOL	CSpecialite::OnCheckDecrypt(
    CArcFile*			pclArc							// Archive
    )
{
    return	CheckTpm( "F78FF15C0BD396080BCDF431AED59211" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Initialize Decryption Process

DWORD	CSpecialite::OnInitDecrypt(
    CArcFile*			pclArc							// Archive
    )
{
    return	0xAF;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption Process

DWORD	CSpecialite::OnDecrypt(
    BYTE*				pbtTarget,						// Data to be decrypted
    DWORD				dwTargetSize,					// Decoding Size
    DWORD				dwOffset,						// Location of data to be decoded (offset)
    DWORD				dwDecryptKey					// Decryption Key
    )
{
    BYTE				btDecryptKey = (BYTE) dwDecryptKey;

    for( DWORD i = 0 ; i < dwTargetSize ; i++ )
    {
        pbtTarget[i] ^= btDecryptKey;
        pbtTarget[i] += 1;
    }

    return	dwTargetSize;
}
