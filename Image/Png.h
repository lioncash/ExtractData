#pragma once

#include "ImageBase.h"

class CPng : public CImageBase
{
public:

	enum
	{
		modeRead  = 0x00000000,
		modeWrite = 0x00000001
	};

	struct  SMemory
	{
		BYTE* pbtData;
		DWORD dwDataPtr;
	};


	CPng();
	virtual ~CPng();

	void SetCompressLevel(int nCompressLevel);
	int  GetCompressLevel();

	void SetCallback();

	void Close();

	BOOL Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize);
	BOOL Compress(LPCTSTR pszPathToDst, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);
	BOOL Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize);
	BOOL Compress(void* pvDst, DWORD dwDstSize, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);

	BOOL Decompress();


private:

	png_structp m_png_ptr;
	png_infop   m_info_ptr;
	png_color   m_astPallet[256];

	png_struct* m_pstPNG;
	png_info*   m_pstPNGInfo;
	void*       m_pvParam;
	DWORD       m_dwFlags;
	int         m_nCompressLevel;

	DWORD       m_dwMode;


protected:

	virtual BOOL OnInit(const YCString& rfclsFileName);
	virtual BOOL OnCreatePallet(const void* pvPallet, DWORD dwPalletSize);

	virtual void WriteLine(const void* pvBuffer);
	virtual void WriteLineWithAlphaBlend(void* pvBuffer24, const void* pvBuffer32);
	virtual void OnWriteFinish();

	void SetMode(DWORD dwMode);
	DWORD GetMode();

	static inline void PNGAPI WritePNG(png_structp png_ptr, png_bytep data, png_size_t length);
	static inline void PNGAPI WritePNGToFile(png_struct* pstPNG, png_byte* pbtData, png_size_t siLength);
	static inline void PNGAPI WritePNGToMemory(png_struct* pstPNG, png_byte* pbtData, png_size_t siLength);
};
