#pragma once

class CArcFile;

class CZlib
{
public:
	bool Decode(CArcFile* archive);
	void DecompressFile(CArcFile* archive);
	int Decompress(u8* dst, u32* dst_size, const u8* src, u32 src_size);
	int Decompress(u8* dst, u32 dst_size, const u8* src, u32 src_size);
};
