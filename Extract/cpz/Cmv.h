#pragma once

class CCmv : public CExtractBase {
public:
    BOOL Mount(CArcFile* pclArc);
    BOOL Decode(CArcFile* pclArc);
};