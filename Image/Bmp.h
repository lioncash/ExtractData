#pragma once

#include "ImageBase.h"

class CBmp final : public CImageBase
{
public:
	bool Mount(CArcFile* archive);

private:
	BITMAPFILEHEADER m_bmp_file_header;
	BITMAPINFOHEADER m_bmp_info_header;
	RGBQUAD          m_pallet[256];

	bool OnInit(const YCString& file_name) override;
	bool OnCreatePallet(const void* pallet, size_t pallet_size) override;

	void WriteHed(const YCString& file_name);
	void WriteLine(const void* buffer) override;
	void WriteLineWithAlphaBlend(void* buffer24, const void* buffer32) override;
};
