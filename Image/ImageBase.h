#pragma once

#include "../ExtractBase.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Base class for dealing with images
//
//  Remark: Use a line to override the handling and initialization

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
		BYTE     abtPixel[4];
		COLORREF crPixel;
	};

	CImageBase();
	virtual ~CImageBase();

	bool Init(CArcFile* pclArc, long lWidth, long lHeight, WORD wBpp, const void* pvPallet = NULL, DWORD dwPalletSize = 1024, const YCString& rfclsFileName = _T(""));

	void SetValidityOfAlphaBlend(bool bValidityOfAlphaBlend);
	bool GetValidityOfAlphaBlend() const;

	void SetBackColorWhenAlphaBlend(COLORREF crBackColor);

	bool Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize);
	bool Compress(LPCTSTR pszPathToDst, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);
	bool Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize);
	bool Compress(void* pvDst, DWORD dwDstSize, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);

	bool ComposeBGRA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize);
	void WriteCompoBGRA(const void* pvBuffer, DWORD dwBufferSize, bool progress = true);
	void WriteCompoBGRAReverse(const void* pvBuffer, DWORD dwBufferSize, bool progress = true);

	bool ComposeRGBA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize);
	void WriteCompoRGBA(const void* pvBuffer, DWORD dwBufferSize, bool progress = true);
	void WriteCompoRGBAReverse(const void* pvBuffer, DWORD dwBufferSize, bool progress = true);

	void Write(const void* pvBuffer, DWORD dwBufferSize, bool progress = true);
	void WriteReverse(const void* pvBuffer, DWORD dwBufferSize, bool progress = true);

	void WriteFinish();

protected:
	static long CalculatePitch( long lWidth, WORD wBpp );

	CArcFile* m_pclArc;
	BYTE m_abtBG[4];

	DWORD m_dwRowSize;
	DWORD m_dwRowSizeOfRemainder;

	bool IsRequireAlphaBlend() const;

	void AlphaBlend(void* pvBuffer24, const void* pvBuffer32);

	virtual bool OnInit(const YCString& rfclsFileName) { return true; }
	virtual bool OnCreatePallet(const void* pvPallet, DWORD dwPalletSize) { return true; }
	virtual bool OnWriteHeader() { return true; }
//  virtual void WriteLine( const void* pvBuffer );
	virtual void OnWriteFinish() {}

	virtual void WriteLine(const void* pvBuffer) {}
	virtual void WriteLineWithAlphaBlend(void* pvBuffer24, const void* pvBuffer32) {}

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
	WORD   m_wBpp;
	WORD   m_wBppWithAlpha;
	UPixel m_unpBackColorWhenAlphaBlend;
};
