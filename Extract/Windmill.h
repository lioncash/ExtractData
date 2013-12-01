#pragma once

class CWindmill : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
	DWORD Decode1(LPBYTE* pbuf2, LPDWORD tmp1, LPDWORD tmp2);
};
