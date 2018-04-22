#pragma once

#include "ExtractBase.h"

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

	using Key = std::array<u8, 32>;
	using MovieTable = std::array<u8, 256>;
	using Table = std::array<u32, 1042>;

	// Initialization of the table of 72 + 4096 bytes
	virtual void InitTable();

	// Initialization of the movie table
	virtual size_t InitMovieTable(const u8* table);

	// Get the base archive filename
	void GetBaseArcName(LPTSTR dst, LPCTSTR archive_name);

	// Set 32 byte keys (Keys differ for each file)
	virtual void InitMountKey(CArcFile* archive) = 0;
	virtual void InitDecodeKey(CArcFile* archive) = 0;
	size_t SetKey(CArcFile* archive, const std::array<KeyInfo, 8>& key_info);

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

	// Decode u32 value
	virtual u32 DecodeValueByTable(u32 value, const Table& table);
	virtual void DecodeValue(u32* value1, u32* value2, const Table& table);

	// Get 
	Table& GetTable();
	MovieTable& GetMovieTable();

	Key& GetKey();
	virtual size_t GetMovieBufSize(const CArcFile* archive);

private:
	std::array<u32, 1042> m_table;
	std::array<u8, 256> m_movie_table;
	std::array<u8, 32> m_key;
};
