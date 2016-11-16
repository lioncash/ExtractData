#pragma once

#include "../../ExtractBase.h"
#include "../../MD5.h"

class CKrkr : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	bool Extract(CArcFile* archive) override;

protected:
	// File chunk
	struct FileChunk
	{
		BYTE name[4];
		QWORD size;
	};

	// Info chunk
	struct InfoChunk
	{
		BYTE  name[4];
		QWORD size;
		DWORD protect;    // (1 << 31) : protected
		QWORD orgSize;
		QWORD arcSize;
		WORD  nameLen;
		wchar_t* filename;   // length : nameLen, Unicode
	};

	// Segment Chunk
	struct SegmChunk
	{
		BYTE  name[4];
		QWORD size;
		DWORD comp;       // 1 : compressed
		QWORD start;      // Offset from the beginning of the data archive
		QWORD orgSize;    // original size
		QWORD arcSize;    // archived size
	};

	// adlr Chunk
	struct AdlrChunk
	{
		BYTE  name[4];
		QWORD size;
		DWORD key;    // Unique file key
	};

	//  Check whether or not it can be decoded
	virtual bool OnCheckDecrypt(CArcFile* archive);

	// Check tpm
	bool CheckTpm(const char* md5) const;

	// Set decryption key
	void InitDecrypt(CArcFile* archive);
	virtual DWORD OnInitDecrypt(CArcFile* archive);

	// Decoding
	DWORD Decrypt(BYTE* target, DWORD target_size, DWORD offset);
	virtual DWORD OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key);

	// Set decryption request
	void SetDecryptRequirement(bool decrypt);

	// Set decryption size
	void SetDecryptSize(DWORD decrypt_size);

	// Find XP3 from within an EXE file
	bool FindXP3FromExecuteFile(CArcFile* archive, DWORD* offset);

private:
	bool        m_bDecrypt;      // Decryption request
	DWORD       m_decrypt_key;
	DWORD       m_decrypt_size; // Decryption size
	YCString    m_tpm_path;
	CArcFile*   m_archive;

	void SetMD5ForTpm(CArcFile* archive);
};
