
#pragma		once

//----------------------------------------------------------------------------------------
//-- TCD2クラス --------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include	"TCDBase.h"

class	CTCD2 : public CTCDBase
{
public:

	struct	STCD2IndexInfo
	{
		DWORD				dwFileSize;					// ファイルサイズ
		DWORD				dwFileCount;				// ファイル数
		DWORD				dwDirCount;					// フォルダ数
		DWORD				dwIndexOffset;				// インデックスへの絶対オフセット値
		DWORD				dwDirNameLength;			// フォルダ名の長さ
		DWORD				dwFileNameLength;			// ファイル名の長さ
		DWORD				dwFileCountEx;				// ファイル数(拡張版)
		DWORD				dwFileNameLengthEx;			// ファイル名の長さ(拡張版)
	};

	struct	STCD2DirInfo
	{
		DWORD				dwFileCount;				// ファイル数
		DWORD				dwFileNameOffset;			// ファイル名へのオフセット値
		DWORD				dwFileOffset;				// ファイルへのオフセット値
		DWORD				dwReserve;					// リザーブ
	};

	virtual	BOOL							Mount( CArcFile* pclArc );


protected:

	virtual	BOOL							DecompRLE2( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
};
