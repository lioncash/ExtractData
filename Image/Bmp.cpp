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
	m_bmp_file_header.bfSize = 54 + m_lPitch * m_lHeight;
	m_bmp_file_header.bfReserved1 = 0;
	m_bmp_file_header.bfReserved2 = 0;
	m_bmp_file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// 8bit
	if (m_wBpp == 8)
	{
		m_bmp_file_header.bfSize += 1024;
		m_bmp_file_header.bfOffBits += 1024;
	}

	// Set info header
	ZeroMemory(&m_bmp_info_header, sizeof(BITMAPINFOHEADER));
	m_bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
	m_bmp_info_header.biWidth = m_lWidth;
	m_bmp_info_header.biHeight = m_lHeight;
	m_bmp_info_header.biPlanes = 1;
	m_bmp_info_header.biBitCount = m_wBpp;
	m_bmp_info_header.biCompression = BI_RGB;
	m_bmp_info_header.biSizeImage = 0;
	m_bmp_info_header.biXPelsPerMeter = 0;
	m_bmp_info_header.biYPelsPerMeter = 0;
	m_bmp_info_header.biClrUsed = (m_wBpp == 8) ? 256 : 0;
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
	m_pclArc->OpenFile(file_name);

	// Output BMP header
	m_pclArc->WriteFile(&m_bmp_file_header, sizeof(m_bmp_file_header), 0);
	m_pclArc->WriteFile(&m_bmp_info_header, sizeof(m_bmp_info_header), 0);

	// Output palette (8-bit)
	if (m_bmp_info_header.biBitCount == 8)
	{
		m_pclArc->WriteFile(m_pallet, sizeof(m_pallet), 0);
	}
}

/// Create Palette
///
/// @param src_pallet      Source (referenced) palette
/// @param src_pallet_size Size Source (reference) palette size
///
bool CBmp::OnCreatePallet(const void* src_pallet, DWORD src_pallet_size)
{
	const BYTE* byte_src_pallet = reinterpret_cast<const BYTE*>(src_pallet);

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
	m_pclArc->WriteFile(buffer, m_lLine, m_dwRowSize);

	// Output dummy data
	if (m_bOutputDummyFromBuffer)
	{
		// Output from buffer

		const BYTE* byte_buffer = reinterpret_cast<const BYTE*>(buffer);

		m_pclArc->WriteFile(&byte_buffer[m_lLine], (m_lPitch - m_lLine), 0);
	}
	else
	{
		for (long i = m_lLine; i < m_lPitch; i++)
		{
			m_pclArc->WriteFile("\0", 1, 0);
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
	ZeroMemory(buffer24, m_lPitch);

	// Alpha blending
	AlphaBlend(buffer24, buffer32);

	// Output
	m_pclArc->WriteFile(buffer24, m_lPitch, m_dwRowSize);
}
