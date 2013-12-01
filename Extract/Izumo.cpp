#include "stdafx.h"
#include "../ExtractBase.h"
#include "Izumo.h"

// Function to get file information from IZUMO *.dat files
BOOL CIzumo::Mount(CArcFile* pclArc)
{
    if ((pclArc->GetArcExten() != _T(".dat")) || (memcmp(pclArc->GetHed(), "PAK0", 4) != 0))
        return FALSE;

    // Get index size (Starting address of the first file)
    DWORD index_size;
    pclArc->Seek(4, FILE_BEGIN);
    pclArc->Read(&index_size, 4);

    // Get number of bytes to be skipped
    DWORD dwDummys;
    DWORD dwSkip;

    pclArc->Read( &dwDummys, 4 );
    dwSkip = dwDummys << 3;

    // Get file count
    DWORD ctFile;
    pclArc->Read(&ctFile, 4);

    // Skip unknown data
    pclArc->Seek(dwSkip, FILE_CURRENT);

    // Skip segments
    index_size -= 16 + dwSkip;

    // Get index
    YCMemory<BYTE> index(index_size);
    LPBYTE pIndex = &index[0];
    pclArc->Read(pIndex, index_size);

    // Get filename index
    LPBYTE pFileNameIndex = pIndex + (ctFile << 4);

    // Remove unneeded filenames 

    for( DWORD i = 1 ; i < dwDummys ; i++ )
    {
        pFileNameIndex += pFileNameIndex[0] + 1;
    }

    for (DWORD i = 0; i < ctFile; i++)
    {
        // Get file name
        TCHAR szFileName[256];
        BYTE len = *pFileNameIndex++;
        memcpy(szFileName, pFileNameIndex, len);
        szFileName[len] = _T('\0');

        // Add to list view
        SFileInfo infFile;
        infFile.name = szFileName;
        infFile.sizeOrg = *(LPDWORD)&pIndex[4];
        infFile.sizeCmp = infFile.sizeOrg;
        infFile.start = *(LPDWORD)&pIndex[0];
        infFile.end = infFile.start + infFile.sizeCmp;
        pclArc->AddFileInfo(infFile);

        pIndex += 16;
        pFileNameIndex += len;
    }

    return TRUE;
}
