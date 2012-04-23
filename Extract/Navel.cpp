#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Wav.h"
#include "Navel.h"

// Function that gets file information from Navel's .pac files
BOOL CNavel::Mount(CArcFile* pclArc)
{
    if (MountPac(pclArc) == TRUE)
        return TRUE;
    if (MountWpd(pclArc) == TRUE)
        return TRUE;

    return FALSE;
}

BOOL CNavel::MountPac(CArcFile* pclArc)
{
    if ((pclArc->GetArcExten() != _T(".pac")) || (memcmp(pclArc->GetHed(), "CAPF", 4) != 0))
        return FALSE;

    // Get index size
    DWORD index_size;
    pclArc->Seek(8, FILE_BEGIN);
    pclArc->Read(&index_size, 4);
    index_size -= 32;

    // Get file count
    DWORD ctFile;
    pclArc->Read(&ctFile, 4);

    // Get index
    YCMemory<BYTE> index(index_size);
    LPBYTE pIndex = &index[0];
    pclArc->Seek(16, FILE_CURRENT);
    pclArc->Read(pIndex, index_size);

    for (int i = 0; i < (int)ctFile; i++)
    {
        // Get file name
        TCHAR szFileName[32];
        memcpy(szFileName, &pIndex[8], 32);

        // Add to list view
        SFileInfo infFile;
        infFile.name = szFileName;
        infFile.sizeOrg = *(LPDWORD)&pIndex[4];
        infFile.sizeCmp = infFile.sizeOrg;
        infFile.start = *(LPDWORD)&pIndex[0];
        infFile.end = infFile.start + infFile.sizeOrg;
        pclArc->AddFileInfo(infFile);

        pIndex += 40;
    }

    return TRUE;
}

BOOL CNavel::MountWpd(CArcFile* pclArc)
{
    if ((pclArc->GetArcExten() != _T(".WPD")) || (memcmp(pclArc->GetHed(), " DPW", 4) != 0))
        return FALSE;

    return pclArc->Mount();
}

// Function to convert to WAV files
BOOL CNavel::Decode(CArcFile* pclArc)
{
    SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

    if (pInfFile->format != _T("WPD"))
        return FALSE;

    // Read WPD Format
    FormatWPD fWPD;
    pclArc->Read(&fWPD, sizeof(FormatWPD));

    // Output
    CWav wav;
    wav.Init(pclArc, pInfFile->sizeOrg - 44, fWPD.freq, fWPD.channels, fWPD.bits);
    wav.Write();

    return TRUE;
}
