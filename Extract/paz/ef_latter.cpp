#include "stdafx.h"
#include "ef_latter.h"

/// Check if files are supported
///
/// @param pclArc Archive
///
BOOL CEFLatter::IsSupported(CArcFile* pclArc)
{
	// Get header
	BYTE abtHeader[32];
	memcpy(abtHeader, pclArc->GetHed(), sizeof(abtHeader));

	// Get decryption key
	m_btKey = abtHeader[9];

	// Decryption
	Decrypt(abtHeader, sizeof(abtHeader));
	return (memcmp(abtHeader, "ef_latter", 9) == 0);
}

/// Mount
///
/// @param pclArc Archive
///
BOOL CEFLatter::Mount(CArcFile* pclArc)
{
	if (!IsSupported(pclArc))
		return FALSE;

	// Skip 32 bytes
	pclArc->SeekHed(32);

	return CPaz::Mount(pclArc);
}

/// Decode
///
/// @param pclArc Archive
///
BOOL CEFLatter::Decode(CArcFile* pclArc)
{
	if (!IsSupported(pclArc))
		return FALSE;

	return CPaz::Decode(pclArc);
}

/// Initialize Mount Key
///
/// @param pclArc Archive
///
void CEFLatter::InitMountKey(CArcFile* pclArc)
{
	static const SKeyInfo astKeyInfo[] =
	{
		{ _T("scr"),   "\x07\x81\x2B\x53\xAF\xA3\x2B\x3B\x2D\xA8\x5A\xBB\x5C\x50\x50\x8B\xF3\xCD\xC3\x23\x84\xF4\x9F\xE8\x24\xFB\x74\xA7\x46\xD9\x1B\x1B" },
		{ _T("bg"),    "\x1A\x59\x52\xD9\x1E\xE4\xDA\x31\xCF\xC6\xA0\xA7\x94\xE5\x41\xA6\xBD\x37\xBB\xDF\xD0\xED\x4B\x3E\xAB\x8D\x4E\x1A\x9B\xF6\xC5\x8B" },
		{ _T("st"),    "\x09\x55\x97\x79\xD4\xE7\x0D\x93\x02\xE1\x9C\x47\x97\x81\xB5\x50\xAE\x9A\x56\xDE\xFC\x83\xE7\x4D\xFD\x71\xBE\x14\xFD\x27\x9B\xE1" },
		{ _T("sys"),   "\xDF\x74\xCF\x44\x3A\xD5\x54\x0E\xB9\x09\x52\xE4\x0F\xB0\x88\x83\x0A\x40\x26\x06\x2E\xA0\x66\x56\x51\xC9\x16\x54\xD1\xDB\xE5\xC3" },
		{ _T("bgm"),   "\xFC\xA7\x93\x36\x69\x34\x61\x41\x0F\x10\x88\xBA\x11\x9E\x50\x3E\xC8\x5F\x91\x89\xB4\xA2\x7E\x67\x33\xA5\x5D\x2F\xA0\x07\xF7\x9A" },
		{ _T("voice"), "\x00\x95\xFE\xF8\x45\x42\x0D\xCB\xFF\x3F\x56\x01\x17\xD4\x26\x32\xD1\x2C\x70\xA7\xFE\x0D\xB8\xC4\x4D\xA2\x63\x97\x6E\xED\xE2\x27" },
		{ _T("se"),    "\xD2\x30\xE2\xEC\xBB\x90\x2A\x53\xAB\x1D\xA5\xE6\xF5\xB4\xD9\x62\xEE\xB6\x6E\x83\xFA\x6B\xDD\x54\x86\x3C\xA5\xAD\x06\x01\xF1\x11" },
		{ _T("mov"),   "\xC2\xEA\xFF\xAB\x99\xCE\xE2\x4B\x9A\xA5\xFD\xEE\x5A\xCA\x77\xCA\x6A\xF3\x71\x15\xD9\x14\x48\x37\xD3\x22\x43\xD1\xBB\x36\x72\x2F" }
	};

	SetKey(pclArc, astKeyInfo);
}

/// Initialize Decode Key
///
/// @param pclArc Archive
///
void CEFLatter::InitDecodeKey(CArcFile* pclArc)
{
	static const SKeyInfo astKeyInfo[] =
	{
		{ _T("scr"),   "\xD3\xE0\x5D\x33\x9F\x4E\xC7\x9D\xCB\xB4\x18\xFE\xBD\xCA\x7D\x1B\x36\x45\xCD\x47\x31\xE3\x2A\x2C\xF1\x04\x5A\xE5\x28\x09\xAB\xA0" },
		{ _T("bg"),    "\x9D\xFD\xE2\x56\xE0\x70\x79\x4E\x3F\x46\x8D\x49\xC6\x68\x22\x32\x00\x2D\x7B\x0A\x47\x2C\x90\x17\x9B\x44\x0B\x61\xE1\xD9\x98\x1F" },
		{ _T("st"),    "\xA2\xCC\xD4\xF8\xB6\xF7\xC9\x2F\x91\x28\xE1\x2D\x91\x8F\x71\xC7\x75\x01\x04\x47\xC8\x26\x2F\x3A\xE7\x9D\x06\xDA\x32\x16\xF6\x1E" },
		{ _T("sys"),   "\x7A\x87\x77\x86\x8F\xB0\xA2\xC3\x9E\xCD\x85\x16\xA3\x06\xF3\xD1\x58\x23\x57\xB4\x34\x90\xED\x41\xF0\x5F\x40\xB8\x3B\x0C\x81\x1C" },
		{ _T("bgm"),   "\x73\x84\x77\x7F\x4D\xFC\x9A\x95\xF6\x0A\xE6\xCD\x50\x9A\xAF\x8F\x5E\xDA\x8D\x02\x12\x0A\x6C\xB7\x40\xC5\x23\xF5\x07\x40\x28\x58" },
		{ _T("voice"), "\x7D\xA1\x2B\xE2\x3C\x15\x3D\x2D\x10\x7C\x22\xA4\x73\x69\x05\x4A\xA0\x18\xA0\x3C\xF5\x83\xEA\x85\x22\xAE\x9B\x3B\x63\xEF\xF8\xAF" },
		{ _T("se"),    "\x83\x56\xE5\xB1\x83\x64\x38\x7D\x97\x1C\x44\xFC\x17\x34\xC2\x93\xF3\x7A\x43\x6D\x90\xF5\xCE\x9E\xCD\x12\x23\xF6\x79\x91\x2C\x51" },
		{ _T("mov"),   "\xA6\x11\x55\x3D\xED\xDC\xDE\x84\x15\xCE\xE6\x97\x44\x7B\x6E\xB3\x3F\xE5\x5F\x1E\x19\x3D\x13\xAF\x88\x2E\x55\xFF\xA1\x86\x70\xCB" }
	};

	SetKey(pclArc, astKeyInfo);

	SetDecryptKey2(pclArc);
}

/// Initialize Movie Table
///
/// @param pvTable table
///
/// @return table size
///
DWORD CEFLatter::InitMovieTable(void* pvTable)
{
	BYTE* pbtMovieTable = GetMovieTable();

	memcpy(pbtMovieTable, pvTable, 256);

	return 256;
}

/// Set decryption key
///
/// @param pclArc Archive
///
void CEFLatter::SetDecryptKey2(CArcFile* pclArc)
{
	BYTE btWork;

	// Get base archive file name
	TCHAR szBaseArcName[_MAX_FNAME];
	GetBaseArcName(szBaseArcName, pclArc->GetArcName());

	// Get base key
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	LPCTSTR pszFileExt = PathFindExtension(pstFileInfo->name);
	YCStringA clsBaseKey;

	if ((lstrcmpi(pszFileExt, _T(".png")) == 0) || (lstrcmpi(pszFileExt, _T(".png-")) == 0))
	{
		clsBaseKey = "957277W1";
	}
	else if (lstrcmpi(pszFileExt, _T(".ogg")) == 0)
	{
		clsBaseKey = "m24lX440";
	}
	else if (lstrcmpi(pszFileExt, _T(".sc")) == 0)
	{
		clsBaseKey = "A00U43Mj";
	}
	else if (lstrcmpi(pszFileExt, _T(".avi")) == 0)
	{
		clsBaseKey = "8fO1Xj6g";
	}

	// No need to decode
	if (clsBaseKey.IsEmpty())
	{
		strcpy(m_szKey, "");

		return;
	}

	// Get file name (Lower-case)
	TCHAR szFileName[_MAX_FNAME];

	lstrcpy(szFileName, pstFileInfo->name);

	if (lstrcmpi(szBaseArcName, _T("voice")) == 0)
	{
		if (lstrcmp(PathFindExtension(szFileName), _T(".ogg")) == 0)
		{
			// .ogg extension is added when you open the voice archive

			PathRemoveExtension(szFileName);
		}
	}

	CharLower(szFileName);

	// Create Key
	sprintf(m_szKey, "%s %08X %s", szFileName, pstFileInfo->sizeOrg, clsBaseKey.GetString());

	// Create Table
	if ((clsBaseKey == "8fO1Xj6g") && (lstrcmpi(pszFileExt, _T(".avi")) == 0))
	{
		// Movie File

		// Construct RC4 Table

		BYTE* pbtMovieTable = GetMovieTable();
		BYTE abtRC4Table[256];

		const char* pszKey = m_szKey;
		size_t dwKeyLength = strlen(m_szKey);

		for (DWORD i = 0; i < 256; i++)
		{
			abtRC4Table[i] = pbtMovieTable[i] ^ pszKey[i % dwKeyLength];
		}

		// Initialize Table
		BYTE abtKeyTable[256];

		for (DWORD i = 0; i < 256; i++)
		{
			abtKeyTable[i] = i;
		}

		// Construct table
		for (DWORD dwIndex1 = 0, dwIndex2 = 0; dwIndex1 < 256; dwIndex1++)
		{
			dwIndex2 = ((dwIndex2 + abtKeyTable[dwIndex1] + abtRC4Table[dwIndex1]) & 0xFF);

			btWork = abtKeyTable[dwIndex1];
			abtKeyTable[dwIndex1] = abtKeyTable[dwIndex2];
			abtKeyTable[dwIndex2] = btWork;
		}

		// Construct movie decoding table
		DWORD dwWork;

		for (DWORD i = 0, dwIndex1 = 0, dwIndex2 = 0; i < 65536; i++)
		{
			dwIndex1 = ((dwIndex1 + 1) & 0xFF);
			dwIndex2 = ((dwIndex2 + abtKeyTable[dwIndex1]) & 0xFF);

			btWork = abtKeyTable[dwIndex1];
			abtKeyTable[dwIndex1] = abtKeyTable[dwIndex2];
			abtKeyTable[dwIndex2] = btWork;

			dwWork = ((abtKeyTable[dwIndex1] + abtKeyTable[dwIndex2]) & 0xFF);

			m_abtMovieDecodeTable[i] = abtKeyTable[dwWork];
		}

		strcpy(m_szKey, "");

		m_dwMovieDecodeTablePtr = 0;
	}
	else
	{
		// Initialize Table
		for (DWORD i = 0; i < 256; i++)
		{
			m_abtKeyTable[i] = i;
		}

		// Construct Table
		const char* pszKey = m_szKey;
		size_t dwKeyLength = strlen(m_szKey);

		for (DWORD dwIndex1 = 0, dwIndex2 = 0; dwIndex1 < 256; dwIndex1++)
		{
			dwIndex2 = ((dwIndex2 + m_abtKeyTable[dwIndex1] + pszKey[dwIndex1 % dwKeyLength]) & 0xFF);

			btWork = m_abtKeyTable[dwIndex1];
			m_abtKeyTable[dwIndex1] = m_abtKeyTable[dwIndex2];
			m_abtKeyTable[dwIndex2] = btWork;
		}

		m_dwKeyTableIndex1 = 0;
		m_dwKeyTableIndex2 = 0;
	}
}

/// Decryption
///
/// @param pvTarget Data to be decoded
/// @param dwSize   Decoding size
///
void CEFLatter::Decrypt(void* pvTarget, DWORD dwSize)
{
	BYTE* pbtTarget = (BYTE*)pvTarget;

	for (DWORD i = 0; i < dwSize; i++)
	{
		pbtTarget[i] ^= m_btKey;
	}
}

/// Decryption
///
/// @param pvTarget Data to be decoded
/// @param dwSize   Decoding size
///
void CEFLatter::Decrypt2(void* pvTarget, DWORD dwSize)
{
	BYTE btWork;
	DWORD dwWork;

	BYTE* pbtTarget = (BYTE*)pvTarget;

	if (strcmp(m_szKey, "") == 0)
	{
		return;
	}

	// Decrypt

	for (DWORD i = 0; i < dwSize; i++)
	{
		m_dwKeyTableIndex1 = ((m_dwKeyTableIndex1 + 1) & 0xFF);
		m_dwKeyTableIndex2 = ((m_dwKeyTableIndex2 + m_abtKeyTable[m_dwKeyTableIndex1]) & 0xFF);

		btWork = m_abtKeyTable[m_dwKeyTableIndex1];
		m_abtKeyTable[m_dwKeyTableIndex1] = m_abtKeyTable[m_dwKeyTableIndex2];
		m_abtKeyTable[m_dwKeyTableIndex2] = btWork;

		dwWork = ((m_abtKeyTable[m_dwKeyTableIndex1] + m_abtKeyTable[m_dwKeyTableIndex2]) & 0xFF);

		pbtTarget[i] ^= m_abtKeyTable[dwWork];
	}
}

/// Decode Movie Data
///
/// @param pvTarget Data to be decoded
/// @param dwSize   Decoding size
///
void CEFLatter::DecodeMovieData(void* pvTarget, DWORD dwSize)
{
	BYTE* pbtTarget = (BYTE*)pvTarget;

	for (DWORD i = 0; i < dwSize; i++)
	{
		pbtTarget[i] ^= m_abtMovieDecodeTable[m_dwMovieDecodeTablePtr];

		m_dwMovieDecodeTablePtr = ((m_dwMovieDecodeTablePtr + 1) & 0xFFFF);
	}
}
