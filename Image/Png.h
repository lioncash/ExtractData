#pragma once

#include "ImageBase.h"

class CPng final : public CImageBase
{
public:
	CPng();
	~CPng() override;

	void SetCompressLevel(int nCompressLevel);
	int  GetCompressLevel() const;

	void SetCallback();

	void Close();

	bool Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize);
	bool Compress(LPCTSTR pszPathToDst, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);
	bool Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize);
	bool Compress(void* pvDst, DWORD dwDstSize, const void* pvDIB, DWORD dwDIBSize, const void* pvPallet, DWORD dwPalletSize, WORD wPalletBpp, long lWidth, long lHeight, WORD wBpp);

	bool Decompress();

private:
	enum class Mode
	{
		Read  = 0x00000000,
		Write = 0x00000001
	};

	struct SMemory
	{
		BYTE* pbtData;
		DWORD dwDataPtr;
	};

	bool OnInit(const YCString& rfclsFileName) override;
	bool OnCreatePallet(const void* pvPallet, DWORD dwPalletSize) override;

	void WriteLine(const void* pvBuffer) override;
	void WriteLineWithAlphaBlend(void* pvBuffer24, const void* pvBuffer32) override;
	void OnWriteFinish() override;

	static inline void PNGAPI WritePNG(png_structp png_ptr, png_bytep data, png_size_t length);
	static inline void PNGAPI WritePNGToFile(png_struct* pstPNG, png_byte* pbtData, png_size_t siLength);
	static inline void PNGAPI WritePNGToMemory(png_struct* pstPNG, png_byte* pbtData, png_size_t siLength);

	png_structp m_png_ptr;
	png_infop   m_info_ptr;
	png_color   m_astPallet[256];

	png_struct* m_pstPNG;
	png_info*   m_pstPNGInfo;
	void*       m_pvParam;
	DWORD       m_dwFlags;
	int         m_nCompressLevel;

	Mode        m_mode = Mode::Read;
};
