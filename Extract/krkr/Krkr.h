#pragma once

#include "../../ExtractBase.h"
#include "../../MD5.h"

class CKrkr : public CExtractBase
{
public:

	// File chunk
	struct FileChunk
	{
		BYTE        name[4];
		QWORD       size;
	};

	// Info chunk
	struct InfoChunk
	{
		BYTE        name[4];
		QWORD       size;
		DWORD       protect;    // (1 << 31) : protected
		QWORD       orgSize;
		QWORD       arcSize;
		WORD        nameLen;
		wchar_t*    filename;   // length : nameLen, Unicode
	};

	// Segment Chunk
	struct SegmChunk
	{
		BYTE        name[4];
		QWORD       size;
		DWORD       comp;       // 1 : compressed
		QWORD       start;      // Offset from the beginning of the data archive
		QWORD       orgSize;    // original size
		QWORD       arcSize;    // archived size
	};

	// adlr Chunk
	struct AdlrChunk
	{
		BYTE  name[4];
		QWORD size;
		DWORD key;    // Unique file key
	};

	virtual BOOL Mount( CArcFile* pclArc );
	virtual BOOL Decode( CArcFile* pclArc );
	virtual BOOL Extract( CArcFile* pclArc );


protected:

	//  Check whether or not it can be decoded
	virtual BOOL    OnCheckDecrypt( CArcFile* pclArc );

	// Check tpm
	BOOL            CheckTpm( const char* pszMD5 );

	// Set decryption key
	void            InitDecrypt( CArcFile* pclArc );
	virtual DWORD   OnInitDecrypt( CArcFile* pclArc );

	// Decoding
	DWORD           Decrypt( BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset );
	virtual DWORD   OnDecrypt( BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey );

	// Set decryption request
	void            SetDecryptRequirement( BOOL bDecrypt );

	// Set decryption size
	void            SetDecryptSize( DWORD dwDecryptSize );

	// Find XP3 from within an EXE file
	BOOL            FindXP3FromExecuteFile( CArcFile* pclArc, DWORD* pdwOffset );


private:

	BOOL        m_bDecrypt;      // Decryption request
	DWORD       m_dwDecryptKey;
	DWORD       m_dwDecryptSize; // Decryption size
	YCString    m_clsTpmPath;
	CArcFile*   m_pclArc;


private:

	void SetMD5ForTpm( CArcFile* pclArc );
};
