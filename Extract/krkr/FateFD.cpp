#include "stdafx.h"
#include "FateFD.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CFateFD::OnCheckDecrypt(CArcFile* pclArc)
{
	if (pclArc->GetArcName() != _T("video.xp3"))
		return false;

	return CheckTpm("9C5BB86A5BBD1B77A311EC504DB45653");
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CFateFD::OnInitDecrypt(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->name == _T("fd_op01.mpg"))
	{
		// OP1

		m_abtKey[0] = 0xCE;
		m_abtKey[1] = 0x94;
		m_abtKey[2] = 0xB3;
		m_abtKey[3] = 0x94;
		m_abtKey[4] = 0xF8;
		m_abtKey[5] = 0x94;

		m_adwOffset[0] = 0;
		m_adwOffset[1] = 0xB6B - file_info->start;
		m_adwOffset[2] = 0x7036 - file_info->start;
		m_adwOffset[3] = 0x7037 - file_info->start;
		m_adwOffset[4] = 0xAF27 - file_info->start;
		m_adwOffset[5] = 0xAF28 - file_info->start;
	}
	else if (file_info->name == _T("fd_op02.mpg"))
	{
		// OP2

		m_abtKey[0] = 0x5A;
		m_abtKey[1] = 0x21;
		m_abtKey[2] = 0xFE;
		m_abtKey[3] = 0x21;
		m_abtKey[4] = 0xDB;
		m_abtKey[5] = 0x21;

		m_adwOffset[0] = 0;
		m_adwOffset[1] = 0x552036D - file_info->start;
		m_adwOffset[2] = 0x552254A - file_info->start;
		m_adwOffset[3] = 0x552254B - file_info->start;
		m_adwOffset[4] = 0x552CDF6 - file_info->start;
		m_adwOffset[5] = 0x552CDF7 - file_info->start;
	}
	else
	{
		// Other

		SetDecryptRequirement(false);
	}

	return 0;
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CFateFD::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		if ((dwOffset + i) < m_adwOffset[1])
		{
			pbtTarget[i] ^= m_abtKey[0];
		}
		else if ((dwOffset + i) < m_adwOffset[2])
		{
			pbtTarget[i] ^= m_abtKey[1];
		}
		else if ((dwOffset + i) < m_adwOffset[3])
		{
			pbtTarget[i] ^= m_abtKey[2];
		}
		else if ((dwOffset + i) < m_adwOffset[4])
		{
			pbtTarget[i] ^= m_abtKey[3];
		}
		else if ((dwOffset + i) < m_adwOffset[5])
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
