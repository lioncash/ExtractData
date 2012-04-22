
#pragma	once

//----------------------------------------------------------------------------------------
//-- ユーティリティ関数 ------------------------------------------------------------------
//----------------------------------------------------------------------------------------

namespace	YCUtil
{
	// 文字列関係

//	YCString								SetFileFormat( const YCString& rfclsFileName );
//	YCString								SetCommaFormat( DWORD dwSize, DWORD dwDigit = 3 );

	// パス関係

	void									CreateDirectory( LPCTSTR pszPathToFile, BOOL bFileName );
	void									ReplaceSlashToBackslash( LPSTR pszFileName );
	void									ReplaceSlashToBackslash( LPWSTR pwszFileName );

	// エンディアン変換

	DWORD									ConvEndian( DWORD dwSrc );
	void									ConvEndian( LPDWORD pdwDst );

	WORD									ConvEndian( WORD wSrc );
	void									ConvEndian( LPWORD pwDst );

	// メモリ関係

	inline BOOL								CompareMemory( const void* pvData1, const void* pvData2, DWORD dwSize );
};
