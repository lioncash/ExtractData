#include "stdafx.h"
#include "Braban.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Determine if it can be decoded

BOOL CBraban::OnCheckDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return CheckTpm( "C42B141D42FCBCDA29850FA9E9FE3FF2" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization of Decoding Process

DWORD CBraban::OnInitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return ((pclArc->GetOpenFileInfo()->key ^ 0xFF) & 0xFF) - 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decoding Process

DWORD CBraban::OnDecrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding Size
	DWORD				dwOffset,						// Location of data to be decoded (offset)
	DWORD				dwDecryptKey					// Decryption key
	)
{
	BYTE btDecryptkey = (BYTE) dwDecryptKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptkey;
	}

	return dwTargetSize;
}
