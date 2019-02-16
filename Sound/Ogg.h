#pragma once

#include "ExtractBase.h"

class CArcFile;

#pragma pack(push, 1)
struct OggPageHeader
{
	u8  pattern[4];
	u8  version;
	u8  type;
	u8  granule_position[8];
	u32 serial_no;
	u32 page_no;
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

	const OggPageHeader& GetHeader() const { return m_vheader; }

private:
	u32 GetSegSize(const OggPageHeader& vheader) const;
	u32 GetPageSize(const OggPageHeader& vheader) const;
	u32 GetPageSize(const OggPageHeader& vheader, u32 segment_size) const;

	CArcFile* m_archive = nullptr;
  OggPageHeader m_vheader{};
	u32 m_segment_size = 0;
	u32 m_page_size = 0;
};
