#pragma once

class CTlg final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc, LPBYTE src);

private:
	bool DecompTLG5(CArcFile* pclArc, LPBYTE src);
	bool DecompTLG6(CArcFile* pclArc, LPBYTE src);
	DWORD DecompLZSS(LPBYTE out, LPBYTE in, DWORD insize, LPBYTE dic, DWORD initialr);
	void ComposeColors(LPBYTE outp, LPBYTE upper, LPBYTE const* buf, DWORD width, DWORD colors);

	void TVPTLG6DecodeGolombValues(LPBYTE pixelbuf, DWORD pixel_count, LPBYTE bit_pool, BYTE color);
	void TVPTLG6DecodeLineGeneric(LPDWORD prevline, LPDWORD curline, DWORD width, DWORD start_block, DWORD block_limit, LPBYTE filtertypes, DWORD skipblockbytes, LPDWORD in, DWORD initialp, DWORD oddskip, DWORD dir);
	void TVPTLG6DecodeLine(LPDWORD prevline, LPDWORD curline, DWORD width, DWORD block_count, LPBYTE filtertypes, DWORD skipblockbytes, LPDWORD in, DWORD initialp, DWORD oddskip, DWORD dir);
	DWORD make_gt_mask(DWORD a, DWORD b);
	DWORD packed_bytes_add(DWORD a, DWORD b);
	DWORD med2(DWORD a, DWORD b, DWORD c);
	DWORD med(DWORD a, DWORD b, DWORD c, DWORD v);
	DWORD avg(DWORD a, DWORD b, DWORD c, DWORD v);
	void InitTLG6Table();
};
