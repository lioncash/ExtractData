#include "stdafx.h"
#include "../ExtractBase.h"
#include "SummerDays.h"

#define TYPE_NONE	0x00000000
#define TYPE_FOLDER	0x00000001

BOOL CSummerDays::Mount(CArcFile* pclArc)
{
    if (memcmp(pclArc->GetHed(), "Í2nY", 4) != 0)
        return FALSE;

    m_ui16ContextCount = 0x8003; // Code starts at 0x8003

    // Get header
    WORD ctHeader;
    pclArc->Seek(10, FILE_BEGIN);
    pclArc->Read(&ctHeader, 2);

    // Get file name
    WORD FileNameLen;
    pclArc->Read(&FileNameLen, 2);
    TCHAR szFileName[256];
    pclArc->Read(szFileName, FileNameLen);
    szFileName[FileNameLen] = _T('\0');

    // Get count
    WORD count;
    pclArc->Read(&count, 2);
    DWORD ctTotal = (ctHeader > 1) ? ctHeader + count - 1 : count;

    TCHAR pcPath[MAX_PATH];
    ZeroMemory(pcPath, sizeof(pcPath));
    for (DWORD i = 0; i < ctTotal; i++)
    {
        if (_sub(pclArc, pcPath) == FALSE)
        {
            m_tContextTable.clear();
            return FALSE;
        }
    }

    m_tContextTable.clear();

    return TRUE;
}

WORD CSummerDays::_context_new(CArcFile* pclArc, WORD ui16Length)
{
    std::vector<TCONTEXT>* ptContextTable = &m_tContextTable;
    m_ui16ContextCount++;

    // Get class name
    TCHAR pcName[256];
    pclArc->Read(pcName, ui16Length);
    pcName[ui16Length] = _T('\0');

    TCONTEXT tNew;
    tNew.pcName = pcName;
    tNew.ui16Code = m_ui16ContextCount;
    tNew.iType = (tNew.pcName == _T("CAutoFolder")) ? TYPE_FOLDER : TYPE_NONE;
    ptContextTable->push_back(tNew);

    return (m_ui16ContextCount);
}

int CSummerDays::_context_add(WORD ui16Code)
{
    std::vector<TCONTEXT>& ptContextTable = m_tContextTable;

    for (size_t i = 0; i < ptContextTable.size(); i++)
    {
        if (ptContextTable[i].ui16Code == ui16Code)
        {
            return (ptContextTable[i].iType);
        }
    }

    return -1;
}

BOOL CSummerDays::_sub(CArcFile* pclArc, LPTSTR pcPath)
{
    union
    {
        DWORD pui32Value[2];
        WORD pui16Value[4];
        BYTE pui8Value[8];
        char pi8Value[8];
    } uData;

    m_ui16ContextCount++;

    // Get file name (Folder name)
    BYTE ui8Length;
    pclArc->Read(&ui8Length, 1);
    TCHAR pcName[256];
    pclArc->Read(pcName, ui8Length);
    pcName[ui8Length] = _T('\0');

    // Get start address
    pclArc->Read(&ui8Length, 1);
    pclArc->Read(uData.pi8Value, 8);
    WORD ui16Context = uData.pui16Value[1];

    if (ui16Context == (WORD)0xffff)
    {
        ui16Context = _context_new(pclArc, uData.pui16Value[3]);
        pclArc->Read(&uData.pui32Value[1], 4);
    }

    int iType = _context_add(ui16Context);
    long i32Position = (long)(uData.pui32Value[1] - (DWORD)0xA2FB6AD1);

    // Get file size
    pclArc->Read(&uData.pui32Value[0], 4);
    DWORD ui32Base = uData.pui32Value[0] + 0x184A2608;
    DWORD ui32Size = (ui32Base >> 13) & 0x0007FFFF;
    ui32Size |= ((ui32Base - ui32Size) & 0x00000FFF) << 19;

    TCHAR pcPath2[MAX_PATH];
    lstrcpy(pcPath2, pcPath);
    PathAppend(pcPath2, pcName);

    if (iType & TYPE_FOLDER)
    {
        lstrcat(pcPath2, _T(".Op"));
        WORD ui16Count;
        pclArc->Read(&ui16Count, 2);
        
        for (WORD i = 0; i < ui16Count; i++)
        {
            if (_sub(pclArc, pcPath2) == FALSE)
            {
                return FALSE;
            }
        }
    }
    else
    {
        WORD ui16Count;
        pclArc->Read(&ui16Count, 2);

        // Add to listview
        SFileInfo infFile;
        infFile.name = pcPath2;
        infFile.sizeCmp = ui32Size;
        infFile.sizeOrg = infFile.sizeCmp;
        infFile.start = i32Position;
        infFile.end = infFile.start + infFile.sizeCmp;
        pclArc->AddFileInfo(infFile);
    }

    return TRUE;
}
