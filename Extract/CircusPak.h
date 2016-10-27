#pragma once

class CCircusPak final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	using FDecrypt = void(*)(void*, DWORD, const void*);

	struct SPakFileInfoType1
	{
		char  szFileName[24];
		DWORD dwOffset;
		DWORD dwCompFileSize;
	};

	struct SPakFileInfoType2
	{
		char  szFileName[32];
		DWORD dwOffset;
		DWORD dwCompFileSize;
	};

	struct SPakFileInfoType3
	{
		wchar_t szFileName[32];
		DWORD   dwFileSize;
		DWORD   dwCompFileSize;
		DWORD   dwOffset;
	};

	bool MountPakForKujiraCons(CArcFile* pclArc);
	bool MountPakForKujira(CArcFile* pclArc);
	bool MountPakForACDC(CArcFile* pclArc);
	bool MountPakForDCGS(CArcFile* pclArc);

	bool DecodePakForKujiraCons(CArcFile* pclArc);
	bool DecodePakForKujira(CArcFile* pclArc);
	bool DecodePakForACDC(CArcFile* pclArc);
	bool DecodePakForDCGS(CArcFile* pclArc);

	bool DecodeBMP(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize);
	bool DecodeCps(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize);
	bool DecodeEtc(CArcFile* pclArc, FDecrypt pfnDecryptFunc, const void* pvKey);

	bool DecompCCC0(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	bool DecompCCM0(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);

	DWORD GetBit(const void* pvSrc, DWORD* pdwSrcPtrOfBit, DWORD dwReadBitLength);

	static void Decrypt1(void* pvTarget, DWORD dwTargetSize, const void* pvKey);
	static void Decrypt2(void* pvTarget, DWORD dwTargetSize, const void* pvKey);
};
