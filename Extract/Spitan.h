#pragma once

#include "../ExtractBase.h"

class CSpitan final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;

private:
	struct FileInfo
	{
		u32 start;
		u32 size;
		TCHAR dummy[20];
	};

	bool MountSound(CArcFile* archive);
	bool MountGraphic1(CArcFile* archive);
	bool MountGraphic2(CArcFile* archive);
	bool MountGraphic3(CArcFile* archive);
};
