#pragma once

#include "ExtractBase.h"
#include "Search/SearchBase.h"

class CExtract
{
public:
	CExtract();
	void SetClass();
	void SetSearchClass();
	static bool Mount(CArcFile* pclArc);
	static bool Decode(CArcFile* pclArc, bool convert);
	static bool Search(CArcFile* pclArc);
	void Close();

private:
	static std::vector<CExtractBase*> m_Class;
	static std::vector<CSearchBase*> m_SearchClass;
	static std::set<CExtractBase*> m_DecodeClass;
};
