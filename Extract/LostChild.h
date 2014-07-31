#pragma once

class CLostChild : public CExtractBase
{
public:
	virtual BOOL Mount(CArcFile* pclArc);
	virtual BOOL Decode(CArcFile* pclArc);
	virtual BOOL Extract(CArcFile* pclArc);

protected:
	BOOL DecodeESUR(CArcFile* pclArc);
	BOOL DecodeLAD(CArcFile* pclArc);

	BOOL DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset);
};
