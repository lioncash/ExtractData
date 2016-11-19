#include "StdAfx.h"
#include "ExtractBase.h"
#include "Image.h"

/// Constructor
CImage::CImage()
{
}

bool CImage::Mount(CArcFile* archive)
{
	return m_bmp.Mount(archive);
}

bool CImage::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("BMP"))
		return false;

	// Read BMP
	std::vector<u8> src(file_info->sizeOrg);
	archive->Read(src.data(), file_info->sizeOrg);

	// Output
	Init(archive, src.data());
	Write(file_info->sizeOrg);

	return true;
}

/// Initialization (When BMP is passed)
///
/// @param archive   Archive
/// @param bmp       BMP data
/// @param file_name Filename
///
bool CImage::Init(CArcFile* archive, void* bmp, const YCString& file_name)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	BYTE*            src = static_cast<BYTE*>(bmp);

	if (file_info->format == _T("BMP") && file_info->title == _T("") && file_info->key == 0)
	{
		// Simple decoding
		archive->InitDecrypt(src);
		archive->Decrypt(src, file_info->sizeOrg);
	}

	m_archive = archive;
	m_bmp_data = src;
	m_bmp_file_header = reinterpret_cast<BITMAPFILEHEADER*>(&src[0]);
	m_bmp_info_header = reinterpret_cast<BITMAPINFOHEADER*>(&src[14]);

	if (m_bmp_file_header->bfType != 0x4D42)
	{
		// No BM or such in an encryption header
		archive->OpenFile();
		archive->WriteFile(src, file_info->sizeOrg);

		m_is_valid_bmp_header = false;

		return false;
	}

	// Normal BMP
	return Init(archive, m_bmp_info_header->biWidth, m_bmp_info_header->biHeight, m_bmp_info_header->biBitCount, &src[54], m_bmp_file_header->bfOffBits - 54, file_name);
}

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
bool CImage::Init(CArcFile* archive, long width, long height, WORD bpp, const void* pallet, DWORD pallet_size, const YCString& file_name)
{
	m_archive = archive;
	m_option = archive->GetOpt();

	// Header output

	// BMP Output
	if (m_option->bDstBMP)
	{
		TCHAR full_file_name[MAX_PATH];

		lstrcpy(full_file_name, file_name);
		PathRenameExtension(full_file_name, _T(".bmp"));

		return m_bmp.Init(archive, width, height, bpp, pallet, pallet_size, full_file_name);
	}

	// PNG Output
	if (m_option->bDstPNG)
	{
		TCHAR full_file_name[MAX_PATH];

		lstrcpy(full_file_name, file_name);
		PathRenameExtension(full_file_name, _T(".png"));

		return m_png.Init(archive, width, height, bpp, pallet, pallet_size, full_file_name);
	}

	return true;
}

/// Close
void CImage::Close()
{
	m_archive->CloseFile();
}

/// Output
///
/// @param bmp_size BMP Size
/// @param progress Request progress bar status
///
bool CImage::Write(DWORD bmp_size, bool progress)
{
	return Write(&m_bmp_data[m_bmp_file_header->bfOffBits], bmp_size - m_bmp_file_header->bfOffBits, progress);
}

/// Output
///
/// @param bmp_data      BMP Data (header not included)
/// @param bmp_data_size BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::Write(const void* bmp_data, DWORD bmp_data_size, bool progress)
{
	// Invalid BMP
	if (!m_is_valid_bmp_header)
		return false;

	// BMP Output
	if (m_option->bDstBMP)
	{
		m_bmp.Write(bmp_data, bmp_data_size, progress);
	}
	// PNG output
	else if (m_option->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_png.WriteReverse(bmp_data, bmp_data_size, progress);
	}

	return true;
}

/// Inverted Output
///
/// @param bmp_size BMP Size
/// @param progress Request progress bar status
///
bool CImage::WriteReverse(DWORD bmp_size, bool progress)
{
	return WriteReverse(&m_bmp_data[m_bmp_file_header->bfOffBits], bmp_size - m_bmp_file_header->bfOffBits, progress);
}

/// Inverted Output
///
/// @param bmp_data      BMP Data (header not included)
/// @param bmp_data_size BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::WriteReverse(const void* bmp_data, DWORD bmp_data_size, bool progress)
{
	// Invalid BMP
	if (!m_is_valid_bmp_header)
		return false;

	// BMP output
	if (m_option->bDstBMP)
	{
		m_bmp.WriteReverse(bmp_data, bmp_data_size, progress);
	}
	// PNG output
	else if (m_option->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_png.Write(bmp_data, bmp_data_size, progress);
	}

	return true;
}

/// Synthesize BGRA Output
///
/// @param bmp_size BMP Size
/// @param progress Request BMP status
///
bool CImage::WriteCompoBGRA(DWORD bmp_size, bool progress)
{
	return WriteCompoBGRA(&m_bmp_data[m_bmp_file_header->bfOffBits], bmp_size - m_bmp_file_header->bfOffBits, progress);
}

/// Synthesize BGRA Output
///
/// @param bmp_data      BMP Data (header not included)
/// @param bmp_data_size BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::WriteCompoBGRA(const void* bmp_data, DWORD bmp_data_size, bool progress)
{
	// Invalid BMP
	if (!m_is_valid_bmp_header)
		return false;

	// BMP output
	if (m_option->bDstBMP)
	{
		m_bmp.WriteCompoBGRA(bmp_data, bmp_data_size, progress);
	}
	// PNG output
	else if (m_option->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_png.WriteCompoBGRAReverse(bmp_data, bmp_data_size, progress);
	}

	return true;
}

/// Synthesize upside-down BGRA Output
///
/// @param bmp_size BMP Size
/// @param progress Request progress bar status
///
bool CImage::WriteCompoBGRAReverse(DWORD bmp_size, bool progress)
{
	return WriteCompoBGRAReverse(&m_bmp_data[m_bmp_file_header->bfOffBits], bmp_size - m_bmp_file_header->bfOffBits, progress);
}

/// Synthesize upside-down BGRA Output
///
/// @param bmp_data      BMP Data (header not included)
/// @param bmp_data_size BMP Data Size
/// @param progress      Request progress bar status 
///
bool CImage::WriteCompoBGRAReverse(const void* bmp_data, DWORD bmp_data_size, bool progress)
{
	// Invalid BMP
	if (!m_is_valid_bmp_header)
		return false;

	// BMP output
	if (m_option->bDstBMP)
	{
		m_bmp.WriteCompoBGRAReverse(bmp_data, bmp_data_size, progress);
	}
	// PNG output
	else if (m_option->bDstPNG)
	{
		// BMP format for top-down to Reverse
		m_png.WriteCompoBGRA(bmp_data, bmp_data_size, progress);
	}

	return true;
}

/// Synthesize RGBA Output
///
/// @param bmp_size BMP Size
/// @param progress Request progress bar status
///
bool CImage::WriteCompoRGBA(DWORD bmp_size, bool progress)
{
	return WriteCompoRGBA(&m_bmp_data[m_bmp_file_header->bfOffBits], bmp_size - m_bmp_file_header->bfOffBits, progress);
}

/// Synthesize RGBA Output
///
/// @param bmp_data      BMP Data (Header not included)
/// @param bmp_data_size BMP Data Size
/// @param progress      Request progress bar status
///
bool CImage::WriteCompoRGBA(const void* bmp_data, DWORD bmp_data_size, bool progress)
{
	// Invalid BMP
	if (!m_is_valid_bmp_header)
		return false;

	// BMP output
	if (m_option->bDstBMP)
	{
		m_bmp.WriteCompoRGBA(bmp_data, bmp_data_size, progress);
	}
	// PNG output
	else if (m_option->bDstPNG)
	{
		m_png.WriteCompoRGBAReverse(bmp_data, bmp_data_size, progress);
	}

	return true;
}

/// Synthesize upside-down RGBA Output
///
/// @param bmp_size BMP Size
/// @param progress Request progress bar status
///
bool CImage::WriteCompoRGBAReverse(DWORD bmp_size, bool progress)
{
	return WriteCompoRGBAReverse(&m_bmp_data[m_bmp_file_header->bfOffBits], bmp_size - m_bmp_file_header->bfOffBits, progress);
}

/// Synthesize upside-down RGBA Output
///
/// @param bmp_data      BMP Data (Header not included)
/// @param bmp_data_size BMP Data Size
/// @param progress      Requesting progress bar status
/// 
bool CImage::WriteCompoRGBAReverse(const void* bmp_data, DWORD bmp_data_size, bool progress)
{
	// Invalid BMP
	if (!m_is_valid_bmp_header)
		return false;

	// BMP output
	if (m_option->bDstBMP)
	{
		m_bmp.WriteCompoRGBAReverse(bmp_data, bmp_data_size, progress);
	}
	// PNG output
	else if (m_option->bDstPNG)
	{
		m_png.WriteCompoRGBA(bmp_data, bmp_data_size, progress);
	}

	return true;
}

/// Get bitmap file header
BITMAPFILEHEADER* CImage::GetBmpFileHeader() const
{
	return m_bmp_file_header;
}

/// Get bitmap info header
BITMAPINFOHEADER* CImage::GetBmpInfoHeader() const
{
	return m_bmp_info_header;
}
