#include	"stdafx.h"
#include	"Natsupochi.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Check if it can be decrypted

BOOL	CNatsupochi::OnCheckDecrypt(
    CArcFile*			pclArc							// Archive
    )
{
    return	CheckTpm( "B1CCCE8E4048B563B316D8BAEDF7E2B1" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Initialize Decryption Process

DWORD	CNatsupochi::OnInitDecrypt(
    CArcFile*			pclArc							// Archive
    )
{
    return	(pclArc->GetOpenFileInfo()->key >> 3);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Decoding Process

DWORD	CNatsupochi::OnDecrypt(
    BYTE*				pbtTarget,						// Data to be decoded
    DWORD				dwTargetSize,					// Decoding size
    DWORD				dwOffset,						// Location of data to be decoded
    DWORD				dwDecryptKey					// Decryption Key
    )
{
    BYTE				btDecryptKey = (BYTE) (dwDecryptKey & 0xFF);

    for( DWORD i = 0 ; i < dwTargetSize ; i++ )
    {
        pbtTarget[i] ^= btDecryptKey;
    }

    return	dwTargetSize;
}
