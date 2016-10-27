#pragma once

class CEthornell final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	struct SNodeInfo
	{
		bool  bValidity; // Validity
		DWORD dwFreq;    // Frequency
		DWORD dwLeft;    // Left Node
		DWORD dwRight;   // Right Node
	};

	bool DecodeDSC(CArcFile* pclArc);
	bool DecodeCBG(CArcFile* pclArc);
	bool DecodeStd(CArcFile* pclArc);

	DWORD GetKey(DWORD* dwKey);
	DWORD GetVariableData(const BYTE* pbtSrc, DWORD* pdwDstOfReadLength);

	void DecompDSC(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
	void DecompCBG(BYTE* pbtDst, BYTE* pbtSrc);

	void DecryptBGType1(BYTE* pbtDst, BYTE* pbtSrc, long lWidth, long lHeight, WORD wBpp);
};
