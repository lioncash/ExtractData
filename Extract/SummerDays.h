#pragma once

#include "../ExtractBase.h"

class CSummerDays final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;

private:
	struct Context
	{
		YCString name;
		int type;
		u16 code;
	};

	bool Sub(CArcFile* archive, LPTSTR path);
	u16 CreateNewContext(CArcFile* archive, u16 length);
	int FindContextTypeWithCode(u16 code);

	std::vector<Context> m_contexts;
	u16 m_context_count = 0;

	u32 m_rest_archive_size = 0;
};
