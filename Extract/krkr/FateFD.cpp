#include "stdafx.h"
#include "FateFD.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decoded

BOOL CFateFD::OnCheckDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->GetArcName() != _T("video.xp3") )
	{
		return FALSE;
	}

	return CheckTpm( "9C5BB86A5BBD1B77A311EC504DB45653" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Routine

DWORD CFateFD::OnInitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->name == _T("fd_op01.mpg") )
	{
		// OP1

		m_abtKey[0] = 0xCE;
		m_abtKey[1] = 0x94;
		m_abtKey[2] = 0xB3;
		m_abtKey[3] = 0x94;
		m_abtKey[4] = 0xF8;
		m_abtKey[5] = 0x94;

		m_adwOffset[0] = 0;
		m_adwOffset[1] = 0xB6B - pstFileInfo->start;
		m_adwOffset[2] = 0x7036 - pstFileInfo->start;
		m_adwOffset[3] = 0x7037 - pstFileInfo->start;
		m_adwOffset[4] = 0xAF27 - pstFileInfo->start;
		m_adwOffset[5] = 0xAF28 - pstFileInfo->start;
	}
	else if( pstFileInfo->name == _T("fd_op02.mpg") )
	{
		// OP2

		m_abtKey[0] = 0x5A;
		m_abtKey[1] = 0x21;
		m_abtKey[2] = 0xFE;
		m_abtKey[3] = 0x21;
		m_abtKey[4] = 0xDB;
		m_abtKey[5] = 0x21;

		m_adwOffset[0] = 0;
		m_adwOffset[1] = 0x552036D - pstFileInfo->start;
		m_adwOffset[2] = 0x552254A - pstFileInfo->start;
		m_adwOffset[3] = 0x552254B - pstFileInfo->start;
		m_adwOffset[4] = 0x552CDF6 - pstFileInfo->start;
		m_adwOffset[5] = 0x552CDF7 - pstFileInfo->start;
	}
	else
	{
		// Other

		SetDecryptRequirement( FALSE );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decoding Process

DWORD CFateFD::OnDecrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding size
	DWORD				dwOffset,						// Location of data to be decoded (offset)
	DWORD				dwDecryptKey					// Decryption key
	)
{
	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		if( (dwOffset + i) < m_adwOffset[1] )
		{
			pbtTarget[i] ^= m_abtKey[0];
		}
		else if( (dwOffset + i) < m_adwOffset[2] )
		{
			pbtTarget[i] ^= m_abtKey[1];
		}
		else if( (dwOffset + i) < m_adwOffset[3] )
		{
			pbtTarget[i] ^= m_abtKey[2];
		}
		else if( (dwOffset + i) < m_adwOffset[4] )
		{
			pbtTarget[i] ^= m_abtKey[3];
		}
		else if( (dwOffset + i) < m_adwOffset[5] )
		{
			pbtTarget[i] ^= m_abtKey[4];
		}
		else
		{
			pbtTarget[i] ^= m_abtKey[5];
		}
	}

	return dwTargetSize;
}
