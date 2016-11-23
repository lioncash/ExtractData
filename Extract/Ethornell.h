#pragma once

#include "../ExtractBase.h"

class CEthornell final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	struct NodeInfo
	{
		bool is_valid; // Validity
		u32  freq;     // Frequency
		u32  left;     // Left Node
		u32  right;    // Right Node
	};

	bool DecodeDSC(CArcFile* archive);
	bool DecodeCBG(CArcFile* archive);
	bool DecodeStd(CArcFile* archive);

	u32 GetKey(u32* key);
	u32 GetVariableData(const u8* src, size_t* read_length);

	void DecompDSC(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	void DecompCBG(u8* dst, u8* src);

	void DecryptBGType1(u8* dst, const u8* src, s32 width, s32 height, u16 bpp);
};
