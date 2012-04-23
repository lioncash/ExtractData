#include	"stdafx.h"
#include	"SisMiko.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Check if it can be decrypted

BOOL	CSisMiko::OnCheckDecrypt(
    CArcFile*			pclArc							// Archive
    )
{
    return	pclArc->CheckExe( _T("SisuMiko.exe") );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Initialize Decryption Process

DWORD	CSisMiko::OnInitDecrypt(
    CArcFile*			pclArc							// Archive
    )
{
    SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();
    LPCTSTR				pszFileExt = PathFindExtension( pstFileInfo->name );
    DWORD				dwKey = pclArc->GetOpenFileInfo()->key;

    if( (lstrcmp( pszFileExt, _T(".dll") ) == 0) || (pstFileInfo->name == _T("startup.tjs")) )
    {
        // Files we don't decrypt

        SetDecryptRequirement( FALSE );
        return	0;
    }

    // Size to decrypt

    if( (lstrcmp( pszFileExt, _T(".ks") ) != 0) && (lstrcmp( pszFileExt, _T(".tjs") ) != 0) && (lstrcmp( pszFileExt, _T(".asd") ) != 0) )
    {
        SetDecryptSize( 256 );
    }

    // Decryption Key

    return	~((dwKey << 16) | (dwKey >> 16));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Decryption Process

DWORD	CSisMiko::OnDecrypt(
    BYTE*				pbtTarget,						// Data to be decoded
    DWORD				dwTargetSize,					// Decoding size
    DWORD				dwOffset,						// Location of data to be decoded (offset)
    DWORD				dwDecryptKey					// Decryption Key
    )
{
    // Decrypt

    for( DWORD i = 0 ; i < dwTargetSize ; i += 4 )
    {
        *(DWORD*) &pbtTarget[i] ^= dwDecryptKey;
    }

    return	dwTargetSize;
}
