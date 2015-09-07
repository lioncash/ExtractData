#pragma once

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
	COgg();

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;
	void Decode(CArcFile* pclArc, LPBYTE buf);

	void Init(CArcFile* pclArc);
	DWORD ReadHed();
	DWORD ReadHed(LPBYTE buf);
	void NextPage();
	void BackHed();

	void FixCRC(LPBYTE data, DWORD PageSize);

	DWORD GetSegSize(VH& vheader);
	DWORD GetPageSize(VH& vheader);
	DWORD GetPageSize(VH& vheader, DWORD SegmentSize);

	VH& GetHed() { return m_vheader; }

private:
	CArcFile* m_pclArc;
	VH m_vheader;
	DWORD m_SegmentSize;
	DWORD m_PageSize;

	DWORD m_pCRCTable[256];
};
