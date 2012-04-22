
#include	"stdafx.h"
#include	"YCString.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	文字コード変換後の長さの取得
//
//	return	ワイド文字に変換したときの長さ
//
//	備考	マルチバイト文字 -> ワイド文字

int		YCBaseString::GetBaseTypeLength(
	const char*			pszSrc							// マルチバイト文字列
	) const
{
	return	(GetBaseTypeLength( pszSrc, -1 ) - 1);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字コード変換後の長さの取得
//
//	return	ワイド文字に変換したときの長さ
//
//	備考	マルチバイト文字 -> ワイド文字

int		YCBaseString::GetBaseTypeLength(
	const char*			pszSrc,							// マルチバイト文字列
	int					nLength							// 長さ
	) const
{
	return	::MultiByteToWideChar( CP_ACP, 0, pszSrc, nLength, NULL, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字コード変換後の長さの取得
//
//	return	マルチバイト文字に変換したときの長さ
//
//	備考	ワイド文字 -> マルチバイト文字

int		YCBaseString::GetBaseTypeLength(
	const wchar_t*		pwszSrc							// ワイド文字列
	) const
{
	return	(GetBaseTypeLength( pwszSrc, -1 ) - 1);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字コード変換後の長さの取得
//
//	return	マルチバイト文字に変換したときの長さ
//
//	備考	ワイド文字 -> マルチバイト文字

int		YCBaseString::GetBaseTypeLength(
	const wchar_t*		pwszSrc,						// ワイド文字列
	int					nLength							// 長さ
	) const
{
	return	::WideCharToMultiByte( CP_ACP, 0, pwszSrc, nLength, NULL, 0, NULL, NULL );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字コードの変換
//
//	return	ワイド文字に変換したときの長さ
//
//	備考	マルチバイト文字 -> ワイド文字

int		YCBaseString::ConvertToBaseType(
	wchar_t*			pwszDst,						// ワイド文字列の格納先
	int					nDstLength,						// 格納先サイズ
	const char*			pszSrc,							// マルチバイト文字列
	int					nSrcLength						// マルチバイト文字列の長さ
	) const
{
	int					nLength;

	nLength = ::MultiByteToWideChar( CP_ACP, 0, pszSrc, nSrcLength, pwszDst, nDstLength );

	if( nSrcLength == -1 )
	{
		nLength -= 1;
	}
	else
	{
		pwszDst[nLength] = L'\0';
	}

	return	nLength;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字コードの変換
//
//	return	マルチバイト文字に変換したときの長さ
//
//	備考	ワイド文字 -> マルチバイト文字

int		YCBaseString::ConvertToBaseType(
	char*				pszDst,							// マルチバイト文字列の格納先
	int					nDstLength,						// 格納先サイズ
	const wchar_t*		pwszSrc,						// ワイド文字列
	int					nSrcLength						// ワイド文字列の長さ
	) const
{
	int					nLength;

	nLength = ::WideCharToMultiByte( CP_ACP, 0, pwszSrc, nSrcLength, pszDst, nDstLength, NULL, NULL );

	if( nSrcLength == -1 )
	{
		nLength -= 1;
	}
	else
	{
		pszDst[nLength] = '\0';
	}

	return	nLength;
}
