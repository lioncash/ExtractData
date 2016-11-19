#pragma once

#include "stdafx.h"
#include "ArcFile.h"

#include <array>
#include <cstddef>

class CSearchBase
{
public:
	static constexpr size_t SEARCH_BUFFER_SIZE = 4096;

	CSearchBase();
	virtual ~CSearchBase();

	void Init();
	void Init(SOption* option);
	virtual void OnInit(SOption* option) {} // Virtual function for when you want to do something during initialization
	void InitHed(const void* pattern, u32 size);
	void InitFot(const void* pattern, u32 size);

	u8*     GetHed()           { return m_pattern[0].pattern; }
	u32     GetHedSize() const { return m_pattern[0].size; }
	u8*     GetFot()           { return m_pattern[1].pattern; }
	u32     GetFotSize() const { return m_pattern[1].size; }
	u32&    GetCtFile()        { return m_num_files; }

	bool    CmpHed(const u8* buf) { return CmpMem(buf, GetHed(), GetHedSize()); }
	bool    CmpFot(const u8* buf) { return CmpMem(buf, GetFot(), GetFotSize()); }

	bool    CmpMem(const u8* data, const u8* pattern, size_t size) const;
	bool    Search(const u8* data, u32 search_size);
	bool    SearchFot(CArcFile* archive);

	void    SetOffset(u32 offset) { m_offset = offset; }
	u32     GetOffset() const     { return m_offset; }

	virtual void Mount(CArcFile* archive) = 0;

private:
	// Pattern string
	struct HF
	{
		u8  pattern[32];
		u32 size;
	};

	std::array<HF, 2> m_pattern;
	u32 m_offset = 0;
	u32 m_num_files = 0;
	void InitPattern(const void* pattern, u32 size, u32 num);
};
