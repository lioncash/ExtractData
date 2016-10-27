#include "stdafx.h"
#include "ExtractBase.h"
#include "Image.h"

/// Constructor
CImage::CImage()
{
}

/// Mount
bool CImage::Mount(CArcFile* pclArc)
{
	return m_clBMP.Mount(pclArc);
}

/// Decode
bool CImage::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("BMP"))
		return false;

	// Read BMP
	YCMemory<BYTE> clmbtSrc(file_info->sizeOrg);
	pclArc->Read(&clmbtSrc[0], file_info->sizeOrg);

	// Output
	Init(pclArc, &clmbtSrc[0]);
	Write(file_info->sizeOrg);

	return true;
}

/// Initialization (When BMP is passed)
///
/// @param pclArc        Archive
/// @param pvBMP         BMP data
/// @param rfclsFileName Filename
///
bool CImage::Init(CArcFile* pclArc, void* pvBMP, const YCString& rfclsFileName)
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

		m_bBMPHeader = false;

		return false;
	}

	// Normal BMP
	return Init(pclArc, m_pstBMPInfoHeader->biWidth, m_pstBMPInfoHeader->biHeight, m_pstBMPInfoHeader->biBitCount, &pbtSrc[54], (m_pstBMPFileHeader->bfOffBits - 54), rfclsFileName);
}

/// Initialization
///
/// @param pclArc        Archive
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
/// @param pvPallet      Palette
/// @param dwPalletSize  Palette size
/// @param rfclsFileName Filename
///
bool CImage::Init(CArcFile* pclArc, long lWidth, long lHeight, WORD wBpp, const void* pvPallet, DWORD dwPalletSize, const YCString& rfclsFileName)
{
	m_pclArc = pclArc;
	m_pstOption = pclArc->GetOpt();

	// Header output

	// BMP Output
	if (m_pstOption->bDstBMP)
	{
		TCHAR szFileName[MAX_PATH];

		lstrcpy(szFileName, rfclsFileName);
		PathRenameExtension(szFileName, _T(".bmp"));
		//sFileName.append(_T(".bmp"));

		return m_clBMP.Init(pclArc, lWidth, lHeight, wBpp, pvPallet, dwPalletSize, szFileName);
	}

	// PNG Output
	if (m_pstOption->bDstPNG)
	{
		TCHAR szFileName[MAX_PATH];

		lstrcpy(szFileName, rfclsFileName);
		PathRenameExtension(szFileName, _T(".png"));
		//sFileName.append(_T(".png"));

		return m_clPNG.Init(pclArc, lWidth, lHeight, wBpp, pvPallet, dwPalletSize, szFileName);
	}

	return true;
}

/// Close
void CImage::Close()
{
	m_pclArc->CloseFile();
}

/// Output
///
/// @param dwBMPSize BMP Size
/// @param progress  Request progress bar status
///
bool CImage::Write(DWORD dwBMPSize, bool progress)
{
	return Write(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), progress);
}

/// Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::Write(const void* pvBMPData, DWORD dwBMPDataSize, bool progress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
		return false;

	// BMP Output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.Write(pvBMPData, dwBMPDataSize, progress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.WriteReverse(pvBMPData, dwBMPDataSize, progress);
	}

	return true;
}

/// Inverted Output
///
/// @param dwBMPSize BMP Size
/// @param progress  Request progress bar status
///
bool CImage::WriteReverse(DWORD dwBMPSize, bool progress)
{
	return WriteReverse(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), progress);
}

/// Inverted Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::WriteReverse(const void* pvBMPData, DWORD dwBMPDataSize, bool progress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
		return false;

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteReverse(pvBMPData, dwBMPDataSize, progress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.Write(pvBMPData, dwBMPDataSize, progress);
	}

	return true;
}

/// Synthesize BGRA Output
///
/// @param dwBMPSize BMP Size
/// @param progress  Request BMP status
///
bool CImage::WriteCompoBGRA(DWORD dwBMPSize, bool progress)
{
	return WriteCompoBGRA(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), progress);
}

/// Synthesize BGRA Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::WriteCompoBGRA(const void* pvBMPData, DWORD dwBMPDataSize, bool progress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
		return false;

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoBGRA(pvBMPData, dwBMPDataSize, progress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.WriteCompoBGRAReverse(pvBMPData, dwBMPDataSize, progress);
	}

	return true;
}

/// Synthesize upside-down BGRA Output
///
/// @param dwBMPSize BMP Size
/// @param bProgress Request progress bar status
///
bool CImage::WriteCompoBGRAReverse(DWORD dwBMPSize, bool progress)
{
	return WriteCompoBGRAReverse(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), progress);
}

/// Synthesize upside-down BGRA Output
///
/// @param pvBMPData     BMP Data (header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param progress      Request progress bar status 
///
bool CImage::WriteCompoBGRAReverse(const void* pvBMPData, DWORD dwBMPDataSize, bool progress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
		return false;

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoBGRAReverse(pvBMPData, dwBMPDataSize, progress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_clPNG.WriteCompoBGRA(pvBMPData, dwBMPDataSize, progress);
	}

	return true;
}

/// Synthesize RGBA Output
///
/// @param dwBMPSize BMP Size
/// @param progress  Request progress bar status
///
bool CImage::WriteCompoRGBA(DWORD dwBMPSize, bool progress)
{
	return WriteCompoRGBA(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), progress);
}

/// Synthesize RGBA Output
///
/// @param pvBMPData     BMP Data (Header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::WriteCompoRGBA(const void* pvBMPData, DWORD dwBMPDataSize, bool progress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
		return false;

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoRGBA(pvBMPData, dwBMPDataSize, progress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		m_clPNG.WriteCompoRGBAReverse(pvBMPData, dwBMPDataSize, progress);
	}

	return true;
}

/// Synthesize upside-down RGBA Output
///
/// @param dwBMPSize BMP Size
/// @param progress  Request progress bar status
///
bool CImage::WriteCompoRGBAReverse(DWORD dwBMPSize, bool progress)
{
	return WriteCompoRGBAReverse(&m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), progress);
}

/// Synthesize upside-down RGBA Output
///
/// @param pvBMPData     BMP Data (Header not included)
/// @param dwBMPDataSize BMP Data Size
/// @param bProgress     Requesting progress bar status
/// 
bool CImage::WriteCompoRGBAReverse(const void* pvBMPData, DWORD dwBMPDataSize, bool progress)
{
	// Invalid BMP
	if (!m_bBMPHeader)
		return false;

	// BMP output
	if (m_pstOption->bDstBMP)
	{
		m_clBMP.WriteCompoRGBAReverse(pvBMPData, dwBMPDataSize, progress);
	}
	// PNG output
	else if (m_pstOption->bDstPNG)
	{
		m_clPNG.WriteCompoRGBA(pvBMPData, dwBMPDataSize, progress);
	}

	return true;
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
