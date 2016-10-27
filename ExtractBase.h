#pragma once

#include "stdafx.h"
#include "Common.h"
#include "File.h"
#include "ArcFile.h"

class CExtractBase
{
public:
	virtual ~CExtractBase() = default;

	virtual bool Mount(CArcFile* pclArc) = 0;
	virtual bool Decode(CArcFile* pclArc);
	virtual bool Extract(CArcFile* pclArc);
};
