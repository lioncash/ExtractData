#pragma once

#include "ExtractBase.h"

class CArcFile;

#pragma pack(push, 1)
struct VH
{
	u8  pattern[4];
	u8  version;
	u8  type;
	u8  granpos[8];
	u32 serialno;
	u32 pageno;
	u32 checksum;
	u8  page_segments;
	u8  segment_table[256];
};
#pragma pack(pop)

class COgg final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	void Decode(CArcFile* archive, u8* buf);

	void Init(CArcFile* archive);
	u32 ReadHeader();
	u32 ReadHeader(const u8* buf);
	void NextPage();
	void SeekToPreviousHeader();

	void FixCRC(u8* data, u32 page_size);

	const VH& GetHeader() const { return m_vheader; }

private:
	u32 GetSegSize(const VH& vheader) const;
	u32 GetPageSize(const VH& vheader) const;
	u32 GetPageSize(const VH& vheader, u32 segment_size) const;

	CArcFile* m_archive = nullptr;
	VH m_vheader{};
	u32 m_segment_size = 0;
	u32 m_page_size = 0;
};
