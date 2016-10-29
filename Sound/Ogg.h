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
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
	void Decode(CArcFile* pclArc, LPBYTE buf);

	void Init(CArcFile* pclArc);
	DWORD ReadHed();
	DWORD ReadHed(const BYTE* buf);
	void NextPage();
	void BackHed();

	void FixCRC(LPBYTE data, DWORD PageSize);

	const VH& GetHed() const { return m_vheader; }

private:
	DWORD GetSegSize(const VH& vheader) const;
	DWORD GetPageSize(const VH& vheader) const;
	DWORD GetPageSize(const VH& vheader, DWORD SegmentSize) const;

	CArcFile* m_pclArc = nullptr;
	VH m_vheader{};
	DWORD m_SegmentSize = 0;
	DWORD m_PageSize = 0;
};
