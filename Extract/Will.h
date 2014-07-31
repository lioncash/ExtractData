#pragma	once

class CWill : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);


protected:
	void DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);

	BOOL AppendMask(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, const BYTE* pbtMask, DWORD dwMaskSize);
};
