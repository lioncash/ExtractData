
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Arc/LZSS.h"
#include	"../Arc/Zlib.h"
#include	"Ahx.h"
#include	"../Image.h"
#include	"../Ogg.h"
#include	"Standard.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CStandard::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	// ファイル情報格納

	SFileInfo			stFileInfo;

	stFileInfo.name = pclArc->GetArcName();
	stFileInfo.sizeOrg = pclArc->GetArcSize();
	stFileInfo.sizeCmp = stFileInfo.sizeOrg;
	stFileInfo.start = 0;
	stFileInfo.end = stFileInfo.sizeOrg;

	pclArc->AddFileInfo( stFileInfo );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CStandard::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( DecodeLZSS( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeZlib( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeAhx( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeImage( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeOgg( pclArc ) )
	{
		return	TRUE;
	}

	return	Extract( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	LZSSのデコード

BOOL	CStandard::DecodeLZSS(
	CArcFile*			pclArc							// アーカイブ
	)
{
	CLZSS				clLZSS;

	return	clLZSS.Decode( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	zlibのデコード

BOOL	CStandard::DecodeZlib(
	CArcFile*			pclArc							// アーカイブ
	)
{
	CZlib				clZlib;

	return	clZlib.Decode( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	AHXのデコード

BOOL	CStandard::DecodeAhx(
	CArcFile*			pclArc							// アーカイブ
	)
{
	CAhx				clAHX;

	return	clAHX.Decode( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	画像のデコード

BOOL	CStandard::DecodeImage(
	CArcFile*			pclArc							// アーカイブ
	)
{
	CImage				clImage;

	return	clImage.Decode( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Ogg Vorbisのデコード

BOOL	CStandard::DecodeOgg(
	CArcFile*			pclArc							// アーカイブ
	)
{
	COgg				clOgg;

	return	clOgg.Decode( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	抽出

BOOL	CStandard::Extract(
	CArcFile*			pclArc							// アーカイブ
	)
{
	pclArc->OpenFile();
	pclArc->ReadWrite();
	pclArc->CloseFile();

	return	TRUE;
}
