
#pragma		once

#include	"../../ExtractBase.h"
#include	"../../MD5.h"

class	CKrkr : public CExtractBase
{
public:

	// ファイルチャンク

	struct	FileChunk
	{
		BYTE			name[4];
		QWORD			size;
	};

	// インフォチャンク

	struct	InfoChunk
	{
		BYTE			name[4];
		QWORD			size;
		DWORD			protect;	// (1 << 31) : protected
		QWORD			orgSize;
		QWORD			arcSize;
		WORD			nameLen;
		wchar_t*		filename;	// length : nameLen, Unicode
	};

	// セグメントチャンク

	struct	SegmChunk
	{
		BYTE			name[4];
		QWORD			size;
		DWORD			comp;		// 1 : compressed
		QWORD			start;		// アーカイブ先頭からデータまでのオフセット
		QWORD			orgSize;	// original size
		QWORD			arcSize;	// archived size
	};

	// adlrチャンク

	struct	AdlrChunk
	{
		BYTE			name[4];
		QWORD			size;
		DWORD			key;		// ファイル固有キー
	};

	virtual	BOOL		Mount( CArcFile* pclArc );
	virtual	BOOL		Decode( CArcFile* pclArc );
	virtual	BOOL		Extract( CArcFile* pclArc );


protected:

	// 復号可能かどうかの確認

	virtual BOOL		OnCheckDecrypt( CArcFile* pclArc );

	// tpmのチェック

	BOOL				CheckTpm( const char* pszMD5 );

	// 復号キーのセット

	void				InitDecrypt( CArcFile* pclArc );
	virtual DWORD		OnInitDecrypt( CArcFile* pclArc );

	// 復号を行う

	DWORD				Decrypt( BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset );
	virtual DWORD		OnDecrypt( BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey );

	// 復号要求の設定

	void									SetDecryptRequirement( BOOL bDecrypt );

	// 復号サイズの設定

	void									SetDecryptSize( DWORD dwDecryptSize );

	// EXE内からXP3を探す

	BOOL				FindXP3FromExecuteFile( CArcFile* pclArc, DWORD* pdwOffset );


private:

	BOOL									m_bDecrypt;			// 復号要求
	DWORD									m_dwDecryptKey;
	DWORD									m_dwDecryptSize;	// 復号サイズ
	YCString								m_clsTpmPath;
	CArcFile*								m_pclArc;


private:

	void				SetMD5ForTpm( CArcFile* pclArc );
};
