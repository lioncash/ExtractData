#pragma once

class CCircusPak : public CExtractBase
{
public:

	typedef BOOL (*FDecrypt)( void*, DWORD, const void* );

	struct SPakFileInfoType1
	{
		char				szFileName[24];
		DWORD				dwOffset;
		DWORD				dwCompFileSize;
	};

	struct SPakFileInfoType2
	{
		char				szFileName[32];
		DWORD				dwOffset;
		DWORD				dwCompFileSize;
	};

	struct SPakFileInfoType3
	{
		wchar_t				szFileName[32];
		DWORD				dwFileSize;
		DWORD				dwCompFileSize;
		DWORD				dwOffset;
	};

	BOOL					Mount( CArcFile* pclArc );
	BOOL					Decode( CArcFile* pclArc );


protected:

	BOOL					MountPakForKujiraCons( CArcFile* pclArc );
	BOOL					MountPakForKujira( CArcFile* pclArc );
	BOOL					MountPakForACDC( CArcFile* pclArc );
	BOOL					MountPakForDCGS( CArcFile* pclArc );

	BOOL					DecodePakForKujiraCons( CArcFile* pclArc );
	BOOL					DecodePakForKujira( CArcFile* pclArc );
	BOOL					DecodePakForACDC( CArcFile* pclArc );
	BOOL					DecodePakForDCGS( CArcFile* pclArc );

	BOOL					DecodeBMP( CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize );
	BOOL					DecodeCps( CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize );
	BOOL					DecodeEtc( CArcFile* pclArc, FDecrypt pfnDecryptFunc, const void* pvKey );

	BOOL					DecompCCC0( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
	BOOL					DecompCCM0( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );

	DWORD					GetBit( const void* pvSrc, DWORD* pdwSrcPtrOfBit, DWORD dwReadBitLength );

	static BOOL				Decrypt1( void* pvTarget, DWORD dwTargetSize, const void* pvKey );
	static BOOL				Decrypt2( void* pvTarget, DWORD dwTargetSize, const void* pvKey );
};
