#include "stdafx.h"
#include "Jpg.h"

//////////////////////////////////////////////////
// Class to read from the memory source to JPEG //
//////////////////////////////////////////////////

// JPEG expansion manager from the memory source
struct memory_source_mgr
{
	jpeg_source_mgr pub; // public fields

	JOCTET* buffer;
	unsigned long buffer_length;
};
typedef memory_source_mgr* memory_src_ptr;

void CJpglib::memory_init_source(j_decompress_ptr cinfo)
{
}

boolean CJpglib::memory_fill_input_buffer(j_decompress_ptr cinfo)
{
	memory_src_ptr src = (memory_src_ptr) cinfo->src;

	src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = 2;
	return TRUE;
}

void CJpglib::memory_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	memory_src_ptr src = (memory_src_ptr) cinfo->src;

	if (num_bytes > 0)
	{
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

void CJpglib::memory_term_source(j_decompress_ptr cinfo)
{
}

void CJpglib::jpeg_memory_src(j_decompress_ptr cinfo, void* data, unsigned long len)
{
	memory_src_ptr src;

	// First time for this JPEG object?
	if (cinfo->src == NULL)
	{
		cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(memory_source_mgr));
		src = (memory_src_ptr)cinfo->src;
		src->buffer = (JOCTET*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, len * sizeof(JOCTET));
	}

	src = (memory_src_ptr)cinfo->src;

	src->pub.init_source = memory_init_source;
	src->pub.fill_input_buffer = memory_fill_input_buffer;
	src->pub.skip_input_data = memory_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = memory_term_source;

	src->pub.bytes_in_buffer = len;
	src->pub.next_input_byte = (JOCTET*)data;
}


//////////////////////////////////////////////////
//        Class for dealing with JPEG           //
//////////////////////////////////////////////////

CJpg::CJpg()
{
	m_dst = NULL;
}

CJpg::~CJpg()
{
	Close();
}

void CJpg::Close()
{
	if (m_dst != NULL)
	{
		for (int i = 0; i < m_cinfo.output_height; i++)
			delete[] m_dst[i];

		delete[] m_dst;
		m_dst = NULL;
	}
}

LPBYTE CJpg::Decomp(LPBYTE src, DWORD len, LPDWORD dstSize)
{
	m_cinfo.err = jpeg_std_error(&m_jerr);

	// Prepare JPEG Decompression
	jpeg_create_decompress(&m_cinfo);
	jpeg_memory_src(&m_cinfo, src, len);
	jpeg_read_header(&m_cinfo, TRUE);
	jpeg_start_decompress(&m_cinfo);

	// Memory allocation for decompression
	m_dst = new JSAMPROW[m_cinfo.output_height];
	for (int i = 0; i < m_cinfo.output_height; i++)
		m_dst[i] = new JSAMPLE[m_cinfo.output_width * 3];

	// Decompression
	while (m_cinfo.output_scanline < m_cinfo.output_height)
	{
		jpeg_read_scanlines(&m_cinfo, m_dst + m_cinfo.output_scanline, m_cinfo.output_height - m_cinfo.output_scanline);
	}

	// Finish decompression
	jpeg_finish_decompress(&m_cinfo);

	*dstSize = m_cinfo.output_height * m_cinfo.output_width * 3;
	m_bmpbuf.resize(*dstSize);
	for (int i = 0; i < m_cinfo.output_height; i++)
		memcpy(&m_bmpbuf[m_cinfo.output_width * 3 * i], m_dst[i], m_cinfo.output_width * 3);

	Close();

	return &m_bmpbuf[0];
}

void CJpg::AlphaBlend(LPBYTE dst, LPBYTE mask, DWORD bg)
{
	BYTE byBg[3];
	for (int i = 0; i < 3; i++)
		byBg[i] = BYTE((bg >> (i << 3)) & 0xff);

	for (LONG y = 0; y < m_cinfo.output_height; y++)
	{
		for (LONG x = 0; x < m_cinfo.output_width; x++)
		{
			//if (*mask == 0)
			//{
			//	for (int i = 0; i < 3; i++)
			//		*dst++ = byBg[i];
			//}
			//else if (*mask == 0xFF)
			//{
			//	for (int i = 0; i < 3; i++)
			//		*dst++ = *mask;
			//}
			//else
			//{
				for (int i = 0; i < 3; i++)
					*dst++ = (*dst - byBg[i]) * *mask / 255 + byBg[i];//((255 - buf32[3]) * pBgRGB[i] + buf32[3] * buf32[i]) / 255;
			//}
			mask++;
		}
	}
}

//void CJpg::InitWrite()
//{
//	LONG width = m_cinfo.output_width;
//	LONG height = m_cinfo.output_height;
//
//	// Destroy JPEG Object
//	jpeg_destroy_decompress(&m_cinfo);
//
//	jpeg_create_compress(&cinfo);
//	FILE *outfile = fopen(argv[2], "wb");
//	jpeg_stdio_dest(&cinfo2, outfile);
//
//	cinfo.image_width = width;
//	cinfo.image_height = height;
//	cinfo.input_components = 3;
//	cinfo.in_color_space = JCS_RGB;
//}

void CJpg::SetQuality(int quality)
{
	// TODO ?
}

void CJpg::Write(CArcFile* pArc, LPBYTE dst, DWORD dstSize)
{
	// TODO ?
}

void CJpg::WriteReverse(CArcFile* pArc, LPBYTE dst, DWORD dstSize)
{
	// TODO ?
}
