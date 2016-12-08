#pragma once

#include "Image/ImageBase.h"

class CArcFile;

class CBmp final : public CImageBase
{
public:
	bool Mount(CArcFile* archive);

private:
	BITMAPFILEHEADER m_bmp_file_header;
	BITMAPINFOHEADER m_bmp_info_header;
	std::array<RGBQUAD, 256> m_pallet;

	bool OnInit(const YCString& file_name) override;
	bool OnCreatePallet(const u8* pallet, size_t pallet_size) override;

	void WriteHed(const YCString& file_name);
	void WriteLine(const u8* buffer) override;
	void WriteLineWithAlphaBlend(u8* buffer24, const u8* buffer32) override;
};
