
#include	"stdafx.h"
#include	"YCLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

YCLibrary::YCLibrary()
{
	m_hModule = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

YCLibrary::~YCLibrary()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	指定されたモジュールをロード

BOOL	YCLibrary::Load(
	LPCTSTR				pszPathToFile					// ロードするモジュール名
	)
{
	m_hModule = ::LoadLibrary( pszPathToFile );

	return	(m_hModule != NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ロードしたモジュールを解放

void	YCLibrary::Free()
{
	if( m_hModule != NULL )
	{
		::FreeLibrary( m_hModule );
		m_hModule = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	関数のアドレスを取得

FARPROC	YCLibrary::GetProcAddress(
	LPCTSTR				pszProcName						// 関数名
	)
{
	if( m_hModule == NULL )
	{
		return	NULL;
	}

	YCStringA			clsProcName = pszProcName;	// GetProcAddressWが存在しないので仕方なく

	return	::GetProcAddress( m_hModule, clsProcName );
}
