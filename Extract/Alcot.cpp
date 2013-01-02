#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/Zlib.h"
#include "../Image.h"
#include "Alcot.h"

BOOL CAlcot::Mount(CArcFile* pclArc)
{
    if ((pclArc->GetArcExten() != _T(".arc")) || (memcmp(pclArc->GetHed(), "ARC\x1a", 4) != 0))
        return FALSE;

    // Get file count
    DWORD ctFile;
    pclArc->Seek(8, FILE_BEGIN);
    pclArc->Read(&ctFile, 4);

    //Get compressed index size
    DWORD index_compsize;
    pclArc->Read(&index_compsize, 4);

    // Get compressed index
    YCMemory<BYTE> z_index(index_compsize);
    LPBYTE z_pIndex = &z_index[0];
    pclArc->Seek(0x30, FILE_BEGIN);
    pclArc->Read(z_pIndex, index_compsize);

    // Get index size
    DWORD index_size = *(LPDWORD)&z_index[16];

    // Secure space to store the index
    YCMemory<BYTE> index(index_size);
    LPBYTE pIndex = &index[0];

    // Unzipped index
    Decomp(pIndex, index_size, z_pIndex);

    DWORD offset = index_compsize + 0x30;
    DWORD size = index_size / ctFile;
    DWORD FileNameLen = size - 16;

    for (int i = 0; i < (int)ctFile; i++)
    {
        // Get filename
        TCHAR szFileName[48];
        memcpy(szFileName, &pIndex[16], FileNameLen);

        // Add to the listview
        SFileInfo infFile;
        infFile.name = szFileName;
        infFile.sizeOrg = *(LPDWORD)&pIndex[4];
        infFile.sizeCmp = infFile.sizeOrg;
        infFile.start = *(LPDWORD)&pIndex[0] + offset;
        infFile.end = infFile.start + infFile.sizeOrg;
        pclArc->AddFileInfo(infFile);

        pIndex += size;
    }

    return TRUE;
}

BOOL CAlcot::Decode(CArcFile* pclArc)
{
    if (DecodeASB(pclArc) == TRUE)
        return TRUE;

    if (DecodeCPB(pclArc) == TRUE)
        return TRUE;

    return FALSE;
}

BOOL CAlcot::DecodeASB(CArcFile* pclArc)
{
    SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

    if (pInfFile->format != _T("ASB"))
        return FALSE;

    // Reading
    YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
    pclArc->Read(&z_buf[0], pInfFile->sizeCmp);

    // Get size
    DWORD z_buf_len = *(LPDWORD)&z_buf[4];
    DWORD buf_len = *(LPDWORD)&z_buf[8];

    // Decryption
    Decrypt(&z_buf[12], z_buf_len, buf_len);

    // ASB not supported, lets force support
    if (memcmp(&z_buf[16], "\x78\xDA", 2) != 0)
    {
        DWORD key = ((*(LPDWORD)&z_buf[16] - 0x78) ^ buf_len) & 0xFF;
        YCMemory<BYTE> tmp(z_buf_len);

        for (QWORD i = 0; i < (QWORD)0xFFFFFFFF; i += 0x00000100)
        {
            DWORD x = i | key;
            memcpy(&tmp[0], &z_buf[16], z_buf_len);

            DWORD key2 = buf_len ^ x;
            key2 ^= ((key2 << 0x0C) | key2) << 0x0B;

            DWORD tmp2 = *(LPDWORD)&z_buf[16] - key2;
            if ((tmp2 & 0x0000FFFF) != 0xDA78)
                continue;

            for (int j = 0; j < (int)z_buf_len / 4; j++)
                *(LPDWORD)&tmp[j*4] -= key2;

            // Decompression
            CZlib zlib;
            YCMemory<BYTE> buf(buf_len);
            zlib.Decompress(&buf[0], buf_len, &tmp[0], z_buf_len);

            BYTE a[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            
            if (memcmp(&buf[buf_len-20], a, 20) == 0)
            {
                TCHAR mes[256];
                _stprintf(mes, _T("Decryption key of this game is %08X . \n We may be able to support this archive type if you report it to us"), x);
                MessageBox(pclArc->GetProg()->GetHandle(), mes, _T("Decyption Key"), MB_OK);

                _stprintf(mes, _T("_%08X.txt"), x);
                pclArc->OpenFile(mes);
                pclArc->WriteFile(&buf[0], buf_len);
                pclArc->CloseFile();
            }
        }
        return TRUE;
    }

    // Decompression
    CZlib zlib;
    YCMemory<BYTE> buf(buf_len);
    zlib.Decompress(&buf[0], buf_len, &z_buf[16], z_buf_len);

    pclArc->OpenScriptFile();
    pclArc->WriteFile(&buf[0], buf_len);

    return TRUE;
}

BOOL CAlcot::DecodeCPB(CArcFile* pclArc)
{
    SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

    if (pInfFile->format != _T("CPB"))
        return FALSE;

    // Read
    YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
    LPBYTE z_pbuf = &z_buf[0];
    pclArc->Read(z_pbuf, pInfFile->sizeCmp);

    BYTE type = z_buf[4];
    BYTE bpp = z_buf[5];
    BYTE colors = bpp >> 3;

    LONG width;
    LONG height;

    DWORD z_colorSize[4];
    DWORD buf_len;
    YCMemory<BYTE> buf;

    // Triptych
    if (type == 0)
    {
        // Product version
        if (*(LPWORD)&z_buf[6] == 1)
            width = *(LPWORD)&z_buf[12];
        // Trial Version
        else if (*(LPWORD)&z_buf[6] == 0x100)
            width = *(LPWORD)&z_buf[8];
        // Other
        else
            return FALSE;

        height = *(LPWORD)&z_buf[14];

        DWORD colorSize = width * height;

        buf_len = colorSize * colors;
        buf.resize(buf_len);
        LPBYTE pbuf = &buf[0];
        pbuf += buf_len;

        z_colorSize[0] = *(LPDWORD)&z_buf[16];
        z_colorSize[1] = *(LPDWORD)&z_buf[28];
        z_colorSize[2] = *(LPDWORD)&z_buf[20];
        z_colorSize[3] = *(LPDWORD)&z_buf[24];

        CZlib zlib;
        z_pbuf += 0x20;
        for (int i = 0; i < 4; i++) {
            if (z_colorSize[i] == 0)
                continue;
            pbuf -= colorSize;
            zlib.Decompress(pbuf, colorSize, &z_pbuf[4], z_colorSize[i]);
            z_pbuf += z_colorSize[i];
        }
    }
    else
	{
        width = *(LPWORD)&z_buf[8];
        height = *(LPWORD)&z_buf[10];

        DWORD colorSize = width * height;

        buf_len = colorSize * colors;
        buf.resize(buf_len);
        LPBYTE pbuf = &buf[0];

        for (int i = 0; i < colors; i++)
            z_colorSize[i] = *(LPDWORD)&z_buf[16 + i*4];

        switch (type)
        {
            // zlib
            case 1:
                CZlib zlib;
                z_pbuf += 0x20;
                for (int i = 0; i < (int)colors; i++)
                {
                    zlib.Decompress(pbuf, colorSize, &z_pbuf[4], z_colorSize[i]);
                    pbuf += colorSize;
                    z_pbuf += z_colorSize[i];
                }
                break;
            // Proprietary compression
            case 3:
                z_pbuf += 0x20;
                for (int i = 0; i < (int)colors; i++)
                {
                    Decomp(pbuf, colorSize, z_pbuf);
                    pbuf += colorSize;
                    z_pbuf += z_colorSize[i];
                }
                break;
            // Unknown
            default:
                return FALSE;
        }
    }

    CImage image;
    image.Init(pclArc, width, height, bpp);
    image.WriteCompoRGBAReverse(&buf[0], buf_len);

    return TRUE;
}

void CAlcot::Decomp(LPBYTE dst, DWORD dstSize, LPBYTE src)
{
    LPBYTE psrc1 = &src[0x14];
    LPBYTE psrc2 = psrc1 + *(LPDWORD)&src[4];
    LPBYTE psrc3 = psrc2 + *(LPDWORD)&src[8];
    LPBYTE dst_end = dst + dstSize;
    BYTE code = 0x80;

    while (dst < dst_end)
    {
        if (code == 0)
        {
            psrc1++;
            code = 0x80;
        }

        if (*psrc1 & code)
        {
            WORD tmp = *(LPWORD)psrc2;
            psrc2 += 2;

            WORD length = (tmp >> 0x0D) + 3;
            if (length > 0)
            {
                WORD offset = (tmp & 0x1FFF) + 1;
                LPBYTE dst2 = dst - offset;
                for (int i = 0; i < (int)length; i++)
                    *dst++ = *dst2++;
            }
        }
        else
        {
            WORD length = *psrc3++ + 1;
            for (int i = 0; i < (int)length; i++)
                *dst++ = *psrc3++;
        }

        code >>= 1;
    }
}

void CAlcot::Decrypt(LPBYTE src, DWORD srcSize, DWORD dstSize)
{
    //            Kurobane, Kurobane DVD,  TOY‚Â‚ß,   Trip trial,   Trip,       Ramune
    DWORD x[] = {0xF44387F3, 0xE1B2097A, 0xD153D863, 0xF389842D, 0x1DE71CB9, 0x99E15CB4};

    for (int i = 0; i < ARRAYSIZE(x); i++)
    {
        DWORD key = dstSize ^ x[i];
        key ^= ((key << 0x0C) | key) << 0x0B;

        DWORD tmp = *(LPDWORD)&src[4] - key;
        if ((tmp & 0x0000FFFF) != 0xDA78)
            continue;

        for (int j = 0; j < (int)srcSize / 4; j++)
        {
            *(LPDWORD)src -= key;
            src += 4;
        }
    }
}
