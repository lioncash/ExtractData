#pragma once

#include "../../ExtractBase.h"

class CPaz : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	struct SKeyInfo
	{
		YCString type;
		BYTE     key[64];
	};

	// Initialization of the table of 72 + 4096 bytes
	virtual void InitTable();

	// Initialization of the movie table
	virtual DWORD InitMovieTable(void* table);

	// Get the base archive filename
	void GetBaseArcName(LPTSTR dst, LPCTSTR archive_name);

	// Set 32 byte keys (Keys differ for each file)
	virtual void InitMountKey(CArcFile* archive) = 0;
	virtual void InitDecodeKey(CArcFile* archive) = 0;
	DWORD SetKey(CArcFile* archive, const SKeyInfo* key_info);

	// Decrypt the table
	virtual void DecodeTable1();
	virtual void DecodeTable2();

	// Cipher-specific decryption
	virtual void Decrypt(void* target, DWORD size) {}
	virtual void Decrypt2(void* target, DWORD size) {}

	// Decrypt the data
	virtual void DecodeData(void* target, DWORD size);

	// Decrypt movie data
	virtual void DecodeMovieData(void* target, DWORD size);

	// Decode DWORD value
	virtual DWORD DecodeValueByTable(DWORD value, const DWORD* table);
	virtual void  DecodeValue(DWORD* value1, DWORD* value2, const DWORD* table);

	// Get 
	virtual DWORD* GetTable();
	virtual BYTE*  GetMovieTable();

	virtual BYTE* GetKey();
	virtual DWORD GetMovieBufSize(CArcFile* archive);

private:
	DWORD m_table[1042];
	BYTE  m_movie_table[256];
	BYTE  m_key[32];
};
