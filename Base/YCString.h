
#pragma		once

#define		YCSTRINGT_BUFFERSIZE_ALIGNMENT				64	// バッファのアライメント
#define		YCSTRINGT_OFFSET_BUFFERSIZE					16
#define		YCSTRINGT_OFFSET_LENGTH						12

//----------------------------------------------------------------------------------------
//-- 基本文字列クラス --------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class	YCBaseString
{
public:

	int										GetBaseTypeLength( const char* pszSrc ) const;
	int										GetBaseTypeLength( const char* pszSrc, int nLength ) const;
	int										GetBaseTypeLength( const wchar_t* pwszSrc ) const;
	int										GetBaseTypeLength( const wchar_t* pwszSrc, int nLength ) const;

	int										ConvertToBaseType( wchar_t* pwszDst, int nDstLength, const char* pszSrc, int nSrcLength = -1 ) const;
	int										ConvertToBaseType( char* pszDst, int nDstLength, const wchar_t* pwszSrc, int nSrcLength = -1 ) const;
};

//----------------------------------------------------------------------------------------
//-- テンプレート文字列クラス ------------------------------------------------------------
//----------------------------------------------------------------------------------------

// 可変長引数に渡すため、仮想関数を作ってはいけない
// 仮想関数を作ると「__vfptr」メンバ変数の存在により可変長引数に渡せなくなる

template<class TYPE>
class	YCStringT : protected YCBaseString
{
private:

	TYPE*									m_ptString;


public:

	// コンストラクタ

											YCStringT();
											YCStringT( const char* pszSrc );
											YCStringT( const char* pszSrc, int nCount );
											YCStringT( const char& rfcSrc );
											YCStringT( const wchar_t* pwszSrc );
											YCStringT( const wchar_t* pwszSrc, int nCount );
											YCStringT( const wchar_t& rfwcSrc );
											YCStringT( const YCStringT<TYPE>& rfclsSrc );
											YCStringT( const YCStringT<TYPE>& rfclsSrc, int nCount );

	// デストラクタ

											~YCStringT();	// 仮想関数にすると__vfptrの存在により可変長引数に渡せなくなる

	// 代入

	BOOL									LoadString( UINT uID );

	void									Copy( const char* pszSrc );
	void									Copy( const char* pszSrc, int nLength );
	void									Copy( const char& rfcSrc );
	void									Copy( const wchar_t* pwszSrc );
	void									Copy( const wchar_t* pwszSrc, int nLength );
	void									Copy( const wchar_t& rfwcSrc );

	// 追加

	void									Append( const char* pszAppend );
	void									Append( const char* pszAppend, int nLength );
	void									Append( const char& rfcAppend );
	void									Append( const wchar_t* pwszAppend );
	void									Append( const wchar_t* pwszAppend, int nLength );
	void									Append( const wchar_t& rfwcAppend );

	// 書式付きデータの設定

	void									Format( const TYPE* pszFormat, ... );
	void									AppendFormat( const TYPE* pszFormat, ... );

	// 挿入

	int										Insert( int nIndex, const TYPE* pszInsert );
	int										Insert( int nIndex, const TYPE& rfcInsert );

	// 削除

	void									Empty();

	int										Delete( int nIndex, int nCount = 1 );

	int										Remove( const TYPE* pszRemove );
	int										Remove( const TYPE& rfcRemove );

	YCStringT<TYPE>&						Trim();
	YCStringT<TYPE>&						Trim( const TYPE* pszTrim );
	YCStringT<TYPE>&						Trim( const TYPE& rfcTrim );

	YCStringT<TYPE>&						TrimLeft();
	YCStringT<TYPE>&						TrimLeft( const TYPE* pszTrim );
	YCStringT<TYPE>&						TrimLeft( const TYPE& rfcTrim );

	YCStringT<TYPE>&						TrimRight();
	YCStringT<TYPE>&						TrimRight( const TYPE* pszTrim );
	YCStringT<TYPE>&						TrimRight( const TYPE& rfcTrim );

	// 置換

	int										Replace( const TYPE* pszOld, const TYPE* pszNew );
	int										Replace( const TYPE& rfcOld, const TYPE& rfcNew );

	// 小文字大文字変換

	YCStringT<TYPE>&						MakeLower();
	YCStringT<TYPE>&						MakeUpper();

	// 比較

	int										Compare( const char* pszSrc ) const;
	int										Compare( const char& rfcSrc ) const;
	int										Compare( const wchar_t* pwszSrc ) const;
	int										Compare( const wchar_t& pwwcSrc ) const;

	int										CompareNoCase( const char* pszSrc ) const;
	int										CompareNoCase( const char& rfcSrc ) const;
	int										CompareNoCase( const wchar_t* pwszSrc ) const;
	int										CompareNoCase( const wchar_t& rfwcSrc ) const;

	// 検索

	int										Find( const TYPE* pszSub, int nStart = 0 ) const;
	int										Find( const TYPE& rfcSub, int nStart = 0 ) const;

	int										ReverseFind( const TYPE* pszSub ) const;
	int										ReverseFind( const TYPE& rfcSub ) const;

	// 抽出

	YCStringT<TYPE>							Left( int nCount ) const;
	YCStringT<TYPE>							Right( int nCount ) const;
	YCStringT<TYPE>							Mid( int nFirst ) const;
	YCStringT<TYPE>							Mid( int nFirst, int nCount ) const;

	// 文字列バッファの取得

	TYPE*									GetBuffer( int nBufferSize );

	// 文字列バッファのポインタの取得

	inline	const TYPE*						GetString() const;

	// 文字列の長さの取得

	inline	int								GetLength() const;

	// 文字列が空かどうか調べる

	bool									IsEmpty() const;

	// パス関連

	void									RenameExtension( const TYPE* ptSrc );

	YCStringT<TYPE>							GetFileName() const;
	int										GetFileNameIndex() const;
	YCStringT<TYPE>							GetFileTitle() const;
	YCStringT<TYPE>							GetFileExt() const;
	int										GetFileExtIndex() const;
	YCStringT<TYPE>							GetDirPath() const;

	// 代入演算子

	YCStringT<TYPE>&						operator=( const char* pszSrc );
	YCStringT<TYPE>&						operator=( const char& rfcSrc );
	YCStringT<TYPE>&						operator=( const wchar_t* pwszSrc );
	YCStringT<TYPE>&						operator=( const wchar_t& rfwcSrc );
	YCStringT<TYPE>&						operator=( const YCStringT<TYPE>& rfclsSrc );

	// +演算子

	YCStringT<TYPE>							operator+( const char* pszAppend ) const;
	YCStringT<TYPE>							operator+( const char& rfcAppend ) const;
	YCStringT<TYPE>							operator+( const wchar_t* pwszAppend ) const;
	YCStringT<TYPE>							operator+( const wchar_t& rfwcAppend ) const;
	YCStringT<TYPE>							operator+( const YCStringT<TYPE>& rfclsAppend ) const;

	// +演算子(外部関数)

	template<class TYPE>
	friend	YCStringT<TYPE>					operator+( const char* pszSrc, const YCStringT<TYPE>& rfclsAppend );

	template<class TYPE>
	friend	YCStringT<TYPE>					operator+( const char& rfcSrc, const YCStringT<TYPE>& rfclsAppend );

	template<class TYPE>
	friend	YCStringT<TYPE>					operator+( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsAppend );

	template<class TYPE>
	friend	YCStringT<TYPE>					operator+( const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsAppend );

	// +=演算子

	YCStringT<TYPE>&						operator+=( const char* pszAppend );
	YCStringT<TYPE>&						operator+=( const char& rfcAppend );
	YCStringT<TYPE>&						operator+=( const wchar_t* pwszAppend );
	YCStringT<TYPE>&						operator+=( const wchar_t& rfwcAppend );
	YCStringT<TYPE>&						operator+=( const YCStringT<TYPE>& rfclsAppend );

	// ==演算子

	bool									operator==( const char* pszSrc ) const;
	bool									operator==( const char& rfcSrc ) const;
	bool									operator==( const wchar_t* pwszSrc ) const;
	bool									operator==( const wchar_t& rfwcSrc ) const;
	bool									operator==( const YCStringT<TYPE>& rfclsSrc ) const;

	// ==演算子(外部関数)

	template<class TYPE>
	friend	bool							operator==( const char* pszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator==( const char& rfcSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator==( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator==( const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsSrc );

	// !=演算子

	bool									operator!=( const char* pszSrc ) const;
	bool									operator!=( const char& rfcSrc ) const;
	bool									operator!=( const wchar_t* pwszSrc ) const;
	bool									operator!=( const wchar_t& rfwcSrc ) const;
	bool									operator!=( const YCStringT<TYPE>& rfclsSrc ) const;

	// !=演算子(外部関数)

	template<class TYPE>
	friend	bool							operator!=( const char* pszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator!=( const char& rfcSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator!=( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator!=( const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsSrc );

	// <演算子

	bool									operator<( const char* pszSrc ) const;
	bool									operator<( const wchar_t* pwszSrc ) const;
	bool									operator<( const YCStringT<TYPE>& rfclsSrc ) const;

	// <演算子(外部関数)

	template<class TYPE>
	friend	bool							operator<( const char* pszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator<( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc );

	// <=演算子

	bool									operator<=( const char* pszSrc ) const;
	bool									operator<=( const wchar_t* pwszSrc ) const;
	bool									operator<=( const YCStringT<TYPE>& rfclsSrc ) const;

	// <=演算子(外部関数)

	template<class TYPE>
	friend	bool							operator<=( const char* pszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator<=( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc );

	// >演算子

	bool									operator>( const char* pszSrc ) const;
	bool									operator>( const wchar_t* pwszSrc ) const;
	bool									operator>( const YCStringT<TYPE>& rfclsSrc ) const;

	// >演算子(外部関数)

	template<class TYPE>
	friend	bool							operator>( const char* pszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator>( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc );

	// >=演算子

	bool									operator>=( const char* pszSrc ) const;
	bool									operator>=( const wchar_t* pwszSrc ) const;
	bool									operator>=( const YCStringT<TYPE>& rfclsSrc ) const;

	// >=演算子(外部関数)

	template<class TYPE>
	friend	bool							operator>=( const char* pszSrc, const YCStringT<TYPE>& rfclsSrc );

	template<class TYPE>
	friend	bool							operator>=( const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc );

	// []演算子

	inline	TYPE&							operator[]( int nPos );
	inline	const TYPE&						operator[]( int nPos ) const;

	// const TYPE*演算子

	inline									operator const TYPE*() const;


protected:

	void									Alloc( int nBufferSize );
	void									Free();

	void									ExtendBuffer( int nBufferSize );

	void									SetBufferSize( int nBufferSize );
	int										GetBufferSize();

	void									SetLength( int nLength );
};

typedef	YCStringT<TCHAR>								YCString;
typedef	YCStringT<char>									YCStringA;
typedef	YCStringT<wchar_t>								YCStringW;

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT()
{
	m_ptString = NULL;

	Empty();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const char*			pszSrc							// 初期値
	)
{
	m_ptString = NULL;

	Copy( pszSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const char*			pszSrc,							// 初期値
	int					nCount							// コピーする長さ
	)
{
	m_ptString = NULL;

	Copy( pszSrc, nCount );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const char&			rfcSrc							// 初期値
	)
{
	m_ptString = NULL;

	Copy( rfcSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const wchar_t*		pwszSrc							// 初期値
	)
{
	m_ptString = NULL;

	Copy( pwszSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const wchar_t*		pwszSrc,						// 初期値
	int					nCount							// コピーする長さ
	)
{
	m_ptString = NULL;

	Copy( pwszSrc, nCount );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const wchar_t&		rfwcSrc							// 初期値
	)
{
	m_ptString = NULL;

	Copy( rfwcSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コピーコンストラクタ

template<class TYPE> YCStringT<TYPE>::YCStringT(
	const YCStringT<TYPE>&	rfclsSrc					// コピー元文字列クラス
	)
{
	m_ptString = NULL;

	Copy( rfclsSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

template<class TYPE> YCStringT<TYPE>::~YCStringT()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列リソースを読み込む

template<class TYPE> BOOL YCStringT<TYPE>::LoadString(
	UINT				uID								// リソースID
	)
{
	return	::LoadString( ::GetModuleHandle( NULL ), uID, GetBuffer( 1024 ), 1024 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の代入

template<class TYPE> void YCStringT<TYPE>::Copy(
	const char*			pszSrc							// コピー元文字列
	)
{
	TYPE*				ptString;
	int					nSrcLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nSrcLength = lstrlenA( pszSrc );

		ptString = GetBuffer( (nSrcLength + 1) );

		lstrcpyA( (char*) ptString, pszSrc );

		break;

	case	2:
		// wchar_t型

		nSrcLength = GetBaseTypeLength( pszSrc );

		ptString = GetBuffer( (nSrcLength + 1) );

		ConvertToBaseType( (wchar_t*) ptString, GetBufferSize(), pszSrc );

		break;
	}

	SetLength( nSrcLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の代入

template<class TYPE> void YCStringT<TYPE>::Copy(
	const char*			pszSrc,							// コピー元文字列
	int					nCount							// コピーする長さ
	)
{
	TYPE*				ptString;
	int					nSrcLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nSrcLength = nCount;

		ptString = GetBuffer( (nSrcLength + 1) );

		lstrcpynA( (char*) ptString, pszSrc, (nCount + 1) );

		break;

	case	2:
		// wchar_t型

		nSrcLength = GetBaseTypeLength( pszSrc, nCount );

		ptString = GetBuffer( (nSrcLength + 1) );

		ConvertToBaseType( (wchar_t*) ptString, GetBufferSize(), pszSrc, nCount );

		break;
	}

	SetLength( nSrcLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字の代入

template<class TYPE> void YCStringT<TYPE>::Copy(
	const char&			rfcSrc							// コピー元文字
	)
{
	char				szSrc[2];

	szSrc[0] = rfcSrc;
	szSrc[1] = '\0';

	Copy( szSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の代入

template<class TYPE> void YCStringT<TYPE>::Copy(
	const wchar_t*		pwszSrc							// コピー元文字列
	)
{
	TYPE*				ptString;
	int					nSrcLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nSrcLength = GetBaseTypeLength( pwszSrc );

		ptString = GetBuffer( (nSrcLength + 1) );

		ConvertToBaseType( (char*) ptString, GetBufferSize(), pwszSrc );

		break;

	case	2:
		// wchar_t型

		nSrcLength = lstrlenW( pwszSrc );

		ptString = GetBuffer( (nSrcLength + 1) );

		lstrcpyW( (wchar_t*) ptString, pwszSrc );

		break;
	}

	SetLength( nSrcLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の代入

template<class TYPE> void YCStringT<TYPE>::Copy(
	const wchar_t*		pwszSrc,						// コピー元文字列
	int					nCount							// コピーする長さ
	)
{
	TYPE*				ptString;
	int					nSrcLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nSrcLength = GetBaseTypeLength( pwszSrc, nCount );

		ptString = GetBuffer( (nSrcLength + 1) );

		ConvertToBaseType( (char*) ptString, GetBufferSize(), pwszSrc, nCount );

		break;

	case	2:
		// wchar_t型

		nSrcLength = nCount;

		ptString = GetBuffer( (nSrcLength + 1) );

		lstrcpynW( (wchar_t*) ptString, (wchar_t*) pwszSrc, (nCount + 1) );

		break;
	}

	SetLength( nSrcLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字の代入

template<class TYPE> void YCStringT<TYPE>::Copy(
	const wchar_t&		rfwcSrc							// コピー元文字
	)
{
	wchar_t				wszSrc[2];

	wszSrc[0] = rfwcSrc;
	wszSrc[1] = L'\0';

	Copy( wszSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の追加

template<class TYPE> void YCStringT<TYPE>::Append(
	const char*			pszAppend						// 追加文字列
	)
{
	int					nStringLength = GetLength();
	int					nNewLength = nStringLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nNewLength += lstrlenA( pszAppend );

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		lstrcpyA( (char*) &m_ptString[nStringLength], pszAppend );

		break;

	case	2:
		// wchar_t型

		nNewLength += GetBaseTypeLength( pszAppend );

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		ConvertToBaseType( (wchar_t*) &m_ptString[nStringLength], (GetBufferSize() - nStringLength), pszAppend );

		break;
	}

	SetLength( nNewLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の追加

template<class TYPE> void YCStringT<TYPE>::Append(
	const char*			pszAppend,						// 追加文字列
	int					nCount							// 追加する文字数
	)
{
	int					nStringLength = GetLength();
	int					nNewLength = nStringLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nNewLength += nCount;

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		lstrcpynA( (char*) &m_ptString[nStringLength], pszAppend, (nCount + 1) );

		break;

	case	2:
		// wchar_t型

		nNewLength += GetBaseTypeLength( pszAppend, nCount );

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		ConvertToBaseType( (wchar_t*) &m_ptString[nStringLength], (GetBufferSize() - nStringLength), pszAppend, nCount );

		break;
	}

	SetLength( nNewLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字の追加

template<class TYPE> void YCStringT<TYPE>::Append(
	const char&			rfcAppend						// 追加文字
	)
{
	char				szAppend[2];

	szAppend[0] = rfcAppend;
	szAppend[1] = '\0';

	Append( szAppend );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の追加

template<class TYPE> void YCStringT<TYPE>::Append(
	const wchar_t*		pwszAppend						// 追加文字列
	)
{
	int					nStringLength = GetLength();
	int					nNewLength = nStringLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nNewLength += GetBaseTypeLength( pwszAppend );

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		ConvertToBaseType( (char*) &m_ptString[nStringLength], (GetBufferSize() - nStringLength), pwszAppend );

		break;

	case	2:
		// wchar_t型

		nNewLength += lstrlenW( pwszAppend );

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		lstrcpyW( (wchar_t*) &m_ptString[nStringLength], pwszAppend );

		break;
	}

	SetLength( nNewLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の追加

template<class TYPE> void YCStringT<TYPE>::Append(
	const wchar_t*		pwszAppend,						// 追加文字列
	int					nCount							// 追加する文字数
	)
{
	int					nStringLength = GetLength();
	int					nNewLength = nStringLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nNewLength += GetBaseTypeLength( pwszAppend, nCount );

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		ConvertToBaseType( (char*) &m_ptString[nStringLength], (GetBufferSize() - nStringLength), pwszAppend, nCount );

		break;

	case	2:
		// wchar_t型

		nNewLength += nCount;

		// バッファの拡張

		ExtendBuffer( (nNewLength + 1) );

		// 文字列の追加

		lstrcpynW( (wchar_t*) &m_ptString[nStringLength], pwszAppend, (nCount + 1) );

		break;
	}

	SetLength( nNewLength );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字の追加

template<class TYPE> void YCStringT<TYPE>::Append(
	const wchar_t&		rfwcAppend						// 追加文字
	)
{
	wchar_t				wszAppend[2];

	wszAppend[0] = rfwcAppend;
	wszAppend[1] = L'\0';

	Append( wszAppend );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	書式付きデータの設定

template<class TYPE> void YCStringT<TYPE>::Format(
	const TYPE*			pszFormat,						// 書式付きデータ
	...													// 可変長引数
	)
{
	va_list				vaArgPtr;

	va_start( vaArgPtr, pszFormat );

	YCMemory<TYPE>		clmBuffer( 1024 );

	while( 1 )
	{
		va_list				vaWork = vaArgPtr;

		if( _vsntprintf( &clmBuffer[0], (clmBuffer.size() - 1), pszFormat, vaWork ) == - 1 )
		{
			clmBuffer.resize( clmBuffer.size() * 2 );
		}
		else
		{
			break;
		}
	}

	va_end( vaArgPtr );

	Copy( &clmBuffer[0] );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	書式付きデータの設定

template<class TYPE> void YCStringT<TYPE>::AppendFormat(
	const TYPE*			pszFormat,						// 書式付きデータ
	...													// 可変長引数
	)
{
	va_list				vaArgPtr;

	va_start( vaArgPtr, pszFormat );

	YCMemory<TYPE>		clmBuffer( 1024 );

	while( 1 )
	{
		va_list				vaWork = vaArgPtr;

		if( _vsntprintf( &clmBuffer[0], (clmBuffer.size() - 1), pszFormat, vaWork ) == - 1 )
		{
			clmBuffer.resize( clmBuffer.size() * 2 );
		}
		else
		{
			break;
		}
	}

	va_end( vaArgPtr );

	Append( &clmBuffer[0] );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	挿入
//
//	return	挿入後の文字列の長さ

template<class TYPE> int YCStringT<TYPE>::Insert(
	int					nIndex,							// 挿入位置
	const TYPE*			pszInsert						// 挿入文字列
	)
{
	if( nIndex < 0 )
	{
		// 先頭に挿入

		nIndex = 0;
	}
	else if( nIndex >= GetLength() )
	{
		// 連結

		Append( pszInsert );

		return	GetLength();
	}

	// 挿入処理

	int					nStringLength = GetLength();
	int					nNewLength = nStringLength;
	int					nInsertLength;
	int					nShiftLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nInsertLength = lstrlenA( (const char*) pszInsert );
		nShiftLength = lstrlenA( (const char*) &m_ptString[nIndex] );
		break;

	case	2:
		// wchar_t型

		nInsertLength = lstrlenW( (const wchar_t*) pszInsert );
		nShiftLength = lstrlenW( (const wchar_t*) &m_ptString[nIndex] );
		break;
	}

	// バッファの拡張

	nNewLength += nInsertLength;

	ExtendBuffer( (nNewLength + 1) );

	// 文字列を挿入する空き領域を作る

	memmove( &m_ptString[nIndex + nInsertLength], &m_ptString[nIndex], (sizeof(TYPE) * (nShiftLength + 1)) );

	// 文字列の挿入

	memcpy( &m_ptString[nIndex], pszInsert, (sizeof(TYPE) * nInsertLength) );

	SetLength( nNewLength );

	return	GetLength();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字の挿入
//
//	return	挿入後の文字列の長さ

template<class TYPE> int YCStringT<TYPE>::Insert(
	int					nIndex,							// 挿入位置
	const TYPE&			rfcInsert						// 挿入文字
	)
{
	TYPE				szInsert[2];

	szInsert[0] = rfcInsert;
	szInsert[1] = 0;

	return	Insert( nIndex, szInsert );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列を空にする

template<class TYPE> void YCStringT<TYPE>::Empty()
{
	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		Copy( "" );
		break;

	case	2:
		// wchar_t型

		Copy( L"" );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の削除
//
//	削除後の文字列の長さ

template<class TYPE> int YCStringT<TYPE>::Delete(
	int					nIndex,							// 削除位置
	int					nCount							// 削除する文字数
	)
{
	int					nStringLength = GetLength();
	int					nNewLength = nStringLength;

	if( nIndex < 0 )
	{
		// 先頭から削除

		nIndex = 0;
	}
	else if( nIndex >= nStringLength )
	{
		// 削除する文字列なし

		return	GetLength();
	}

	if( nCount <= 0 )
	{
		// 削除する文字列なし

		return	GetLength();
	}

	if( (nIndex + nCount) >= nStringLength )
	{
		// 削除位置以降の文字列を削除

		m_ptString[nIndex] = 0;

		SetLength( nIndex );

		return	GetLength();
	}

	// 削除処理

	int					nShiftLength = (GetLength() - (nIndex + nCount));

	// 文字列の削除
	// 文字列を左に詰めて上書きする

	memmove( &m_ptString[nIndex], &m_ptString[nIndex + nCount], (sizeof(TYPE) * (nShiftLength + 1)) );

	nNewLength -= nCount;

	SetLength( nNewLength );

	return	GetLength();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の削除
//
//	文字列から削除した回数

template<class TYPE> int YCStringT<TYPE>::Remove(
	const TYPE*			pszRemove						// 削除対象文字列
	)
{
	int					nIndex = 0;
	int					nCount = 0;
	int					nRemoveLength = 0;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nRemoveLength = lstrlenA( pszRemove );
		break;

	case	2:
		// wchar_t型

		nRemoveLength = lstrlenW( pszRemove );
		break;
	}

	if( nRemoveLength <= 0 )
	{
		// 削除文字列なし

		return	nCount;
	}

	while( 1 )
	{
		nIndex = Find( pszRemove, nIndex );

		if( nIndex >= 0 )
		{
			// 削除対象文字列が存在する

			Delete( nIndex, nRemoveLength );

			nCount++;
		}
		else
		{
			// 削除対象文字列が存在しない

			break;
		}
	}

	return	nCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字の削除
//
//	文字列から削除した回数

template<class TYPE> int YCStringT<TYPE>::Remove(
	const TYPE&			rfcRemove						// 削除対象文字
	)
{
	int					nIndex = 0;
	int					nCount = 0;
	int					nRemoveLength = 1;

	while( 1 )
	{
		nIndex = Find( rfcRemove, nIndex );

		if( nIndex >= 0 )
		{
			// 削除対象文字列が存在する

			Delete( nIndex, nRemoveLength );

			nCount++;
		}
		else
		{
			// 削除対象文字列が存在しない

			break;
		}
	}

	return	nCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	先頭と末尾にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::Trim()
{
	TrimLeft();
	TrimRight();

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	先頭と末尾にある空白文字の削除

template<class TYPE> YCStringT<TYPE>&  YCStringT<TYPE>::Trim(
	const TYPE*			pszTrim							// 削除対象文字列
	)
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	先頭と末尾にある空白文字の削除

template<class TYPE> YCStringT<TYPE>&  YCStringT<TYPE>::Trim(
	const TYPE&			rfcTrim							// 削除対象文字
	)
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	先頭にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::TrimLeft()
{
	BOOL				bResult;
	int					nCount;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		bResult = TRUE;

		for( nCount = 0 ; nCount < GetLength() ; )
		{
			if( ::IsDBCSLeadByte( m_ptString[nCount] ) )
			{
				// 全角

				if( strncmp( (char*) &m_ptString[nCount], "　", 2 ) == 0 )
				{
					// トリミング対象文字
				}
				else
				{
					// その他

					bResult = FALSE;
				}

				nCount += 2;
			}
			else
			{
				// 半角

				switch( m_ptString[nCount] )
				{
				case	' ':
				case	'\t':
				case	'\r':
				case	'\n':
					// トリミング対象文字

					break;

				default:
					// その他

					bResult = FALSE;
				}

				nCount += 1;
			}
		}

		break;

	case	2:
		// wchar_t型

		bResult = TRUE;

		for( nCount = 0 ; nCount < GetLength() ; nCount += 2 )
		{
			switch( m_ptString[nCount] )
			{
			case	L' ':
			case	L'\t':
			case	L'\r':
			case	L'\n':
			case	L'　':
				// トリミング対象文字

				break;

			default:
				// その他

				bResult = FALSE;
			}
		}

		break;
	}

	Delete( 0, nCount );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	先頭にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::TrimLeft( const TYPE* pszTrim )
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	先頭にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::TrimLeft( const TYPE& rfcTrim )
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	末尾にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::TrimRight()
{
	BOOL				bResult;
	int					nIndex;
	int					nTrimIndex;
	int					nCount;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		bResult = TRUE;
		nIndex = 0;
		nTrimIndex = 0;

		for( nCount = 0 ; nCount < 2 ; nCount++ )
		{
			if( ::IsDBCSLeadByte( m_ptString[nIndex] ) )
			{
				// 全角
			}
			else
			{
				// 半角

				switch( m_ptString[nIndex] )
				{

				}
			}
		}


		break;

	case	2:
		// wchar_t型

		break;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////
//	末尾にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::TrimRight( const TYPE* pszTrim )
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	末尾にある空白文字の削除

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::TrimRight( const TYPE& rfcTrim )
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	置換
//
//	return	置き換えられた回数

template<class TYPE> int YCStringT<TYPE>::Replace(
	const TYPE*			pszOld,							// 置換対象文字
	const TYPE*			pszNew							// 置換後の文字
	)
{
	int					nIndex = 0;
	int					nCount = 0;
	int					nStringLength = GetLength();
	int					nNewStringLength = nStringLength;
	int					nOldLength;
	int					nNewLength;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nOldLength = lstrlenA( (const char*) pszOld );
		nNewLength = lstrlenA( (const char*) pszNew );
		break;

	case		2:
		// wchar_t型

		nOldLength = lstrlenW( (const wchar_t*) pszOld );
		nNewLength = lstrlenW( (const wchar_t*) pszNew );
		break;
	}

	while( 1 )
	{
		nIndex = Find( pszOld, nIndex );

		if( nIndex >= 0 )
		{
			// 置換対象文字列が存在する

			if( nNewLength == nOldLength )
			{
				// 置換後も文字列の長さは変わらない

				memcpy( &m_ptString[nIndex], pszNew, (sizeof(TYPE) * nNewLength) );
			}
			else
			{
				// 置換後は文字列が増加／減少する

				nNewStringLength += (nNewLength - nOldLength);

				ExtendBuffer( (nNewStringLength + 1) );

				memmove( &m_ptString[nIndex + nNewLength], &m_ptString[nIndex + nOldLength], (sizeof(TYPE) * ((nStringLength + 1) - (nIndex + nOldLength))) );

				memcpy( &m_ptString[nIndex], pszNew, (sizeof(TYPE) * nNewLength) );

				nStringLength = nNewStringLength;
			}

			nCount++;
			nIndex += nNewLength;
		}
		else
		{
			// 置換対象文字列が存在しない

			break;
		}
	}

	SetLength( nStringLength );

	return	nCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	置換
//
//	return	置き換えられた回数

template<class TYPE> int YCStringT<TYPE>::Replace(
	const TYPE&			rfcOld,							// 置換対象文字
	const TYPE&			rfcNew							// 置換後の文字
	)
{
	int					nIndex = 0;
	int					nCount = 0;
	int					nNewIndex = GetLength();

	while( 1 )
	{
		nIndex = Find( rfcOld, nIndex );

		if( nIndex >= 0 )
		{
			// 置換対象文字が存在する

			m_ptString[nIndex] = rfcNew;

			nCount++;
		}
		else
		{
			// 置換対象文字が存在しない

			break;
		}
	}

	return	nCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	小文字変換

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::MakeLower()
{
	::CharLower( m_ptString );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	大文字変換

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::MakeUpper()
{
	::CharUpper( m_ptString );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	pszSrcより小さい
//			正の値	pszSrcより大きい

template<class TYPE> int YCStringT<TYPE>::Compare(
	const char*			pszSrc							// 比較文字列
	) const
{
	int					nReturn;
	TYPE*				ptWork;
	int					nWork;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nReturn = lstrcmpA( (const char*) m_ptString, pszSrc );
		break;

	case	2:
		// wchar_t型

		nWork = GetBaseTypeLength( pszSrc );

		ptWork = new TYPE[nWork];

		ConvertToBaseType( (wchar_t*) ptWork, nWork, pszSrc );

		nReturn = lstrcmpW( (const wchar_t*) m_ptString, (const wchar_t*) ptWork );

		delete[]	ptWork;

		break;
	}

	return	nReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	rfcSrcより小さい
//			正の値	rfcSrcより大きい

template<class TYPE> int YCStringT<TYPE>::Compare(
	const char&			rfcSrc							// 比較文字
	) const
{
	char				szSrc[2];

	szSrc[0] = rfcSrc;
	szSrc[1] = '\0';

	return	Compare( szSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	pszSrcより小さい
//			正の値	pszSrcより大きい

template<class TYPE> int YCStringT<TYPE>::Compare(
	const wchar_t*		pwszSrc							// 比較文字列
	) const
{
	int					nReturn;
	TYPE*				ptWork;
	int					nWork;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nWork = GetBaseTypeLength( pwszSrc );

		ptWork = new TYPE[nWork];

		ConvertToBaseType( (char*) ptWork, nWork, pwszSrc );

		nReturn = lstrcmpA( (const char*) m_ptString, (const char*) ptWork );

		delete[]	ptWork;

		break;

	case	2:
		// wchar_t型

		nReturn = lstrcmpW( (const wchar_t*) m_ptString, pwszSrc );
		break;
	}

	return	nReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	rfwcSrcより小さい
//			正の値	rfwcSrcより大きい

template<class TYPE> int YCStringT<TYPE>::Compare(
	const wchar_t&		rfwcSrc							// 比較文字
	) const
{
	wchar_t				wszSrc[2];

	wszSrc[0] = pwszSrc;
	wszSrc[1] = L'\0';

	return	Compare( wszSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	pszSrcより小さい
//			正の値	pszSrcより大きい
//
//	備考	大文字・小文字の比較なし

template<class TYPE> int YCStringT<TYPE>::CompareNoCase(
	const char*			pszSrc							// 比較文字列
	) const
{
	int					nReturn;
	TYPE*				ptWork;
	int					nWork;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nReturn = lstrcmpiA( (const char*) m_ptString, pszSrc );
		break;

	case	2:
		// wchar_t型

		nWork = GetBaseTypeLength( pszSrc );

		ptWork = new TYPE[nWork];

		ConvertToBaseType( (wchar_t*) ptWork, nWork, pszSrc );

		nReturn = lstrcmpiW( (const wchar_t*) m_ptString, (const wchar_t*) ptWork );

		delete[]	ptWork;

		break;
	}

	return	nReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	rfcSrcより小さい
//			正の値	rfcSrcより大きい
//
//	備考	大文字・小文字の比較なし

template<class TYPE> int YCStringT<TYPE>::CompareNoCase(
	const char&			rfcSrc							// 比較文字
	) const
{
	char				szSrc[2];

	szSrc[0] = rfcSrc;
	szSrc[1] = '\0';

	return	CompareNoCase( szSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	pszSrcより小さい
//			正の値	pszSrcより大きい
//
//	備考	大文字・小文字の比較なし

template<class TYPE> int YCStringT<TYPE>::CompareNoCase(
	const wchar_t*		pwszSrc							// 比較文字列
	) const
{
	int					nReturn;
	TYPE*				ptWork;
	int					nWork;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nWork = GetBaseTypeLength( pwszSrc );

		ptWork = new TYPE[nWork];

		ConvertToBaseType( (char*) ptWork, nWork, pwszSrc );

		nReturn = lstrcmpiA( (const char*) m_ptString, (const char*) ptWork );

		delete[]	ptWork;

		break;

	case	2:
		// wchar_t型

		nReturn = lstrcmpiW( (const wchar_t*) m_ptString, pwszSrc );
		break;
	}

	return	nReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	比較
//
//	return	0		一致した
//			負の値	rfwcSrcより小さい
//			正の値	rfwcSrcより大きい
//
//	備考	大文字・小文字の比較なし

template<class TYPE> int YCStringT<TYPE>::CompareNoCase(
	const wchar_t&		rfwcSrc							// 比較文字
	) const
{
	wchar_t				wszSrc[2];

	wszSrc[0] = rfwcSrc;
	wszSrc[1] = L'\0';

	return	CompareNoCase( wszSrc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	前方検索
//
//	return	一致した位置
//
//	備考	一致する文字列が存在しない場合は -1 を返す

template<class TYPE> int YCStringT<TYPE>::Find(
	const TYPE*			pszSub,							// 検索対象文字列
	int					nStart							// 検索開始位置
	) const
{
	INT_PTR				npResult = -1;

	if( nStart < 0 )
	{
		// 文字列の先頭から検索

		nStart = 0;
	}
	else if( nStart >= GetLength() )
	{
		// 文字列の最後から検索

		return	(int) npResult;
	}

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		const BYTE*			pbtFound;

		pbtFound = _mbsstr( (const BYTE*) &m_ptString[nStart], (const BYTE*) pszSub );

		if( pbtFound != NULL )
		{
			// 対象文字列が存在する

			npResult = (pbtFound - (BYTE*) m_ptString);
		}

		break;

	case	2:
		// wchar_t型

		const wchar_t*		pwszFound;

		pwszFound = wcsstr( (const wchar_t*) &m_ptString[nStart], (const wchar_t*) pszSub );

		if( pwszFound != NULL )
		{
			//  対象文字列が存在する

			npResult = (pwszFound - (wchar_t*) m_ptString);
		}

		break;
	}

	return	(int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	前方検索

template<class TYPE> int YCStringT<TYPE>::Find(
	const TYPE&			rfcSub,							// 検索対象文字
	int					nStart							// 検索開始位置
	) const
{
	INT_PTR				npResult = -1;

	if( nStart <  0 )
	{
		// 文字列の先頭から検索

		nStart = 0;
	}
	else if( nStart >= GetLength() )
	{
		// 文字列の最後から検索

		return	(int) npResult;
	}

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		const BYTE*			pbtFound;

		pbtFound = _mbschr( (const BYTE*) &m_ptString[nStart], rfcSub );

		if( pbtFound != NULL )
		{
			// 対象文字列が存在する

			npResult = (pbtFound - (BYTE*) m_ptString);
		}

		break;

	case	2:
		// wchar_t型

		const wchar_t*		pwszFound;

		pwszFound = wcschr( (const wchar_t*) &m_ptString[nStart], rfcSub );

		if( pwszFound != NULL )
		{
			// 対象文字列が存在する

			npResult = (pwszFound - (wchar_t*) m_ptString);
		}

		break;
	}

	return	(int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	後方検索
//
//	return	見つかった位置
//
//	備考	マルチバイトの場合、検索速度は遅いため、出来るだけ使用しないことが望ましい

template<class TYPE> int YCStringT<TYPE>::ReverseFind(
	const TYPE*			pszSub							// 検索対象文字列
	) const
{
	INT_PTR				npResult = -1;

	// 検索対象文字列の長さの取得

	int					nSubLength = 0;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nSubLength = lstrlenA( (const char*) pszSub );
		break;

	case	2:
		// wchar_t型

		nSubLength = lstrlenW( (const wchar_t*) pszSub );
		break;
	}

	if( nSubLength <= 0 )
	{
		// 検索対象文字列なし

		return	(int) npResult;
	}

	// 文字列の最後から検索

	int					nStart = (GetLength() - nSubLength);
	int					nIndex;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		char*				pszWork;

		pszWork = (char*) &m_ptString[nStart];

		while( 1 )
		{
			if( lstrcmpA( pszWork, (const char*) pszSub ) == 0 )
			{
				// 一致した

				npResult = (pszWork - (char*) m_ptString);
				break;
			}

			if( pszWork == (char*) m_ptString )
			{
				break;
			}

			pszWork = ::CharPrevA( (char*) m_ptString, pszWork );
		}

		break;

	case	2:
		// wchar_t型

		for( nIndex = nStart ; nIndex >= 0 ; nIndex-- )
		{
			if( lstrcmpW( (wchar_t*) &m_ptString[nIndex], (const wchar_t*) pszSub ) == 0 )
			{
				// 一致した

				npResult = nIndex;
				break;
			}
		}

		break;
	}

	return	(int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	後方検索

template<class TYPE> int YCStringT<TYPE>::ReverseFind(
	const TYPE&			rfcSub							// 検索対象文字
	) const
{
	INT_PTR				npResult = -1;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		const BYTE*			pbtFound;

		pbtFound = _mbsrchr( (const BYTE*) m_ptString, rfcSub );

		if( pbtFound != NULL )
		{
			// 対象文字列が存在する

			npResult = (pbtFound - (BYTE*) m_ptString);
		}

		break;

	case	2:
		// wchar_t型

		const wchar_t*		pwszFound;

		pwszFound = wcsrchr( (wchar_t*) m_ptString, rfcSub );

		if( pwszFound != NULL )
		{
			// 対象文字列が存在する

			npResult = (pwszFound - (wchar_t*) m_ptString);
		}

		break;
	}

	return	(int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	左の部分を抽出
//
//	return	抽出した文字列

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::Left(
	int					nCount							// 抽出する文字数
	) const
{
	YCStringT<TYPE>		clsResult = *this;

	clsResult.Delete( nCount, clsResult.GetLength() );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	右の部分を抽出
//
//	return	抽出した文字列

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::Right( int nCount ) const
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	中央の部分を抽出
//
//	return	抽出した文字列

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::Mid( int nFirst ) const
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//	中央の部分を抽出
//
//	return	抽出した文字列

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::Mid( int nFirst, int nCount ) const
{

}








//////////////////////////////////////////////////////////////////////////////////////////
//	バッファの取得

template<class TYPE> TYPE* YCStringT<TYPE>::GetBuffer(
	int					nBufferSize						// バッファサイズ
	)
{
	if( nBufferSize > GetBufferSize() )
	{
		// 現在のバッファサイズより大きい

		Free();

		// メモリの確保

		Alloc( nBufferSize );
	}

	return	m_ptString;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列長の取得

template<class TYPE> int YCStringT<TYPE>::GetLength() const
{
	if( m_ptString == NULL )
	{
		return	0;
	}

	return	*(int*) (m_ptString - (YCSTRINGT_OFFSET_LENGTH / sizeof(TYPE)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の取得

template<class TYPE> const TYPE* YCStringT<TYPE>::GetString() const
{
	return	m_ptString;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列が空かどうか調べる

template<class TYPE> bool YCStringT<TYPE>::IsEmpty() const
{
	bool				bReturn = false;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		bReturn = (*this == "");
		break;

	case	2:
		// wchar_t型

		bReturn = (*this == L"");
		break;
	}

	return	bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子のリネーム

template<class TYPE> void YCStringT<TYPE>::RenameExtension(
	const TYPE*			ptRenameExt						// リネーム後の拡張子
	)
{
	int					nFileNameIndex = GetFileNameIndex();
	int					nFileExtIndex = GetFileExtIndex();

	if( nFileExtIndex >= nFileNameIndex )
	{
		// 拡張子を発見

		int					nNewStringLength = GetLength();

		switch( sizeof(TYPE) )
		{
		case	1:
			// char型

			nNewStringLength += lstrlenA( (const char*) ptRenameExt ) - lstrlenA( (const char*) &m_ptString[nFileExtIndex] );

			ExtendBuffer( (nNewStringLength + 1) );

			lstrcpyA( (char*) &m_ptString[nFileExtIndex], (const char*) ptRenameExt );
			break;

		case	2:
			// wchar_t型

			nNewStringLength += lstrlenA( (const char*) ptRenameExt ) - lstrlenA( (const char*) &m_ptString[nFileExtIndex] );

			ExtendBuffer( (nNewStringLength + 1) );

			lstrcpyW( (wchar_t*) &m_ptString[nFileExtIndex], (const wchar_t*) ptRenameExt );
			break;
		}

		SetLength( nNewStringLength );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル名の取得

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::GetFileName() const
{
	int					nIndex = GetFileNameIndex();

	return	&m_ptString[nIndex];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル名へのインデックスの取得

template<class TYPE> int YCStringT<TYPE>::GetFileNameIndex() const
{
	int					nFound = -1;
	int					nIndex = 0;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nFound = ReverseFind( '\\' );
		break;

	case	2:
		// wchar_t型

		nFound = ReverseFind( L'\\' );
		break;
	}

	if( nFound >= 0 )
	{
		// ファイル名を発見

		nIndex = (nFound + 1);
	}
	else
	{
		// 文字列全体がファイル名と断定

		nIndex = 0;
	}

	return	nIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルタイトルの取得

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::GetFileTitle() const
{
	YCStringT<TYPE>		clsFileTitle = GetFileName();
	TYPE				szWork[1];

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		szWork[0] = '\0';
		break;

	case	2:
		// wchar_t型

		szWork[0] = L'\0';
		break;
	}

	clsFileTitle.RenameExtension( szWork );

	return	clsFileTitle;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子の取得

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::GetFileExt() const
{
	int					nIndex = GetFileExtIndex();

	return	&m_ptString[nIndex];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子へのインデックスの取得

template<class TYPE> int YCStringT<TYPE>::GetFileExtIndex() const
{
	int					nFound = -1;
	int					nIndex = 0;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nFound = ReverseFind( '.' );
		break;

	case	2:
		// wchar_t型

		nFound = ReverseFind( L'.' );
		break;
	}

	if( nFound >= 0 )
	{
		// 拡張子を発見

		nIndex = nFound;
	}
	else
	{
		// 拡張子がない

		nIndex = GetLength();
	}

	return	nIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	フォルダパスの取得

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::GetDirPath() const
{
	int					nTargetIndex = 0;
	int					nFileNameIndex = GetFileNameIndex();
	int					nDriveIndex = -1;

	if( nFileNameIndex >= 1 )
	{
		// フォルダ名を発見

		nTargetIndex = (nFileNameIndex - 1);
	}

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		nDriveIndex = ReverseFind( ':' );
		break;

	case	2:
		// wchar_t型

		nDriveIndex = ReverseFind( L':' );
		break;
	}

	if( nDriveIndex >= 0 )
	{
		// ドライブ名が含まれている

		if( nTargetIndex == (nDriveIndex + 1) )
		{
			// ':'の直後に'\\'が存在する

			nTargetIndex = nFileNameIndex;
		}
		else
		{
			// ':'より前は削除しない

			nTargetIndex = (nTargetIndex < nDriveIndex) ? (nDriveIndex + 1) : nTargetIndex;
		}
	}

	return	Left( nTargetIndex );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	代入演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator=(
	const char*			pszSrc							// 代入文字列
	)
{
	Copy( pszSrc );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	代入演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator=(
	const char&			rfcSrc							// 代入文字
	)
{
	Copy( rfcSrc );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	代入演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator=(
	const wchar_t*		pwszSrc							// 代入文字列
	)
{
	Copy( pwszSrc );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	代入演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator=(
	const wchar_t&		rfwcSrc							// 代入文字
	)
{
	Copy( rfwcSrc );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	代入演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator=(
	const YCStringT<TYPE>&	rfclsSrc					// 代入文字列
	)
{
	Copy( rfclsSrc );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::operator+(
	const char*			pszAppend						// 繋げる文字列
	) const
{
	YCStringT<TYPE>		clsResult = *this;

	clsResult.Append( pszAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::operator+(
	const char&			rfcAppend						// 繋げる文字
	) const
{
	YCStringT<TYPE>		clsResult = *this;

	clsResult.Append( rfcAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::operator+(
	const wchar_t*		pwszAppend						// 繋げる文字列
	) const
{
	YCStringT<TYPE>		clsResult = *this;

	clsResult.Append( pwszAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::operator+(
	const wchar_t&		rfwcAppend						// 繋げる文字
	) const
{
	YCStringT<TYPE>		clsResult = *this;

	clsResult.Append( rfwcAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子

template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::operator+(
	const YCStringT<TYPE>&	rfclsAppend					// 繋げる文字列
	) const
{
	YCStringT<TYPE>		clsResult = *this;

	clsResult.Append( rfclsAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子(外部関数)

template<class TYPE> YCStringT<TYPE> operator+(
	const char*				pszSrc,						// 繋げられる文字列
	const YCStringT<TYPE>&	rfclsAppend					// 繋げる文字列
	)
{
	YCStringT<TYPE>		clsResult = pszSrc;

	clsResult.Append( rfclsAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子(外部関数)

template<class TYPE> YCStringT<TYPE> operator+(
	const char&				rfcSrc,						// 繋げられる文字
	const YCStringT<TYPE>&	rfclsAppend					// 繋げる文字列
	)
{
	YCStringT<TYPE>		clsResult = rfcSrc;

	clsResult.Append( rfclsAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子(外部関数)

template<class TYPE> YCStringT<TYPE> operator+(
	const wchar_t*			pwszSrc,					// 繋げられる文字列
	const YCStringT<TYPE>&	rfclsAppend					// 繋げる文字列
	)
{
	YCStringT<TYPE>		clsResult = pwszSrc;

	clsResult.Append( rfclsAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+演算子(外部関数)

template<class TYPE> YCStringT<TYPE> operator+(
	const wchar_t&			rfwcSrc,					// 繋げられる文字
	const YCStringT<TYPE>&	rfclsAppend					// 繋げる文字列
	)
{
	YCStringT<TYPE>		clsResult = rfwcSrc;

	clsResult.Append( rfclsAppend );

	return	clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+=演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator+=(
	const char*			pszAppend						// 繋げる文字列
	)
{
	Append( pszAppend );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+=演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator+=(
	const char&			rfcAppend						// 繋げる文字
	)
{
	Append( rfcAppend );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+=演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator+=(
	const wchar_t*		pwszAppend						// 繋げる文字列
	)
{
	Append( pwszAppend );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+=演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator+=(
	const wchar_t&		rfwcAppend						// 繋げる文字
	)
{
	Append( rfwcAppend );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	+=演算子

template<class TYPE> YCStringT<TYPE>& YCStringT<TYPE>::operator+=(
	const YCStringT<TYPE>&	rfclsAppend					// 繋げる文字列
	)
{
	Append( rfclsAppend );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	==演算子

template<class TYPE> bool YCStringT<TYPE>::operator==(
	const char*			pszSrc							// 比較文字列
	) const
{
	return	(Compare( pszSrc ) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	==演算子

template<class TYPE> bool YCStringT<TYPE>::operator==(
	const char&			rfcSrc							// 比較文字
	) const
{
	return	(Compare( rfcSrc ) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	==演算子

template<class TYPE> bool YCStringT<TYPE>::operator==(
	const wchar_t*			pwszSrc						// 比較文字列
	) const
{
	return	(Compare( pwszSrc ) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	==演算子

template<class TYPE> bool YCStringT<TYPE>::operator==(
	const wchar_t&		rfwcSrc							// 比較文字
	) const
{
	return	(Compare( rfwcSrc ) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	==演算子

template<class TYPE> bool YCStringT<TYPE>::operator==(
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	) const
{
	return	(Compare( rfclsSrc ) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	!=演算子

template<class TYPE> bool YCStringT<TYPE>::operator!=(
	const char*			pszSrc							// 比較文字列
	) const
{
	return	(Compare( pszSrc ) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	!=演算子

template<class TYPE> bool YCStringT<TYPE>::operator!=(
	const char&			rfcSrc							// 比較文字
	) const
{
	return	(Compare( rfcSrc ) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	!=演算子

template<class TYPE> bool YCStringT<TYPE>::operator!=(
	const wchar_t*			pwszSrc						// 比較文字列
	) const
{
	return	(Compare( pwszSrc ) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	!=演算子

template<class TYPE> bool YCStringT<TYPE>::operator!=(
	const wchar_t&		rfwcSrc							// 比較文字
	) const
{
	return	(Compare( rfwcSrc ) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	!=演算子

template<class TYPE> bool YCStringT<TYPE>::operator!=(
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	) const
{
	return	(Compare( rfclsSrc ) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<演算子
//
//	return	TRUE	pszSrc未満
//			FALSE	pszSrc以上

template<class TYPE> bool YCStringT<TYPE>::operator<(
	const char*			pszSrc							// 比較文字列
	) const
{
	return	(Compare( pszSrc ) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<演算子
//
//	return	TRUE	pwszSrc未満
//			FALSE	pwszSrc以上

template<class TYPE> bool YCStringT<TYPE>::operator<(
	const wchar_t*		pwszSrc							// 比較文字列
	) const
{
	return	(Compare( pwszSrc ) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<演算子
//
//	return	TRUE	rfclsSrc未満
//			FALSE	rfclsSrc以上

template<class TYPE> bool YCStringT<TYPE>::operator<(
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	) const
{
	return	(Compare( rfclsSrc ) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<演算子(外部関数)
//
//	return	TRUE	rfclsSrc未満
//			FALSE	rfclsSrc以上

template<class TYPE> bool operator<(
	const char*				pszSrc,						// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pszSrc ) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<演算子(外部関数)
//
//	return	TRUE	rfclsSrc未満
//			FALSE	rfclsSrc以上

template<class TYPE> bool operator<(
	const wchar_t*			pwszSrc,					// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pwszSrc ) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<=演算子
//
//	return	TRUE	pszSrc以下
//			FALSE	pszSrc超

template<class TYPE> bool YCStringT<TYPE>::operator<=(
	const char*			pszSrc							// 比較文字列
	) const
{
	return	(Compare( pszSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<=演算子
//
//	return	TRUE	pwszSrc以下
//			FALSE	pwszSrc超

template<class TYPE> bool YCStringT<TYPE>::operator<=(
	const wchar_t*		pwszSrc							// 比較文字列
	) const
{
	return	(Compare( pwszSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<=演算子
//
//	return	TRUE	rfclsSrc以下
//			FALSE	rfclsSrc超

template<class TYPE> bool YCStringT<TYPE>::operator<=(
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	) const
{
	return	(Compare( rfclsSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<=演算子(外部関数)
//
//	return	TRUE	rfclsSrc以下
//			FALSE	rfclsSrc超

template<class TYPE> bool operator<=(
	const char*				pszSrc,						// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pszSrc ) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	<=演算子(外部関数)
//
//	return	TRUE	rfclsSrc以下
//			FALSE	rfclsSrc超

template<class TYPE> bool operator<=(
	const wchar_t*			pwszSrc,					// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pwszSrc ) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>演算子
//
//	return	TRUE	pszSrc超
//			FALSE	pszSrc以下

template<class TYPE> bool YCStringT<TYPE>::operator>(
	const char*			pszSrc							// 比較文字列
	) const
{
	return	(Compare( pszSrc ) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>演算子
//
//	return	TRUE	pwszSrc超
//			FALSE	pwszSrc以下

template<class TYPE> bool YCStringT<TYPE>::operator>(
	const wchar_t*		pwszSrc							// 比較文字列
	) const
{
	return	(Compare( pwszSrc ) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>演算子
//
//	return	TRUE	rfclsSrc超
//			FALSE	rfclsSrc以下

template<class TYPE> bool YCStringT<TYPE>::operator>(
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	) const
{
	return	(Compare( rfclsSrc ) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>演算子(外部関数)
//
//	return	TRUE	rfclsSrc超
//			FALSE	rfclsSrc以下

template<class TYPE> bool operator>(
	const char*				pszSrc,						// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pszSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>演算子(外部関数)
//
//	return	TRUE	rfclsSrc超
//			FALSE	rfclsSrc以下

template<class TYPE> bool operator>(
	const wchar_t*			pwszSrc,					// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pszSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>=演算子
//
//	return	TRUE	pszSrc以下
//			FALSE	pszSrc超

template<class TYPE> bool YCStringT<TYPE>::operator>=(
	const char*			pszSrc							// 比較文字列
	) const
{
	return	(Compare( pszSrc ) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>=演算子
//
//	return	TRUE	pwszSrc以下
//			FALSE	pwszSrc超

template<class TYPE> bool YCStringT<TYPE>::operator>=(
	const wchar_t*		pwszSrc							// 比較文字列
	) const
{
	return	(Compare( pwszSrc ) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>=演算子
//
//	return	TRUE	rfclsSrc以下
//			FALSE	rfclsSrc超

template<class TYPE> bool YCStringT<TYPE>::operator>=( 
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	) const
{
	return	(Compare( rfclsSrc ) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>=演算子(外部関数)
//
//	return	TRUE	rfclsSrc以下
//			FALSE	rfclsSrc超

template<class TYPE> bool operator>=(
	const char*				pszSrc,						// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pszSrc ) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	>=演算子(外部関数)
//
//	return	TRUE	rfclsSrc以下
//			FALSE	rfclsSrc超

template<class TYPE> bool operator>=(
	const wchar_t*			pwszSrc,					// 比較文字列
	const YCStringT<TYPE>&	rfclsSrc					// 比較文字列
	)
{
	return	(rfclsSrc.Compare( pwszSrc ) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	要素にアクセス
//
//	備考	範囲外の参照に対して何もしないため高速だが、落ちる危険性があるので注意

template<class TYPE> TYPE& YCStringT<TYPE>::operator[](
	int					nPos							// 参照位置
	)
{
	return	m_ptString[nPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	要素の参照
//
//	備考	範囲外の参照に対して何もしないため高速だが、落ちる危険性があるので注意

template<class TYPE> const TYPE& YCStringT<TYPE>::operator[](
	int					nPos							// 参照位置
	) const
{
	return	m_ptString[nPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列を返す

template<class TYPE> YCStringT<TYPE>::operator const TYPE*() const
{
	return	GetString();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリの確保

template<class TYPE> void YCStringT<TYPE>::Alloc(
	int					nBufferSize						// バッファサイズ
	)
{
	// アライメント

	int					nPaddingSize = (YCSTRINGT_BUFFERSIZE_ALIGNMENT - (nBufferSize % YCSTRINGT_BUFFERSIZE_ALIGNMENT));

	if( nPaddingSize < YCSTRINGT_BUFFERSIZE_ALIGNMENT )
	{
		nBufferSize += nPaddingSize;
	}

	// バッファの確保

	int					nDataSize = (16 / sizeof(TYPE));

	m_ptString = new TYPE[nDataSize + nBufferSize];

	m_ptString += nDataSize;

	// バッファサイズの格納

	SetBufferSize( nBufferSize );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリの解放

template<class TYPE> void YCStringT<TYPE>::Free()
{
	if( m_ptString != NULL )
	{
		m_ptString -= (16 / sizeof(TYPE));

		delete[]	m_ptString;

		m_ptString = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	バッファの拡張

template<class TYPE> void YCStringT<TYPE>::ExtendBuffer(
	int					nBufferSize						// バッファサイズ
	)
{
	if( nBufferSize < GetBufferSize() )
	{
		// バッファを拡張する必要なし

		return;
	}

	TYPE*				ptString = m_ptString;

	switch( sizeof(TYPE) )
	{
	case	1:
		// char型

		Alloc( nBufferSize );

		lstrcpyA( (char*) m_ptString, (const char*) ptString );

		break;

	case	2:
		// wchar_t型

		Alloc( nBufferSize );

		lstrcpyW( (wchar_t*) m_ptString, (const wchar_t*) ptString );

		break;
	}

	// 前のバッファの解放

	delete[]	(ptString - (16 / sizeof(TYPE)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	バッファサイズの設定

template<class TYPE> void YCStringT<TYPE>::SetBufferSize(
	int					nBufferSize						// バッファサイズ
	)
{
	*(int*) (m_ptString - (YCSTRINGT_OFFSET_BUFFERSIZE / sizeof(TYPE))) = nBufferSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	バッファサイズの取得	
//
//	return	バッファサイズ

template<class TYPE> int YCStringT<TYPE>::GetBufferSize()
{
	if( m_ptString == NULL )
	{
		return	0;
	}

	return	*(int*) (m_ptString - (YCSTRINGT_OFFSET_BUFFERSIZE / sizeof(TYPE)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列の長さの設定

template<class TYPE> void YCStringT<TYPE>::SetLength(
	int					nLength							// 文字列の長さ
	)
{
	*(int*) (m_ptString - (YCSTRINGT_OFFSET_LENGTH / sizeof(TYPE))) = nLength;
}


