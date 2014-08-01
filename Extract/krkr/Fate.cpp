#include "stdafx.h"
#include "Fate.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Determine if decodable
//
// Parameters:
//   - pclArc - Archive

BOOL CFate::OnCheckDecrypt(CArcFile* pclArc)
{
	static const char* apszMD5[] =
	{
		"4BE5F676CE80429B3245DA9E721FE23B",
		"7570E3AFA14F221268DCC48CAEE1277F",
	};

	for (DWORD i = 0; i < _countof(apszMD5); i++)
	{
		if (CheckTpm(apszMD5[i]))
		{
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization of Decryption Routine
//
// Parameters:
//   - pclArc - Archive

DWORD CFate::OnInitDecrypt(CArcFile* pclArc)
{
	return 0x36;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decoding Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CFate::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		pbtTarget[i] ^= dwDecryptKey;

		if ((dwOffset + i) == 0x13)
		{
			pbtTarget[i] ^= 0x01;
		}

		if ((dwOffset + i) == 0x2EA29)
		{
			pbtTarget[i] ^= 0x03;
		}
	}

	return dwTargetSize;
}
