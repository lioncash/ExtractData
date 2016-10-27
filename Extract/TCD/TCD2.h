#pragma once

//----------------------------------------------------------------------------------------
//-- TCD2 Class --------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include "TCDBase.h"

class CTCD2 final : public CTCDBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;

private:
	struct STCD2IndexInfo
	{
		DWORD dwFileSize;             // File size
		DWORD dwFileCount;            // Number of files
		DWORD dwDirCount;             // Number of folders
		DWORD dwIndexOffset;          // Absolute offset value to the index
		DWORD dwDirNameLength;        // Folder name length
		DWORD dwFileNameLength;       // File name length
		DWORD dwFileCountEx;          // File count(Extended version)
		DWORD dwFileNameLengthEx;     // File name length(Extended version)
	};

	struct STCD2DirInfo
	{
		DWORD dwFileCount;            // Number of files
		DWORD dwFileNameOffset;       // File name offset value
		DWORD dwFileOffset;           // File offset value
		DWORD dwReserve;              // Reserved
	};

	bool DecompRLE2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize) override;
};
