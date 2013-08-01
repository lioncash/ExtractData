#include "stdafx.h"
#include "../ExtractBase.h"
#include "MeltyBlood.h"

// Function that gets file information from MELTY BLOOD *.data files
BOOL CMeltyBlood::Mount(CArcFile* pclArc)
{
    if ((pclArc->GetHed()[0] != 0x00) && (pclArc->GetHed()[0] != 0x01))
        return FALSE;

    if ((pclArc->GetArcName().Left(5) != _T("data0")) || (pclArc->GetArcExten() != _T(".p")))
        return FALSE;

    DWORD deckey = 0xE3DF59AC;

    // Get file count
    DWORD ctFile;
    pclArc->Seek(4, FILE_BEGIN);
    pclArc->Read(&ctFile, 4);
    ctFile ^= deckey;

    // Get index size from file count
    DWORD index_size = ctFile * 68;

    // Get index
    YCMemory<BYTE> index(index_size);
    LPBYTE pIndex = &index[0];
    pclArc->Read(pIndex, index_size);

    for (int i = 0; i < (int)ctFile; i++)
    {
        // Get file name
        TCHAR szFileName[60];
        memcpy(szFileName, pIndex, 60);
        
        // Decrypt filename
        for (int j = 0; j < 59; j++)
        {
            szFileName[j] ^= i * j * 3 + 61;
        }

        // Add to listview
        SFileInfo infFile;
        infFile.name = szFileName;
        infFile.sizeOrg = *(LPDWORD)&pIndex[64] ^ deckey;
        infFile.sizeCmp = infFile.sizeOrg;
        infFile.start = *(LPDWORD)&pIndex[60];
        infFile.end = infFile.start + infFile.sizeOrg;
        infFile.title = _T("MeltyBlood");
        pclArc->AddFileInfo(infFile);

        pIndex += 68;
    }

    return TRUE;
}

// Extraction function
BOOL CMeltyBlood::Decode(CArcFile* pclArc)
{
    SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

    if (pInfFile->format != _T("MeltyBlood"))
        return FALSE;

    // Create output file
    pclArc->OpenFile();

    // Decodes data range 0x00`0x2173, Output
    Decrypt(pclArc);

    // Output earlier than 0x2173
    if (pInfFile->sizeOrg > 0x2173)
        pclArc->ReadWrite(pInfFile->sizeOrg - 0x2173);

    return TRUE;
}

// Data decryption function
void CMeltyBlood::Decrypt(CArcFile* pclArc)
{
    SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

    // Ensure buffer
    int lim = (pInfFile->sizeOrg < 0x2173) ? pInfFile->sizeOrg : 0x2173;
    YCMemory<BYTE> buf(lim);

    pclArc->Read(&buf[0], lim);

    // Decryption
    int keylen = (int) pInfFile->name.GetLength();
    
    for (int i = 0; i < lim; i++)
    {
        buf[i] ^= pInfFile->name[i % keylen] + i + 3;
    }

    pclArc->WriteFile(&buf[0], lim);
}
