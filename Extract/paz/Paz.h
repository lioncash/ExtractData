
#pragma	once

#include	"../../ExtractBase.h"

class	CPaz : public CExtractBase
{
public:

	// キー情報

	struct	SKeyInfo
	{
		YCString			clsType;
		BYTE				abtKey[64];
	};


	virtual	BOOL							Mount( CArcFile* pclArc );
	virtual	BOOL							Decode( CArcFile* pclArc );


protected:

	// 72 + 4096バイトのテーブルの初期化

	virtual	void							InitTable();

	// ムービーテーブルの初期化

	virtual	DWORD							InitMovieTable( void* pvTable );

	// ベースアーカイブファイル名の取得

	void									GetBaseArcName( LPTSTR pszDst, LPCTSTR pszArcName );

	// 32バイトのキーをセットする(書庫ファイルによってキーは異なる)

	virtual	void							InitMountKey( CArcFile* pclArc ) = 0;
	virtual	void							InitDecodeKey( CArcFile* pclArc ) = 0;
	DWORD									SetKey( CArcFile* pclArc, const SKeyInfo* pstKeyInfo );

	// テーブルを復号する

	virtual	void							DecodeTable1();
	virtual	void							DecodeTable2();

	// 固有暗号を復号する

	virtual	void							Decrypt( void* pvTarget, DWORD dwSize ) {}
	virtual	void							Decrypt2( void* pvTarget, DWORD dwSize ) {}

	// データを復号する

	virtual	void							DecodeData( void* pvTarget, DWORD dwSize );

	// ムービーデータを復号する

	virtual	void							DecodeMovieData( void* pvTarget, DWORD dwSize );

	// DWORD値を復号する

	virtual	DWORD							DecodeValueByTable( DWORD dwValue, void* pvTable );
	virtual	void							DecodeValue( DWORD* pdwValue1, DWORD* pdwValue2, void* pvTable );

	// 

	virtual	DWORD*							GetTable();
	virtual	BYTE*							GetMovieTable();

	virtual	BYTE*							GetKey();
	virtual	DWORD							GetMovieBufSize( CArcFile* pclArc );

private:

	DWORD				m_adwTable[1042];
	BYTE				m_abtMovieTable[256];
	BYTE				m_abtKey[32];
};
