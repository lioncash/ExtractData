#pragma once

class CFateFD : public CExtractBase
{
protected:
	// Decode Info
	struct DecodeInfo
	{
		BYTE key;
		DWORD size;
	};

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};
