#pragma once

#include "stdafx.h"
#include "ArcFile.h"
#include <cstddef>

// Pattern string
struct HF
{
	BYTE  pattern[32];
	DWORD size;
};

class CSearchBase
{
public:
	static constexpr size_t SEARCH_BUFFER_SIZE = 4096;

	CSearchBase();
	virtual ~CSearchBase();

	void Init();
	void Init(SOption* pOption);
	virtual void OnInit(SOption* pOption) {} // Virtual function for when you want to do something during initialization
	void InitHed(const void* pattern, DWORD size);
	void InitFot(const void* pattern, DWORD size);

	LPBYTE  GetHed()           { return m_pattern[0].pattern; }
	DWORD   GetHedSize() const { return m_pattern[0].size; }
	LPBYTE  GetFot()           { return m_pattern[1].pattern; }
	DWORD   GetFotSize() const { return m_pattern[1].size; }
	DWORD&  GetCtFile()        { return m_ctFile; }

	bool    CmpHed(LPCBYTE buf) { return CmpMem(buf, GetHed(), GetHedSize()); }
	bool    CmpFot(LPCBYTE buf) { return CmpMem(buf, GetFot(), GetFotSize()); }

	bool    CmpMem(LPCBYTE data, LPCBYTE pattern, DWORD size) const;
	bool    Search(LPCBYTE buf, DWORD dwSearchSize);
	bool    SearchFot(CArcFile* pclArc);

	void    SetOffset(DWORD offset) { m_offset = offset; }
	DWORD   GetOffset() const       { return m_offset; }

	virtual void Mount(CArcFile* pclArc) = 0;

private:
	HF m_pattern[2];
	DWORD m_offset;
	DWORD m_ctFile;
	void InitPattern(const void* pattern, DWORD size, DWORD num);
};
