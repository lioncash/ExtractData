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

	BOOL Init(CArcFile* pclArc, long lWidth, long lHeight, WORD wBpp, const void* pvPallet = NULL, DWORD dwPalletSize = 1024, const YCString& rfclsFileName = _T(""));

	void SetValidityOfAlphaBlend(BOOL bValidityOfAlphaBlend);
	BOOL GetValidityOfAlphaBlend();

	void SetBackColorWhenAlphaBlend(COLORREF crBackColor);

	BOOL Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize);
	BOOL Compress(LPCTSTR pszPathToDst, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);
	BOOL Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize);
	BOOL Compress(void* pvDst, DWORD dwDstSize, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);

	BOOL ComposeBGRA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize);
	void WriteCompoBGRA(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress = TRUE);
	void WriteCompoBGRAReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress = TRUE);

	BOOL ComposeRGBA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize);
	void WriteCompoRGBA(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress = TRUE);
	void WriteCompoRGBAReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress = TRUE);

	void Write(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress = TRUE);
	void WriteReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress = TRUE);

	void WriteFinish();

protected:
	inline long GetWidth();
	inline long GetHeight();
	inline WORD GetBitCount();
	inline WORD GetBitCountWithAlpha();
	inline long GetPitch();
	inline long GetPitchWithAlpha();

	static long CalculatePitch( long lWidth, WORD wBpp );

	CArcFile* m_pclArc;
	BYTE m_abtBG[4];

	DWORD m_dwRowSize;
	DWORD m_dwRowSizeOfRemainder;

	BOOL IsRequireAlphaBlend();

	void AlphaBlend(void* pvBuffer24, const void* pvBuffer32);

	virtual BOOL OnInit(const YCString& rfclsFileName) { return TRUE; }
	virtual BOOL OnCreatePallet(const void* pvPallet, DWORD dwPalletSize) { return TRUE; }
	virtual BOOL OnWriteHeader() { return TRUE; }
//  virtual void WriteLine( const void* pvBuffer );
	virtual void OnWriteFinish() {}

	virtual void WriteLine(const void* pvBuffer) {}
	virtual void WriteLineWithAlphaBlend(void* pvBuffer24, const void* pvBuffer32) {}

protected:
	BOOL   m_bValidityOfAlphaBlend;
	BOOL   m_bAlphaBlendRequirement;
	BOOL   m_bOutputDummyFromBuffer;

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
