#include "StdAfx.h"
#include "../ExtractBase.h"
#include "Bmp.h"

/// Mount
///
/// @param archive Archive
///
bool CBmp::Mount(CArcFile* archive)
{
	if (lstrcmpi(archive->GetArcExten(), _T(".bmp")) != 0)
		return false;

	return archive->Mount();
}

/// Initialization
///
/// @param file_name File name
///
bool CBmp::OnInit(const YCString& file_name)
{
	// Set file header
	ZeroMemory(&m_bmp_file_header, sizeof(BITMAPFILEHEADER));
	m_bmp_file_header.bfType = 'MB';
	m_bmp_file_header.bfSize = 54 + m_pitch * m_height;
	m_bmp_file_header.bfReserved1 = 0;
	m_bmp_file_header.bfReserved2 = 0;
	m_bmp_file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// 8bit
	if (m_bpp == 8)
	{
		m_bmp_file_header.bfSize += 1024;
		m_bmp_file_header.bfOffBits += 1024;
	}

	// Set info header
	ZeroMemory(&m_bmp_info_header, sizeof(BITMAPINFOHEADER));
	m_bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
	m_bmp_info_header.biWidth = m_width;
	m_bmp_info_header.biHeight = m_height;
	m_bmp_info_header.biPlanes = 1;
	m_bmp_info_header.biBitCount = m_bpp;
	m_bmp_info_header.biCompression = BI_RGB;
	m_bmp_info_header.biSizeImage = 0;
	m_bmp_info_header.biXPelsPerMeter = 0;
	m_bmp_info_header.biYPelsPerMeter = 0;
	m_bmp_info_header.biClrUsed = (m_bpp == 8) ? 256 : 0;
	m_bmp_info_header.biClrImportant = 0;

	// Output BMP header
	WriteHed(file_name);

	return true;
}

/// Output BMP Header
///
/// @param file_name Filename
///
void CBmp::WriteHed(const YCString& file_name)
{
	m_archive->OpenFile(file_name);

	// Output BMP header
	m_archive->WriteFile(&m_bmp_file_header, sizeof(m_bmp_file_header), 0);
	m_archive->WriteFile(&m_bmp_info_header, sizeof(m_bmp_info_header), 0);

	// Output palette (8-bit)
	if (m_bmp_info_header.biBitCount == 8)
	{
		m_archive->WriteFile(m_pallet, sizeof(m_pallet), 0);
	}
}

/// Create Palette
///
/// @param src_pallet      Source (referenced) palette
/// @param src_pallet_size Size Source (reference) palette size
///
bool CBmp::OnCreatePallet(const void* src_pallet, size_t src_pallet_size)
{
	const u8* byte_src_pallet = reinterpret_cast<const u8*>(src_pallet);

	ZeroMemory(m_pallet, sizeof(m_pallet));

	if (byte_src_pallet == nullptr)
	{
		// Use default palette (Grayscale)
		for (size_t i = 0; i < 256; i++)
		{
			m_pallet[i].rgbBlue = static_cast<u8>(i);
			m_pallet[i].rgbGreen = static_cast<u8>(i);
			m_pallet[i].rgbRed = static_cast<u8>(i);
			m_pallet[i].rgbReserved = 0;
		}
	}
	else
	{
		// Source palette

		if (src_pallet_size == 1024)
		{
			// 1024 byte palette
			for (size_t i = 0; i < 256; i++)
			{
				m_pallet[i].rgbBlue = *byte_src_pallet++;
				m_pallet[i].rgbGreen = *byte_src_pallet++;
				m_pallet[i].rgbRed = *byte_src_pallet++;
				m_pallet[i].rgbReserved = *byte_src_pallet++;
			}
		}
		else if (src_pallet_size == 768)
		{
			// 768 byte palette (No alpha)
			for (size_t i = 0; i < 256; i++)
			{
				m_pallet[i].rgbBlue = *byte_src_pallet++;
				m_pallet[i].rgbGreen = *byte_src_pallet++;
				m_pallet[i].rgbRed = *byte_src_pallet++;
				m_pallet[i].rgbReserved = 0;
			}
		}
		else
		{
			// Other palette sizes
			for (size_t i = 0; i < src_pallet_size / 4; i++)
			{
				m_pallet[i].rgbBlue = *byte_src_pallet++;
				m_pallet[i].rgbGreen = *byte_src_pallet++;
				m_pallet[i].rgbRed = *byte_src_pallet++;
				m_pallet[i].rgbReserved = *byte_src_pallet++;
			}
		}
	}

	return true;
}

/// Write 1 Lin
void CBmp::WriteLine(const void* buffer)
{
	// Output
	m_archive->WriteFile(buffer, m_line, m_row_size);

	// Output dummy data
	if (m_output_dummy_from_buffer)
	{
		// Output from buffer

		const u8* byte_buffer = reinterpret_cast<const u8*>(buffer);

		m_archive->WriteFile(&byte_buffer[m_line], m_pitch - m_line, 0);
	}
	else
	{
		for (long i = m_line; i < m_pitch; i++)
		{
			m_archive->WriteFile("\0", 1, 0);
		}
	}
}

/// Write a line with alpha blending
///
/// @param buffer24 Data storage destination alpha blending
/// @param buffer32 32-bit Data
///
void CBmp::WriteLineWithAlphaBlend(void* buffer24, const void* buffer32)
{
	// Fill with 0's (So that the dummy data can be outputted)
	ZeroMemory(buffer24, m_pitch);

	// Alpha blending
	AlphaBlend(buffer24, buffer32);

	// Output
	m_archive->WriteFile(buffer24, m_pitch, m_row_size);
}
