#pragma once

class CSummerDays final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;

private:
	struct TCONTEXT
	{
		YCString pcName;
		int iType;
		WORD ui16Code;
	};

	int _sub(CArcFile* pclArc, LPTSTR pcPath);
	WORD _context_new(CArcFile* pclArc, WORD ui16Length);
	int _context_add(WORD ui16Code);

	std::vector<TCONTEXT> m_tContextTable;
	WORD m_ui16ContextCount;

	DWORD m_RestArchiveSize;
};
