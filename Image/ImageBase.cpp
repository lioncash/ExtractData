#include "stdafx.h"
#include "ImageBase.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

CImageBase::CImageBase()
{
	m_bAlphaBlendRequirement = FALSE;
	m_bOutputDummyFromBuffer = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destructor

CImageBase::~CImageBase()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization
/*
BOOL CImageBase::OnInit(
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp,							// Number of bits
	const void*			pvPallet,						// Palette
	DWORD				dwPalletSize,					// Palette Size
	WORD				wPalletBpp						// Number of bits on palette
	)
{
	// Initialize member variables
	m_bAlphaBlendRequirement = FALSE;
	m_wBppWithAlpha = 0;
	m_lLineWithAlpha = 0;
	m_lPitchWithAlpha = 0;

	// Fix top-down, bottom-up
	if( lHeight < 0 )
	{
		lHeight = -lHeight;
	}

	// Number of bits processed
	switch( wBpp )
	{
	case 8: // 8bit
		OnCreatePallet( pvPallet, dwPalletSize );
		break;

	case 32: // 32bit
		if( GetValidityOfAlphaBlend() )
		{
			// Alpha-blending is enabled

			m_bAlphaBlendRequirement = TRUE;
			m_wBppWithAlpha = wBpp;
			m_lLineWithAlpha = (lWidth * (wBpp >> 3));
			m_lPitchWithAlpha = CalculatePitch( lWidth, wBpp );

			wBpp = 24;
		}
		break;
	}

	// Set parameters
	m_wBpp = wBpp;
	m_lWidth = lWidth;
	m_lHeight = lHeight;
	m_lLine = lWidth * (wBpp >> 3);
	m_lPitch = CalculatePitch( lWidth, wBpp );

	DWORD dwOriginSize = pclArc->GetOpenFileInfo()->sizeOrg;

	m_dwRowSize = dwOriginSize / lHeight;
	m_dwRowSizeOfRemainder = dwOriginSize % lHeight;

	return TRUE;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization

BOOL CImageBase::Init(
	CArcFile*       pclArc,       // Archive
	long            lWidth,       // Width
	long            lHeight,      // Height
	WORD            wBpp,         // Number of bits
	const void*     pvPallet,     // Palette
	DWORD           dwPalletSize, // Palette Size
	const YCString& rfclsFileName // Filename
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

//////////////////////////////////////////////////////////////////////////////////////////
// Set the validity of alpha blending
//
// Parameters:
//   - bValidityOfAlphaBlend - Check validity of alpha-blending

void CImageBase::SetValidityOfAlphaBlend(BOOL bValidityOfAlphaBlend)
{
	m_bValidityOfAlphaBlend = bValidityOfAlphaBlend;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get the validity of alpha blending
//
// return Check validity of alpha blending

BOOL CImageBase::GetValidityOfAlphaBlend()
{
	return m_bValidityOfAlphaBlend;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Set background color when alpha blending
//
// Parameters:
//   - crBackColor - Background color

void CImageBase::SetBackColorWhenAlphaBlend(COLORREF crBackColor)
{
	m_unpBackColorWhenAlphaBlend.crPixel = crBackColor;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get width

long CImageBase::GetWidth()
{
	return m_lWidth;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get height

long CImageBase::GetHeight()
{
	return m_lHeight;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get number of bits

WORD CImageBase::GetBitCount()
{
	return m_wBpp;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get the number of alpha bits

WORD CImageBase::GetBitCountWithAlpha()
{
	return m_wBppWithAlpha;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get Pitch

long CImageBase::GetPitch()
{
	return m_lPitch;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get the pitch with alpha

long CImageBase::GetPitchWithAlpha()
{
	return m_lPitchWithAlpha;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Compression
//
// Parameters:
//   - pszPathToDst - Destination file path
//   - pvBMP        - Bitmap data
//   - dwBMPSize    - Bitmap data size

BOOL CImageBase::Compress(LPCTSTR pszPathToDst, const void* pvBMP, DWORD dwBMPSize)
{

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Compression

BOOL CImageBase::Compress(
	LPCTSTR     pszPathToDst, // Destination file path
	const void* pvDIB,        // DIB Data
	DWORD       dwDIBSize,    // DIB Data Size
	const void* pvPallet,     // Palette
	DWORD       dwPalletSize, // Palette Size
	WORD        wPalletBpp,   // Number of palette bits
	long        lWidth,       // Width
	long        lHeight,      // Height
	WORD        wBpp          // Number of bits
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

//////////////////////////////////////////////////////////////////////////////////////////
// Compression
//
// Parameters:
//   - pvDst     - Destination
//   - dwDstSize - Destination size
//   - pvBMP     - Bitmap data
//   - dwBMPSize - Bitmap data size

BOOL CImageBase::Compress(void* pvDst, DWORD dwDstSize, const void* pvBMP, DWORD dwBMPSize)
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Compression

BOOL CImageBase::Compress(
	void*       pvDst,        // Destination
	DWORD       dwDstSize,    // Destination Size
	const void* pvDIB,        // DIB Data
	DWORD       dwDIBSize,    // DIB Data Size
	const void* pvPallet,     // Palette
	DWORD       dwPalletSize, // Palette Size
	WORD        wPalletBpp,   // Number of palette bits
	long        lWidth,       // Width
	long        lHeight,      // Height
	WORD        wBpp          // Number of bits
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





//////////////////////////////////////////////////////////////////////////////////////////
// Receive alpha-blending requests
//
// return Alpha-blend request
//
// Remark: If TRUE, then alpha-blending is enabled

BOOL CImageBase::IsRequireAlphaBlend()
{
	return m_bAlphaBlendRequirement;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Alpha Blending
//
// Parameters:
//   - pvBuffer24 - Storage location
//   - pvBuffer32 - 32-bit Data

void CImageBase::AlphaBlend(void* pvBuffer24, const void* pvBuffer32)
{
	BYTE*       pbtBuffer24 = (BYTE*)pvBuffer24;
	const BYTE* pbtBuffer32 = (const BYTE*)pvBuffer32;
	const BYTE* pbtBG = m_abtBG;
	long        lWidth = m_lWidth;

	if (m_pclArc->GetOpt()->bFastAlphaBlend)
	{
		// High-speed alpha blending processing requests

		// eax  Counter
		// edi  Destination
		// esi  Input Source
		//
		// mm0  Input Data 1
		// mm1  Input Data 2
		// mm2  Alpha value of input data 1
		// mm3  Alpha value of input data 2
		// mm4  Work
		// mm5  00FF00FF 00FF00FF
		// mm6  Background color
		// mm7  Do unpacking (00000000 00000000)

		_asm
		{
			mov         edi, pvBuffer24
			mov         esi, pvBuffer32

			// Initializing for unpacking
			pxor        mm7, mm7

			// Get background color
			mov         eax, pbtBG
			movd        mm6, [eax]
			punpcklbw   mm6, mm7

			// 

			mov         eax, 0x01000100
			movd        mm5, eax
			punpckldq   mm5, mm5

			// Main loop
			mov         eax, lWidth
			shr         eax, 1         // 2 pixels to be processed

		loop2000:

			// Get 8 bytes (2 pixels)
			movq        mm0, [esi]
			movq        mm1, mm0

			add         esi, 8

			// Get alpha value
			/*
			movq        mm2, mm0
			psrld       mm2, 24

			movq        mm3, mm2

			punpcklwd   mm2, mm2
			punpckldq   mm2, mm2

			punpckhwd   mm3, mm3
			punpckldq   mm3, mm3
			*/
			//-- Input Data 1 -------------------------------------------------------------------------

			// Unpack
			punpcklbw   mm0, mm7

			// Get alpha value
			movq        mm2, mm0
			psrlq       mm2, 48

			movd        edx, mm2
			test        edx, edx
			jnz         pass3200        // Alpha value is not 0

			// Alpha value is 0

			movq        mm0, mm6
			jmp         pass3800

			// Alpha value is 1~255
		pass3200 :

			cmp         edx, 0xFF
			jne         pass3400        // Alpha value is not 255

			// Alpha value is 255
			jmp         pass3800

			// Alpha value is 1~254
		pass3400 :

			punpcklwd   mm2, mm2
			punpckldq   mm2, mm2

			// RGBÅ~A
			pmullw      mm0, mm2

			// Background x (0xFF - A)
			movq        mm4, mm5
			psubw       mm4, mm2
			pmullw      mm4, mm6

			// 256 divided by sum
			paddw       mm0, mm4
			psrlw       mm0, 8

			pass3800:

			// Summarized in lower 32bit
			packuswb    mm0, mm7

			// Store
			movd[edi], mm0

			//-- Input data 2 -------------------------------------------------------------------------

			// Unpack 
			punpckhbw   mm1, mm7

			// Get alpha value
			movq        mm3, mm1
			psrlq       mm3, 48

			movd        edx, mm3
			test        edx, edx
			jnz         pass4200        // Alpha value is not 0

			// Alpha value is 0
			movq        mm1, mm6
			jmp         pass4800

				// Alpha value is 1~255
			pass4200 :

			cmp         edx, 0xFF
			jne         pass4400        // Alpha value is not 255

			// Alpha value is 255
			jmp         pass4800

		pass4400:

			punpcklwd   mm3, mm3
			punpckldq   mm3, mm3

			// RGB x A
			pmullw      mm1, mm3

			// Background x (0xFF - A)
			movq        mm4, mm5
			psubw       mm4, mm3
			pmullw      mm4, mm6

			// 256 divided by the sum
			paddw       mm1, mm4
			psrlw       mm1, 8

			pass4800:

			// Summarized in the lower 32bit
			packuswb    mm1, mm7

			// Store
			movd[edi + 3], mm1
			//movntq      [edi], mm1
			add         edi, 6

			//----------------------------------------------------------------------------------------

			// Advance to next pixel
			dec         eax
			jnz         loop2000        // Continuity

			// One pixel remaining in processing
			mov         eax, lWidth
			and         eax, 1
			jz          pass9900        // No outstanding pixels

			// Get 4 bytes (1 Pixel)
			movd        mm0, [esi]

			// Unpack
			punpcklbw   mm0, mm7

			// Get alpha value
			movq        mm2, mm0
			psrlq       mm2, 48
			punpcklwd   mm2, mm2
			punpckldq   mm2, mm2

			// RGB x A
			pmullw      mm0, mm2

			// Background x (0xFF - A)
			movq        mm4, mm5
			psubw       mm4, mm2
			pmullw      mm4, mm6

			// 256 divided by the sum
			paddw       mm0, mm4
			psrlw       mm0, 8

			// Summarized in the lower 32 bits
			packuswb    mm0, mm7

			// Store
			movd[edi], mm0

			// Exit
		pass9900:

			emms
		}
	}
	else
	{
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
					//					*pbtBuffer24++ = (pbtBuffer32[i] - pbtBG[i]) * pbtBuffer32[3] / 255 + pbtBG[i];
				}
			}

			pbtBuffer32 += 4;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Output
//
// Parameters:
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size
//   - bProgress    - Request progress bar status

void CImageBase::Write(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	const BYTE* pbtBuffer = (const BYTE*)pvBuffer;

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

//////////////////////////////////////////////////////////////////////////////////////////
// Upside-Down Output
//
// Parameters:
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size
//   - bProgress    - Request progress bar status

void CImageBase::WriteReverse(const void* pvBuffer, DWORD dwBufferSize, BOOL bProgress)
{
	const BYTE* pbtBuffer = (const BYTE*)pvBuffer;

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

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize BGRA
//
// Parameters:
//   - pvDst        - Destination
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size

BOOL CImageBase::ComposeBGRA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize)
{
	BYTE*       pbtDst = (BYTE*)pvDst;
	const BYTE* pbtBuffer = (const BYTE*)pvBuffer;
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

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize BGRA Output
//
// Parameters:
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size
//   - bProgress    - Request progress bar status

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

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize BGRA Upside-Down
//
// Parameters:
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size
//   - bProgress    - Request progress bar status

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
// Synthesize RGBA
//
// Parameters:
//   - pvDst        - Destination
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size

BOOL CImageBase::ComposeRGBA(void* pvDst, const void* pvBuffer, DWORD dwBufferSize)
{
	BYTE*       pbtDst = (BYTE*)pvDst;
	const BYTE* pbtBuffer = (const BYTE*)pvBuffer;
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

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesize RGBA Output
//
// Parameters:
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size
//   - bProgress    - Synthesize BGRA output

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
// Synthesize RGBA Upside-Down Output
//
// Parameters:
//   - pvBuffer     - Data
//   - dwBufferSize - Data Size
//   - bProgress    - Request progress bar status

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

//////////////////////////////////////////////////////////////////////////////////////////
// Exit Output

void CImageBase::WriteFinish()
{
	m_pclArc->GetProg()->UpdatePercent(m_dwRowSizeOfRemainder);
	OnWriteFinish();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Calculate Pitch
//
// Parameters:
//   - lWidth - Width
//   - wBpp   - Number of bits

long CImageBase::CalculatePitch(long lWidth, WORD wBpp)
{
	return ((lWidth * (wBpp >> 3) + 3) & 0xFFFFFFFC);
}
