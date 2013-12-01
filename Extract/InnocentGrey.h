#pragma once

class CInnocentGrey : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};
