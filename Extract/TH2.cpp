#include "StdAfx.h"
#include "../Arc/LZSS.h"
#include "../Image.h"
#include "../Image/Tga.h"
#include "TH2.h"

/// Mounting
bool CTH2::Mount(CArcFile* pclArc)
{
	if (MountKCAP(pclArc))
		return true;

	if (MountLAC(pclArc))
		return true;

	if (MountDpl(pclArc))
		return true;

	if (MountWMV(pclArc))
		return true;

	return false;
}

/// KCAP Mounting
bool CTH2::MountKCAP(CArcFile* pclArc)
{
	if (pclArc->GetArcExten().CompareNoCase(_T(".pak")) != 0)
		return false;

	if (memcmp(pclArc->GetHed(), "KCAP", 4) != 0)
		return false;

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed(4);
	pclArc->Read(&dwFiles, 4);

	// Get index size from file count
	DWORD dwIndexSize = (dwFiles * 36);

	// Get index
	YCMemory<BYTE> clmIndex(dwIndexSize);
	pclArc->Read(&clmIndex[0], dwIndexSize);

	DWORD dwIndexPtr = 0;
	for (DWORD i = 0; i < dwFiles; i++)
	{
		DWORD dwType = *(DWORD*)&clmIndex[dwIndexPtr + 0];

		// Skip garbage files
		if (dwType == 0xCCCCCCCC)
		{
			dwIndexPtr += 36;
			continue;
		}

		// Get filename
		char szFileName[25];
		memcpy(szFileName, &clmIndex[dwIndexPtr + 4], 24);
		szFileName[24] = '\0';

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*)&clmIndex[dwIndexPtr + 28];
		stFileInfo.sizeCmp = *(DWORD*)&clmIndex[dwIndexPtr + 32];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		stFileInfo.title = _T("TH2");

		if (dwType == 1)
		{
			stFileInfo.format = _T("LZ");
		}

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 36;
	}

	return true;
}

/// LAC Mounting
bool CTH2::MountLAC(CArcFile* pclArc)
{
	if (pclArc->GetArcExten().CompareNoCase(_T(".pak")) != 0)
		return false;

	if (memcmp(pclArc->GetHed(), "LAC", 3) != 0)
		return false;

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed(4);
	pclArc->Read(&dwFiles, 4);

	// Get index size from file count
	DWORD dwIndexSize = (dwFiles * 40);

	// Get index
	YCMemory<BYTE> clmIndex(dwIndexSize);
	pclArc->Read(&clmIndex[0], dwIndexSize);

	DWORD dwIndexPtr = 0;
	for (DWORD i = 0; i < dwFiles; i++)
	{
		// Get filename
		char szFileName[33];
		memcpy(szFileName, &clmIndex[dwIndexPtr + 0], 32);
		szFileName[32] = '\0';
		for (int j = 0; j < lstrlen(szFileName); j++)
		{
			szFileName[j] ^= 0xFF;
		}

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*)&clmIndex[dwIndexPtr + 32];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*)&clmIndex[dwIndexPtr + 36];
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 40;
	}

	return true;
}

/// Dpl mounting
bool CTH2::MountDpl(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".a"))
		return false;

	if (memcmp(pclArc->GetHed(), "\x1E\xAF", 2) != 0)
		return false;

	// Get file count
	WORD wFiles;
	pclArc->SeekHed(2);
	pclArc->Read(&wFiles, 2);

	// Get index size from file count
	DWORD dwIndexSize = (wFiles << 5);

	// Get index
	YCMemory<BYTE> clmIndex(dwIndexSize);
	pclArc->Read(&clmIndex[0], dwIndexSize);

	DWORD dwOffset = (dwIndexSize + 4);
	DWORD dwIndexPtr = 0;
	for (WORD i = 0; i < wFiles; i++)
	{
		// Get filename

		char szFileName[25];
		memcpy(szFileName, &clmIndex[dwIndexPtr + 0], 24);
		szFileName[24] = '\0';

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*)&clmIndex[dwIndexPtr + 24];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*)&clmIndex[dwIndexPtr + 28] + dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 32;
	}

	return true;
}

/// WMV Mounting
bool CTH2::MountWMV(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".wmv"))
		return false;

	if (memcmp(pclArc->GetHed(), "\x00\x00\x00\x00\x00\x00\x00\x00\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16) != 0)
		return false;

	return pclArc->Mount();
}

/// Decoding
bool CTH2::Decode(CArcFile* pclArc)
{
	if (DecodeWMV(pclArc))
		return true;

	if (DecodeEtc(pclArc))
		return true;

	return false;
}

/// WMV Decoding
bool CTH2::DecodeWMV(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (pclArc->GetArcExten() != _T(".wmv"))
		return false;

	if (memcmp(pclArc->GetHed(), "\x00\x00\x00\x00\x00\x00\x00\x00\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16) != 0)
		return false;

	// Get buffer
	DWORD          dwBufferSize = pclArc->GetBufSize();
	YCMemory<BYTE> clmBuffer(dwBufferSize);

	// Output
	pclArc->OpenFile();
	pclArc->SeekCur(8);
	pclArc->WriteFile("\x30\x26\xB2\x75\x8E\x66\xCF\x11", 8);
	pclArc->ReadWrite((file_info->sizeCmp - 8));
	pclArc->CloseFile();

	return true;
}

/// Decoding other files
bool CTH2::DecodeEtc(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();
	if (file_info->title != _T("TH2"))
		return false;

	if ((file_info->format != _T("LZ")) && (file_info->format != _T("TGA")))
		return false;

	YCMemory<BYTE> clmDst;
	DWORD          dwDstSize;

	// LZ Decoding
	if (file_info->format == _T("LZ"))
	{
		// Get input size
		DWORD dwSrcSize = (file_info->sizeCmp - 8);

		// Get output size
		pclArc->SeekCur(4);
		pclArc->Read(&dwDstSize, 4);

		// Ensure buffers exist
		YCMemory<BYTE> clmSrc(dwSrcSize);
		clmDst.resize(dwDstSize);

		// Read
		pclArc->Read(&clmSrc[0], dwSrcSize);

		// LZSS Decompression
		CLZSS clLZSS;
		clLZSS.Decomp(&clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3);
	}
	else
	{
		// Uncompressed
		dwDstSize = file_info->sizeOrg;
		clmDst.resize(dwDstSize);
		pclArc->Read(&clmDst[0], dwDstSize);
	}

	YCString clsFileExt = file_info->name.GetFileExt().MakeLower();
	if (clsFileExt == _T(".tga"))
	{
		// TGA
		CTga clTGA;
		clTGA.Decode(pclArc, &clmDst[0], dwDstSize);
	}
	else if (clsFileExt == _T(".bmp"))
	{
		// BMP
		CImage clImage;
		clImage.Init(pclArc, &clmDst[0]);
		clImage.Write(dwDstSize);
	}
	else
	{
		// Other
		pclArc->OpenFile();
		pclArc->WriteFile(&clmDst[0], dwDstSize);
	}

	return true;
}
