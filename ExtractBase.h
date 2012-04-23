#pragma once

#include "stdafx.h"
#include "Common.h"
#include "File.h"
#include "ArcFile.h"

class CExtractBase
{
public:

    virtual	BOOL Mount( CArcFile* pclArc ) = 0;
    virtual	BOOL Decode( CArcFile* pclArc );
    virtual	BOOL Extract( CArcFile* pclArc );
};
