#pragma once

#include "../../ExtractBase.h"

class CKrkr : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	bool Extract(CArcFile* archive) override;

protected:
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
	bool FindXP3FromExecuteFile(CArcFile* archive, size_t* offset);

private:
	bool        m_decrypt = false; // Decryption request
	u32         m_decrypt_key = 0;
	size_t      m_decrypt_size = 0; // Decryption size
	YCString    m_tpm_path;
	CArcFile*   m_archive = nullptr;

	void SetMD5ForTpm(CArcFile* archive);
};
