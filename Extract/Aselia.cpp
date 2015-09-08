#include "stdafx.h"
#include "../ExtractBase.h"
#include "Aselia.h"

BOOL CAselia::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".gd"))
		return FALSE;

	TCHAR szDllPath[MAX_PATH];
	lstrcpy(szDllPath, pclArc->GetArcPath());
	PathRenameExtension(szDllPath, _T(".dll"));

	if (PathFileExists(szDllPath) == FALSE)
		return FALSE;

	// Open DLL File
	CFile DllFile;
	if (DllFile.Open(szDllPath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE;

	// Get filecount
	DWORD ctFile;
	DllFile.Read(&ctFile, 4);
	ctFile--;

	// Number of files retrieved from the index size
	DWORD index_size = ctFile << 3;

	// Get Index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	DllFile.Read(pIndex, index_size);

	// Get file extension
	YCString sFileExt;
	if (memcmp(&pclArc->GetHed()[4], "OggS", 4) == 0)
	{
		sFileExt = _T(".ogg");
	}
	else if (memcmp(&pclArc->GetHed()[4], "PNG" /*‰PNG" NOTE: is that the actual way it is (‰PNG), or was it an encoding error in this cpp file? */, 4) == 0)
	{
		sFileExt = _T(".png");
	}

	for (int i = 0; i < (int)ctFile; i++)
	{
		// Get filename
		TCHAR szFileName[MAX_PATH];
		_stprintf(szFileName, _T("%s_%06d%s"), pclArc->GetArcName().GetString(), i + 1, sFileExt.GetString());

		SFileInfo infFile;
		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.sizeOrg = *(LPDWORD)&pIndex[4];

		// Add to listview
		infFile.name = szFileName;
		infFile.sizeCmp = infFile.sizeOrg;
		infFile.end = infFile.start + infFile.sizeOrg;
		pclArc->AddFileInfo(infFile);

		pIndex += 8;
	}

	return TRUE;
}
