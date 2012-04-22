
#pragma	once

//----------------------------------------------------------------------------------------
//-- INIクラス ---------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class	YCIni
{
public:

											YCIni( LPCTSTR pszIniName );

	void									SetSection( UINT uID );
	void									SetSection( LPCTSTR pszSection );

	void									SetKey( LPCTSTR pszKey );

	template<typename TYPE> TYPE			ReadDec( const TYPE& rftDefault );
	template<typename TYPE> void			ReadDec( TYPE* ptDst, const TYPE& rftDefault );
	template<typename TYPE> void			ReadDec( TYPE* ptDst );

	template<typename TYPE> TYPE			ReadHex( LPCTSTR pszDef );
	template<typename TYPE> void			ReadHex( TYPE* ptDst, LPCTSTR pszDefault );

	void									ReadStr( LPTSTR pszDst, DWORD dwDstSize, LPCTSTR pszDefault );
	void									ReadStr( YCString& rfclsDst, const YCString& rfclsDefault );

	template<typename TYPE> void			WriteDec( const TYPE& rftDec );

	template<typename TYPE> void			WriteHex( const TYPE& rftHex, DWORD dwNumber = 1 );

	void									WriteStr( LPCTSTR pszStr );

	BOOL									DeleteSection( LPCTSTR pszSection = NULL );


private:

	YCString								m_clsPathToIni;
	YCString								m_clsSection;
	YCString								m_clsKey;
};

//////////////////////////////////////////////////////////////////////////////////////////
//	数値を取得

template<typename TYPE> TYPE	YCIni::ReadDec(
	const TYPE&			rftDefault						// デフォルト値
	)
{
	return	::GetPrivateProfileInt( m_clsSection, m_clsKey, rftDefault, m_clsPathToIni );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	数値を取得

template<typename TYPE> void	YCIni::ReadDec(
	TYPE*				ptDst,							// 数値の格納先
	const TYPE&			rftDefault						// デフォルト値
	)
{
	*ptDst = ReadDec( rftDefault );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	数値を取得

template<typename TYPE> void	YCIni::ReadDec(
	TYPE*				ptDst							// 数値の格納先(先にデフォルト値が格納されていること)
	)
{
	*ptDst = ReadDec( *ptDst );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	16進数を取得

template<typename TYPE> TYPE	YCIni::ReadHex(
	LPCTSTR				pszDefault						// デフォルト値
	)
{
	TCHAR				szWork[256];

	::GetPrivateProfileString( m_clsSection, m_clsKey, pszDefault, szWork, sizeof( szWork ), m_clsPathToIni );

	return	strtoul( szWork, NULL, 16 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	16進数を取得

template<typename TYPE> void	YCIni::ReadHex(
	TYPE*				ptDst,							// 16進数の格納先
	LPCTSTR				pszDefault						// デフォルト値
	)
{
	*ptDst = ReadHex( pszDefault );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	数値を設定

template<typename TYPE> void	YCIni::WriteDec(
	const TYPE&			rftDec							// 設定値
	)
{
	TCHAR				szWork[256];

	_stprintf( szWork, _T("%d"), rftDec );

	WriteStr( szWork );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	16進数を設定

template<typename TYPE> void	YCIni::WriteHex(
	const TYPE&			rftHex,							// 設定値
	DWORD				dwNumber						// 桁数
	)
{
	TCHAR				szWork[256];

	_stprintf( szWork, _T("%0*X"), dwNumber, rftHex );

	WriteStr( szWork );
}


