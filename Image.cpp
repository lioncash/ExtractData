#include "stdafx.h"
#include "ExtractBase.h"
#include "Image.h"

/// Constructor
CImage::CImage()
{
	m_pbtBMP = nullptr;
	m_pstBMPFileHeader = nullptr;
	m_pstBMPInfoHeader = nullptr;
	m_bBMPHeader = TRUE;
}

/// Mount
BOOL CImage::Mount(CArcFile* pclArc)
{
	if (m_clBMP.Mount(pclArc))
	{
		return TRUE;
	}

	return FALSE;
}

/// Decode
BOOL CImage::Decode(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if (pstFileInfo->format != _T("BMP"))
	{
		return FALSE;
	}

	// Read BMP
	YCMemory<BYTE> clmbtSrc(pstFileInfo->sizeOrg);
	pclArc->Read(&clmbtSrc[0], pstFileInfo->sizeOrg);

	// Output
	Init(pclArc, &clmbtSrc[0]);
	Write(pstFileInfo->sizeOrg);

	return TRUE;
}

/// Initialization (When BMP is passed)
///
/// @param pclArc        Archive
/// @param pvBMP         BMP data
/// @param rfclsFileName Filename
///
BOOL CImage::Init(CArcFile* pclArc, void* pvBMP, const YCString& rfclsFileName)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	BYTE*      pbtSrc = static_cast<BYTE*>(pvBMP);

	if ((pstFileInfo->format == _T("BMP")) && (pstFileInfo->title == _T("")) && (pstFileInfo->key == 0))
	{
		// Simple decoding

		pclArc->InitDecrypt(pbtSrc);
		pclArc->Decrypt(pbtSrc, pstFileInfo->sizeOrg);
	}

	m_pclArc = pclArc;
	m_pbtBMP = pbtSrc;
	m_pstBMPFileHeader = reinterpret_cast<BITMAPFILEHEADER*>(&pbtSrc[0]);
	m_pstBMPInfoHeader = reinterpret_cast<BITMAPINFOHEADER*>(&pbtSrc[14]);

	if (m_pstBMPFileHeader->bfType != 0x4D42)
	{
		// No BM or such in an encryption header

		pclArc->OpenFile();
		pclArc->WriteFile(pbtSrc, pstFileInfo->sizeOrg);

		m_bBMPHeader = FALSE;

		return FALSE;
	}

	// Normal BMP
	return Init(pclArc, m_pstBMPInfoHeader->biWidth, m_pstBMPInfoHeader->biHeight, m_pstBMPInfoHeader->biBitCount, &pbtSrc[54], (m_pstBMPFileHeader->bfOffBits - 54), rfclsFileName);
}

/// Initialization
///
/// @param pclArc        Archive
/// @param lWidth        Width
/// @param lheight       Height
/// @param wBpp          Number of bits
/// @param pvPallet      Palette
/// @param dwPalletSize  Palette size
/// @param rfclsFileName Filename
///
BOOL CImage::Init(CArcFile* pclArc, long lWidth, long lHeight, WORD wBpp, const void* pvPallet, DWORD dwPalletSize, const YCString& rfclsFileName)
{
	BOOL bReturn = TRUE;

	m_pclArc = pclArc;
	m_pstOption = pclArc->GetOpt();

	// Header output

	if (m_pstOption->bDstBMP)
	{
		// BMP Output

		TCHAR szFileName[MAX_PATH];

		lstrcpy(szFileName, rfclsFileName);
		PathRenameExtension(szFileName, _T(".bmp"));
		//sFileName.append(_T(".bmp"));

		bReturn = m_clBMP.Init(pclArc, lWidth, lHeight, wBpp, pvPallet, dwPalletSize, szFileName);
	}
	else if (m_pstOption->bDstPNG)
	{
		// PNG Output

		TCHAR szFileName[MAX_PATH];

		lstrcpy(szFileName, rfclsFileName);
		PathRenameExtension(szFileName, _T(".png"));
		//sFileName.append(_T(".png"));

		bReturn = m_clPNG.Init(pclArc, lWidth, lHeight, wBpp, pvPallet, dwPalletSize, szFileName);
	}

	return bReturn;
}

/// Close
void CImage::Close()
{
	m_pclArc->CloseFile();
}

/// Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request progress bar status
///
BOOL CImage::Write(DWORD dwBMPSize, BOOL bProgress)
{
	return Write(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress);
}

/// Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Request progress bar status
///
BOOL CImage::Write(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
	{
		return FALSE;
	}

	// BMP Output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.Write(pvBMPData, dwBMPDataSize, bProgress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.WriteReverse(pvBMPData, dwBMPDataSize, bProgress);
	}

	return TRUE;
}

/// Inverted Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request progress bar status
///
BOOL CImage::WriteReverse(DWORD dwBMPSize, BOOL bProgress)
{
	return WriteReverse(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress);
}

/// Inverted Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Request progress bar status
///
BOOL CImage::WriteReverse(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
	{
		return FALSE;
	}

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteReverse(pvBMPData, dwBMPDataSize, bProgress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.Write(pvBMPData, dwBMPDataSize, bProgress);
	}

	return TRUE;
}

/// Synthesize BGRA Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request BMP status
///
BOOL CImage::WriteCompoBGRA(DWORD dwBMPSize, BOOL bProgress)
{
	return WriteCompoBGRA(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress);
}

/// Synthesize BGRA Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Request progress bar status
///
BOOL CImage::WriteCompoBGRA(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
	{
		return FALSE;
	}

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoBGRA(pvBMPData, dwBMPDataSize, bProgress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.WriteCompoBGRAReverse(pvBMPData, dwBMPDataSize, bProgress);
	}

	return TRUE;
}

/// Synthesize upside-down BGRA Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request progress bar status
///
BOOL CImage::WriteCompoBGRAReverse(DWORD dwBMPSize, BOOL bProgress)
{
	return WriteCompoBGRAReverse(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress);
}

/// Synthesize upside-down BGRA Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Request progress bar status 
///
BOOL CImage::WriteCompoBGRAReverse(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
	{
		return FALSE;
	}

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoBGRAReverse(pvBMPData, dwBMPDataSize, bProgress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.WriteCompoBGRA(pvBMPData, dwBMPDataSize, bProgress);
	}

	return TRUE;
}

/// Synthesize RGBA Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request progress bar status
///
BOOL CImage::WriteCompoRGBA(DWORD dwBMPSize, BOOL bProgress)
{
	return WriteCompoRGBA(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress);
}

/// Synthesize RGBA Output
///
/// @param pvBMPData     BMP Data (Header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Request progress bar status
///
BOOL CImage::WriteCompoRGBA(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
	{
		return FALSE;
	}

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoRGBA(pvBMPData, dwBMPDataSize, bProgress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		m_clPNG.WriteCompoRGBAReverse(pvBMPData, dwBMPDataSize, bProgress);
	}

	return TRUE;
}

/// Synthesize upside-down RGBA Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request progress bar status
///
BOOL CImage::WriteCompoRGBAReverse(DWORD dwBMPSize, BOOL bProgress)
{
	return WriteCompoRGBAReverse(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress);
}

/// Synthesize upside-down RGBA Output
///
/// @param pvBMPData     BMP Data (Header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Requesting progress bar status
/// 
BOOL CImage::WriteCompoRGBAReverse(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
	{
		return FALSE;
	}

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoRGBAReverse(pvBMPData, dwBMPDataSize, bProgress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		m_clPNG.WriteCompoRGBA(pvBMPData, dwBMPDataSize, bProgress);
	}

	return TRUE;
}

/// Get bitmap file header
BITMAPFILEHEADER* CImage::GetBmpFileHeader()
{
	return m_pstBMPFileHeader;
}

/// Get bitmap info header
BITMAPINFOHEADER* CImage::GetBmpInfoHeader()
{
	return m_pstBMPInfoHeader;
}
