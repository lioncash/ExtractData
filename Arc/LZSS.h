#pragma once

class CArcFile;

class CLZSS
{
public:
	bool Decode(CArcFile* archive);

	bool Decomp(CArcFile* archive, size_t dic_size, size_t dic_ptr, size_t length_offset);
	bool Decomp(void* dst, size_t dst_size, const void* src, size_t src_size, size_t dic_size, size_t dic_ptr, size_t length_offset);
};
