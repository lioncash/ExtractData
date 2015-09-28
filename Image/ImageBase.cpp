#include "stdafx.h"
#include "ImageBase.h"

/// Constructor
CImageBase::CImageBase()
{
	m_bAlphaBlendRequirement = FALSE;
	m_bOutputDummyFromBuffer = FALSE;
}

/// Destructor
CImageBase::~CImageBase()
{
}

/// Initialization
///
/// @param pclArc        Archive
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
/// @param pvPallet      Palette
/// @param dwPalletSize  Palette size
/// @param rfclsFileName File name
///
BOOL CImageBase::Init(
	CArcFile*       pclArc,
	long            lWidth,
	long            lHeight,
	WORD            wBpp,
	const void*     pvPallet,
	DWORD           dwPalletSize,
	const YCString& rfclsFileName
	)
{
	// Fix top-down, bottom-up images
	if (lHeight < 0)
	{
		lHeight = -lHeight;
	}

	// Number of bits
	switch (wBpp)
	{
	case 8: // 8bit
		OnCreatePallet(pvPallet, dwPalletSize);
		break;

	case 32: // 32bit
		if (pclArc->GetOpt()->bAlphaBlend)
		{
			// Alpha-blending is enabled
			m_bAlphaBlendRequirement = TRUE;
			wBpp = 24;

			for (int i = 0; i < 3; i++)
			{
				m_abtBG[i] = BYTE((pclArc->GetOpt()->BgRGB >> (i << 3)) & 0xFF);
			}
		}
	}

	m_wBpp = wBpp;
	m_pclArc = pclArc;
	m_lWidth = lWidth;
	m_lHeight = lHeight;
	m_lLine = lWidth * (wBpp >> 3);
	m_lPitch = CalculatePitch(lWidth, wBpp);
	m_lPitchWithAlpha = lWidth * 4;

	DWORD dwOriginSize = pclArc->GetOpenFileInfo()->sizeOrg;

	m_dwRowSize = dwOriginSize / lHeight;
	m_dwRowSizeOfRemainder = dwOriginSize % lHeight;

	return OnInit(rfclsFileName);
}

/// Set the validity of alpha blending
///
/// @param bValidityOfAlphaBlend Check validity of alpha-blending
///
void CImageBase::SetValidityOfAlphaBlend(BOOL bValidityOfAlphaBlend)
{
	m_bValidityOfAlphaBlend = bValidityOfAlphaBlend;
}

/// Get the validity of alpha blending
///
/// @return Check validity of alpha blending
///
BOOL CImageBase::GetValidityOfAlphaBlend()
{
	return m_bValidityOfAlphaBlend;
}

/// Set background color when alpha blending
///
/// @param crBackColor Background color
///
void CImageBase::SetBackColorWhenAlphaBlend(COLORREF crBackColor)
{
	m_unpBackColorWhenAlphaBlend.crPixel = crBackColor;
}

/// Get width
long CImageBase::GetWidth()
{
	return m_lWidth;
}

/// Get height
long CImageBase::GetHeight()
{
	return m_lHeight;
}

/// Get number of bits
WORD CImageBase::GetBitCount()
{
	return m_wBpp;
}

/// Get the number of alpha bits
WORD CImageBase::GetBitCountWithAlpha()
{
	return m_wBppWithAlpha;
}

/// Get Pitch
long CImageBase::GetPitch()
{
	return m_lPitch;
}

/// Get the pitch with alpha
long CImageBase::GetPitchWithAlpha()
{
	return m_lPitchWithAlpha;
}

/// Compression
///
/// @param pszPathToDst Destination file path
/// @param pvBMP        Bitmap data
/// @param dwBMPSize    Bitmap data size
///
BOOL CImageBase::Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize)
{
	return TRUE;
}

/// Compression
///
/// @param pszPathToDst Destination file path
/// @param pvDIB        DIB data
/// @param dwDIBSize    DIB data size
/// @param pvPallet     Palette
/// @param dwPalletSize Palette size
/// @param wPalletBpp   Number of bits in palette
/// @param lWidth       Width
/// @param lHeight      Height
/// @param wBpp         Number of bits
///
BOOL CImageBase::Compress(
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
/*	const BYTE* pbtDIB = (const BYTE*) pvDIB;
	const BYTE* pbtPallet = (const BYTE*) pvPallet;

	// Initialization
	OnInit( lWidth, lHeight, wBpp, pvPallet, dwPalletSize, wPalletBpp );

	// Output header
	OnWriteHeader();

	// Output data
	if( IsRequireAlphaBlend() )
	{
		// Alpha blending is enabled

		long           lDstSizeAfterAlphaBlend = (GetPitch() + 2); // Ensure high-speed processing for alpha

		YCMemory<BYTE> clmDstAfterAlphaBlend( lDstSizeAfterAlphaBlend );

		ZeroMemory( &clmDstOfAfterAlphaBlend[0], lDstSizeAfterAlphaBlend );

		for( long lY = 0 ; lY < GetHeight() ; lY++ )
		{
			AlphaBlend( &clmDstAfeterAlphaBlend[0], pbtDIB, m_unpBackColorWhenAlphaBlend, lWidth );

			OnWriteLine( &clmDstAfeterAlphaBlend[0], GetPitch() );

			pbtDIB += GetPitchWithAlpha();
		}
	}
	else
	{
		// Request without alpha blending

		long lLine = m_lLine;

		if( dwBufferSize >= m_lPitch * m_lHeight )
		{
			// Read dummy data from buffer

			m_bOutputDummyFromBuffer = TRUE;
			lLine = m_lPitch;
		}

		for( long lY = 0 ; lY < m_lHeight ; lY++ )
		{
			OnWriteLine( pbtBuffer );
			pbtBuffer += lLine;
		}
	}

	OnWriteFinish();
*/
	return TRUE;
}

/// Compression
///
/// @param pvDst     Destination
/// @param dwDstSize Destination size
/// @param pvBMP     Bitmap data
/// @param dwBMPSize Bitmap data size
///
BOOL CImageBase::Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize)
{
	return TRUE;
}

/// Compression
///
/// @param pvDst        Destination
/// @param dwDstSize    Destination size
/// @param pvDIB        DIB data
/// @param dwDIBSize    DIB data size
/// @param pvPallet     Palette
/// @param dwPalletSize Palette size
/// @param wPalletBpp   Number of bits in the palette
/// @param lWidth       Width
/// @param lHeight      Height
/// @param wBpp         Number of bits
///
BOOL CImageBase::Compress(
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

	// Initialization
	OnInit( lWidth, lHeight, wBpp, pvPallet, dwPalletSize, wPalletBpp );

	// Output header
	OnWriteHeader();

	// Output data
	if( IsRequireAlphaBlend() )
	{
		// Request alpha-blending

		long           lDstSizeAfterAlphaBlend = (GetPitch() + 2);	// Ensure high-speed alpha blending

		YCMemory<BYTE> clmDstAfterAlphaBlend( lDstSizeAfterAlphaBlend );

		ZeroMemory( &clmDstOfAfterAlphaBlend[0], lDstSizeAfterAlphaBlend );

		for( long lY = 0 ; lY < GetHeight() ; lY++ )
		{
			AlphaBlend( &clmDstAfeterAlphaBlend[0], pbtDIB );

			OnWriteLine( 

			OnWriteLineWithAlphaBlending( &clmbtBuffer24[0], pbtBuffer );
			pbtBuffer += m_lLine32;
		}
	}
	else
	{
		// No Alpha blending

		long lLine = m_lLine;

		if( dwBufferSize >= m_lPitch * m_lHeight )
		{
			// Read dummy data from buffer

			m_bOutputDummyFromBuffer = TRUE;
			lLine = m_lPitch;
		}

		for( long lY = 0 ; lY < m_lHeight ; lY++ )
		{
			OnWriteLine( pbtBuffer );
			pbtBuffer += lLine;
		}
	}

	OnWriteFinish();*/

	return TRUE;
}

/// Receive alpha-blending requests
///
/// @return Alpha-blend request
///
/// @remark If TRUE, then alpha-blending is enabled
///
BOOL CImageBase::IsRequireAlphaBlend()
{
	return m_bAlphaBlendRequirement;
}

/// Alpha Blending
///
/// Parameters:
/// @param pvBuffer24 Storage location
/// @param pvBuffer32 32-bit Data
///
void CImageBase::AlphaBlend(void* pvBuffer24, const void* pvBuffer32)
{
	BYTE*       pbtBuffer24 = reinterpret_cast<BYTE*>(pvBuffer24);
	const BYTE* pbtBuffer32 = reinterpret_cast<const BYTE*>(pvBuffer32);
	const BYTE* pbtBG = m_abtBG;
	long        lWidth = m_lWidth;

	for (long lX = 0; lX < lWidth; lX++)
	{
		switch (pbtBuffer32[3])
		{
		case 0x00: // Alpha value is 0
			for (int i = 0; i < 3; i++)
			{
				*pbtBuffer24++ = pbtBG[i];
			}
			break;

		case 0xFF: // Alpha value is 255
			for (int i = 0; i < 3; i++)
			{
				*pbtBuffer24++ = pbtBuffer32[i];
			}
			break;

		default: // Other
			for (int i = 0; i < 3; i++)
			{
				*pbtBuffer24++ = (pbtBuffer32[i] * pbtBuffer32[3] + pbtBG[i] * (255 - pbtBuffer32[3])) / 255;
				// *pbtBuffer24++ = (pbtBuffer32[i] - pbtBG[i]) * pbtBuffer32[3] / 255 + pbtBG[i];
			}
		}

		pbtBuffer32 += 4;
	}
}

/// Output
///
/// @param pvBuffer     Data
/// @param dwBufferSize Data Size
/// @param bProgress    Request progress bar status
///
void CImageBase::Write(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	const BYTE* pbtBuffer = reinterpret_cast<const BYTE*>(pvBuffer);

	// Don't update progressbar (No multiple outputs)
	if (!bProgress)
	{
		m_dwRowSize = 0;
		m_dwRowSizeOfRemainder = 0;
	}

	// Output
	if (IsRequireAlphaBlend())
	{
		// Alpha-blending enabled

		YCMemory<BYTE> clmbtBuffer24(m_lPitch + 2);

		for (long lY = 0; lY < m_lHeight; lY++)
		{
			WriteLineWithAlphaBlend(&clmbtBuffer24[0], pbtBuffer);
			pbtBuffer += GetPitchWithAlpha();
		}
	}
	else
	{
		long lLine = m_lLine;

		if (dwBufferSize >= m_lPitch * m_lHeight)
		{
			// Read dummy data from the buffer

			m_bOutputDummyFromBuffer = TRUE;
			lLine = m_lPitch;
		}

		for (long lY = 0; lY < m_lHeight; lY++)
		{
			WriteLine(pbtBuffer);
			pbtBuffer += lLine;
		}
	}

	WriteFinish();
}

/// Upside-Down Output
///
/// @param pvBuffer     Data
/// @param dwBufferSize Data Size
/// @param bProgress    Request progress bar status
///
void CImageBase::WriteReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	const BYTE* pbtBuffer = reinterpret_cast<const BYTE*>(pvBuffer);

	// Don't update the progressbar (No multiple outputs)
	if (!bProgress)
	{
		m_dwRowSize = 0;
		m_dwRowSizeOfRemainder = 0;
	}

	if (IsRequireAlphaBlend())
	{
		// Alpha-blending is enabled

		// Ensure a buffer for 24-bit

		YCMemory<BYTE> clmbtBuffer24(m_lPitch + 2);
		pbtBuffer += GetPitchWithAlpha() * m_lHeight;

		for (long lY = 0; lY < m_lHeight; lY++)
		{
			pbtBuffer -= GetPitchWithAlpha();
			WriteLineWithAlphaBlend(&clmbtBuffer24[0], pbtBuffer);
		}
	}
	else
	{
		long lLine = m_lLine;

		if (dwBufferSize >= m_lPitch * m_lHeight)
		{
			// Read dummy data from the buffer

			m_bOutputDummyFromBuffer = TRUE;
			lLine = m_lPitch;
		}

		pbtBuffer += lLine * m_lHeight;

		for (long lY = 0; lY < m_lHeight; lY++)
		{
			pbtBuffer -= lLine;
			WriteLine(pbtBuffer);
		}
	}

	WriteFinish();
}

/// Synthesize BGRA
///
/// @param pvDst        Destination
/// @param pvBuffer     Data
/// @param dwBufferSize Data Size
///
BOOL CImageBase::ComposeBGRA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize)
{
	BYTE*       pbtDst = reinterpret_cast<BYTE*>(pvDst);
	const BYTE* pbtBuffer = reinterpret_cast<const BYTE*>(pvBuffer);
	WORD        wBpp = IsRequireAlphaBlend() ? 32 : m_wBpp;

	if (wBpp == 24 || wBpp == 32)
	{
		// Only supports 24-bit and 32-bit
		WORD  wByteCount = (wBpp >> 3);
		DWORD dwColorSize = dwBufferSize / wByteCount;

		// Get pointers to B, G, R and A
		const BYTE* apbtBGRA[4];

		for (WORD i = 0; i < wByteCount; i++)
		{
			apbtBGRA[i] = &pbtBuffer[dwColorSize * i];
		}

		// Synthesize BGRA
		for (DWORD i = 0; i < dwColorSize; i++)
		{
			for (WORD j = 0; j < wByteCount; j++)
			{
				*pbtDst++ = *apbtBGRA[j]++;
			}
		}

		return TRUE;
	}

	return FALSE;
}

/// Synthesize BGRA Output
///
/// @param pvBuffer     Data
/// @param dwBufferSize Data Size
/// @param bProgress    Request progress bar status
///
void CImageBase::WriteCompoBGRA(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	// Allocation of memory for storing data during synthesis
	YCMemory<BYTE> clmbtDst(dwBufferSize);

	// Synthesize
	if (ComposeBGRA(&clmbtDst[0], pvBuffer, dwBufferSize))
	{
		Write(&clmbtDst[0], dwBufferSize, bProgress);
	}
	else // Synthesis Failed
	{
		Write(pvBuffer, dwBufferSize, bProgress);
	}
}

/// Synthesize BGRA Upside-Down
///
/// @param pvBuffer     Data
/// @param dwBufferSize Data Size
/// @param bProgress    Request progress bar status
///
void CImageBase::WriteCompoBGRAReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	// Allocate memory for storage during synthesis
	YCMemory<BYTE> clmbtDst(dwBufferSize);

	// Synthesize
	if (ComposeBGRA(&clmbtDst[0], pvBuffer, dwBufferSize))
	{
		WriteReverse(&clmbtDst[0], dwBufferSize, bProgress);
	}
	else // Synthesis Failed
	{
		WriteReverse(pvBuffer, dwBufferSize, bProgress);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Synthesize RGBA
///
/// @param pvDst        Destination
/// @param pvBuffer     Data
/// @param dwBufferSize Data Size
///
BOOL CImageBase::ComposeRGBA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize)
{
	BYTE*       pbtDst = reinterpret_cast<BYTE*>(pvDst);
	const BYTE* pbtBuffer = reinterpret_cast<const BYTE*>(pvBuffer);
	WORD        wBpp = IsRequireAlphaBlend() ? 32 : m_wBpp;

	if (wBpp == 24 || wBpp == 32)
	{
		// Only 24-bit and 32-bit are supported

		WORD  wByteCount = (wBpp >> 3);
		DWORD dwColorSize = dwBufferSize / wByteCount;

		// Get pointers to R, G, B and A
		const BYTE* apbtBGRA[4];

		for (int i = 0, j = 2; i < 3; i++, j--)
		{
			apbtBGRA[i] = &pbtBuffer[dwColorSize * j];
		}
		if (wByteCount == 4)
		{
			apbtBGRA[3] = &pbtBuffer[dwColorSize * 3];
		}

		// Synthesize BGRA
		for (DWORD i = 0; i < dwColorSize; i++)
		{
			for (WORD j = 0; j < wByteCount; j++)
			{
				*pbtDst++ = *apbtBGRA[j]++;
			}
		}

		return TRUE;
	}

	return FALSE;
}

/// Synthesize RGBA Output
///
/// @param pvBuffer     Data
/// @param dwBufferSize Data size
/// @param bProgress    Synthesize BGRA output
///
void CImageBase::WriteCompoRGBA(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	// Allocation of memory for storage during RGBA synthesis
	YCMemory<BYTE> clmbtDst(dwBufferSize);

	// Synthesize
	if (ComposeRGBA(&clmbtDst[0], pvBuffer, dwBufferSize))
	{
		Write(&clmbtDst[0], dwBufferSize, bProgress);
	}
	else // Synthesis failed
	{
		Write(pvBuffer, dwBufferSize, bProgress);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Synthesize RGBA Upside-Down Output
///
/// @param pvBuffer     Data
/// @param dwBufferSize Data size
/// @param bProgress    Request progress bar status
///
void CImageBase::WriteCompoRGBAReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	// Allocate memory for storage during RGBA synthesis
	YCMemory<BYTE> clmbtDst(dwBufferSize);

	// Synthesize
	if (ComposeRGBA(&clmbtDst[0], pvBuffer, dwBufferSize))
	{
		WriteReverse(&clmbtDst[0], dwBufferSize, bProgress);
	}
	else // Synthesis failed
	{
		WriteReverse(pvBuffer, dwBufferSize, bProgress);
	}
}

/// Finish output
void CImageBase::WriteFinish()
{
	m_pclArc->GetProg()->UpdatePercent(m_dwRowSizeOfRemainder);
	OnWriteFinish();
}

/// Calculate Pitch
///
/// @param lWidth Width
/// @param wBpp   Number of bits
///
long CImageBase::CalculatePitch(long lWidth, WORD wBpp)
{
	return ((lWidth * (wBpp >> 3) + 3) & 0xFFFFFFFC);
}
