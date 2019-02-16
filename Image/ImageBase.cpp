#include "StdAfx.h"
#include "Image/ImageBase.h"

#include "ArcFile.h"

namespace
{
/// Calculate Pitch
///
/// @param width Width
/// @param bpp   Number of bits
///
s32 CalculatePitch(s32 width, u16 bpp)
{
  return (width * (bpp >> 3) + 3) & 0xFFFFFFFC;
}
} // Anonymous namespace

/// Constructor
CImageBase::CImageBase() = default;

/// Destructor
CImageBase::~CImageBase() = default;

/// Initialization
///
/// @param archive     Archive
/// @param width       Width
/// @param height      Height
/// @param bpp         Number of bits
/// @param pallet      Palette
/// @param pallet_size Palette size
/// @param file_name   File name
///
bool CImageBase::Init(
	CArcFile*       archive,
	s32             width,
	s32             height,
	u16             bpp,
	const void*     pallet,
	size_t          pallet_size,
	const YCString& file_name
	)
{
	// Fix top-down, bottom-up images
	if (height < 0)
	{
		height = -height;
	}

	// Number of bits
	switch (bpp)
	{
	case 8: // 8bit
		OnCreatePallet(static_cast<const u8*>(pallet), pallet_size);
		break;

	case 32: // 32bit
		if (archive->GetOpt()->bAlphaBlend)
		{
			// Alpha-blending is enabled
			m_alpha_blend_requirement = true;
			bpp = 24;

			for (int i = 0; i < 3; i++)
			{
				m_bg[i] = static_cast<u8>((archive->GetOpt()->BgRGB >> (i << 3)) & 0xFF);
			}
		}
		break;
	}

	m_bpp = bpp;
	m_archive = archive;
	m_width = width;
	m_height = height;
	m_line = width * (bpp >> 3);
	m_pitch = CalculatePitch(width, bpp);
	m_pitch_with_alpha = width * 4;

	const u32 original_size = archive->GetOpenFileInfo()->size_org;

	m_row_size = original_size / height;
	m_row_size_remainder = original_size % height;

	return OnInit(file_name);
}

/// Set the validity of alpha blending
///
/// @param validity_of_alpha_blend Check validity of alpha-blending
///
void CImageBase::SetValidityOfAlphaBlend(bool validity_of_alpha_blend)
{
	m_validity_of_alpha_blend = validity_of_alpha_blend;
}

/// Get the validity of alpha blending
///
/// @return Check validity of alpha blending
///
bool CImageBase::GetValidityOfAlphaBlend() const
{
	return m_validity_of_alpha_blend;
}

/// Set background color when alpha blending
///
/// @param background_color Background color as RGBA
///
void CImageBase::SetBackColorWhenAlphaBlend(u32 background_color)
{
	m_background_color_when_alpha_blending.color = background_color;
}

/// Compression
///
/// @param dst_path Destination file path
/// @param bmp      Bitmap data
/// @param bmp_size Bitmap data size
///
bool CImageBase::Compress(LPCTSTR dst_path, const void* bmp, size_t bmp_size)
{
	return true;
}

/// Compression
///
/// @param dst_path    Destination file path
/// @param dib         DIB data
/// @param dib_size    DIB data size
/// @param pallet      Palette
/// @param pallet_size Palette size
/// @param pallet_bpp  Number of bits in palette
/// @param width       Width
/// @param height      Height
/// @param bpp         Number of bits
///
bool CImageBase::Compress(
	LPCTSTR     dst_path,
	const void* dib,
	size_t      dib_size,
	const void* pallet,
	size_t      pallet_size,
	u16         pallet_bpp,
	s32         width,
	s32         height,
	u16         bpp
	)
{
/*	const u8* pbtDIB = (const u8*) dib;
	const u8* pbtPallet = (const u8*) pallet;

	// Initialization
	OnInit(width, height, bpp, pallet, pallet_size, pallet_bpp);

	// Output header
	OnWriteHeader();

	// Output data
	if (IsRequireAlphaBlend())
	{
		// Alpha blending is enabled
		s32 dst_size_after_alpha_blend = GetPitch() + 2; // Ensure high-speed processing for alpha
		std::vector<u8> dst_after_alpha_blend(dst_size_after_alpha_blend);

		for (s32 y = 0; y < GetHeight(); y++)
		{
			AlphaBlend(dst_after_alpha_blend.data(), pbtDIB, m_unpBackColorWhenAlphaBlend, width);

			OnWriteLine(dst_after_alpha_blend.data(), GetPitch());

			pbtDIB += GetPitchWithAlpha();
		}
	}
	else
	{
		// Request without alpha blending
		s32 line = m_lLine;

		if (buffer_size >= m_lPitch * m_lHeight)
		{
			// Read dummy data from buffer
			m_bOutputDummyFromBuffer = true;
			line = m_lPitch;
		}

		for (s32 y = 0; y < m_lHeight; y++)
		{
			OnWriteLine(pbtBuffer);
			pbtBuffer += line;
		}
	}

	OnWriteFinish();
*/
	return true;
}

/// Compression
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param bmp      Bitmap data
/// @param bmp_size Bitmap data size
///
bool CImageBase::Compress(void* dst, size_t dst_size, const void* bmp, size_t bmp_size)
{
	return true;
}

/// Compression
///
/// @param dst         Destination
/// @param dst_size    Destination size
/// @param dib         DIB data
/// @param dib_size    DIB data size
/// @param pallet      Palette
/// @param pallet_size Palette size
/// @param pallet_bpp  Number of bits in the palette
/// @param width       Width
/// @param height      Height
/// @param bpp         Number of bits
///
bool CImageBase::Compress(
	void*       dst,
	size_t      dst_size,
	const void* dib,
	size_t      dib_size,
	const void* pallet,
	size_t      pallet_size,
	u16         pallet_bpp,
	s32         width,
	s32         height,
	u16         bpp
	)
{
/*	u8*       pbtDst = (u8*) dst;
	const u8* pbtDIB = (const u8*) dib;
	const u8* pbtPallet = (const u8*) pallet;

	// Initialization
	OnInit(width, height, bpp, pallet, pallet_size, pallet_bpp);

	// Output header
	OnWriteHeader();

	// Output data
	if (IsRequireAlphaBlend())
	{
		// Request alpha-blending
		s32 dst_size_after_alpha_blend = GetPitch() + 2; // Ensure high-speed alpha blending
		std::vector<u8> dst_after_alpha_blend(dst_size_after_alpha_blend);

		for (s32 y = 0 ; y < GetHeight() ; y++)
		{
			AlphaBlend(dst_after_alpha_blend.data(), pbtDIB);

			OnWriteLine( 

			OnWriteLineWithAlphaBlending(&clmbtBuffer24[0], pbtBuffer);
			pbtBuffer += m_lLine32;
		}
	}
	else
	{
		// No Alpha blending
		s32 line = m_lLine;

		if (buffer_size >= m_pitch * m_height)
		{
			// Read dummy data from buffer

			m_output_dummy_from_buffer = true;
			line = m_pitch;
		}

		for (s32 y = 0; y < m_height; y++)
		{
			OnWriteLine(pbtBuffer);
			pbtBuffer += line;
		}
	}

	OnWriteFinish();*/

	return true;
}

/// Receive alpha-blending requests
///
/// @return Alpha-blend request
///
/// @remark If true, then alpha-blending is enabled
///
bool CImageBase::IsRequireAlphaBlend() const
{
	return m_alpha_blend_requirement;
}

/// Alpha Blending
///
/// Parameters:
/// @param buffer24 Storage location
/// @param buffer32 32-bit Data
///
void CImageBase::AlphaBlend(u8* buffer24, const u8* buffer32)
{
	for (s32 x = 0; x < m_width; x++)
	{
		switch (buffer32[3])
		{
		case 0x00: // Alpha value is 0
			for (size_t i = 0; i < 3; i++)
			{
				*buffer24++ = m_bg[i];
			}
			break;

		case 0xFF: // Alpha value is 255
			for (size_t i = 0; i < 3; i++)
			{
				*buffer24++ = buffer32[i];
			}
			break;

		default: // Other
			for (size_t i = 0; i < 3; i++)
			{
				*buffer24++ = (buffer32[i] * buffer32[3] + m_bg[i] * (255 - buffer32[3])) / 255;
				// *pbtBuffer24++ = (pbtBuffer32[i] - pbtBG[i]) * pbtBuffer32[3] / 255 + pbtBG[i];
			}
			break;
		}

		buffer32 += 4;
	}
}

/// Output
///
/// @param buffer      Data
/// @param buffer_size Data Size
/// @param progress    Request progress bar status
///
void CImageBase::Write(const void* buffer, size_t buffer_size, bool progress)
{
	const u8* pbtBuffer = reinterpret_cast<const u8*>(buffer);

	// Don't update progressbar (No multiple outputs)
	if (!progress)
	{
		m_row_size = 0;
		m_row_size_remainder = 0;
	}

	// Output
	if (IsRequireAlphaBlend())
	{
		// Alpha-blending enabled
		std::vector<u8> buffer24(m_pitch + 2);

		for (s32 y = 0; y < m_height; y++)
		{
			WriteLineWithAlphaBlend(buffer24.data(), pbtBuffer);
			pbtBuffer += m_pitch_with_alpha;
		}
	}
	else
	{
		s32 line = m_line;

		if (buffer_size >= m_pitch * m_height)
		{
			// Read dummy data from the buffer
			m_output_dummy_from_buffer = true;
			line = m_pitch;
		}

		for (s32 y = 0; y < m_height; y++)
		{
			WriteLine(pbtBuffer);
			pbtBuffer += line;
		}
	}

	WriteFinish();
}

/// Upside-Down Output
///
/// @param buffer      Data
/// @param buffer_size Data Size
/// @param progress    Request progress bar status
///
void CImageBase::WriteReverse(const void* buffer, size_t buffer_size, bool progress)
{
	const u8* pbtBuffer = reinterpret_cast<const u8*>(buffer);

	// Don't update the progressbar (No multiple outputs)
	if (!progress)
	{
		m_row_size = 0;
		m_row_size_remainder = 0;
	}

	if (IsRequireAlphaBlend())
	{
		// Alpha-blending is enabled
		std::vector<u8> buffer24(m_pitch + 2);
		pbtBuffer += m_pitch_with_alpha * m_height;

		for (s32 y = 0; y < m_height; y++)
		{
			pbtBuffer -= m_pitch_with_alpha;
			WriteLineWithAlphaBlend(buffer24.data(), pbtBuffer);
		}
	}
	else
	{
		s32 line = m_line;

		if (buffer_size >= m_pitch * m_height)
		{
			// Read dummy data from the buffer

			m_output_dummy_from_buffer = true;
			line = m_pitch;
		}

		pbtBuffer += line * m_height;

		for (s32 y = 0; y < m_height; y++)
		{
			pbtBuffer -= line;
			WriteLine(pbtBuffer);
		}
	}

	WriteFinish();
}

/// Synthesize BGRA
///
/// @param dst         Destination
/// @param buffer      Data
/// @param buffer_size Data Size
///
bool CImageBase::ComposeBGRA(void* dst, const void* buffer, size_t buffer_size)
{
	u8*       pbtDst = reinterpret_cast<u8*>(dst);
	const u8* pbtBuffer = reinterpret_cast<const u8*>(buffer);
	const u16 bpp = IsRequireAlphaBlend() ? 32 : m_bpp;

	if (bpp == 24 || bpp == 32)
	{
		// Only supports 24-bit and 32-bit
		const u16 byte_count = bpp >> 3;
		const size_t color_size = buffer_size / byte_count;

		// Get pointers to B, G, R and A
		std::array<const u8*, 4> bgra;

		for (size_t i = 0; i < byte_count; i++)
		{
			bgra[i] = &pbtBuffer[color_size * i];
		}

		// Synthesize BGRA
		for (size_t i = 0; i < color_size; i++)
		{
			for (size_t j = 0; j < byte_count; j++)
			{
				*pbtDst++ = *bgra[j]++;
			}
		}

		return true;
	}

	return false;
}

/// Synthesize BGRA Output
///
/// @param buffer      Data
/// @param buffer_size Data Size
/// @param progress    Request progress bar status
///
void CImageBase::WriteCompoBGRA(const void* buffer, size_t buffer_size, bool progress)
{
	// Allocation of memory for storing data during synthesis
	std::vector<u8> dst(buffer_size);

	// Synthesize
	if (ComposeBGRA(dst.data(), buffer, buffer_size))
	{
		Write(dst.data(), buffer_size, progress);
	}
	else // Synthesis Failed
	{
		Write(buffer, buffer_size, progress);
	}
}

/// Synthesize BGRA Upside-Down
///
/// @param buffer      Data
/// @param buffer_size Data Size
/// @param progress    Request progress bar status
///
void CImageBase::WriteCompoBGRAReverse(const void* buffer, size_t buffer_size, bool progress)
{
	// Allocate memory for storage during synthesis
	std::vector<u8> dst(buffer_size);

	// Synthesize
	if (ComposeBGRA(dst.data(), buffer, buffer_size))
	{
		WriteReverse(dst.data(), buffer_size, progress);
	}
	else // Synthesis Failed
	{
		WriteReverse(buffer, buffer_size, progress);
	}
}

/// Synthesize RGBA
///
/// @param dst         Destination
/// @param buffer      Data
/// @param buffer_size Data Size
///
bool CImageBase::ComposeRGBA(void* dst, const void* buffer, size_t buffer_size)
{
	u8*       pbtDst = reinterpret_cast<u8*>(dst);
	const u8* pbtBuffer = reinterpret_cast<const u8*>(buffer);
	const u16 bpp = IsRequireAlphaBlend() ? 32 : m_bpp;

	if (bpp == 24 || bpp == 32)
	{
		// Only 24-bit and 32-bit are supported

		const u16 byte_count = bpp >> 3;
		const size_t color_size = buffer_size / byte_count;

		// Get pointers to R, G, B and A
		std::array<const u8*, 4> bgra;

		for (size_t i = 0, j = 2; i < 3; i++, j--)
		{
			bgra[i] = &pbtBuffer[color_size * j];
		}
		if (byte_count == 4)
		{
			bgra[3] = &pbtBuffer[color_size * 3];
		}

		// Synthesize BGRA
		for (size_t i = 0; i < color_size; i++)
		{
			for (size_t j = 0; j < byte_count; j++)
			{
				*pbtDst++ = *bgra[j]++;
			}
		}

		return true;
	}

	return false;
}

/// Synthesize RGBA Output
///
/// @param buffer      Data
/// @param buffer_size Data size
/// @param progress    Synthesize BGRA output
///
void CImageBase::WriteCompoRGBA(const void* buffer, size_t buffer_size, bool progress)
{
	// Allocation of memory for storage during RGBA synthesis
	std::vector<u8> dst(buffer_size);

	// Synthesize
	if (ComposeRGBA(dst.data(), buffer, buffer_size))
	{
		Write(dst.data(), buffer_size, progress);
	}
	else // Synthesis failed
	{
		Write(buffer, buffer_size, progress);
	}
}

/// Synthesize RGBA Upside-Down Output
///
/// @param buffer      Data
/// @param buffer_size Data size
/// @param progress    Request progress bar status
///
void CImageBase::WriteCompoRGBAReverse(const void* buffer, size_t buffer_size, bool progress)
{
	// Allocate memory for storage during RGBA synthesis
	std::vector<u8> dst(buffer_size);

	// Synthesize
	if (ComposeRGBA(dst.data(), buffer, buffer_size))
	{
		WriteReverse(dst.data(), buffer_size, progress);
	}
	else // Synthesis failed
	{
		WriteReverse(buffer, buffer_size, progress);
	}
}

/// Finish output
void CImageBase::WriteFinish()
{
	m_archive->GetProg()->UpdatePercent(m_row_size_remainder);
	OnWriteFinish();
}
