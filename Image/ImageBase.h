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
		u8       abtPixel[4];
		COLORREF crPixel;
	};

	CImageBase();
	virtual ~CImageBase();

	bool Init(CArcFile* archive, long width, long height, u16 bpp, const void* pallet = nullptr, size_t pallet_size = 1024, const YCString& file_name = _T(""));

	void SetValidityOfAlphaBlend(bool bValidityOfAlphaBlend);
	bool GetValidityOfAlphaBlend() const;

	void SetBackColorWhenAlphaBlend(COLORREF crBackColor);

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

	CArcFile* m_pclArc;
	u8 m_abtBG[4];

	u32 m_dwRowSize;
	u32 m_dwRowSizeOfRemainder;

	bool IsRequireAlphaBlend() const;

	void AlphaBlend(void* pvBuffer24, const void* pvBuffer32);

	virtual bool OnInit(const YCString& file_name) { return true; }
	virtual bool OnCreatePallet(const void* pallet, size_t pallet_size) { return true; }
	virtual bool OnWriteHeader() { return true; }
	virtual void OnWriteFinish() {}

	virtual void WriteLine(const void* buffer) {}
	virtual void WriteLineWithAlphaBlend(void* buffer24, const void* buffer32) {}

protected:
	bool   m_bValidityOfAlphaBlend = false;
	bool   m_bAlphaBlendRequirement = false;
	bool   m_bOutputDummyFromBuffer = false;

	long   m_lWidth;
	long   m_lHeight;
	long   m_lLine;
	long   m_lPitch;
	long   m_lLineWithAlpha;
	long   m_lPitchWithAlpha;
	u16    m_wBpp;
	u16    m_wBppWithAlpha;
	UPixel m_unpBackColorWhenAlphaBlend;
};
