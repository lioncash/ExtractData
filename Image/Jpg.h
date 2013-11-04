#pragma once

#include "ImageBase.h"

class CJpglib
{
protected:
	static void memory_init_source(j_decompress_ptr cinfo);
	static boolean memory_fill_input_buffer(j_decompress_ptr cinfo);
	static void memory_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
	static void memory_term_source(j_decompress_ptr cinfo);
	static void jpeg_memory_src(j_decompress_ptr cinfo, void* data, unsigned long len);
};

class CJpg : public CJpglib
{
private:
	jpeg_decompress_struct m_cinfo;
	jpeg_error_mgr m_jerr;

	JSAMPARRAY m_dst;
	std::valarray<BYTE> m_bmpbuf;

public:
	CJpg();
	~CJpg();

	void Close();

	// Expand the JPEG data stored in the src
	LPBYTE Decomp(LPBYTE src, DWORD len, LPDWORD dstSize);
	// Alpha blending using the mask
	void AlphaBlend(LPBYTE dst, LPBYTE mask, DWORD bg);

	// jpg compression rate
	void SetQuality(int quality);
	// jpg compressed output
	void Write(CArcFile* pArc, LPBYTE dst, DWORD dstSize);
	// jpg compressed output (reversed)
	void WriteReverse(CArcFile* pArc, LPBYTE dst, DWORD dstSize);

	LONG GetWidth() { return m_cinfo.output_width; }
	LONG GetHeight() { return m_cinfo.output_height; }
};
