
#pragma	once

class	CPajamas : public CExtractBase
{
public:

	BOOL									Mount( CArcFile* pclArc );
	BOOL									Decode( CArcFile* pclArc );


protected:

	BOOL									MountDat1( CArcFile* pclArc );
	BOOL									MountDat2( CArcFile* pclArc );

	BOOL									DecodeEPA( CArcFile* pclArc );

	BOOL									DecompEPA( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth );
};
