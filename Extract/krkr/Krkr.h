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
	virtual u32 OnInitDecrypt(CArcFile* archive);

	// Decoding
	size_t Decrypt(u8* target, size_t target_size, size_t offset);
	virtual size_t OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key);

	// Set decryption request
	void SetDecryptRequirement(bool decrypt);

	// Set decryption size
	void SetDecryptSize(size_t decrypt_size);

	// Find XP3 from within an EXE file
	bool FindXP3FromExecuteFile(CArcFile* archive, DWORD* offset);

private:
	bool        m_decrypt;      // Decryption request
	u32         m_decrypt_key;
	size_t      m_decrypt_size; // Decryption size
	YCString    m_tpm_path;
	CArcFile*   m_archive;

	void SetMD5ForTpm(CArcFile* archive);
};
