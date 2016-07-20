#include "stdafx.h"
#include "../ExtractBase.h"
#include "Png.h"

/// Constructor
CPng::CPng()
{
	m_pstPNG = nullptr;
	m_pstPNGInfo = nullptr;
	m_nCompressLevel = 1;
	m_dwMode = 0;
}


/// Destructor
CPng::~CPng()
{
	Close();
}

/// Set compression rate
void CPng::SetCompressLevel(int nCompressLevel)
{
	m_nCompressLevel = nCompressLevel;
}

/// Get compression rate
///
/// @return Compression Level
///
int CPng::GetCompressLevel() const
{
	return m_nCompressLevel;
}

/// Set callback function
void CPng::SetCallback()
{

}

/// Set mode
void CPng::SetMode(DWORD dwMode)
{
	m_dwMode = dwMode;
}

/// Get mode
///
/// @return Mode
///
DWORD CPng::GetMode() const
{
	return m_dwMode;
}

/// Release PNG structure
void CPng::Close()
{
	if (m_pstPNG != nullptr)
	{
		switch (GetMode())
		{
		case modeRead: // Read mode
			/*
			if (m_pstPNGInfo != nullptr)
			{
				png_read_end(m_pstPNG, m_pstPNGInfo);
				png_destroy_read_struct(&m_pstPNG, &m_pstPNGInfo, nullptr);
			}
			else
			{
				png_destroy_read_struct(&m_pstPNG, nullptr, nullptr);
			}
			*/
			break;

		case modeWrite: // Write mode
			if (m_pstPNGInfo != nullptr)
			{
				png_write_end(m_pstPNG, m_pstPNGInfo);
				png_destroy_write_struct(&m_pstPNG, &m_pstPNGInfo);
			}
			else
			{
				png_destroy_write_struct(&m_pstPNG, nullptr);
			}

			break;
		}

		m_pstPNG = nullptr;
		m_pstPNGInfo = nullptr;
	}
}

/// Convert from BMP to PNG
///
/// @param pszPathToDst Destination file path
/// @param pvBMP        Bitmap data
/// @param dwBMPSize    Bitmap data size

BOOL CPng::Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize)
{
	return TRUE;
}

/// Convert from DIB to PNG
///
/// @param pszPathToDst Destination file path
/// @param pvDIB        DIB data
/// @param dwDIBSize    DIB data size
/// @param pvPallet     Palette
/// @param dwPalletSize Palette size
/// @param wPalletBpp   Number of bits in the palette
/// @param lWidth       Width
/// @param lHeight      Height
/// @param wBpp         Number of bits
///
BOOL CPng::Compress(
	LPCTSTR     pszPathToDst,
	const void* pvDIB,
	DWORD       dwDIBSize,
	const void* pvPallet,
	DWORD       dwPalletSize,
	WORD        wPalletBpp,
	long        lWidth,
	long        lHeight,
	WORD        wBpp
	)
{
	return TRUE;
}

/// Convert from BMP to PNG
///
/// @param pvDst     Destination
/// @param dwDstSize Destination size
/// @param pvBMP     Bitmap data
/// @param dwBMPSize Bitmap data size
///
BOOL CPng::Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize)
{
	return TRUE;
}

/// Converted from DIB to PNG
///
/// @param pszPathToDst Destination file path
/// @param pvDIB        DIB data
/// @param dwDIBSize    DIB data size
/// @param pvPallet     Palette
/// @param dwPalletSize Palette size
/// @param wPalletBpp   Number of bits in the palette
/// @param lWidth       Width
/// @param lHeight      Height
/// @param wBpp         Number of bits
///
BOOL CPng::Compress(
	void*       pvDst,
	DWORD       dwDstSize,
	const void* pvDIB,
	DWORD       dwDIBSize,
	const void* pvPallet,
	DWORD       dwPalletSize,
	WORD        wPalletBpp,
	long        lWidth,
	long        lHeight,
	WORD        wBpp
	)
{
/*	BYTE*       pbtDst = (BYTE*) pvDst;
	const BYTE* pbtDIB = (const BYTE*) pvDIB;
	const BYTE* pbtPallet = (const BYTE*) pvPallet;

	// Set Mode
	SetMode(modeWrite);

	// Get the color type
	int nColorType;

	switch (wBpp)
	{
		case 8: // 8bit
			nColorType = PNG_COLOR_TYPE_PALETTE;
			break;

		case 24: // 24bit
			nColorType = PNG_COLOR_TYPE_RGB;
			break;

		case 32: // 32bit
			nColorType = PNG_COLOR_TYPE_RGB_ALPHA;
			break;

		default: // Other
			return FALSE;
	}

	// Create PNG structure
	m_pstPNG = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (m_pstPNG == nullptr)
		return FALSE;

	// Create PNG structure information
	m_pstPNGInfo = png_create_info_struct(m_pstPNG);
	if (m_pstPNGInfo == nullptr)
		return FALSE;

	// Set parameters
	SMemory stmDst;
	stmDst.pbtData = pbtDst;
	stmDst.dwDataPtr = 0;

	png_set_write_fn(m_pstPNG, &stmDst, WritePNGToMemory, nullptr);

	png_set_bgr(m_pstPNG);
	png_set_compression_level(m_pstPNG, GetCompressLevel());

	if (wBpp == 8)
	{
		png_set_PLTE(m_pstPNG, m_pstPNGInfo, m_astPallet, 256);
	}

	png_set_IHDR(m_pstPNG, m_pstPNGInfo, lWidth, lHeight, 8, nColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// PNG output header
	png_write_info(m_pstPNG, m_pstPNGInfo);

	// PNG output data
	if (IsRequireAlphaBlend())
	{
		// Alpha-blend on request

		long           lPitch = CalculatePitch(lWidth, wBpp);
		YCMemory<BYTE> clmAlphaBlendBuffer(lPitch);

		for (long lY = 0 ; lY < lHeight ; lY++)
		{
			AlphaBlend(&clmAlphaBlendBuffer[0], &pbtDIB[lY * lPitch]);

			png_write_row(m_pstPNG, &clmAlphaBlendBuffer[0]);
		}
	}
	else // No alpha-blending
	{
		long lPitch = CalculatePitch(lWidth, wBpp);

		for (long lY = 0 ; lY < lHeight ; lY++)
		{
			png_write_row(m_pstPNG, &pbtDIB[lY * lPitch]);
		}
	}
*/
	return TRUE;
}

/// Convert to DIB
BOOL CPng::Decompress()
{
	return TRUE;
}

/// Initialization
BOOL CPng::OnInit(const YCString& rfclsFileName)
{
	int nColorType;

	switch (m_wBpp)
	{
	case 8: // 8bit
		nColorType = PNG_COLOR_TYPE_PALETTE;
		break;

	case 24: // 24bit
		nColorType = PNG_COLOR_TYPE_RGB;
		break;

	case 32: // 32bit
		nColorType = PNG_COLOR_TYPE_RGB_ALPHA;
		break;

	default: // Other
		return FALSE;
	}

	m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (m_png_ptr == nullptr)
		return FALSE;

	m_info_ptr = png_create_info_struct(m_png_ptr);
	if (m_info_ptr == nullptr)
	{
		png_destroy_write_struct(&m_png_ptr, nullptr);
		return FALSE;
	}

	png_set_write_fn(m_png_ptr, m_pclArc, WritePNG, nullptr);

	png_set_bgr(m_png_ptr);
	png_set_compression_level(m_png_ptr, m_pclArc->GetOpt()->CmplvPng);

	if (m_wBpp == 8)
	{
		png_set_PLTE(m_png_ptr, m_info_ptr, m_astPallet, 256);
	}

	png_set_IHDR(m_png_ptr, m_info_ptr, m_lWidth, m_lHeight, 8, nColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Open the output file
	m_pclArc->OpenFile(rfclsFileName);

	// Output header
	png_write_info(m_png_ptr, m_info_ptr);

	return TRUE;
}

/// Create Palette
///
/// @param pvSrcPallet     Reference palette
/// @param dwSrcPalletSize Reference palette size
///
BOOL CPng::OnCreatePallet(const void* pvSrcPallet, DWORD dwSrcPalletSize)
{
	png_colorp  pstPallet = m_astPallet;
	const BYTE* pbtSrcPallet = reinterpret_cast<const BYTE*>(pvSrcPallet);

	ZeroMemory(m_astPallet, sizeof(m_astPallet));

	// Use the default palette (Grayscale)
	if (pbtSrcPallet == nullptr)
	{
		for (int i = 0; i < 256; i++)
		{
			pstPallet[i].blue = i;
			pstPallet[i].green = i;
			pstPallet[i].red = i;
		}
	}
	else // Refer to the palette
	{
		// 1024 byte palette
		if (dwSrcPalletSize == 1024)
		{
			for (int i = 0; i < 256; i++)
			{
				pstPallet[i].blue = *pbtSrcPallet++;
				pstPallet[i].green = *pbtSrcPallet++;
				pstPallet[i].red = *pbtSrcPallet++;
				pbtSrcPallet++;
			}
		}
		// 768 byte palette (No alpha)
		else if (dwSrcPalletSize == 768)
		{
			for (int i = 0; i < 256; i++)
			{
				pstPallet[i].blue = *pbtSrcPallet++;
				pstPallet[i].green = *pbtSrcPallet++;
				pstPallet[i].red = *pbtSrcPallet++;
			}
		}
		// Other palette sizes
		else
		{
			for (int i = 0; i < dwSrcPalletSize / 4; i++)
			{
				pstPallet[i].blue = *pbtSrcPallet++;
				pstPallet[i].green = *pbtSrcPallet++;
				pstPallet[i].red = *pbtSrcPallet++;
				pbtSrcPallet++;
			}
		}
	}

	return TRUE;
}

/// Write 1 Line
void CPng::WriteLine(const void* pvBuffer)
{
	png_write_row(m_png_ptr, (png_bytep) pvBuffer);
	m_pclArc->GetProg()->UpdatePercent(m_dwRowSize);
}

/// Write a line with alpha blending
///
/// @param pvBuffer24 Storage Location
/// @param pvBuffer32 32-bit data
///
void CPng::WriteLineWithAlphaBlend(void* pvBuffer24, const void* pvBuffer32)
{
	// Alpha blending
	AlphaBlend(pvBuffer24, pvBuffer32);

	png_write_row(m_png_ptr, reinterpret_cast<png_bytep>(pvBuffer24));
	m_pclArc->GetProg()->UpdatePercent(m_dwRowSize);
}

/// Finish output
void CPng::OnWriteFinish()
{
	png_write_end(m_png_ptr, m_info_ptr);
	png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
}

/// PNG Output
void CPng::WritePNG(png_structp png_ptr, png_bytep data, png_size_t length)
{
	CArcFile* pclArc = reinterpret_cast<CArcFile*>(png_get_io_ptr(png_ptr));

	pclArc->WriteFile(data, length, 0);
}

/// File Output
void CPng::WritePNGToFile(png_struct* pstPNG, png_byte* pbtData, png_size_t siLength)
{
	YCFile* pclfDst = reinterpret_cast<YCFile*>(png_get_io_ptr(pstPNG));

	pclfDst->Write(pbtData, siLength);
}

/// Memory Output
void CPng::WritePNGToMemory(png_struct* pstPNG, png_byte* pbtData, png_size_t siLength)
{
	SMemory* pstmDst = reinterpret_cast<SMemory*>(png_get_io_ptr(pstPNG));

	memcpy(&pstmDst->pbtData[pstmDst->dwDataPtr], pbtData, siLength);

	pstmDst->dwDataPtr += siLength;
}
