#pragma once

#include "../ExtractBase.h"

#pragma pack(push, 1)

struct VH
{
	BYTE pattern[4];
	BYTE version;
	BYTE type;
	BYTE granpos[8];
	DWORD serialno;
	DWORD pageno;
	DWORD checksum;
	BYTE page_segments;
	BYTE segment_table[256];
};

#pragma pack(pop, 1)

class COgg final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	void Decode(CArcFile* archive, u8* buf);

	void Init(CArcFile* archive);
	u32 ReadHed();
	u32 ReadHed(const u8* buf);
	void NextPage();
	void BackHed();

	void FixCRC(u8* data, u32 page_size);

	const VH& GetHed() const { return m_vheader; }

private:
	u32 GetSegSize(const VH& vheader) const;
	u32 GetPageSize(const VH& vheader) const;
	u32 GetPageSize(const VH& vheader, u32 segment_size) const;

	CArcFile* m_pclArc = nullptr;
	VH m_vheader{};
	u32 m_segment_size = 0;
	u32 m_page_size = 0;
};
