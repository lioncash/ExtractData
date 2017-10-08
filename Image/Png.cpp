#include "StdAfx.h"
#include "Png.h"

#include "ArcFile.h"

namespace
{
struct SMemory
{
	u8* data;
	u32 data_ptr;
};

void PNGAPI WritePNG(png_structp png_ptr, png_bytep data, png_size_t length)
{
	auto* archive = static_cast<CArcFile*>(png_get_io_ptr(png_ptr));

	archive->WriteFile(data, length, 0);
}

/// File Output
void PNGAPI WritePNGToFile(png_structp png, png_bytep data, png_size_t length)
{
	auto* dst = static_cast<YCFile*>(png_get_io_ptr(png));

	dst->Write(data, length);
}

/// Memory Output
void PNGAPI WritePNGToMemory(png_structp png, png_bytep data, png_size_t length)
{
	auto* dst = static_cast<SMemory*>(png_get_io_ptr(png));

	memcpy(&dst->data[dst->data_ptr], data, length);

	dst->data_ptr += length;
}
} // Anonymous namespace

CPng::CPng()
{
}

CPng::~CPng()
{
	Close();
}

/// Set compression rate
void CPng::SetCompressLevel(int compression_level)
{
	m_compression_level = compression_level;
}

/// Get compression rate
///
/// @return Compression Level
///
int CPng::GetCompressLevel() const
{
	return m_compression_level;
}

/// Set callback function
void CPng::SetCallback()
{
}

/// Release PNG structure
void CPng::Close()
{
	if (m_png == nullptr)
		return;

	switch (m_mode)
	{
	case Mode::Read: // Read mode
		/*
		if (m_png_info != nullptr)
		{
			png_read_end(m_png, m_png_info);
			png_destroy_read_struct(&m_png, &m_png_info, nullptr);
		}
		else
		{
			png_destroy_read_struct(&m_png, nullptr, nullptr);
		}
		*/
		break;

	case Mode::Write: // Write mode
		if (m_png_info != nullptr)
		{
			png_write_end(m_png, m_png_info);
			png_destroy_write_struct(&m_png, &m_png_info);
		}
		else
		{
			png_destroy_write_struct(&m_png, nullptr);
		}

		break;
	}

	m_png = nullptr;
	m_png_info = nullptr;
}

/// Convert from BMP to PNG
///
/// @param dst_path Destination file path
/// @param bmp      Bitmap data
/// @param bmp_size Bitmap data size

bool CPng::Compress(LPCTSTR dst_path, const void* bmp, size_t bmp_size)
{
	return true;
}

/// Convert from DIB to PNG
///
/// @param dst_path    Destination file path
/// @param dib         DIB data
/// @param dib_size    DIB data size
/// @param pallet      Palette
/// @param pallet_size Palette size
/// @param pallet_bpp  Number of bits in the palette
/// @param width       Width
/// @param height      Height
/// @param bpp         Number of bits
///
bool CPng::Compress(
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
	return true;
}

/// Convert from BMP to PNG
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param bmp      Bitmap data
/// @param bmp_size Bitmap data size
///
bool CPng::Compress(void* dst, size_t dst_size, const void* bmp, size_t bmp_size)
{
	return true;
}

/// Converted from DIB to PNG
///
/// @param dst         Destination
/// @param dib         DIB data
/// @param dib_size    DIB data size
/// @param pallet      Palette
/// @param pallet_size Palette size
/// @param pallet_bpp  Number of bits in the palette
/// @param width       Width
/// @param height      Height
/// @param bpp         Number of bits
///
bool CPng::Compress(
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
/*	u8*       pbtDst = (u8*) pvDst;
	const u8* pbtDIB = (const u8*) pvDIB;
	const u8* pbtPallet = (const u8*) pvPallet;

	// Set Mode
	SetMode(modeWrite);

	// Get the color type
	int color_type;

	switch (wBpp)
	{
	case 8: // 8bit
		color_type = PNG_COLOR_TYPE_PALETTE;
		break;

	case 24: // 24bit
		color_type = PNG_COLOR_TYPE_RGB;
		break;

	case 32: // 32bit
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		break;

	default: // Other
		return false;
	}

	// Create PNG structure
	m_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (m_png == nullptr)
		return false;

	// Create PNG structure information
	m_png_info = png_create_info_struct(m_png);
	if (m_png_info == nullptr)
		return false;

	// Set parameters
	SMemory mem_dst;
	mem_dst.data = pbtDst;
	mem_dst.data_ptr = 0;

	png_set_write_fn(m_png, &mem_dst, WritePNGToMemory, nullptr);

	png_set_bgr(m_png);
	png_set_compression_level(m_png, GetCompressLevel());

	if (bpp == 8)
	{
		png_set_PLTE(m_png, m_png_info, m_pallet, 256);
	}

	png_set_IHDR(m_png, m_png_info, width, height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// PNG output header
	png_write_info(m_png, m_png_info);

	// PNG output data
	if (IsRequireAlphaBlend())
	{
		// Alpha-blend on request
		const s32 pitch = CalculatePitch(width, bpp);
		std::vector<u8> alpha_blend_buffer(pitch);

		for (s32 y = 0; y < height; y++)
		{
			AlphaBlend(alpha_blend_buffer.data(), &pbtDIB[y * pitch]);

			png_write_row(m_png, alpha_blend_buffer.data());
		}
	}
	else // No alpha-blending
	{
		const s32 pitch = CalculatePitch(width, bpp);

		for (s32 y = 0; y < height; y++)
		{
			png_write_row(m_png, &pbtDIB[y * pitch]);
		}
	}
*/
	return true;
}

/// Convert to DIB
bool CPng::Decompress()
{
	return true;
}

/// Initialization
bool CPng::OnInit(const YCString& file_name)
{
	int color_type;

	switch (m_bpp)
	{
	case 8: // 8bit
		color_type = PNG_COLOR_TYPE_PALETTE;
		break;

	case 24: // 24bit
		color_type = PNG_COLOR_TYPE_RGB;
		break;

	case 32: // 32bit
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		break;

	default: // Other
		return false;
	}

	m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (m_png_ptr == nullptr)
		return false;

	m_info_ptr = png_create_info_struct(m_png_ptr);
	if (m_info_ptr == nullptr)
	{
		png_destroy_write_struct(&m_png_ptr, nullptr);
		return false;
	}

	png_set_write_fn(m_png_ptr, m_archive, WritePNG, nullptr);

	png_set_bgr(m_png_ptr);
	png_set_compression_level(m_png_ptr, m_archive->GetOpt()->CmplvPng);

	if (m_bpp == 8)
	{
		png_set_PLTE(m_png_ptr, m_info_ptr, m_pallet.data(), static_cast<int>(m_pallet.size()));
	}

	png_set_IHDR(m_png_ptr, m_info_ptr, m_width, m_height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Open the output file
	m_archive->OpenFile(file_name);

	// Output header
	png_write_info(m_png_ptr, m_info_ptr);

	return true;
}

/// Create Palette
///
/// @param src_pallet      Reference palette
/// @param src_pallet_size Reference palette size
///
bool CPng::OnCreatePallet(const u8* src_pallet, size_t src_pallet_size)
{
	m_pallet = {};

	// Use the default palette (Grayscale)
	if (src_pallet == nullptr)
	{
		for (size_t i = 0; i < m_pallet.size(); i++)
		{
			m_pallet[i].blue = static_cast<u8>(i);
			m_pallet[i].green = static_cast<u8>(i);
			m_pallet[i].red = static_cast<u8>(i);
		}
	}
	else // Refer to the palette
	{
		// 1024 byte palette
		if (src_pallet_size == 1024)
		{
			for (size_t i = 0; i < m_pallet.size(); i++)
			{
				m_pallet[i].blue = *src_pallet++;
				m_pallet[i].green = *src_pallet++;
				m_pallet[i].red = *src_pallet++;
				src_pallet++;
			}
		}
		// 768 byte palette (No alpha)
		else if (src_pallet_size == 768)
		{
			for (size_t i = 0; i < m_pallet.size(); i++)
			{
				m_pallet[i].blue = *src_pallet++;
				m_pallet[i].green = *src_pallet++;
				m_pallet[i].red = *src_pallet++;
			}
		}
		// Other palette sizes
		else
		{
			for (size_t i = 0; i < src_pallet_size / 4; i++)
			{
				m_pallet[i].blue = *src_pallet++;
				m_pallet[i].green = *src_pallet++;
				m_pallet[i].red = *src_pallet++;
				src_pallet++;
			}
		}
	}

	return true;
}

/// Write 1 Line
void CPng::WriteLine(const u8* buffer)
{
	png_write_row(m_png_ptr, buffer);
	m_archive->GetProg()->UpdatePercent(m_row_size);
}

/// Write a line with alpha blending
///
/// @param buffer24 Storage Location
/// @param buffer32 32-bit data
///
void CPng::WriteLineWithAlphaBlend(u8* buffer24, const u8* buffer32)
{
	// Alpha blending
	AlphaBlend(buffer24, buffer32);

	png_write_row(m_png_ptr, buffer24);
	m_archive->GetProg()->UpdatePercent(m_row_size);
}

/// Finish output
void CPng::OnWriteFinish()
{
	png_write_end(m_png_ptr, m_info_ptr);
	png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
}
