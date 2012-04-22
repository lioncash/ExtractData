#pragma once

class CSummerDays : public CExtractBase {
protected:
	struct TCONTEXT {
		YCString pcName;
		int iType;
		WORD ui16Code;
	};

	std::vector<TCONTEXT> m_tContextTable;
	WORD m_ui16ContextCount;

	DWORD m_RestArchiveSize;

public:
	BOOL Mount(CArcFile* pclArc);

	int _sub(CArcFile* pclArc, LPTSTR pcPath);
	WORD _context_new(CArcFile* pclArc, WORD ui16Length);
	int _context_add(WORD ui16Code);
};