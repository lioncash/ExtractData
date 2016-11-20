#pragma once

class CArcFile;
class CExtractBase;
class CSearchBase;

class CExtract
{
public:
	CExtract();
	void SetClass();
	void SetSearchClass();
	static bool Mount(CArcFile* archive);
	static bool Decode(CArcFile* archive, bool convert);
	static bool Search(CArcFile* archive);
	void Close();

private:
	static std::vector<CExtractBase*> m_Class;
	static std::vector<CSearchBase*> m_SearchClass;
	static std::set<CExtractBase*> m_DecodeClass;
};
