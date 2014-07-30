#pragma once

#include "stdafx.h"
#include "ArcFile.h"

#define BUFSIZE 4096

// Pattern string
struct HF
{
	BYTE  pattern[32];
	DWORD size;
};


class CSearchBase
{
private:
	HF m_pattern[2];
	DWORD m_offset;
	DWORD m_ctFile;
	void InitPattern(const LPVOID pattern, DWORD size, DWORD num);

public:
	CSearchBase();
	virtual ~CSearchBase();

	void Init();
	void Init(SOption* pOption);
	virtual void OnInit(SOption* pOption) {} // Virtual function for when you want to do something during initialization
	void InitHed(const LPVOID pattern, DWORD size);
	void InitFot(const LPVOID pattern, DWORD size);

	LPBYTE  GetHed()           { return m_pattern[0].pattern; }
	DWORD   GetHedSize()       { return m_pattern[0].size; }
	LPBYTE  GetFot()           { return m_pattern[1].pattern; }
	DWORD   GetFotSize()       { return m_pattern[1].size; }
	DWORD&  GetCtFile()        { return m_ctFile; }

	BOOL    CmpHed(LPBYTE buf) { return CmpMem(buf, GetHed(), GetHedSize()); }
	BOOL    CmpFot(LPBYTE buf) { return CmpMem(buf, GetFot(), GetFotSize()); }

	BOOL    CmpMem(const LPBYTE data, const LPBYTE pattern, DWORD size);
	BOOL    Search(CArcFile* pclArc, const LPBYTE buf, DWORD dwReadSize, DWORD dwSearchSize);
	BOOL    SearchFot(CArcFile* pclArc);

	void    SetOffset(DWORD offset) { m_offset = offset; }
	DWORD   GetOffset()             { return m_offset; }

	virtual void Mount(CArcFile* pclArc) = 0;
};
