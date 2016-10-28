#pragma once

class CSummerDays final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;

private:
	struct TCONTEXT
	{
		YCString pcName;
		int iType;
		WORD ui16Code;
	};

	bool _sub(CArcFile* pclArc, LPTSTR pcPath);
	WORD _context_new(CArcFile* pclArc, WORD ui16Length);
	int FindContextTypeWithCode(WORD code);

	std::vector<TCONTEXT> m_tContextTable;
	WORD m_ui16ContextCount;

	DWORD m_RestArchiveSize;
};
