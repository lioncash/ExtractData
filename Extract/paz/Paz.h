#pragma once

#include "../../ExtractBase.h"

class CPaz : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	struct KeyInfo
	{
		YCString type;
		u8       key[64];
	};

	// Initialization of the table of 72 + 4096 bytes
	virtual void InitTable();

	// Initialization of the movie table
	virtual u32 InitMovieTable(const u8* table);

	// Get the base archive filename
	void GetBaseArcName(LPTSTR dst, LPCTSTR archive_name);

	// Set 32 byte keys (Keys differ for each file)
	virtual void InitMountKey(CArcFile* archive) = 0;
	virtual void InitDecodeKey(CArcFile* archive) = 0;
	u32 SetKey(CArcFile* archive, const KeyInfo* key_info);

	// Decrypt the table
	virtual void DecodeTable1();
	virtual void DecodeTable2();

	// Cipher-specific decryption
	virtual void Decrypt(u8* target, size_t size) {}
	virtual void Decrypt2(u8* target, size_t size) {}

	// Decrypt the data
	virtual void DecodeData(u8* target, size_t size);

	// Decrypt movie data
	virtual void DecodeMovieData(u8* target, size_t size);

	// Decode DWORD value
	virtual u32 DecodeValueByTable(u32 value, const u32* table);
	virtual void DecodeValue(u32* value1, u32* value2, const u32* table);

	// Get 
	virtual u32* GetTable();
	virtual u8*  GetMovieTable();

	virtual u8* GetKey();
	virtual size_t GetMovieBufSize(CArcFile* archive);

private:
	u32 m_table[1042];
	u8  m_movie_table[256];
	u8  m_key[32];
};
