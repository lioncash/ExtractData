#pragma once

#include "ImageBase.h"

class CBmp final : public CImageBase
{
public:
	BOOL Mount(CArcFile* pclArc);

private:
	BITMAPFILEHEADER m_stBMPFileHeader;
	BITMAPINFOHEADER m_stBMPInfoHeader;
	RGBQUAD          m_astPallet[256];

	BOOL OnInit(const YCString& rfclsFileName) override;
	BOOL OnCreatePallet(const void* pvPallet, DWORD dwPalletSize) override;

	void WriteHed(const YCString& rfclsFileName);
	void WriteLine(const void* pvBuffer) override;
	void WriteLineWithAlphaBlend(void* pvBuffer24, const void* pvBuffer32) override;
};
