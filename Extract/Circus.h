
#pragma		once

#include	"../ExtractBase.h"

class	CCircus : public CExtractBase
{
public:

	struct	SPCMHeader
	{
		char				szIdentifier[4];			// 識別子
		DWORD				dwDataSize;					// データサイズ
		DWORD				dwFlags;					// フラグ
		WORD				wFormatID;					// フォーマットID
		WORD				wChannels;					// チャンネル数
		DWORD				dwFreq;						// サンプリングレート
		DWORD				dwBps;						// データ速度
		WORD				wBlockSize;					// ブロックサイズ
		WORD				wBits;						// ビット数
	};

	BOOL									Mount( CArcFile* pclArc );
	BOOL									Decode( CArcFile* pclArc );


protected:

	BOOL									MountPCK( CArcFile* pclArc );
	BOOL									MountVoiceDat( CArcFile* pclArc );
	BOOL									MountCRX( CArcFile* pclArc );
	BOOL									MountPCM( CArcFile* pclArc );

	BOOL									DecodeCRX( CArcFile* pclArc );
	BOOL									DecodeCRX1( CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize );
	BOOL									DecodeCRX2( CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize );
	BOOL									DecodePCM( CArcFile* pclArc );
	BOOL									DecodePCM1( CArcFile* pclArc, const SPCMHeader& rfstPCMHeader );
	BOOL									DecodePCM2( CArcFile* pclArc, const SPCMHeader& rfstPCMHeader );

	BOOL									DecompLZSS( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
	BOOL									DecompCRX2( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, WORD wFlags );
	BOOL									DecompPCM1( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
	BOOL									DecompPCM2( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
};
