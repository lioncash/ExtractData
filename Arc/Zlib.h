#pragma once

#include "../ArcFile.h"

class CZlib
{
public:
    BOOL Decode(CArcFile* pclArc);
    void DecompressFile(CArcFile* pclArc);
    int Decompress(BYTE* dst, DWORD* dstSize, const BYTE* src, DWORD srcSize);
    int Decompress(BYTE* dst, DWORD dstSize, const BYTE* src, DWORD srcSize);
};
