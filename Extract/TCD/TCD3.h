
#pragma		once

//----------------------------------------------------------------------------------------
//-- TCD3クラス --------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include	"TCDBase.h"

class	CTCD3 : public CTCDBase
{
public:

	struct	STCD3IndexInfo
	{
		DWORD				dwFileSize;					// ファイルサイズ
		DWORD				dwIndexOffset;				// インデックスへの絶対オフセット値
		DWORD				dwDirCount;					// フォルダ数
		DWORD				dwDirNameLength;			// フォルダ名の長さ(4バイト単位。例：SYSTEM == 8バイト)
		DWORD				dwFileCount;				// ファイル数
		DWORD				dwFileNameLength;			// ファイル名の長さ
		DWORD				dwFileCountEx;				// ファイル数(拡張版)
		DWORD				dwFileNameLengthEx;			// ファイル名の長さ(拡張版)
	};

	struct	STCD3DirInfo
	{
		DWORD				dwFileCount;				// ファイル数
		DWORD				dwFileNameOffset;			// ファイル名へのオフセット値
		DWORD				dwFileOffset;				// ファイルへのオフセット値
		DWORD				dwReserve;					// リザーブ
	};

	BOOL									Mount( CArcFile* pclArc );


protected:

	virtual	BOOL							DecompRLE2( void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize );
};
