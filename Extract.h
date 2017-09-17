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
	static std::vector<CExtractBase*> m_class;
	static std::vector<CSearchBase*> m_search_class;
	static std::set<CExtractBase*> m_decode_class;
};
