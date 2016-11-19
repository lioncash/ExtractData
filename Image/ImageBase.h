#pragma once

#include "../ExtractBase.h"

///
/// Base class for dealing with images
///
/// @remark Use a line to override the handling and initialization
///
class CImageBase
{
public:
	enum
	{
		BMP = 0,
		PNG
	};

	union UPixel
	{
		u8       color_rgba[4];
		COLORREF color;
	};

	CImageBase();
	virtual ~CImageBase();

	bool Init(CArcFile* archive, long width, long height, u16 bpp, const void* pallet = nullptr, size_t pallet_size = 1024, const YCString& file_name = _T(""));

	void SetValidityOfAlphaBlend(bool validity_of_alpha_blend);
	bool GetValidityOfAlphaBlend() const;

	void SetBackColorWhenAlphaBlend(COLORREF background_color);

	bool Compress(LPCTSTR dst_path, const void* bmp, size_t bmp_size);
	bool Compress(LPCTSTR dst_path, const void* dib, size_t dib_size, const void* pallet, size_t pallet_size, u16 pallet_bpp, long width, long height, u16 bpp);
	bool Compress(void* dst, size_t dst_size, const void* bmp, size_t bmp_size);
	bool Compress(void* dst, size_t dst_size, const void* dib, size_t dib_size, const void* pallet, size_t pallet_size, u16 pallet_bpp, long width, long height, u16 bpp);

	bool ComposeBGRA(void* dst, const void* buffer, size_t buffer_size);
	void WriteCompoBGRA(const void* buffer, size_t buffer_size, bool progress = true);
	void WriteCompoBGRAReverse(const void* buffer, size_t buffer_size, bool progress = true);

	bool ComposeRGBA(void* dst, const void* buffer, size_t buffer_size);
	void WriteCompoRGBA(const void* buffer, size_t buffer_size, bool progress = true);
	void WriteCompoRGBAReverse(const void* buffer, size_t buffer_size, bool progress = true);

	void Write(const void* buffer, size_t buffer_size, bool progress = true);
	void WriteReverse(const void* buffer, size_t buffer_size, bool progress = true);

	void WriteFinish();

protected:
	static long CalculatePitch(long width, u16 bpp);

	CArcFile* m_archive = nullptr;
	u8 m_bg[4];

	u32 m_row_size = 0;
	u32 m_row_size_remainder = 0;

	bool IsRequireAlphaBlend() const;

	void AlphaBlend(void* pvBuffer24, const void* pvBuffer32);

	virtual bool OnInit(const YCString& file_name) { return true; }
	virtual bool OnCreatePallet(const void* pallet, size_t pallet_size) { return true; }
	virtual bool OnWriteHeader() { return true; }
	virtual void OnWriteFinish() {}

	virtual void WriteLine(const void* buffer) {}
	virtual void WriteLineWithAlphaBlend(void* buffer24, const void* buffer32) {}

protected:
	bool   m_validity_of_alpha_blend = false;
	bool   m_alpha_blend_requirement = false;
	bool   m_output_dummy_from_buffer = false;

	long   m_width = 0;
	long   m_height = 0;
	long   m_line = 0;
	long   m_pitch = 0;
	long   m_line_with_alpha = 0;
	long   m_pitch_with_alpha = 0;
	u16    m_bpp = 0;
	u16    m_bpp_with_alpha = 0;
	UPixel m_background_color_when_alpha_blending;
};
