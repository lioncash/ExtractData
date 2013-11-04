#pragma once

#include "ExtractBase.h"
#include "SearchBase.h"

class CExtract
{
private:
	static std::vector<CExtractBase*> m_Class;
	static std::vector<CSearchBase*> m_SearchClass;
	static std::set<CExtractBase*> m_DecodeClass;

public:
	CExtract();
	void SetClass();
	void SetSearchClass();
	static BOOL Mount(CArcFile* pclArc);
	static BOOL Decode(CArcFile* pclArc, BOOL bConvert);
	static BOOL Search(CArcFile* pclArc);
	void Close();
};