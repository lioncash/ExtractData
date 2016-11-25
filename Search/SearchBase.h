#pragma once

class CArcFile;
struct SOption;

class CSearchBase
{
public:
	static constexpr size_t SEARCH_BUFFER_SIZE = 4096;

	CSearchBase();
	virtual ~CSearchBase();

	void Init();
	void Init(SOption* option);

	u8*     GetHeader()           { return m_pattern[0].pattern; }
	u32     GetHeaderSize() const { return m_pattern[0].size; }
	u8*     GetFooter()           { return m_pattern[1].pattern; }
	u32     GetFooterSize() const { return m_pattern[1].size; }
	u32&    GetNumFiles()         { return m_num_files; }

	bool    CmpHeader(const u8* buf) { return CmpMem(buf, GetHeader(), GetHeaderSize()); }
	bool    CmpFooter(const u8* buf) { return CmpMem(buf, GetFooter(), GetFooterSize()); }

	bool    CmpMem(const u8* data, const u8* pattern, size_t size) const;
	bool    Search(const u8* data, u32 search_size);
	bool    SearchFooter(CArcFile* archive);

	void    SetOffset(u32 offset) { m_offset = offset; }
	u32     GetOffset() const     { return m_offset; }

	virtual void Mount(CArcFile* archive) = 0;

protected:
	// Virtual function for when you want to do something during initialization
	virtual void OnInit(SOption* option) {}

	void InitHeader(const void* pattern, u32 size);
	void InitFooter(const void* pattern, u32 size);

private:
	struct Pattern
	{
		// Pattern string
		u8  pattern[32];
		u32 size;
	};

	std::array<Pattern, 2> m_pattern;
	u32 m_offset = 0;
	u32 m_num_files = 0;
	void InitPattern(const void* pattern, u32 size, u32 num);
};
