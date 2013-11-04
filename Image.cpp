#include "stdafx.h"
#include "ExtractBase.h"
#include "Image.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

CImage::CImage()
{
	m_pbtBMP = NULL;
	m_pstBMPFileHeader = NULL;
	m_pstBMPInfoHeader = NULL;
	m_bBMPHeader = TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mount

BOOL CImage::Mount(
	CArcFile*			pclArc							// Archive
	)
{
	if( m_clBMP.Mount( pclArc) )
	{
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decode

BOOL CImage::Decode(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("BMP") )
	{
		return FALSE;
	}

	// Read BMP
	YCMemory<BYTE> clmbtSrc( pstFileInfo->sizeOrg );
	pclArc->Read( &clmbtSrc[0], pstFileInfo->sizeOrg );

	// Output
	Init( pclArc, &clmbtSrc[0] );
	Write( pstFileInfo->sizeOrg );

	return TRUE;
}

////////////////////////////////////////////////////////////
// Initialization (When BMP is passed)

BOOL CImage::Init(
	CArcFile*			pclArc,							// Archive
	void*				pvBMP,							// BMP
	const YCString&		rfclsFileName					// Filename
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	BYTE*      pbtSrc = (BYTE*) pvBMP;

	if( (pstFileInfo->format == _T("BMP")) && (pstFileInfo->title == _T("")) && (pstFileInfo->key == 0) )
	{
		// Simple decoding

		pclArc->InitDecrypt( pbtSrc );
		pclArc->Decrypt( pbtSrc, pstFileInfo->sizeOrg );
	}

	m_pclArc = pclArc;
	m_pbtBMP = pbtSrc;
	m_pstBMPFileHeader = (BITMAPFILEHEADER*) &pbtSrc[0];
	m_pstBMPInfoHeader = (BITMAPINFOHEADER*) &pbtSrc[14];

	if( m_pstBMPFileHeader->bfType != 0x4D42 )
	{
		// No BM or such in an encryption header

		pclArc->OpenFile();
		pclArc->WriteFile( pbtSrc, pstFileInfo->sizeOrg );

		m_bBMPHeader = FALSE;

		return FALSE;
	}

	// Normal BMP
	return Init( pclArc, m_pstBMPInfoHeader->biWidth, m_pstBMPInfoHeader->biHeight, m_pstBMPInfoHeader->biBitCount, &pbtSrc[54], (m_pstBMPFileHeader->bfOffBits - 54), rfclsFileName );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization

BOOL CImage::Init(
	CArcFile*			pclArc,							// Archive
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp,							// Number of bits
	const void*			pvPallet,						// Palette
	DWORD				dwPalletSize,					// Palette Size
	const YCString&		rfclsFileName					// Filename
	)
{
	BOOL bReturn = TRUE;

	m_pclArc = pclArc;
	m_pstOption = pclArc->GetOpt();

	// Header output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		TCHAR szFileName[MAX_PATH];

		lstrcpy( szFileName, rfclsFileName );
		PathRenameExtension( szFileName, _T(".bmp") );
		//sFileName.append(_T(".bmp"));

		bReturn = m_clBMP.Init( pclArc, lWidth, lHeight, wBpp, pvPallet, dwPalletSize, szFileName );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		TCHAR szFileName[MAX_PATH];

		lstrcpy( szFileName, rfclsFileName );
		PathRenameExtension( szFileName, _T(".png") );
		//sFileName.append(_T(".png"));

		bReturn = m_clPNG.Init( pclArc, lWidth, lHeight, wBpp, pvPallet, dwPalletSize, szFileName );
	}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Close

void CImage::Close()
{
	m_pclArc->CloseFile();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Output

BOOL CImage::Write(
	DWORD				dwBMPSize,						// BMP Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	return Write( &m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Output

BOOL CImage::Write(
	const void*			pvBMPData,						// BMP Data(Header not included)
	DWORD				dwBMPDataSize,					// BMP Data Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	if( !m_bBMPHeader )
	{
		// Illegal BMP

		return FALSE;
	}

	// Output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		m_clBMP.Write( pvBMPData, dwBMPDataSize, bProgress );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		m_clPNG.WriteReverse( pvBMPData, dwBMPDataSize, bProgress ); // BMP format for top-down to Reverse
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Inverted Output

BOOL CImage::WriteReverse(
	DWORD				dwBMPSize,						// BMP Size
	BOOL				bProgress						// Request progressbar progresss
	)
{
	return WriteReverse( &m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Inverted Output

BOOL CImage::WriteReverse(
	const void*			pvBMPData,						// BMP Data(Header not included)
	DWORD				dwBMPDataSize,					// BMP Data Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	if( !m_bBMPHeader )
	{
		// Illegal BMP

		return FALSE;
	}

	// Output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		m_clBMP.WriteReverse( pvBMPData, dwBMPDataSize, bProgress );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		m_clPNG.Write( pvBMPData, dwBMPDataSize, bProgress ); // BMP format for top-down to Reverse
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize BGRA Output

BOOL CImage::WriteCompoBGRA(
	DWORD				dwBMPSize,						// BMP Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	return WriteCompoBGRA( &m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize BGRA Output

BOOL CImage::WriteCompoBGRA(
	const void*			pvBMPData,						// BMP Data (Header not included)
	DWORD				dwBMPDataSize,					// BMP Data Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	if( !m_bBMPHeader )
	{
		// Illegal BMP

		return FALSE;
	}

	// Output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		m_clBMP.WriteCompoBGRA( pvBMPData, dwBMPDataSize, bProgress );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		m_clPNG.WriteCompoBGRAReverse( pvBMPData, dwBMPDataSize, bProgress ); // BMP format for top-down to Reverse
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize upside-down BGRA Output 

BOOL CImage::WriteCompoBGRAReverse(
	DWORD				dwBMPSize,						// BMP Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	return WriteCompoBGRAReverse( &m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize upside-down BGRA Output

BOOL CImage::WriteCompoBGRAReverse(
	const void*			pvBMPData,						// BMP Data (Header is not included)
	DWORD				dwBMPDataSize,					// BMP Data Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	if( !m_bBMPHeader )
	{
		// Illegal BMP

		return FALSE;
	}

	// Output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		m_clBMP.WriteCompoBGRAReverse( pvBMPData, dwBMPDataSize, bProgress );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		m_clPNG.WriteCompoBGRA( pvBMPData, dwBMPDataSize, bProgress ); // BMP format for top-down to Reverse
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize RGBA Output

BOOL CImage::WriteCompoRGBA(
	DWORD				dwBMPSize,						// BMP Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	return WriteCompoRGBA( &m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize RGBA Output

BOOL CImage::WriteCompoRGBA(
	const void*			pvBMPData,						// BMP Data (Header not included)
	DWORD				dwBMPDataSize,					// BMP Data Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	if( !m_bBMPHeader )
	{
		// Illegal BMP

		return FALSE;
	}

	// Output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		m_clBMP.WriteCompoRGBA( pvBMPData, dwBMPDataSize, bProgress );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		m_clPNG.WriteCompoRGBAReverse( pvBMPData, dwBMPDataSize, bProgress );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize upside-down RGBA Output

BOOL CImage::WriteCompoRGBAReverse(
	DWORD				dwBMPSize,						// BMP Size
	BOOL				bProgress						// Request progressbar progress
	)
{
	return WriteCompoRGBAReverse( &m_pbtBMP[m_pstBMPFileHeader->bfOffBits], (dwBMPSize - m_pstBMPFileHeader->bfOffBits), bProgress );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize upside-down RGBA Output

BOOL CImage::WriteCompoRGBAReverse(
	const void*			pvBMPData,						// BMP Data (Header is not included)
	DWORD				dwBMPDataSize,					// BMP Data Size
	BOOL				bProgress						// Request progresbar progress
	)
{
	if( !m_bBMPHeader )
	{
		// Illegal BMP

		return FALSE;
	}

	// Output

	if( m_pstOption->bDstBMP )
	{
		// BMP Output

		m_clBMP.WriteCompoRGBAReverse( pvBMPData, dwBMPDataSize, bProgress );
	}
	else if( m_pstOption->bDstPNG )
	{
		// PNG Output

		m_clPNG.WriteCompoRGBA( pvBMPData, dwBMPDataSize, bProgress );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap file header

BITMAPFILEHEADER* CImage::GetBmpFileHeader()
{
	return m_pstBMPFileHeader;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Get bitmap info header

BITMAPINFOHEADER* CImage::GetBmpInfoHeader()
{
	return m_pstBMPInfoHeader;
}
