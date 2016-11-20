#pragma once

#include "../ExtractBase.h"

class CTaskForce final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	struct FileEntry
	{
		char file_name[256];
		u32  offset;
		u32  original_size;
		u32  compressed_size;
	};

	bool MountDat(CArcFile* archive);
	bool MountTlz(CArcFile* archive);
	bool MountBma(CArcFile* archive);

	bool DecodeTlz(CArcFile* archive);
	bool DecodeBma(CArcFile* archive);
	bool DecodeTGA(CArcFile* archive);
};
