#pragma once

#include "Extract/TCD/TCDBase.h"

class CTCD3 final : public CTCDBase
{
public:
	bool Mount(CArcFile* archive) override;

private:
	bool DecompRLE2(u8* dst, size_t dst_size, const u8* src, size_t src_size) override;
};
