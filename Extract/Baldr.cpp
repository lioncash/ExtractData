#include "stdafx.h"
#include "Baldr.h"

namespace
{

struct SPACFileInfo
{
	char filename[64];
	u32  offset;
	u32  file_size;
	u32  compressed_file_size;
};

} // Anonymous namespace

/// Mounting
///
/// @param pclArc Archive
///
bool CBaldr::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".pac"))
		return false;

	if (memcmp(pclArc->GetHed(), "PAC", 3) != 0)
		return false;

	if (memcmp(&pclArc->GetHed()[72], "\0\0\0\0", 4) != 0)
		return false;

	// Get filecount
	u32 dwFiles;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&dwFiles, 4);

	// Get flags
	u32 dwFlags;
	pclArc->Read(&dwFlags, 4);

	// Get compressed formats
	YCString clsFormat;
	switch (dwFlags)
	{
	case 0: // No compression
		break;

	case 1: // LZSS
		clsFormat = _T("LZ");
		break;

	case 2: // Unknown
		break;

	case 3: // ZLIB
		clsFormat = _T("zlib");
		break;
	}

	// Get index
	YCMemory<SPACFileInfo> clmpacfiIndex(dwFiles);
	pclArc->Read(&clmpacfiIndex[0], (sizeof(SPACFileInfo) * dwFiles));

	// Get file info
	for (u32 i = 0; i < dwFiles; i++)
	{
		// Get filename
		char szFileName[65];
		memcpy(szFileName, clmpacfiIndex[i].filename, 64);
		szFileName[64] = '\0';

		if (strlen(szFileName) <= 4)
		{
			pclArc->SeekHed();
			return false;
		}

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.sizeOrg = clmpacfiIndex[i].file_size;
		stFileInfo.sizeCmp = clmpacfiIndex[i].compressed_file_size;
		stFileInfo.start = clmpacfiIndex[i].offset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		stFileInfo.format = clsFormat;

//		if ((stFileInfo.sizeOrg != infFile.sizeCmp) && (lstrcmp(PathFindExtension(infFile.name), _T(".wav")) != 0))
//			infFile.format = _T("LZ");

		pclArc->AddFileInfo(stFileInfo);
	}

	return true;
}
