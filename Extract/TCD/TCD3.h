#pragma once

//----------------------------------------------------------------------------------------
//-- TCD3 Class --------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include "TCDBase.h"

class CTCD3 : public CTCDBase
{
public:

	struct STCD3IndexInfo
	{
		DWORD       dwFileSize;             // File size
		DWORD       dwIndexOffset;          // Absolute offset value to the index
		DWORD       dwDirCount;             // Number of folders
		DWORD       dwDirNameLength;        // Folder name length (4 byte unites. Example ÅF SYSTEM == 8 bytes)
		DWORD       dwFileCount;            // Number of files
		DWORD       dwFileNameLength;       // File name length
		DWORD       dwFileCountEx;          // File count (Extended version)
		DWORD       dwFileNameLengthEx;     // File name length (Extended version)
	};

	struct STCD3DirInfo
	{
		DWORD       dwFileCount;            // Number of files
		DWORD       dwFileNameOffset;       // File name offset value
		DWORD       dwFileOffset;           // File offset value
		DWORD       dwReserve;              // Reserved
	};

	BOOL Mount( CArcFile* pclArc );


protected:

	virtual BOOL DecompRLE2( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
};
