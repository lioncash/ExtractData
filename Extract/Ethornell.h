#pragma once

class CEthornell final : public CExtractBase
{
public:
	struct SNodeInfo
	{
		BOOL  bValidity; // Validity
		DWORD dwFreq;    // Frequency
		DWORD dwLeft;    // Left Node
		DWORD dwRight;   // Right Node
	};

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL DecodeDSC(CArcFile* pclArc);
	BOOL DecodeCBG(CArcFile* pclArc);
	BOOL DecodeStd(CArcFile* pclArc);

	DWORD GetKey(DWORD* dwKey);
	DWORD GetVariableData(const BYTE* pbtSrc, DWORD* pdwDstOfReadLength);

	void DecompDSC(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
	void DecompCBG(BYTE* pbtDst, BYTE* pbtSrc);

	void DecryptBGType1(BYTE* pbtDst, BYTE* pbtSrc, long lWidth, long lHeight, WORD wBpp);
};
