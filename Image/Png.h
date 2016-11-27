#pragma once

#include "Image/ImageBase.h"

class CPng final : public CImageBase
{
public:
	CPng();
	~CPng() override;

	void SetCompressLevel(int nCompressLevel);
	int  GetCompressLevel() const;

	void SetCallback();

	void Close();

	bool Compress(LPCTSTR dst_path, const void* bmp, size_t bmp_size);
	bool Compress(LPCTSTR dst_path, const void* dib, size_t dib_size, const void* pallet, size_t palle_size, u16 pallet_bpp, long width, long height, u16 bpp);
	bool Compress(void* dst, size_t dst_size, const void* bmp, size_t bmp_size);
	bool Compress(void* dst, size_t dst_size, const void* dib, size_t dib_size, const void* pallet, size_t pallet_size, u16 pallet_bpp, long width, long height, u16 bpp);

	bool Decompress();

private:
	enum class Mode
	{
		Read  = 0x00000000,
		Write = 0x00000001
	};

	struct SMemory
	{
		u8* data;
		u32 data_ptr;
	};

	bool OnInit(const YCString& file_name) override;
	bool OnCreatePallet(const void* pallet, size_t pallet_size) override;

	void WriteLine(const void* buffer) override;
	void WriteLineWithAlphaBlend(void* buffer24, const void* buffer32) override;
	void OnWriteFinish() override;

	static inline void PNGAPI WritePNG(png_structp png, png_bytep data, png_size_t length);
	static inline void PNGAPI WritePNGToFile(png_structp png, png_bytep data, png_size_t length);
	static inline void PNGAPI WritePNGToMemory(png_structp png, png_bytep data, png_size_t length);

	png_structp m_png_ptr;
	png_infop   m_info_ptr;
	std::array<png_color, 256> m_pallet;

	png_structp m_png = nullptr;
	png_infop   m_png_info = nullptr;
	void*       m_param = nullptr;
	u32         m_flags = 0;
	int         m_compression_level = 1;

	Mode        m_mode = Mode::Read;
};
